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
		CreatePipelineConfig()
	)),
	success(false)
	{
		if (!this->pipeline->success) {
			return;
		}

		this->success = true;
	}


	void RenderSystem::RenderModels(
		VkCommandBuffer command_buffer,
		std::vector<std::shared_ptr<Object>> objects,
		const Camera& camera
	) {
		this->pipeline->BindCommandBuffer(command_buffer);

		// TODO: Move the matrix multi to gpu once uniforms are supported
		glm::mat4 projection_view = camera.GetProjectionMatrix() * camera.GetViewMatrix();

		for (std::shared_ptr<Object> object : objects) {
			object->transform.rotation.y = glm::mod(
				object->transform.rotation.y + 0.001f,
				glm::two_pi<float>()
			);
			object->transform.rotation.x = glm::mod(
				object->transform.rotation.x + 0.0005f,
				glm::two_pi<float>()
			);

			PushConstant push_constant = { };

			// TODO: Move the matrix multi to gpu once uniforms are supported
			push_constant.transform = projection_view * object->transform.GetMatrix();

			vkCmdPushConstants(
				command_buffer,
				this->pipeline->GetPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PushConstant),
				&push_constant
			);

			object->model->Bind(command_buffer);
			object->model->Draw(command_buffer);
		}
	}


	PipelineConfig RenderSystem::CreatePipelineConfig() const {
		PipelineConfig config = Pipeline::CreateDefaultConfig(this->render_pass);

		VkPushConstantRange push_constant_range = {};

		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(PushConstant);

		config.push_constant_ranges = {
			push_constant_range
		};

		config.pipeline_layout_info.pushConstantRangeCount = config.push_constant_ranges.size();
		config.pipeline_layout_info.pPushConstantRanges = config.push_constant_ranges.data();

		return config;
	}
}