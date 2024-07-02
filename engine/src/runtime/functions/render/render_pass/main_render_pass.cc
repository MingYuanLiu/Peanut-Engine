#include "main_render_pass.h"

#include "functions/assets/mesh.h"
#include "functions/render/shader_manager.h"
#include "functions/assets/asset_manager.h"

namespace peanut
{
	void MainRenderPass::Initialize(PassInitInfo* init_info)
	{
		IRenderPassBase::Initialize(init_info);

		/************************** use to debug ********************************/
		/************************************************************************/
		// todo(temp): load static mesh render data and skybox render data
		std::shared_ptr<StaticMeshRenderData> mesh_render_data = std::make_shared<StaticMeshRenderData>();
		auto native_mesh_data = AssetsManager::GetInstance().LoadMeshBuffer("assets/mesh/cerberus.fbx");
		mesh_render_data->vertex_buffer = native_mesh_data->vertex_buffer;
		mesh_render_data->index_buffer = native_mesh_data->index_buffer;
		mesh_render_data->index_counts = std::vector<uint32_t>{ native_mesh_data->num_elements };
		mesh_render_data->index_offsets = std::vector<uint32_t>{ 0 };
		// load pbr texture
		auto pbr_albedo_texture = AssetsManager::GetInstance().LoadTextureData("assets/textures/cerberus_A.png", VK_FORMAT_R8G8B8A8_SRGB);
		auto pbr_normal_texture = AssetsManager::GetInstance().LoadTextureData("assets/textures/cerberus_N.png", VK_FORMAT_R8G8B8A8_UNORM);
		auto pbr_metallic_roughness_texture = AssetsManager::GetInstance().LoadTextureArrayData({ "assets/textures/cerberus_M.png", "assets/textures/cerberus_R.png" }, VK_FORMAT_R8G8B8A8_UNORM);
		
		PbrMaterial pbr_material;
		pbr_material.base_color_texture = pbr_albedo_texture;
		pbr_material.normal_texture = pbr_normal_texture;
		pbr_material.metallic_roughness_occlusion_texture = pbr_metallic_roughness_texture;
		pbr_material.emissive_texture = rhi_.lock()->CreateTexture(pbr_albedo_texture->width, pbr_albedo_texture->height,
			1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

		mesh_render_data->pbr_materials.push_back(pbr_material);
		// init pbr_material data
		MaterialPCO material_pco;
		material_pco.base_color_factor = glm::vec4{1.0f, 1.0f, 1.f, 1.f};
		material_pco.metallic_factor = 1.0;
		material_pco.roughness_factor = 1.0;
		material_pco.has_metallic_roughness_occlusion_texture = true;
		material_pco.has_emissive_texture = false;
		material_pco.has_normal_texture = true;
		material_pco.has_base_color_texture = true;

		mesh_render_data->material_pcos.push_back(material_pco);
		TransformUBO mesh_transform_ubo;
		// get transform matrix from location, rotation, scale
		mesh_transform_ubo.model = glm::mat4(1.0f);
		mesh_transform_ubo.model_view_projection = glm::mat4(1.0f);
		mesh_transform_ubo.normal_model = glm::mat4(1.0f);
		mesh_render_data->transform_ubo_data = mesh_transform_ubo;

		// todo(temp): load skybox render data
		environment_map_compute_pass_ = std::make_shared<EnvironmentMapComputePass>();
		environment_map_compute_pass_->Initialize(rhi_, "assets/textures/environment.hdr");
		environment_map_compute_pass_->Dispatch();
		environment_map_compute_pass_->FillIblTextureResource(lighting_render_data_.ibl_light_texture);
		environment_map_compute_pass_->FillSkyboxRenderData(skybox_render_data_.value());
		// todo(temp): load skybox mesh data
		auto native_skybox_mesh_data = AssetsManager::GetInstance().LoadMeshBuffer("assets/mesh/skybox.obj");
		skybox_render_data_->index_buffer = native_skybox_mesh_data->index_buffer;
		skybox_render_data_->index_counts = native_skybox_mesh_data->num_elements;
		skybox_render_data_->vertex_buffer = native_skybox_mesh_data->vertex_buffer;
		TransformUBO transform_ubo;
		transform_ubo.model = glm::mat4(1.0f);
		transform_ubo.model_view_projection = glm::mat4(1.0f);
		transform_ubo.normal_model = glm::mat4(1.0f);
		skybox_render_data_->transform_ubo_data = transform_ubo;
		
		// todo(temp): load color grading render data
		color_grading_render_data_ = std::make_optional<ColorGradingRenderData>();
		color_grading_render_data_->color_grading_lut_texture = *AssetsManager::GetInstance().LoadTextureData("assets/textures/color_grading_lut.png", VK_FORMAT_R8G8B8A8_SRGB);
		
		// todo(temp): create empty shadow map texture with size 1024, which should be calculate by shadow pass
		std::shared_ptr<VulkanRHI> vulkan_rhi = std::static_pointer_cast<VulkanRHI>(rhi_.lock());
		lighting_render_data_.directional_light_shadow_map = *vulkan_rhi->CreateTexture(1024, 1024, 1, 1, vulkan_rhi->GetDepthImageFormat(), 0);

		LightingUBO light_ubo;
		light_ubo.sky_light.color = glm::vec3(1.0f, 1.0f, 1.0f);
		light_ubo.has_directional_light = true;
		light_ubo.has_sky_light = true;
		light_ubo.point_light_num = 0;
		light_ubo.spot_light_num = 0;
		light_ubo.camera_dir = glm::vec3(0, 0, 1);
		light_ubo.camera_pos = glm::vec3(0, 0, 0);
		light_ubo.camera_view = glm::mat4();
		light_ubo.directional_light.color = glm::vec3(0.0f, 0.0f, 1.0f);
		light_ubo.directional_light.direction = glm::vec3(0.f, 0.f, 0.f);
		light_ubo.inv_camera_view_proj = glm::mat4();

		auto num_frames = vulkan_rhi->GetNumberFrames();
		lighting_render_data_.lighting_ubo_data.resize(num_frames);
		for (uint32_t i = 0; i < num_frames; ++i)
		{
			lighting_render_data_.lighting_ubo_data[i] = light_ubo;
		}
		
		/************************************************************************/

		CreateUniformBuffer(sizeof(LightingUBO) + 128);

		UpdateDeferredLightDescriptor();
		UpdateSkyboxDescriptor();
		UpdateColorGradingDescriptor();
	}

	void MainRenderPass::Render()
	{
		std::shared_ptr<VulkanRHI> vulkan_rhi = std::static_pointer_cast<VulkanRHI>(rhi_.lock());
		assert(vulkan_rhi.get() != nullptr);
		
		uint32_t current_frame_index = vulkan_rhi->GetCurrentFrameIndex();
		uint32_t frame_width = vulkan_rhi->GetDisplayWidth();
		uint32_t frame_height = vulkan_rhi->GetDisplayHeight();

		// begin render pass
		VkRenderPassBeginInfo render_pass_begin_info{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		render_pass_begin_info.renderPass = render_pass_.value();
		render_pass_begin_info.framebuffer = render_pass_framebuffer_[current_frame_index];
		render_pass_begin_info.renderArea.offset = { 0, 0 };
		render_pass_begin_info.renderArea.extent = { frame_width, frame_height };

		// clear render target
		std::array<VkClearValue, AttachmentType::AttachmentTypeCount> clear_values;
		clear_values[AttachmentType::GBufferA_Normal].color							= { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clear_values[AttachmentType::GBufferB_Metallic_Roughness_Occlusion].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clear_values[AttachmentType::GBufferC_BaseColor].color						= { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clear_values[AttachmentType::DepthImage].depthStencil						= { 1.0f, 0 };
		clear_values[AttachmentType::BackupBuffer].color							= { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clear_values[AttachmentType::SwapChain].color								  = { { 0.0f, 0.0f, 0.0f, 0.0f } };

		render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_begin_info.pClearValues = clear_values.data();

		VkCommandBuffer command_buffer = vulkan_rhi->GetCommandBuffer();
		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(frame_width);
		viewport.height = static_cast<float>(frame_height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		VkRect2D scissor;
		scissor.offset = { 0, 0 };
		scissor.extent = { frame_width, frame_height };
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);

		// mesh gbuffer render pass
		for (const auto& render_data : render_data_)
        {
            RenderMesh(command_buffer, render_data);
        }

		// deferred lighting render pass
		vkCmdNextSubpass(command_buffer, VK_SUBPASS_CONTENTS_INLINE);
		if (!render_data_.empty())
		{
			RenderDeferredLighting(command_buffer, current_frame_index);
		}
		
		// forward lighting pass [skybox light]
		vkCmdNextSubpass(command_buffer, VK_SUBPASS_CONTENTS_INLINE);

		// draw skybox
		if (skybox_render_data_)
		{
			// update skybox uniform buffer
			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_pipelines_[RenderPipelineType::Skybox].pipeline_);

			// bind vertex
			VkBuffer vertex_buffer[] = {skybox_render_data_->vertex_buffer.resource};
			vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffer, nullptr);
			vkCmdBindIndexBuffer(command_buffer, skybox_render_data_->index_buffer.resource, 0, VK_INDEX_TYPE_UINT32);

			// bind skybox descriptor
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				render_pipelines_[RenderPipelineType::Skybox].pipeline_layout_, 0, 1,
				&render_descriptors_[DescriptorLayoutType::Skybox].descritptor_set_, 0, nullptr);
			
			// update push constant
			std::vector<VkPushConstantRange> skybox_push_constant_range = { {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(TransformUBO)} };
			UpdatePushConstants(command_buffer, render_pipelines_[RenderPipelineType::Skybox].pipeline_layout_,
				{&skybox_render_data_->transform_ubo_data},skybox_push_constant_range);

			vkCmdDrawIndexed(command_buffer, skybox_render_data_->index_counts, 1, 0, 0, 0);
		}

		// draw transparency objects
		{
			for (const auto& render_data : transparency_render_data_)
			{
				RenderMesh(command_buffer, render_data, true);
			}
		}

		// color grading post process -- todo: remove to an single render pass
		{
			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_pipelines_[RenderPipelineType::ColorGrading].pipeline_);

			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				render_pipelines_[RenderPipelineType::ColorGrading].pipeline_layout_, 0, 1,
				&render_descriptors_[RenderPipelineType::ColorGrading].descritptor_set_, 0, nullptr);

			vkCmdDraw(command_buffer, 3, 1, 0, 0);
		}

		vkCmdEndRenderPass(command_buffer);
	}

