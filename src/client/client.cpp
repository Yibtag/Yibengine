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

		// TODO: Remove this
		if (!CreateObjects()) {
			return;
		}

		if (!CreateDescriptorPool()) {
			return;
		}

		this->success = true;
	}

	void Client::Run() {
		Texture texture = Texture(
			this->device,
			"icon.png"
		);
		if (!texture.success) {
			this->running = false;
			return;
		}

		VkDescriptorImageInfo texture_info = texture.GetDescriptorInfo();
		
		Camera camera = Camera();
		camera.SetViewYXZ(glm::vec3(), glm::vec3(0.0f, 0.0f, 0.0f));

		std::vector<std::unique_ptr<Buffer>> uniform_buffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uniform_buffers.size(); i++) {
			uniform_buffers.at(i) = std::make_unique<Buffer>(
				this->device,
				sizeof(GlobalUBO),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				this->device.GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment
			);

			if (!uniform_buffers.at(i)->Map()) {
				this->running = false;
				return;
			}
		}

		DescriptorSetLayout::Builder set_layout_builder = DescriptorSetLayout::Builder(this->device);

		set_layout_builder.AddBinding(
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT
		);
		set_layout_builder.AddBinding(
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT
		);

		std::unique_ptr<DescriptorSetLayout> set_layout = set_layout_builder.Build();

		std::vector<VkDescriptorSet> descriptor_sets(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < descriptor_sets.size(); i++) {
			VkDescriptorBufferInfo buffer_info = uniform_buffers.at(i)->DescriptorInfo();

			DescriptorWriter writer = DescriptorWriter(
				*set_layout,
				*this->descriptor_pool
			);
			
			if (!writer.WriteBuffer(0, &buffer_info)) {
				this->running = false;
				return;
			}

			if (!writer.WriteImage(1, &texture_info)) {
				this->running = false;
				return;
			}

			if (!writer.Build(descriptor_sets.at(i))) {
				this->running = false;
				return;
			}
		}

		RenderSystem render_system = RenderSystem(
			this->device,
			this->width,
			this->height,
			this->renderer.GetRenderPass(),
			set_layout->GetDescriptorSetLayout()
		);
		if (!render_system.success) {
			this->running = false;
			return;
		}

		while (this->window.Running() && this->running) {
			this->window.Run();

			VkExtent2D extent = this->renderer.GetExtent();
			float aspect_ratio = extent.width / extent.height;

			camera.SetPerspectiveProjection(glm::radians(50.0f), aspect_ratio, 0.1f, 10.0f);

			std::optional<VkCommandBuffer> command_buffer = this->renderer.BeginFrame();
			if (!command_buffer.has_value()) {
				this->running = false;
				break;
			}

			if (command_buffer.value() == VK_NULL_HANDLE) {
				continue;
			}

			std::optional<uint32_t> frame_index = this->renderer.GetFrameIndex();
			if (!frame_index.has_value()) {
				this->running = false;
				break;
			}

			// Update
			GlobalUBO UBO = GlobalUBO();
			UBO.projection_view_matrix = camera.GetProjectionMatrix() * camera.GetViewMatrix();

			uniform_buffers.at(frame_index.value())->Write(&UBO);
			uniform_buffers.at(frame_index.value())->Flush();

			if (!this->renderer.BeginRenderPass(command_buffer.value())) {
				this->running = false;
				break;
			}
			
			render_system.RenderModels(
				command_buffer.value(),
				objects,
				camera,
				descriptor_sets.at(frame_index.value())
			);

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
	bool Client::CreateObjects() {
		std::optional<ModelData> data = ModelData::LoadModel("suzanne.obj");
		if (!data.has_value()) {
			return false;
		}

		std::shared_ptr<Model> model = std::make_shared<Model>(this->device, data.value());
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

	bool Client::CreateDescriptorPool() {
		DescriptorPool::Builder builder = DescriptorPool::Builder(this->device);

		builder.SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
		builder.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT);
		builder.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT);

		this->descriptor_pool = builder.Build();
		if (!this->descriptor_pool->success) {
			return false;
		}

		return true;
	}
}
