#include "client.h"

namespace yib {
	Client::Client(
		const std::string name,
		const uint32_t width,
		const uint32_t height
	) :
		name(name),
		width(width),
		height(height),
		window(
			name,
			width,
			height
		),
		device(
			name,
			&window
		),
		renderer(
			name,
			width,
			height,
			window,
			device
		),
		render_system(
			device,
			width,
			height,
			renderer.GetRenderPass()
		),
		running(true),
		success(false)
	{
		if (!this->window.success) {
			return;
		}

		if (!this->device.success) {
			return;
		}

		if (!this->renderer.success) {
			return;
		}

		if (!this->render_system.success) {
			return;
		}

		if (!CreateModels()) {
			return;
		}

		this->success = true;
	}

	void Client::Run() {
		while (this->window.Running() && this->running) {
			this->window.Run();

			std::optional<VkCommandBuffer> command_buffer = this->renderer.BeginFrame();
			if (!command_buffer.has_value()) {
				this->running = false;
				break;
			}

			if (command_buffer.value() == VK_NULL_HANDLE) {
				continue;
			}

			if (!this->renderer.BeginRenderPass(command_buffer.value())) {
				this->running = false;
				break;
			}
			
			render_system.RenderModels(command_buffer.value(), models);

			if (!this->renderer.EndRenderPass(command_buffer.value())) {
				this->running = false;
				break;
			}

			if (!this->renderer.EndFrame()) {
				this->running = false;
				break;
			}
		}
	}

	bool Client::Running() const {
		return this->running;
	}


	// TODO: Remove this
	bool Client::CreateModels() {
		std::vector<Vertex> vertecies = {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		std::shared_ptr<Model> model = std::make_shared<Model>(
			this->device,
			vertecies
		);
		if (!model->success) {
			return false;
		}

		this->models = {};
		this->models.push_back(std::move(model));

		return true;
	}
}