	void MainRenderPass::RenderMesh(VkCommandBuffer command_buffer, const std::shared_ptr<RenderData>& render_data, bool is_forward)
	{
		auto rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		std::shared_ptr<StaticMeshRenderData> static_mesh_render_data = std::static_pointer_cast<StaticMeshRenderData>(render_data);

		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
		if (!is_forward)
		{
			pipeline = render_pipelines_[RenderPipelineType::MeshGbuffer].pipeline_;
			pipeline_layout = render_pipelines_[RenderPipelineType::MeshGbuffer].pipeline_layout_;
		}
		else
		{
			pipeline = render_pipelines_[RenderPipelineType::ForwardLighting].pipeline_;
			pipeline_layout = render_pipelines_[RenderPipelineType::ForwardLighting].pipeline_layout_;
		}

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		// bind vertex bufer and index buffer
		VkBuffer vertex_buffer[] = { static_mesh_render_data->vertex_buffer.resource };
		constexpr VkDeviceSize vertex_buffer_offset = { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffer, vertex_buffer_offset);
		vkCmdBindIndexBuffer(command_buffer, static_mesh_render_data->index_buffer.resource, 0, VK_INDEX_TYPE_UINT32);

		uint32_t submesh_counts = static_cast<uint32_t>(static_mesh_render_data->index_counts.size());
		for (uint32_t i = 0; i < submesh_counts; ++i)
		{
			if (!is_forward)
			{
				UpdatePushConstants(command_buffer, pipeline_layout,
					{ &static_mesh_render_data->transform_ubo_data, &static_mesh_render_data->material_pcos[i] },
					all_push_constant_range_[RenderPipelineType::MeshGbuffer]);
			}
			else
			{

				UpdatePushConstants(command_buffer, pipeline_layout,
					{ &static_mesh_render_data->transform_ubo_data, &static_mesh_render_data->material_pcos[i] },
					all_push_constant_range_[RenderPipelineType::ForwardLighting]);
			}

			const PbrMaterial& material = static_mesh_render_data->pbr_materials[i];
			
			if (!is_forward)
			{
				// allocate descriptor set
				VkDescriptorSet mesh_descriptor_set = CreateGbufferDescriptor();
				// update descriptor set
				UpdateGbufferDescriptor(command_buffer, material, mesh_descriptor_set);
				// bind descriptor set
				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					render_pipelines_[RenderPipelineType::MeshGbuffer].pipeline_layout_,
					0, 1, &mesh_descriptor_set, 0, nullptr);
			}
			else
			{
				VkDescriptorSet forward_descriptor_set = CreateForwardLightDescriptor();
				UpdateForwardLightDescriptor(command_buffer, material, forward_descriptor_set);
				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
					render_pipelines_[RenderPipelineType::ForwardLighting].pipeline_layout_,
					0, 1, &forward_descriptor_set, 0, nullptr);
			}

