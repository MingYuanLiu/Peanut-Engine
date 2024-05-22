#include "main_render_pass.h"

namespace peanut
{
	void MainRenderPass::Initialize()
	{

	}


	void MainRenderPass::CreateRenderTargets()
	{
		std::vector<RenderPassAttachment>& render_attachments = render_target_->attchments_;
		render_attachments.resize(AttachmentTypeCount);

		render_attachments[GBufferA_Normal].format_ = VK_FORMAT_R16G16B16A16_SFLOAT;
		render_attachments[GBufferB_Metallic_Occlusion_Roughness].format_ = VK_FORMAT_R8G8B8A8_UNORM;
		render_attachments[GBufferC_BaseColor].format_ = VK_FORMAT_R8G8B8A8_SRGB;
		render_attachments[EmissiveColor].format_ = VK_FORMAT_R8G8B8A8_UNORM;
		render_attachments[DethImage].format_ = VK_FORMAT_R8G8B8A8_UNORM;
	}

	void MainRenderPass::CreateRenderPass()
	{
		
	}

	void MainRenderPass::CreateDescriptorSetLayouts()
	{

	}

	void MainRenderPass::CreateDescriptorSets()
	{

	}
}