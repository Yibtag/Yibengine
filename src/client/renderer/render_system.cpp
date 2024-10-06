#include "render_system.h"

namespace yib {
	RenderSystem::RenderSystem(
		Device& device,
		uint32_t width,
		uint32_t height,
		VkRenderPass render_pass,
		VkDescriptorSetLayout set_layout
	) :
	device(device),
	render_pass(render_pass),
	pipeline(std::make_unique<Pipeline>(
		device,
		width,
		height,
		"D:/documents/projects/Yibengine/src/client/shaders/simple.vert.spv",
		"D:/documents/projects/Yibengine/src/client/shaders/simple.frag.spv",
		CreatePipelineConfig(set_layout)
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
		const Camera& camera,
		VkDescriptorSet descriptor_set
	) {
		this->pipeline->BindCommandBuffer(command_buffer);

		vkCmdBindDescriptorSets(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->pipeline->GetPipelineLayout(),
			0,
			1,
			&descriptor_set,
			0,
			nullptr
		);

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
			push_constant.model_matrix = object->transform.GetMatrix();

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


	PipelineConfig RenderSystem::CreatePipelineConfig(VkDescriptorSetLayout set_layout) const {
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

		config.set_layouts = {
			set_layout
		};

		config.pipeline_layout_info.setLayoutCount = config.set_layouts.size();
		config.pipeline_layout_info.pSetLayouts = config.set_layouts.data();

		return config;
	}
}