			// command draw
			vkCmdDrawIndexed(command_buffer, static_mesh_render_data->index_counts[i], 1, static_mesh_render_data->index_offsets[i], 0, 0);
		}
		
	}

	void MainRenderPass::RenderDeferredLighting(VkCommandBuffer command_buffer, uint32_t current_frame_index)
	{
		VkPipeline deferred_lighting_pipeline = render_pipelines_[RenderPipelineType::DeferredLighting].pipeline_;
		VkPipelineLayout pipeline_layout = render_pipelines_[RenderPipelineType::DeferredLighting].pipeline_layout_;

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, deferred_lighting_pipeline);

		// update lighting uniform buffer
		auto lighting_data = lighting_render_data_.lighting_ubo_data[current_frame_index];
		LightingUBO* light_ubo_data = lighting_data_uniform_buffer_->as<LightingUBO>(); // get uniform buffer
		light_ubo_data->CopyFrom(lighting_data);

		// bind descriptor set
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, 
			&render_descriptors_[DescriptorLayoutType::DeferredLighting].descritptor_set_, 0, nullptr);

		vkCmdDraw(command_buffer, 3, 1, 0, 0);
	}

	void MainRenderPass::CreateRenderTargets()
	{
		std::vector<RenderPassAttachment>& render_attachments = render_target_->attachments_;
		render_attachments.resize(AttachmentType::AttachmentTypeCount);

		std::shared_ptr<VulkanRHI> vulkan_rhi = std::static_pointer_cast<VulkanRHI>(rhi_.lock());
		uint32_t frame_width = vulkan_rhi->GetDisplayWidth();
		uint32_t frame_height = vulkan_rhi->GetDisplayHeight();

		render_attachments[AttachmentType::GBufferA_Normal].format_ = VK_FORMAT_R8G8B8A8_UNORM;
		render_attachments[AttachmentType::GBufferB_Metallic_Roughness_Occlusion].format_ = VK_FORMAT_R8G8B8A8_UNORM;
		render_attachments[AttachmentType::GBufferC_BaseColor].format_ = VK_FORMAT_R8G8B8A8_UNORM;
		render_attachments[AttachmentType::DepthImage].format_ = vulkan_rhi->GetDepthImageFormat();
		render_attachments[AttachmentType::BackupBuffer].format_ = VK_FORMAT_R8G8B8A8_UNORM;

		for (uint32_t i = 0; i < AttachmentType::AttachmentTypeCount; i++)
		{
			// create image
			Resource<VkImage> image;
			if (i == AttachmentType::GBufferA_Normal)
			{
				image = vulkan_rhi->CreateImage(frame_width, frame_height, 0, 1, 1, render_attachments[AttachmentType::GBufferA_Normal].format_,
					VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
				render_attachments[AttachmentType::GBufferA_Normal].image_ = image;
			}
			else if (i == AttachmentType::DepthImage)
			{
				image = vulkan_rhi->CreateImage(frame_width, frame_height, 0, 1, 1, render_attachments[AttachmentType::GBufferA_Normal].format_,
					VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
				render_attachments[AttachmentType::DepthImage].image_ = image;
			}
			else if (i == AttachmentType::SwapChain)
			{
				PEANUT_LOG_INFO("Skip swapchain image creat");
				continue;
			}
			else
			{
				image = vulkan_rhi->CreateImage(frame_width, frame_height, 0, 1, 1, render_attachments[i].format_,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
				render_attachments[i].image_ = image;
			}

			assert(image.resource != VK_NULL_HANDLE);
			
			// create image view
			render_attachments[i].image_view_ = vulkan_rhi->CreateImageView(image.resource, render_attachments[i].format_, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1);
		}
	}
	
	void MainRenderPass::CreateFramebuffer()
	{
		std::shared_ptr<VulkanRHI> vulkan_rhi = std::static_pointer_cast<VulkanRHI>(rhi_.lock());
		uint32_t frame_width = vulkan_rhi->GetDisplayWidth();
		uint32_t frame_height = vulkan_rhi->GetDisplayHeight();
		
		auto swapchain_image_view = vulkan_rhi->GetSwapchainImageView();
		render_pass_framebuffer_.resize(swapchain_image_view.size());
		
		uint32_t framebuffer_size = static_cast<uint32_t>(swapchain_image_view.size());
		for (uint32_t i = 0; i < framebuffer_size; i++)
		{
			std::vector<RenderPassAttachment>& render_attachments = render_target_->attachments_;
			std::vector<VkImageView> attachment_image_views =
			{
				render_attachments[AttachmentType::GBufferA_Normal].image_view_,
				render_attachments[AttachmentType::GBufferB_Metallic_Roughness_Occlusion].image_view_,
				render_attachments[AttachmentType::GBufferC_BaseColor].image_view_,
				render_attachments[AttachmentType::DepthImage].image_view_,
				render_attachments[AttachmentType::BackupBuffer].image_view_,
				// render_attachments[AttachmentType::BackupBufferEven].image_view_,
				swapchain_image_view[i]
			};

			VkFramebufferCreateInfo framebuffer_create_info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			framebuffer_create_info.renderPass = render_pass_.value();
			framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachment_image_views.size());
			framebuffer_create_info.pAttachments = attachment_image_views.data();
			framebuffer_create_info.width = frame_width;
			framebuffer_create_info.height = frame_height;
			framebuffer_create_info.layers = 1;
			
			vulkan_rhi->CreateFrameBuffer(&framebuffer_create_info, &render_pass_framebuffer_[i]);
		}
	}

	void MainRenderPass::CreateRenderPass()
	{
		std::vector<RenderPassAttachment>& render_attachments = render_target_->attachments_;
		std::vector<VkAttachmentDescription> attachments;
		attachments.resize(AttachmentType::AttachmentTypeCount);

		// setup attachment description
		attachments[AttachmentType::GBufferA_Normal] =
		{
			0, // flag
			render_attachments[AttachmentType::GBufferA_Normal].format_,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR, // load op
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencil load op
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		attachments[AttachmentType::GBufferB_Metallic_Roughness_Occlusion] =
		{
			0,
			render_attachments[AttachmentType::GBufferB_Metallic_Roughness_Occlusion].format_,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		attachments[AttachmentType::GBufferC_BaseColor] =
		{
			0,
			render_attachments[AttachmentType::GBufferC_BaseColor].format_,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		attachments[AttachmentType::DepthImage] =
		{
			0,
			render_attachments[AttachmentType::DepthImage].format_,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		attachments[AttachmentType::BackupBuffer] =
		{
			0,
			render_attachments[AttachmentType::BackupBuffer].format_,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		//attachments[AttachmentType::BackupBufferEven] =
		//{
		//	0,
		//	render_attachments[AttachmentType::BackupBufferEven].format_,
		//	VK_SAMPLE_COUNT_1_BIT,
		//	VK_ATTACHMENT_LOAD_OP_CLEAR,
		//	VK_ATTACHMENT_STORE_OP_DONT_CARE,
		//	VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		//	VK_ATTACHMENT_STORE_OP_DONT_CARE,
		//	VK_IMAGE_LAYOUT_UNDEFINED,
		//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		//};

		// create swapchain image description
		auto rhi = rhi_.lock();

		VulkanRHI* vulkan_rhi = reinterpret_cast<VulkanRHI*>(rhi.get());
		attachments[AttachmentType::SwapChain] =
		{
			0,
			vulkan_rhi->GetSwapChainImageFormat(),
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		// setup subpass information
		std::vector<VkSubpassDescription> subpasses_desc;
		subpasses_desc.resize(SubpassType::SubpassTypeCount);

		// create base subpass (gbuffer pass) decription
		std::array<VkAttachmentReference, 3> base_pass_color_attachment_refs{};
		base_pass_color_attachment_refs[0].attachment = AttachmentType::GBufferA_Normal;
		base_pass_color_attachment_refs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		base_pass_color_attachment_refs[1].attachment = AttachmentType::GBufferB_Metallic_Roughness_Occlusion;
		base_pass_color_attachment_refs[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		base_pass_color_attachment_refs[2].attachment = AttachmentType::GBufferC_BaseColor;
		base_pass_color_attachment_refs[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference base_pass_depth_attachment_refs;
		base_pass_depth_attachment_refs.attachment = AttachmentType::DepthImage;
		base_pass_depth_attachment_refs.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpasses_desc[SubpassType::BasePass] =
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0,		 // input attachment counts
			nullptr, // pointer of input attachment
			static_cast<uint32_t>(base_pass_color_attachment_refs.size()),		// color attachment counts
			base_pass_color_attachment_refs.data(),
			nullptr, // pointer of resolve attachment
			&base_pass_depth_attachment_refs,
			0,		 // reserve attachment counts
			nullptr // pointer of reserve attachment
		};

		// create deferred lighting subpass description
		std::array<VkAttachmentReference, 4> deferred_lighting_input_attachment_refs{};
		deferred_lighting_input_attachment_refs[0].attachment = AttachmentType::GBufferA_Normal;
		deferred_lighting_input_attachment_refs[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		deferred_lighting_input_attachment_refs[1].attachment = AttachmentType::GBufferB_Metallic_Roughness_Occlusion;
		deferred_lighting_input_attachment_refs[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		deferred_lighting_input_attachment_refs[2].attachment = AttachmentType::GBufferC_BaseColor;
		deferred_lighting_input_attachment_refs[2].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		deferred_lighting_input_attachment_refs[3].attachment = AttachmentType::DepthImage;
		deferred_lighting_input_attachment_refs[3].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference deferred_lighting_color_attachment_refs;
		deferred_lighting_color_attachment_refs.attachment = AttachmentType::BackupBuffer;
		deferred_lighting_color_attachment_refs.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpasses_desc[SubpassType::DeferredLightingPass] =
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			4,		 // input attachment counts
			deferred_lighting_input_attachment_refs.data(), // pointer of input attachment
			1,		// color attachment counts
			&deferred_lighting_color_attachment_refs,
			nullptr, // resolve attachment
			nullptr,  // depth attachment
			0,		 // reserve attachment counts
			nullptr // reserve attachment
		};

		// create forward lighting subpass description to draw transparent objects
		VkAttachmentReference forward_lighting_color_attachment_refs;
		forward_lighting_color_attachment_refs.attachment = AttachmentType::BackupBuffer;
		forward_lighting_color_attachment_refs.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference forward_lighting_depth_attachment_refs;
		forward_lighting_depth_attachment_refs.attachment = AttachmentType::DepthImage;
		forward_lighting_depth_attachment_refs.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpasses_desc[SubpassType::ForwardLightingPass] =
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0,		 // input attachment counts
			nullptr, // input attachment
			1,		// color attachment counts
			&forward_lighting_color_attachment_refs,
			nullptr, // resolve attachment
			&forward_lighting_depth_attachment_refs,  // depth attachment
			0,		 // reserve attachment counts
			nullptr // reserve attachment
		};

		// create tone mapping subpass description
		//VkAttachmentReference tone_mapping_input_attachment_refs;
		//tone_mapping_input_attachment_refs.attachment = AttachmentType::BackupBufferOdd; // from lighting pass
		//tone_mapping_input_attachment_refs.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		//VkAttachmentReference tone_mapping_color_attachment_refs;
		//tone_mapping_color_attachment_refs.attachment = AttachmentType::BackupBufferEven; // to color grading pass
		//tone_mapping_color_attachment_refs.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//subpasses_desc[SubpassType::ToneMappingPass] =
		//{
		//	0,
		//	VK_PIPELINE_BIND_POINT_GRAPHICS,
		//	1,		 // input attachment counts
		//	&tone_mapping_input_attachment_refs, // input attachment
		//	1,		// color attachment counts
		//	&tone_mapping_color_attachment_refs,
		//	nullptr, // resolve attachment
		//	nullptr,  // depth attachment
		//	0,		 // reserve attachment counts
		//	nullptr // reserve attachment
		//};

		// create color grading subpass description
		VkAttachmentReference color_grading_input_attachment_refs;
		color_grading_input_attachment_refs.attachment = AttachmentType::BackupBuffer; // from tone mapping pass
		color_grading_input_attachment_refs.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference color_grading_color_attachment_refs;
		color_grading_color_attachment_refs.attachment = AttachmentType::SwapChain; // to swap chain
		color_grading_color_attachment_refs.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		subpasses_desc[SubpassType::ColorGradingPass] =
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			1,		 // input attachment counts
			&color_grading_input_attachment_refs, // input attachment
			1,		// color attachment counts
			&color_grading_color_attachment_refs,
			nullptr, // resolve attachment
			nullptr,  // depth attachment
			0,		 // reserve attachment counts
			nullptr // reserve attachment
		};

		// create subpass dependency
		std::vector<VkSubpassDependency> subpass_dependencies{};
		subpass_dependencies.resize(SubpassType::SubpassTypeCount);
		
		// fixme: need it?
		subpass_dependencies[0] =
		{
			VK_SUBPASS_EXTERNAL,
			SubpassType::BasePass,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			0
		};

		subpass_dependencies[1] =
		{
			SubpassType::BasePass,
			SubpassType::DeferredLightingPass,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		};

		subpass_dependencies[2] =
		{
			SubpassType::DeferredLightingPass,
			SubpassType::ForwardLightingPass,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		};

		subpass_dependencies[3] =
		{
			SubpassType::ForwardLightingPass,
			SubpassType::ColorGradingPass,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		};

		//subpass_dependencies[4] =
		//{
		//	SubpassType::ToneMappingPass,
		//	SubpassType::ColorGradingPass,
		//	VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		//	VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		//	VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		//	VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
		//	VK_DEPENDENCY_BY_REGION_BIT
		//};

		// create render pass
		VkRenderPassCreateInfo render_pass_ci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
		render_pass_ci.attachmentCount = static_cast<uint32_t>(attachments.size());
		render_pass_ci.pAttachments = attachments.data();
		render_pass_ci.subpassCount = static_cast<uint32_t>(subpasses_desc.size());
		render_pass_ci.pSubpasses = subpasses_desc.data();
		render_pass_ci.dependencyCount = static_cast<uint32_t>(subpass_dependencies.size());
		render_pass_ci.pDependencies = subpass_dependencies.data();
		
		assert(rhi.get() != nullptr);

		VkRenderPass renderpass;
		rhi->CreateRenderPass(&render_pass_ci, &renderpass);
		render_pass_ = renderpass;
	}

	void MainRenderPass::CreateDescriptorSetLayouts()
	{
		render_descriptors_.resize(DescriptorLayoutType::DescriptorLayoutTypeCount);

		auto rhi = rhi_.lock();
		if (!rhi)
		{
			PEANUT_LOG_ERROR("RHI is nullptr when create descriptorset layout");
			return;
		}

		// gbuffer descriptor layout
		std::vector<VkDescriptorSetLayoutBinding> gbuffer_descriptor_layout_binding =
		{
			{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // normal
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // metallic & roughness & occlusion
			{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // base color
			{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} //  emissive
		};

		render_descriptors_[DescriptorLayoutType::MeshGbuffer].descriptor_set_layout_ = 
			rhi->CreateDescriptorSetLayout(gbuffer_descriptor_layout_binding);


		// deferred lighting layout
		std::vector<VkDescriptorSetLayoutBinding> deferred_lighting_descriptor_layout_binding =
		{
			{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // gbuffer a
			{1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // gbuffer b
			{2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // gbuffer c
			{3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // depth image
			{4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // irradiance
			{5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // prefiltered
			{6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // brdf lut
			{7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // directonal light shadow
			// {8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // point light shadow
			{9, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // light uniform buffer
		};

		render_descriptors_[DescriptorLayoutType::DeferredLighting].descriptor_set_layout_ =
			rhi->CreateDescriptorSetLayout(deferred_lighting_descriptor_layout_binding);

		// forward lighting layout used by transparency objects
		std::vector<VkDescriptorSetLayoutBinding> forward_lighting_descriptor_layout_binding =
		{
			{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // normal texture
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // metallic roughness occlusion texture
			{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // base color texture
			{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // emissive texture
			{4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // irradiance
			{5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // prefiltered
			{6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // brdf lut
			{7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // directonal light shadow
			// {8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // point light shadow
			{9, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // light uniform buffer
		};

		render_descriptors_[DescriptorLayoutType::ForwardLighting].descriptor_set_layout_ =
			rhi->CreateDescriptorSetLayout(forward_lighting_descriptor_layout_binding);

		// skybox layout
		std::vector<VkDescriptorSetLayoutBinding> skybox_descriptor_layout_binding =
		{
			{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
		};

		render_descriptors_[DescriptorLayoutType::Skybox].descriptor_set_layout_ =
			rhi->CreateDescriptorSetLayout(skybox_descriptor_layout_binding);

		// color grading descriptor layout
		std::vector<VkDescriptorSetLayoutBinding> color_grading_descriptor_layout_binding =
		{
			{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // backbuffer
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // color grading lut
		};

		render_descriptors_[DescriptorLayoutType::ColorGrading].descriptor_set_layout_ =
			rhi->CreateDescriptorSetLayout(color_grading_descriptor_layout_binding);
	}

	void MainRenderPass::CreatePipelineLayouts()
	{
		auto rhi = rhi_.lock();
		if (!rhi)
		{
			PEANUT_LOG_ERROR("RHI is nullptr when create pipeline layout");
			return;
		}

		// mesh gbuffer pipeline layout
		std::vector<VkPushConstantRange> push_constant_range =
		{
			// transform ubo
			{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(TransformUBO)},
			// material ubo
			{VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(TransformUBO), sizeof(MaterialPCO)}
		};
		
		all_push_constant_range_.insert(std::make_pair(RenderPipelineType::MeshGbuffer, push_constant_range));
		
		std::vector<VkDescriptorSetLayout> set_layouts;
		set_layouts.push_back(render_descriptors_[DescriptorLayoutType::MeshGbuffer].descriptor_set_layout_);
		render_pipelines_[RenderPipelineType::MeshGbuffer].pipeline_layout_ =
			rhi->CreatePipelineLayout(set_layouts, push_constant_range);

		set_layouts.clear();
		push_constant_range.clear();

		// deferred lighting pipeline layout
		set_layouts.push_back(render_descriptors_[DescriptorLayoutType::DeferredLighting].descriptor_set_layout_);
		render_pipelines_[RenderPipelineType::DeferredLighting].pipeline_layout_ =
			rhi->CreatePipelineLayout(set_layouts, push_constant_range);

		set_layouts.clear();
		push_constant_range.clear();
		
		// forward lighting pipeline layout
		set_layouts.push_back(render_descriptors_[DescriptorLayoutType::ForwardLighting].descriptor_set_layout_);
		render_pipelines_[RenderPipelineType::ForwardLighting].pipeline_layout_ =
			rhi->CreatePipelineLayout(set_layouts, push_constant_range);

		set_layouts.clear();
		push_constant_range.clear();
		
		// skybox pipeline layout
		set_layouts.push_back(render_descriptors_[DescriptorLayoutType::Skybox].descriptor_set_layout_);
		render_pipelines_[RenderPipelineType::Skybox].pipeline_layout_ =
			rhi->CreatePipelineLayout(set_layouts, push_constant_range);

		// color grading pipeline layout
		set_layouts.clear();
		set_layouts.push_back(render_descriptors_[DescriptorLayoutType::ColorGrading].descriptor_set_layout_);
		render_pipelines_[RenderPipelineType::ColorGrading].pipeline_layout_ =
			rhi->CreatePipelineLayout(set_layouts, push_constant_range);

	}

	void MainRenderPass::CreatePipelines()
	{
		auto rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		std::vector<VkVertexInputBindingDescription> vertex_input_binding_descs;
		vertex_input_binding_descs.resize(1, VkVertexInputBindingDescription());
		vertex_input_binding_descs[0].binding = 0;
		vertex_input_binding_descs[0].stride = sizeof(Mesh::Vertex);
		vertex_input_binding_descs[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		// vertex attributes
		std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descs;
		vertex_input_attribute_descs.resize(4, VkVertexInputAttributeDescription());
		
		vertex_input_attribute_descs[0].binding = 0;
		vertex_input_attribute_descs[0].location = 0;
		vertex_input_attribute_descs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertex_input_attribute_descs[0].offset = offsetof(Mesh::Vertex, position);

		vertex_input_attribute_descs[1].binding = 0;
		vertex_input_attribute_descs[1].location = 1;
		vertex_input_attribute_descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertex_input_attribute_descs[1].offset = offsetof(Mesh::Vertex, normal);

		vertex_input_attribute_descs[2].binding = 0;
		vertex_input_attribute_descs[2].location = 2;
		vertex_input_attribute_descs[2].format = VK_FORMAT_R32G32_SFLOAT;
		vertex_input_attribute_descs[2].offset = offsetof(Mesh::Vertex, tangent);

		vertex_input_attribute_descs[3].binding = 0;
		vertex_input_attribute_descs[3].location = 3;
		vertex_input_attribute_descs[3].format = VK_FORMAT_R32G32_SFLOAT;
		vertex_input_attribute_descs[3].offset = offsetof(Mesh::Vertex, texcoord);

		VkPipelineVertexInputStateCreateInfo vertex_input_ci{};
		vertex_input_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_ci.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_input_binding_descs.size());
		vertex_input_ci.pVertexBindingDescriptions = vertex_input_binding_descs.data();
		vertex_input_ci.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_input_attribute_descs.size());
		vertex_input_ci.pVertexAttributeDescriptions = vertex_input_attribute_descs.data();

		std::vector<VkPipelineShaderStageCreateInfo> shader_stage_cis =
		{
			ShaderManager::Get().GetShaderStageCreateInfo(rhi_, "mesh.vert", VK_SHADER_STAGE_VERTEX_BIT),
			ShaderManager::Get().GetShaderStageCreateInfo(rhi_, "mesh_gbuffer.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
		};

		for (int i = 0; i < 4; ++i)
		{
			color_blend_attachment_states.push_back(color_blend_attachment_states.front());
		}

		color_blend_state_ci.attachmentCount = static_cast<uint32_t>(color_blend_attachment_states.size());
		color_blend_state_ci.pAttachments = color_blend_attachment_states.data();

		// create pipeline 
		pipeline_create_info_.stageCount = static_cast<uint32_t>(shader_stage_cis.size());
		pipeline_create_info_.pStages = shader_stage_cis.data();
		pipeline_create_info_.pVertexInputState = &vertex_input_ci;
		pipeline_create_info_.renderPass = *render_pass_;
		pipeline_create_info_.subpass = SubpassType::BasePass;
		pipeline_create_info_.layout = render_pipelines_[RenderPipelineType::MeshGbuffer].pipeline_layout_;

		// mesh gbuffer
		render_pipelines_[RenderPipelineType::MeshGbuffer].pipeline_ = rhi->CreateGraphicsPipeline(pipeline_cache_, 1, &pipeline_create_info_);

		// foward lighting
		color_blend_state_ci.attachmentCount = 1;
		color_blend_attachment_states[0].blendEnable = VK_TRUE;
		shader_stage_cis[1] = ShaderManager::Get().GetShaderStageCreateInfo(rhi_, "forward_lighting.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
		pipeline_create_info_.layout = render_pipelines_[RenderPipelineType::ForwardLighting].pipeline_layout_;
		pipeline_create_info_.subpass = SubpassType::ForwardLightingPass;
		render_pipelines_[RenderPipelineType::ForwardLighting].pipeline_ = rhi->CreateGraphicsPipeline(pipeline_cache_, 1, &pipeline_create_info_);

		// skybox pipeline
		shader_stage_cis =
		{
			ShaderManager::Get().GetShaderStageCreateInfo(rhi_, "skybox.vert", VK_SHADER_STAGE_VERTEX_BIT),
			ShaderManager::Get().GetShaderStageCreateInfo(rhi_, "skybox.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
		};
		color_blend_attachment_states[0].blendEnable = VK_FALSE;
		depth_stencil_state_ci.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		rasterization_state_ci.cullMode = VK_CULL_MODE_NONE;
		pipeline_create_info_.layout = render_pipelines_[RenderPipelineType::Skybox].pipeline_layout_;
		pipeline_create_info_.subpass = SubpassType::ForwardLightingPass;
		render_pipelines_[RenderPipelineType::Skybox].pipeline_ = rhi->CreateGraphicsPipeline(pipeline_cache_, 1, &pipeline_create_info_);

		// deferred lighting
		shader_stage_cis = 
		{
			ShaderManager::Get().GetShaderStageCreateInfo(rhi_, "screen.vert", VK_SHADER_STAGE_VERTEX_BIT),
			ShaderManager::Get().GetShaderStageCreateInfo(rhi_, "deferred_lighting.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
		};
		rasterization_state_ci.cullMode = VK_CULL_MODE_NONE;
		depth_stencil_state_ci.depthTestEnable = VK_FALSE;
		depth_stencil_state_ci.depthWriteEnable = VK_FALSE;

		color_blend_state_ci.attachmentCount = 1;
		color_blend_attachment_states[0].blendEnable = VK_FALSE;

		VkPipelineVertexInputStateCreateInfo deferred_light_vertex_input_ci{};
		deferred_light_vertex_input_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		pipeline_create_info_.pVertexInputState = &deferred_light_vertex_input_ci;
		pipeline_create_info_.layout = render_pipelines_[RenderPipelineType::DeferredLighting].pipeline_layout_;
		pipeline_create_info_.subpass = SubpassType::DeferredLightingPass;

		render_pipelines_[RenderPipelineType::DeferredLighting].pipeline_ = rhi->CreateGraphicsPipeline(pipeline_cache_, 1, &pipeline_create_info_);

		// color grading -- todo: remove to post process pass
		shader_stage_cis =
		{
			ShaderManager::Get().GetShaderStageCreateInfo(rhi_, "screen.vert", VK_SHADER_STAGE_VERTEX_BIT),
			ShaderManager::Get().GetShaderStageCreateInfo(rhi_, "color_grading.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
		};

		pipeline_create_info_.layout = render_pipelines_[RenderPipelineType::ColorGrading].pipeline_layout_;
		pipeline_create_info_.subpass = SubpassType::ColorGradingPass;
		render_pipelines_[RenderPipelineType::ColorGrading].pipeline_ = rhi->CreateGraphicsPipeline(pipeline_cache_, 1, &pipeline_create_info_);
	}

	void MainRenderPass::CreateDescriptorSets()
	{
		CreateDeferredLightDescriptor();
		CreateForwardLightDescriptor();
		CreateSkyboxDescriptor();
		CreateColorGradingDescriptor();
	}

	VkDescriptorSet MainRenderPass::CreateGbufferDescriptor()
	{
		auto rhi = rhi_.lock();

		assert(rhi.get() != nullptr);

		render_descriptors_[DescriptorLayoutType::MeshGbuffer].descritptor_set_ =
			rhi->AllocateDescriptor(render_descriptors_[DescriptorLayoutType::MeshGbuffer].descriptor_set_layout_);
		return render_descriptors_[DescriptorLayoutType::ForwardLighting].descritptor_set_;
	}

	void MainRenderPass::CreateDeferredLightDescriptor()
	{
		auto rhi = rhi_.lock();

		assert(rhi.get() != nullptr);

		render_descriptors_[DescriptorLayoutType::DeferredLighting].descritptor_set_ =
			rhi->AllocateDescriptor(render_descriptors_[DescriptorLayoutType::DeferredLighting].descriptor_set_layout_);
	}

	VkDescriptorSet MainRenderPass::CreateForwardLightDescriptor()
	{
		auto rhi = rhi_.lock();

		assert(rhi.get() != nullptr);

		render_descriptors_[DescriptorLayoutType::ForwardLighting].descritptor_set_ =
			rhi->AllocateDescriptor(render_descriptors_[DescriptorLayoutType::ForwardLighting].descriptor_set_layout_);
		return render_descriptors_[DescriptorLayoutType::ForwardLighting].descritptor_set_;
	}

	void MainRenderPass::CreateSkyboxDescriptor()
	{
		auto rhi = rhi_.lock();

		assert(rhi.get() != nullptr);

		render_descriptors_[DescriptorLayoutType::Skybox].descritptor_set_ =
			rhi->AllocateDescriptor(render_descriptors_[DescriptorLayoutType::Skybox].descriptor_set_layout_);
	}

	void MainRenderPass::CreateColorGradingDescriptor()
	{
		auto rhi = rhi_.lock();

		assert(rhi.get() != nullptr);

		render_descriptors_[DescriptorLayoutType::ColorGrading].descritptor_set_ =
			rhi->AllocateDescriptor(render_descriptors_[DescriptorLayoutType::ColorGrading].descriptor_set_layout_);
	}

	void MainRenderPass::UpdateColorGradingDescriptor()
	{
		auto rhi = rhi_.lock();

		assert(rhi.get() != nullptr);
		VulkanRHI* vulkan_rhi = reinterpret_cast<VulkanRHI*>(rhi.get());

		VkDescriptorImageInfo color_image_info = {};
		color_image_info.sampler = vulkan_rhi->GetOrCreateSampler(VulkanRHI::Linear);
		color_image_info.imageView = render_target_->attachments_[AttachmentType::BackupBuffer].image_view_;
		color_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo lut_image_info = {};
		lut_image_info.sampler = color_grading_render_data_->color_grading_lut_texture.image_sampler; // todo: get lut image sampler
		lut_image_info.imageView = color_grading_render_data_->color_grading_lut_texture.image_view; // todo: add lut image view
		lut_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


		std::array<VkWriteDescriptorSet, 2> descriptor_writes = {};
		descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[0].descriptorCount = 1;
		descriptor_writes[0].dstArrayElement = 0;
		descriptor_writes[0].dstBinding = 0;
		descriptor_writes[0].dstSet = render_descriptors_[DescriptorLayoutType::ColorGrading].descritptor_set_;
		descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

		descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[1].descriptorCount = 1;
		descriptor_writes[1].dstArrayElement = 0;
		descriptor_writes[1].dstBinding = 1;
		descriptor_writes[1].dstSet = render_descriptors_[DescriptorLayoutType::ColorGrading].descritptor_set_;
		descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		vulkan_rhi->UpdateDescriptorSets(descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
	}

	void MainRenderPass::UpdateDeferredLightDescriptor()
	{
		auto rhi = rhi_.lock();

		assert(rhi.get() != nullptr);
		VulkanRHI* vulkan_rhi = reinterpret_cast<VulkanRHI*>(rhi.get());
		
		VkDescriptorImageInfo gbuffer_normal_image_info = {};
		gbuffer_normal_image_info.sampler = vulkan_rhi->GetOrCreateSampler(VulkanRHI::Nearest);
		gbuffer_normal_image_info.imageView = render_target_->attachments_[AttachmentType::GBufferA_Normal].image_view_;
		gbuffer_normal_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo gbuffer_metallic_roughtness_occlusion_image_info = {};
		gbuffer_metallic_roughtness_occlusion_image_info.sampler = vulkan_rhi->GetOrCreateSampler(VulkanRHI::Nearest);
		gbuffer_metallic_roughtness_occlusion_image_info.imageView = render_target_->attachments_[AttachmentType::GBufferB_Metallic_Roughness_Occlusion].image_view_;
		gbuffer_metallic_roughtness_occlusion_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo gbuffer_basecolor_image_info = {};
		gbuffer_basecolor_image_info.sampler = vulkan_rhi->GetOrCreateSampler(VulkanRHI::Nearest);
		gbuffer_basecolor_image_info.imageView = render_target_->attachments_[AttachmentType::GBufferC_BaseColor].image_view_;
		gbuffer_basecolor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo depth_image_info = {};
		depth_image_info.sampler = vulkan_rhi->GetOrCreateSampler(VulkanRHI::Nearest);
		depth_image_info.imageView = render_target_->attachments_[AttachmentType::DepthImage].image_view_;
		depth_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// ibl irradiance texture
		VkDescriptorImageInfo irradiance_texture_info = {};
		irradiance_texture_info.sampler = lighting_render_data_.ibl_light_texture.ibl_irradiance_texture.image_sampler;
		irradiance_texture_info.imageView = lighting_render_data_.ibl_light_texture.ibl_irradiance_texture.image_view; // todo: get irradiance texture image view
		irradiance_texture_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// ibl prefiltered texture
		VkDescriptorImageInfo prefiltered_texture_info = {};
		prefiltered_texture_info.sampler = lighting_render_data_.ibl_light_texture.ibl_prefilter_texture.image_sampler;
		prefiltered_texture_info.imageView = lighting_render_data_.ibl_light_texture.ibl_prefilter_texture.image_view; // todo: get prefiltered texture image view
		prefiltered_texture_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// ibl brdf lut
		VkDescriptorImageInfo brdf_lut_info = {};
		brdf_lut_info.sampler = lighting_render_data_.ibl_light_texture.brdf_lut_texture.image_sampler;
		brdf_lut_info.imageView = lighting_render_data_.ibl_light_texture.brdf_lut_texture.image_view;
		brdf_lut_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// directional light shadow texture
		VkDescriptorImageInfo directional_light_shadow_info = {};
		directional_light_shadow_info.sampler = lighting_render_data_.directional_light_shadow_map.image_sampler;
		directional_light_shadow_info.imageView = lighting_render_data_.directional_light_shadow_map.image_view;
		directional_light_shadow_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// point light shadow texture
		//VkDescriptorImageInfo point_light_shadow_info = {};
		//point_light_shadow_info.sampler = vulkan_rhi->GetOrCreateSampler(VulkanRHI::Nearest);
		//point_light_shadow_info.imageView = VK_NULL_HANDLE;
		//point_light_shadow_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// light uniform buffer
		if (!lighting_data_uniform_buffer_.has_value())
		{
			lighting_data_uniform_buffer_ = AllocateSubstorageFromUniformBuffer<LightingUBO>();
		}
		
		VkDescriptorBufferInfo& light_uniform_buffer_info = lighting_data_uniform_buffer_->descriptor_info;

		std::array<VkWriteDescriptorSet, 9> descriptor_writes = {};

		descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[0].dstSet = render_descriptors_[DescriptorLayoutType::DeferredLighting].descritptor_set_;
		descriptor_writes[0].dstBinding = 0;
		descriptor_writes[0].dstArrayElement = 0;
		descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		descriptor_writes[0].descriptorCount = 1;
		descriptor_writes[0].pImageInfo = &gbuffer_normal_image_info;

		descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[1].dstSet = render_descriptors_[DescriptorLayoutType::DeferredLighting].descritptor_set_;
		descriptor_writes[1].dstBinding = 1;
		descriptor_writes[1].dstArrayElement = 0;
		descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		descriptor_writes[1].descriptorCount = 1;
		descriptor_writes[1].pImageInfo = &gbuffer_metallic_roughtness_occlusion_image_info;

		descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[2].dstSet = render_descriptors_[DescriptorLayoutType::DeferredLighting].descritptor_set_;
		descriptor_writes[2].dstBinding = 2;
		descriptor_writes[2].dstArrayElement = 0;
		descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		descriptor_writes[2].descriptorCount = 1;
		descriptor_writes[2].pImageInfo = &gbuffer_basecolor_image_info;

		descriptor_writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[3].dstSet = render_descriptors_[DescriptorLayoutType::DeferredLighting].descritptor_set_;
		descriptor_writes[3].dstBinding = 3;
		descriptor_writes[3].dstArrayElement = 0;
		descriptor_writes[3].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		descriptor_writes[3].descriptorCount = 1;
		descriptor_writes[3].pImageInfo = &depth_image_info;

		descriptor_writes[4] = descriptor_writes[3];
		descriptor_writes[4].dstBinding = 4;
		descriptor_writes[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_writes[4].descriptorCount = 1;
		descriptor_writes[4].pImageInfo = &irradiance_texture_info;

		descriptor_writes[5] = descriptor_writes[4];
		descriptor_writes[5].dstBinding = 5;
		descriptor_writes[5].pImageInfo = &prefiltered_texture_info;

		descriptor_writes[6] = descriptor_writes[4];
		descriptor_writes[6].dstBinding = 6;
		descriptor_writes[6].pImageInfo = &brdf_lut_info;

		descriptor_writes[7] = descriptor_writes[4];
		descriptor_writes[7].dstBinding = 7;
		descriptor_writes[7].pImageInfo = &directional_light_shadow_info;

		//descriptor_writes[8] = descriptor_writes[4];
		//descriptor_writes[8].dstBinding = 8;
		//descriptor_writes[8].pImageInfo = &point_light_shadow_info;

		descriptor_writes[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[8].dstSet = render_descriptors_[DescriptorLayoutType::DeferredLighting].descritptor_set_;
		descriptor_writes[8].dstBinding = 9;
		descriptor_writes[8].dstArrayElement = 0;
		descriptor_writes[8].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_writes[8].descriptorCount = 1;
		descriptor_writes[8].pBufferInfo = &light_uniform_buffer_info;

		vulkan_rhi->UpdateDescriptorSets(descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
	}

	void MainRenderPass::UpdateForwardLightDescriptor(VkCommandBuffer command_buffer, const PbrMaterial& material_data, VkDescriptorSet dst_descriptor_set)
	{
		// same as deffred lighting
		auto rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		VulkanRHI* vulkan_rhi = reinterpret_cast<VulkanRHI*>(rhi.get());

		VkDescriptorImageInfo normal_image_info = {};
		normal_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		normal_image_info.imageView = material_data.normal_texture->image_view;
		normal_image_info.sampler = vulkan_rhi->GetMipmapSampler(material_data.normal_texture->width, material_data.normal_texture->height);

		VkDescriptorImageInfo metallic_roughness_occlusion_image_info = {};
		metallic_roughness_occlusion_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		metallic_roughness_occlusion_image_info.imageView = material_data.metallic_roughness_occlusion_texture->image_view;
		metallic_roughness_occlusion_image_info.sampler = vulkan_rhi->GetMipmapSampler(material_data.metallic_roughness_occlusion_texture->width,
			material_data.metallic_roughness_occlusion_texture->height);

		VkDescriptorImageInfo basecolor_image_info = {};
		basecolor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		basecolor_image_info.imageView = material_data.base_color_texture->image_view;
		basecolor_image_info.sampler = vulkan_rhi->GetMipmapSampler(material_data.base_color_texture->width,
			material_data.base_color_texture->height);

		VkDescriptorImageInfo emissive_image_info = {};
		emissive_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		emissive_image_info.imageView = material_data.emissive_texture->image_view;
		emissive_image_info.sampler = vulkan_rhi->GetMipmapSampler(material_data.emissive_texture->width,
			material_data.emissive_texture->height);

		// ibl irradiance texture
		VkDescriptorImageInfo irradiance_texture_info = {};
		irradiance_texture_info.sampler = lighting_render_data_.ibl_light_texture.ibl_irradiance_texture.image_sampler;
		irradiance_texture_info.imageView = lighting_render_data_.ibl_light_texture.ibl_irradiance_texture.image_view; // todo: get irradiance texture image view
		irradiance_texture_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// ibl prefiltered texture
		VkDescriptorImageInfo prefiltered_texture_info = {};
		prefiltered_texture_info.sampler = lighting_render_data_.ibl_light_texture.ibl_prefilter_texture.image_sampler;
		prefiltered_texture_info.imageView = lighting_render_data_.ibl_light_texture.ibl_prefilter_texture.image_view; // todo: get prefiltered texture image view
		prefiltered_texture_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// ibl brdf lut
		VkDescriptorImageInfo brdf_lut_info = {};
		brdf_lut_info.sampler = lighting_render_data_.ibl_light_texture.brdf_lut_texture.image_sampler;
		brdf_lut_info.imageView = lighting_render_data_.ibl_light_texture.brdf_lut_texture.image_view;
		brdf_lut_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// directional light shadow texture
		VkDescriptorImageInfo directional_light_shadow_info = {};
		directional_light_shadow_info.sampler = lighting_render_data_.directional_light_shadow_map.image_sampler;
		directional_light_shadow_info.imageView = lighting_render_data_.directional_light_shadow_map.image_view;
		directional_light_shadow_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if (!lighting_data_uniform_buffer_.has_value())
		{
			lighting_data_uniform_buffer_ = AllocateSubstorageFromUniformBuffer<LightingUBO>();
		}

		VkDescriptorBufferInfo& light_uniform_buffer_info = lighting_data_uniform_buffer_->descriptor_info;

		std::array<VkWriteDescriptorSet, 9> descriptor_writes = {};

		descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[0].dstSet = render_descriptors_[DescriptorLayoutType::ForwardLighting].descritptor_set_;
		descriptor_writes[0].dstBinding = 0;
		descriptor_writes[0].dstArrayElement = 0;
		descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_writes[0].descriptorCount = 1;
		descriptor_writes[0].pImageInfo = &normal_image_info;

		descriptor_writes[1] = descriptor_writes[0];
		descriptor_writes[1].dstBinding = 1;
		descriptor_writes[1].pImageInfo = &metallic_roughness_occlusion_image_info;

		descriptor_writes[2] = descriptor_writes[0];
		descriptor_writes[2].dstBinding = 2;
		descriptor_writes[2].pImageInfo = &basecolor_image_info;

		descriptor_writes[3] = descriptor_writes[0];
		descriptor_writes[3].dstBinding = 3;
		descriptor_writes[3].pImageInfo = &emissive_image_info;

		descriptor_writes[4] = descriptor_writes[0];
		descriptor_writes[4].dstBinding = 4;
		descriptor_writes[4].pImageInfo = &irradiance_texture_info;

		descriptor_writes[5] = descriptor_writes[0];
		descriptor_writes[5].dstBinding = 5;
		descriptor_writes[5].pImageInfo = &prefiltered_texture_info;

		descriptor_writes[6] = descriptor_writes[0];
		descriptor_writes[6].dstBinding = 6;
		descriptor_writes[6].pImageInfo = &brdf_lut_info;

		descriptor_writes[7] = descriptor_writes[0];
		descriptor_writes[7].dstBinding = 7;
		descriptor_writes[7].pImageInfo = &directional_light_shadow_info;

		//descriptor_writes[8] = descriptor_writes[4];
		//descriptor_writes[8].dstBinding = 8;
		//descriptor_writes[8].pImageInfo = &point_light_shadow_info;

		descriptor_writes[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[8].dstSet = render_descriptors_[DescriptorLayoutType::ForwardLighting].descritptor_set_;
		descriptor_writes[8].dstBinding = 9;
		descriptor_writes[8].dstArrayElement = 0;
		descriptor_writes[8].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_writes[8].descriptorCount = 1;
		descriptor_writes[8].pBufferInfo = &light_uniform_buffer_info;

		vulkan_rhi->UpdateDescriptorSets(descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
	}

	void MainRenderPass::UpdateGbufferDescriptor(VkCommandBuffer command_buffer, const PbrMaterial& material_data, VkDescriptorSet dst_descriptor_set)
	{
		auto rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		VulkanRHI* vulkan_rhi = reinterpret_cast<VulkanRHI*>(rhi.get());

		VkDescriptorImageInfo normal_image_info = {};
		normal_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		normal_image_info.imageView = material_data.normal_texture->image_view;
		normal_image_info.sampler = vulkan_rhi->GetMipmapSampler(material_data.normal_texture->width, material_data.normal_texture->height);

		VkDescriptorImageInfo metallic_roughness_occlusion_image_info = {};
		metallic_roughness_occlusion_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		metallic_roughness_occlusion_image_info.imageView = material_data.metallic_roughness_occlusion_texture->image_view;
		metallic_roughness_occlusion_image_info.sampler = vulkan_rhi->GetMipmapSampler(material_data.metallic_roughness_occlusion_texture->width,
																					   material_data.metallic_roughness_occlusion_texture->height);

		VkDescriptorImageInfo basecolor_image_info = {};
		basecolor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		basecolor_image_info.imageView = material_data.base_color_texture->image_view;
		basecolor_image_info.sampler = vulkan_rhi->GetMipmapSampler(material_data.base_color_texture->width,
																	material_data.base_color_texture->height);

		VkDescriptorImageInfo emissive_image_info = {};
		emissive_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		emissive_image_info.imageView = material_data.emissive_texture->image_view;
		emissive_image_info.sampler = vulkan_rhi->GetMipmapSampler(material_data.emissive_texture->width,
																	material_data.emissive_texture->height);
		
		VkWriteDescriptorSet write_descriptor_set[4] = {};
		write_descriptor_set[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_set[0].dstSet = dst_descriptor_set;
		write_descriptor_set[0].dstBinding = 0;
		write_descriptor_set[0].dstArrayElement = 0;
		write_descriptor_set[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_descriptor_set[0].descriptorCount = 1;
		write_descriptor_set[0].pImageInfo = &normal_image_info;

		write_descriptor_set[1] = write_descriptor_set[0];
		write_descriptor_set[1].dstBinding = 1;
		write_descriptor_set[1].pImageInfo = &metallic_roughness_occlusion_image_info;

		write_descriptor_set[2] = write_descriptor_set[0];
		write_descriptor_set[2].dstBinding = 2;
		write_descriptor_set[2].pImageInfo = &basecolor_image_info;

		write_descriptor_set[3] = write_descriptor_set[0];
		write_descriptor_set[3].dstBinding = 3;
		write_descriptor_set[3].pImageInfo = &emissive_image_info;

		vulkan_rhi->UpdateDescriptorSets(4, write_descriptor_set, 0, nullptr);
	}

	void MainRenderPass::UpdateSkyboxDescriptor()
	{
		std::shared_ptr<VulkanRHI> vulkan_rhi = std::static_pointer_cast<VulkanRHI>(rhi_.lock());
		VkDescriptorImageInfo skybox_image_info = {};
        skybox_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        skybox_image_info.imageView = skybox_render_data_->skybox_env_texture.image_view;
        skybox_image_info.sampler = skybox_render_data_->skybox_env_texture.image_sampler;

		VkWriteDescriptorSet descriptor_writes[1] = {};
        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = render_descriptors_[DescriptorLayoutType::Skybox].descritptor_set_;
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pImageInfo = &skybox_image_info;

		vulkan_rhi->UpdateDescriptorSets(1, descriptor_writes, 0, nullptr);
	}
}
