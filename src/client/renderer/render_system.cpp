#include "render_system.h"

namespace yib {
	RenderSystem::RenderSystem(
		Device& device,
		uint32_t width,
		uint32_t height,
		VkRenderPass render_pass
	) :
	device(device),
	render_pass(render_pass),
	pipeline(std::make_unique<Pipeline>(
		device,
		width,
		height,
		"D:/documents/projects/Yibengine/src/client/shaders/simple.vert.spv",
		"D:/documents/projects/Yibengine/src/client/shaders/simple.frag.spv",
		Pipeline::CreateDefaultConfig(render_pass)
	)),
	success(false)
	{
		if (!this->pipeline->success) {
			return;
		}

		this->success = true;
	}

	void RenderSystem::RenderModels(VkCommandBuffer command_buffer, std::vector<std::shared_ptr<Model>> models) {
		this->pipeline->BindCommandBuffer(command_buffer);

		for (std::shared_ptr<Model> model : models) {
			model->Bind(command_buffer);
			model->Draw(command_buffer);
		}
	}
}