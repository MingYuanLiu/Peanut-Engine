#include "main_render_pass.h"

namespace peanut
{
	void MainRenderPass::Initialize(PassInitInfo* init_info)
	{
		rhi_ = static_cast<MainPassInitInfo*>(init_info)->rhi_;
	}


	void MainRenderPass::CreateRenderTargets()
	{
		std::vector<RenderPassAttachment>& render_attachments = render_target_->attchments_;
		render_attachments.resize(AttachmentTypeCount);

		std::shared_ptr<RHI> vulkan_rhi = rhi_.lock();
		int frame_width = static_cast<VulkanRHI*>(vulkan_rhi.get())->GetDisplayWidth();
		int frame_height = static_cast<VulkanRHI*>(vulkan_rhi.get())->GetDisplayHeight();

		render_attachments[GBufferA_Normal].format_ = VK_FORMAT_R8G8B8A8_UNORM;
		render_attachments[GBufferB_Metallic_Occlusion_Roughness].format_ = VK_FORMAT_R8G8B8A8_UNORM;
		render_attachments[GBufferC_BaseColor].format_ = VK_FORMAT_R8G8B8A8_SRGB;
		render_attachments[DepthImage].format_ = static_cast<VulkanRHI*>(vulkan_rhi.get())->GetDepthImageFormat();
		render_attachments[BackupBufferOdd].format_ = VK_FORMAT_R16G16B16A16_SFLOAT;
		render_attachments[BackupBufferEven].format_ = VK_FORMAT_R16G16B16A16_SFLOAT;

		for (uint32_t i = 0; i < AttachmentTypeCount; i++)
		{
			// create image
			Resource<VkImage> image;
			if (i == GBufferA_Normal)
			{
				image = vulkan_rhi->CreateImage(frame_width, frame_height, 0, 1, 1, render_attachments[GBufferA_Normal].format_,
					VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
				render_attachments[GBufferA_Normal].image_ = image;
			}
			else if (i == DepthImage)
			{
				image = vulkan_rhi->CreateImage(frame_width, frame_height, 0, 1, 1, render_attachments[GBufferA_Normal].format_,
					VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
				render_attachments[GBufferA_Normal].image_ = image;
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

		std::array<VkAttachmentDescription, AttachmentTypeCount> attachments{};
		std::array<VkAttachmentReference, AttachmentTypeCount> color_attachment_refs{};
		std::array<VkAttachmentReference, AttachmentTypeCount> input_attachment_refs{};

		// setup attachment description
		attachments[GBufferA_Normal] =
		{
			0, // flag
			render_attachments[GBufferA_Normal].format_,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR, // load op
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencil load op
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		attachments[GBufferB_Metallic_Occlusion_Roughness] =
		{
			0,
			render_attachments[GBufferB_Metallic_Occlusion_Roughness].format_,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		attachments[GBufferC_BaseColor] =
		{
			0,
			render_attachments[GBufferC_BaseColor].format_,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		attachments[DepthImage] =
		{
			0,
			render_attachments[DepthImage].format_,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		attachments[BackupBufferOdd] =
		{
			0,
			render_attachments[BackupBufferOdd].format_,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		attachments[BackupBufferEven] =
		{
			0,
			render_attachments[BackupBufferEven].format_,
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
		std::array<VkSubpassDescription, SubpassTypeCount> subpasses_desc{};

		// create base subpass (gbuffer pass) decription
		std::array<VkAttachmentReference, 3> base_pass_color_attchment_refs{};
		base_pass_color_attchment_refs[0].attachment = GBufferA_Normal;
		base_pass_color_attchment_refs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		base_pass_color_attchment_refs[1].attachment = GBufferB_Metallic_Occlusion_Roughness;
		base_pass_color_attchment_refs[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		base_pass_color_attchment_refs[2].attachment = GBufferC_BaseColor;
		base_pass_color_attchment_refs[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference base_pass_depth_attchment_refs{};
		base_pass_depth_attchment_refs.attachment = DepthImage;
		base_pass_depth_attchment_refs.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpasses_desc[BasePass] =
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
		deferred_lighting_input_attchment_refs[0].attachment = GBufferA_Normal;
		deferred_lighting_input_attchment_refs[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		deferred_lighting_input_attchment_refs[1].attachment = GBufferB_Metallic_Occlusion_Roughness;
		deferred_lighting_input_attchment_refs[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		deferred_lighting_input_attchment_refs[2].attachment = GBufferC_BaseColor;
		deferred_lighting_input_attchment_refs[2].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		deferred_lighting_input_attchment_refs[3].attachment = DepthImage;
		deferred_lighting_input_attchment_refs[3].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference deferred_lighting_color_attchment_refs;
		deferred_lighting_color_attchment_refs.attachment = BackupBufferOdd;
		deferred_lighting_color_attchment_refs.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpasses_desc[DeferredLightingPass] =
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
		forward_lighting_color_attchment_refs.attachment = BackupBufferOdd;
		forward_lighting_color_attchment_refs.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference forward_lighting_depth_attchment_refs;
		forward_lighting_depth_attchment_refs.attachment = DepthImage;
		forward_lighting_depth_attchment_refs.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpasses_desc[ForwardLightingPass] =
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
		tone_mapping_input_attchment_refs.attachment = BackupBufferOdd; // from lighting pass
		tone_mapping_input_attchment_refs.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference tone_mapping_color_attchment_refs;
		tone_mapping_color_attchment_refs.attachment = BackupBufferEven; // to color grading
		tone_mapping_color_attchment_refs.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpasses_desc[ToneMappingPass] =
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
		color_grading_input_attchment_refs.attachment = BackupBufferEven; // from tone mapping pass
		color_grading_input_attchment_refs.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference color_grading_color_attchment_refs;
		color_grading_color_attchment_refs.attachment = BackupBufferOdd; // todo: to swapchain
		color_grading_color_attchment_refs.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpasses_desc[ColorGradingPass] =
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
		std::array<VkSubpassDependency, SubpassTypeCount> subpass_dependencies{};
		subpass_dependencies[0] =
		{
			VK_SUBPASS_EXTERNAL,
			DeferredLighting,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			0
		};

		subpass_dependencies[1] =
		{
			BasePass,
			DeferredLighting,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		};

		subpass_dependencies[2] =
		{
			DeferredLighting,
			ForwardLightingPass,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		};

		subpass_dependencies[3] =
		{
			ForwardLightingPass,
			ToneMappingPass,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		};

		subpass_dependencies[4] =
		{
			ToneMappingPass,
			ColorGradingPass,
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
		render_descriptors_.resize(DescriptorLayoutTypeCount);

		auto rhi = rhi_.lock();
		if (!rhi)
		{
			PEANUT_LOG_ERROR("RHI is nullptr when create descriptorset layout");
			return;
		}

		// gbuffer descriptor layout
		std::vector<VkDescriptorSetLayoutBinding> gbuffer_descriptor_layout_binding =
		{
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
			{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
			{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
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
			{6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // irradiance
			{7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // prefiltered
			{8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // brdf lut
			{9, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // directonal light shadow
			{10, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // point light shadow
			{12, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // light uniform buffer
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
		std::vector<VkPushConstantRange> push_constant_range;
		std::vector<VkDescriptorSetLayout> set_layouts;
		set_layouts.push_back(render_descriptors_[DescriptorLayoutType::MeshGbuffer].descriptor_set_layout_);
		render_pipelines_[RenderPipelineType::MeshGbuffer].pipeline_layout_ =
			rhi->CreatePipelineLayout(set_layouts, push_constant_range);

		set_layouts.clear();

		// deferred lighting pipeline layout
		set_layouts.push_back(render_descriptors_[DescriptorLayoutType::DeferredLighting].descriptor_set_layout_);
		render_pipelines_[RenderPipelineType::DeferredLighting].pipeline_layout_ =
			rhi->CreatePipelineLayout(set_layouts, push_constant_range);

		set_layouts.clear();

		// forward lighting pipeline layout
		set_layouts.push_back(render_descriptors_[DescriptorLayoutType::ForwardLighting].descriptor_set_layout_);
		render_pipelines_[RenderPipelineType::ForwardLighting].pipeline_layout_ =
			rhi->CreatePipelineLayout(set_layouts, push_constant_range);

		// skybox pipeline layout
		set_layouts.push_back(render_descriptors_[DescriptorLayoutType::Skybox].descriptor_set_layout_);
		render_pipelines_[RenderPipelineType::Skybox].pipeline_layout_ =
			rhi->CreatePipelineLayout(set_layouts, push_constant_range);
	}

	void MainRenderPass::CreateDescriptorSets()
	{

	}
}