#include "main_render_pass.h"

namespace peanut
{
	void MainRenderPass::Initialize(PassInitInfo* init_info)
	{
		rhi_ = init_info->rhi_;
	}


	void MainRenderPass::CreateRenderTargets()
	{
		std::vector<RenderPassAttachment>& render_attachments = render_target_->attchments_;
		render_attachments.resize(AttachmentType::AttachmentTypeCount);

		std::shared_ptr<RHI> vulkan_rhi = rhi_.lock();
		int frame_width = static_cast<VulkanRHI*>(vulkan_rhi.get())->GetDisplayWidth();
		int frame_height = static_cast<VulkanRHI*>(vulkan_rhi.get())->GetDisplayHeight();

		render_attachments[AttachmentType::GBufferA_Normal].format_ = VK_FORMAT_R8G8B8A8_UNORM;
		render_attachments[AttachmentType::GBufferB_Metallic_Occlusion_Roughness].format_ = VK_FORMAT_R8G8B8A8_UNORM;
		render_attachments[AttachmentType::GBufferC_BaseColor].format_ = VK_FORMAT_R8G8B8A8_SRGB;
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

		attachments[AttachmentType::GBufferB_Metallic_Occlusion_Roughness] =
		{
			0,
			render_attachments[AttachmentType::GBufferB_Metallic_Occlusion_Roughness].format_,
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

		// todo: create swapchain image description

		// setup subpass information
		std::array<VkSubpassDescription, SubpassType::SubpassTypeCount> subpasses_desc{};

		// create base subpass (gbuffer pass) decription
		std::array<VkAttachmentReference, 3> base_pass_color_attchment_refs{};
		base_pass_color_attchment_refs[0].attachment = AttachmentType::GBufferA_Normal;
		base_pass_color_attchment_refs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		base_pass_color_attchment_refs[1].attachment = AttachmentType::GBufferB_Metallic_Occlusion_Roughness;
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

		deferred_lighting_input_attchment_refs[1].attachment = AttachmentType::GBufferB_Metallic_Occlusion_Roughness;
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
		tone_mapping_color_attchment_refs.attachment = AttachmentType::BackupBufferEven; // to color grading
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
		color_grading_color_attchment_refs.attachment = AttachmentType::BackupBufferOdd; // todo: to swapchain
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

		auto rhi = rhi_.lock();
		
		if (rhi.get() != nullptr)
		{
			VkRenderPass renderpass;
			rhi->CreateRenderPass(&render_pass_ci, &renderpass);
			render_pass_ = renderpass;
		}

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
			{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
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
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
			{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
			{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
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

	void MainRenderPass::CreateDescriptorSets()
	{
		CreateDeferredLightDescriptor();
		CreateForwardLightDescriptor();
		CreateSkyboxDescriptor();
	}

	void MainRenderPass::CreateDeferredLightDescriptor()
	{
		
	}

	void MainRenderPass::CreateForwardLightDescriptor()
	{
		
	}

	void MainRenderPass::CreateSkyboxDescriptor()
	{
		
	}
}