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

		// TODO: Remove this
		if (!CreateObjects()) {
			return;
		}

		this->success = true;
	}

	void Client::Run() {
		Camera camera = Camera();
		camera.SetViewYXZ(glm::vec3(), glm::vec3(0.0f, 0.4f, 0.0f));

		std::chrono::steady_clock::time_point current_time = std::chrono::high_resolution_clock::now();

		while (this->window.Running() && this->running) {
			this->window.Run();

			std::chrono::steady_clock::time_point new_time = std::chrono::high_resolution_clock::now();
			float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
			// frame_time = glm::min(frame_time, MAX_FRAME_TIME);
			current_time = new_time;

			VkExtent2D extent = this->renderer.GetExtent();
			float aspect_ratio = extent.width / extent.height;

			// camera.SetOrthographicProjection(-aspect_ratio, aspect_ratio, -1, 1, -1, 1);
			camera.SetPerspectiveProjection(glm::radians(50.0f), aspect_ratio, 0.1f, 10.0f);

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
			
			render_system.RenderModels(command_buffer.value(), objects, camera);

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
	static std::unique_ptr<Model> CreateCubeModel(Device& device, glm::vec3 offset) {
		std::vector<Vertex> vertices{

			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

		};

		for (auto& v : vertices) {
			v.position += offset;
		}

		return std::make_unique<Model>(device, vertices);
	}


	// TODO: Remove this
	bool Client::CreateObjects() {
		std::shared_ptr<Model> model = CreateCubeModel(this->device, {});
		if (!model->success) {
			return false;
		}

		std::shared_ptr<Object> object = std::make_unique<Object>();

		object->model = model;

		object->transform = {};
		object->transform.scale = glm::vec3(0.5f, 0.5f, 0.5f);
		object->transform.translation = glm::vec3(0.0f, 0.0f, 2.5f);

		this->objects = {};
		this->objects.push_back(std::move(object));

		return true;
	}
}