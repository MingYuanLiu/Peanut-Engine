#include "main_render_pass.h"
#include "functions/assets/mesh.h"
#include "functions/render/shader_manager.h"

namespace peanut
{
	void MainRenderPass::Initialize(PassInitInfo* init_info)
	{
		rhi_ = init_info->rhi_;
	}

	void MainRenderPass::Render()
	{
		// update descriptor

	}

	void MainRenderPass::RenderMesh(const std::shared_ptr<RenderData>& render_data, RenderDataType render_type)
	{
		auto rhi = rhi_.lock();
		assert(rhi.get() != nullptr);

		std::shared_ptr<StaticMeshRenderData> static_mesh_render_data = std::static_pointer_cast<StaticMeshRenderData>(render_data);

		VkCommandBuffer command_buffer = rhi->GetCommandBuffer();

		// todo: bind pipeline

		// bind vertex bufer and index buffer
		VkBuffer vertex_buffer[] = { static_mesh_render_data->vertex_buffer.resource };
		const VkDeviceSize vertex_buffer_offset = { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffer, vertex_buffer_offset);
		vkCmdBindIndexBuffer(command_buffer, static_mesh_render_data->vertex_buffer.resource, 0, VK_INDEX_TYPE_UINT32);

		uint32_t submesh_counts = static_mesh_render_data->index_counts.size();
		for (uint32_t i = 0; i < submesh_counts; ++i)
		{
			const PbrMaterial& material = static_mesh_render_data->pbr_materials[i];

			// allocate descriptor set
			VkDescriptorSet mesh_descriptor_set = CreateGbufferDescriptor();
			// update descriptor set
			UpdateGbufferDescriptor(command_buffer, material, mesh_descriptor_set);

			// bind descriptor set
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				render_pipelines_[RenderPipelineType::MeshGbuffer].pipeline_layout_,
				0, 1, &mesh_descriptor_set, 0, nullptr);

			// command draw
			vkCmdDrawIndexed(command_buffer, static_mesh_render_data->index_counts[i], 1, static_mesh_render_data->index_offsets[i], 0, 0);
		}
		
	}

	void MainRenderPass::RenderDeferredLighting()
	{

	}

	void MainRenderPass::CreateRenderTargets()
	{
		std::vector<RenderPassAttachment>& render_attachments = render_target_->attchments_;
		render_attachments.resize(AttachmentType::AttachmentTypeCount);

		std::shared_ptr<RHI> vulkan_rhi = rhi_.lock();
		int frame_width = static_cast<VulkanRHI*>(vulkan_rhi.get())->GetDisplayWidth();
		int frame_height = static_cast<VulkanRHI*>(vulkan_rhi.get())->GetDisplayHeight();

		render_attachments[AttachmentType::GBufferA_Normal].format_ = VK_FORMAT_R8G8B8A8_UNORM;
		render_attachments[AttachmentType::GBufferB_Metallic_Roughness_Occlusion].format_ = VK_FORMAT_R8G8B8A8_UNORM;
		render_attachments[AttachmentType::GBufferC_BaseColor].format_ = VK_FORMAT_R8G8B8A8_UNORM;
		render_attachments[AttachmentType::DepthImage].format_ = static_cast<VulkanRHI*>(vulkan_rhi.get())->GetDepthImageFormat();
		render_attachments[AttachmentType::BackupBufferOdd].format_ = VK_FORMAT_R16G16B16A16_SFLOAT;
		render_attachments[AttachmentType::BackupBufferEven].format_ = VK_FORMAT_R16G16B16A16_SFLOAT;

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
				render_attachments[AttachmentType::GBufferA_Normal].image_ = image;
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

	void MainRenderPass::CreateRenderPass()
	{
		std::vector<RenderPassAttachment>& render_attachments = render_target_->attchments_;

		std::array<VkAttachmentDescription, AttachmentType::AttachmentTypeCount> attachments{};
		std::array<VkAttachmentReference, AttachmentType::AttachmentTypeCount> color_attachment_refs{};
		std::array<VkAttachmentReference, AttachmentType::AttachmentTypeCount> input_attachment_refs{};

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

		attachments[AttachmentType::BackupBufferOdd] =
		{
			0,
			render_attachments[AttachmentType::BackupBufferOdd].format_,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		attachments[AttachmentType::BackupBufferEven] =
		{
			0,
			render_attachments[AttachmentType::BackupBufferEven].format_,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		// create swapchain image description
		auto rhi = rhi_.lock();

		VulkanRHI* vulkan_rhi = reinterpret_cast<VulkanRHI*>(rhi.get());
		VkAttachmentDescription swapchain_image_attchment_desc =
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
		std::array<VkSubpassDescription, SubpassType::SubpassTypeCount> subpasses_desc{};

		// create base subpass (gbuffer pass) decription
		std::array<VkAttachmentReference, 3> base_pass_color_attchment_refs{};
		base_pass_color_attchment_refs[0].attachment = AttachmentType::GBufferA_Normal;
		base_pass_color_attchment_refs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		base_pass_color_attchment_refs[1].attachment = AttachmentType::GBufferB_Metallic_Roughness_Occlusion;
		base_pass_color_attchment_refs[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		base_pass_color_attchment_refs[2].attachment = AttachmentType::GBufferC_BaseColor;
		base_pass_color_attchment_refs[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference base_pass_depth_attchment_refs{};
		base_pass_depth_attchment_refs.attachment = AttachmentType::DepthImage;
		base_pass_depth_attchment_refs.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpasses_desc[SubpassType::BasePass] =
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0,		 // input attchment counts
			nullptr, // pointer of input attchment
			3,		// color attchment counts
			base_pass_color_attchment_refs.data(),
			nullptr, // pointer of resolve attchment
			&base_pass_depth_attchment_refs,
			0,		 // reserve attchment counts
			nullptr // pointer of reserve attchment
		};

		// create deferred lighting subpass decription
		std::array<VkAttachmentReference, 4> deferred_lighting_input_attchment_refs{};
		deferred_lighting_input_attchment_refs[0].attachment = AttachmentType::GBufferA_Normal;
		deferred_lighting_input_attchment_refs[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		deferred_lighting_input_attchment_refs[1].attachment = AttachmentType::GBufferB_Metallic_Roughness_Occlusion;
		deferred_lighting_input_attchment_refs[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		deferred_lighting_input_attchment_refs[2].attachment = AttachmentType::GBufferC_BaseColor;
		deferred_lighting_input_attchment_refs[2].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		deferred_lighting_input_attchment_refs[3].attachment = AttachmentType::DepthImage;
		deferred_lighting_input_attchment_refs[3].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference deferred_lighting_color_attchment_refs;
		deferred_lighting_color_attchment_refs.attachment = AttachmentType::BackupBufferOdd;
		deferred_lighting_color_attchment_refs.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpasses_desc[SubpassType::DeferredLightingPass] =
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			4,		 // input attchment counts
			deferred_lighting_input_attchment_refs.data(), // pointer of input attchment
			1,		// color attchment counts
			&deferred_lighting_color_attchment_refs,
			nullptr, // resolve attchment
			nullptr,  // depth attchment
			0,		 // reserve attchment counts
			nullptr // reserve attchment
		};

		// create forward lighting subpass description
		VkAttachmentReference forward_lighting_color_attchment_refs;
		forward_lighting_color_attchment_refs.attachment = AttachmentType::BackupBufferOdd;
		forward_lighting_color_attchment_refs.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference forward_lighting_depth_attchment_refs;
		forward_lighting_depth_attchment_refs.attachment = AttachmentType::DepthImage;
		forward_lighting_depth_attchment_refs.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpasses_desc[SubpassType::ForwardLightingPass] =
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0,		 // input attchment counts
			nullptr, // input attchment
			1,		// color attchment counts
			&forward_lighting_color_attchment_refs,
			nullptr, // resolve attchment
			&forward_lighting_depth_attchment_refs,  // depth attchment
			0,		 // reserve attchment counts
			nullptr // reserve attchment
		};

		// create tone mapping subpass description
		VkAttachmentReference tone_mapping_input_attchment_refs;
		tone_mapping_input_attchment_refs.attachment = AttachmentType::BackupBufferOdd; // from lighting pass
		tone_mapping_input_attchment_refs.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference tone_mapping_color_attchment_refs;
		tone_mapping_color_attchment_refs.attachment = AttachmentType::BackupBufferEven; // to tone mapping
		tone_mapping_color_attchment_refs.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpasses_desc[SubpassType::ToneMappingPass] =
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			1,		 // input attchment counts
			&tone_mapping_input_attchment_refs, // input attchment
			1,		// color attchment counts
			&tone_mapping_color_attchment_refs,
			nullptr, // resolve attchment
			nullptr,  // depth attchment
			0,		 // reserve attchment counts
			nullptr // reserve attchment
		};

		// create color grading subpass description
		VkAttachmentReference color_grading_input_attchment_refs;
		color_grading_input_attchment_refs.attachment = AttachmentType::BackupBufferEven; // from tone mapping pass
		color_grading_input_attchment_refs.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference color_grading_color_attchment_refs;
		color_grading_color_attchment_refs.attachment = AttachmentType::AttachmentTypeCount; // to swapchain
		color_grading_color_attchment_refs.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpasses_desc[SubpassType::ColorGradingPass] =
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			1,		 // input attchment counts
			&color_grading_input_attchment_refs, // input attchment
			1,		// color attchment counts
			&color_grading_color_attchment_refs,
			nullptr, // resolve attchment
			nullptr,  // depth attchment
			0,		 // reserve attchment counts
			nullptr // reserve attchment
		};

		// create subpass dependency
		std::array<VkSubpassDependency, SubpassType::SubpassTypeCount> subpass_dependencies{};
		// fixme: need it?
		subpass_dependencies[0] =
		{
			VK_SUBPASS_EXTERNAL,
			SubpassType::DeferredLightingPass,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
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
			SubpassType::ToneMappingPass,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		};

		subpass_dependencies[4] =
		{
			SubpassType::ToneMappingPass,
			SubpassType::ColorGradingPass,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		};

		// create render pass
		VkRenderPassCreateInfo render_pass_ci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
		render_pass_ci.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
		render_pass_ci.pAttachments = attachments.data();
		render_pass_ci.subpassCount = (sizeof(subpasses_desc) / sizeof(subpasses_desc[0]));
		render_pass_ci.pSubpasses = subpasses_desc.data();
		render_pass_ci.dependencyCount = (sizeof(subpass_dependencies) / sizeof(subpass_dependencies[0]));
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
			{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
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
			{8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // point light shadow
			{9, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // light uniform buffer
		};

		render_descriptors_[DescriptorLayoutType::DeferredLighting].descriptor_set_layout_ =
			rhi->CreateDescriptorSetLayout(deferred_lighting_descriptor_layout_binding);

		// forward lighting layout used by transparency objects
		std::vector<VkDescriptorSetLayoutBinding> forward_lighting_descriptor_layout_binding =
		{
			{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // depth image
			{4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // irradiance
			{5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // prefiltered
			{6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // brdf lut
			{7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // directonal light shadow
			{8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // point light shadow
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
	}

	void MainRenderPass::CreateDescriptorSets()
	{
		CreateDeferredLightDescriptor();
		CreateForwardLightDescriptor();
		CreateSkyboxDescriptor();
	}

	VkDescriptorSet MainRenderPass::CreateGbufferDescriptor()
	{
		auto rhi = rhi_.lock();

		assert(rhi.get() != nullptr);

		return rhi->AllocateDescriptor(render_descriptors_[DescriptorLayoutType::MeshGbuffer].descriptor_set_layout_);
	}

	void MainRenderPass::CreateDeferredLightDescriptor()
	{
		auto rhi = rhi_.lock();

		assert(rhi.get() != nullptr);

		render_descriptors_[DescriptorLayoutType::DeferredLighting].descritptor_set_ =
			rhi->AllocateDescriptor(render_descriptors_[DescriptorLayoutType::DeferredLighting].descriptor_set_layout_);
	}

	void MainRenderPass::CreateForwardLightDescriptor()
	{
		auto rhi = rhi_.lock();

		assert(rhi.get() != nullptr);

		render_descriptors_[DescriptorLayoutType::ForwardLighting].descritptor_set_ =
			rhi->AllocateDescriptor(render_descriptors_[DescriptorLayoutType::ForwardLighting].descriptor_set_layout_);
	}

	void MainRenderPass::CreateSkyboxDescriptor()
	{
		auto rhi = rhi_.lock();

		assert(rhi.get() != nullptr);

		render_descriptors_[DescriptorLayoutType::Skybox].descritptor_set_ =
			rhi->AllocateDescriptor(render_descriptors_[DescriptorLayoutType::Skybox].descriptor_set_layout_);
	}

	void MainRenderPass::UpdateDeferredLightDescriptor()
	{
		auto rhi = rhi_.lock();

		assert(rhi.get() != nullptr);
		VulkanRHI* vulkan_rhi = reinterpret_cast<VulkanRHI*>(rhi.get());
		
		VkDescriptorImageInfo gbuffer_normal_image_info = {};
		gbuffer_normal_image_info.sampler = vulkan_rhi->GetOrCreateSampler(VulkanRHI::Nearest);
		gbuffer_normal_image_info.imageView = render_target_->attchments_[AttachmentType::GBufferA_Normal].image_view_;
		gbuffer_normal_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo gbuffer_metallic_roughtness_occlusion_image_info = {};
		gbuffer_metallic_roughtness_occlusion_image_info.sampler = vulkan_rhi->GetOrCreateSampler(VulkanRHI::Nearest);
		gbuffer_metallic_roughtness_occlusion_image_info.imageView = render_target_->attchments_[AttachmentType::GBufferB_Metallic_Roughness_Occlusion].image_view_;
		gbuffer_metallic_roughtness_occlusion_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo gbuffer_basecolor_image_info = {};
		gbuffer_basecolor_image_info.sampler = vulkan_rhi->GetOrCreateSampler(VulkanRHI::Nearest);
		gbuffer_basecolor_image_info.imageView = render_target_->attchments_[AttachmentType::GBufferC_BaseColor].image_view_;
		gbuffer_basecolor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo depth_image_info = {};
		depth_image_info.sampler = vulkan_rhi->GetOrCreateSampler(VulkanRHI::Nearest);
		depth_image_info.imageView = render_target_->attchments_[AttachmentType::DepthImage].image_view_;
		depth_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::array<VkWriteDescriptorSet, 4> descriptor_writes = {};

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

		vulkan_rhi->UpdateDescriptorSets(sizeof(descriptor_writes) / sizeof(VkWriteDescriptorSet),
											descriptor_writes.data(), 0, nullptr);
	}

	void MainRenderPass::UpdateForwardLightDescriptor()
	{
		// same as deffred lighting
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
}