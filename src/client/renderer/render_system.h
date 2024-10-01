#pragma once

#include <memory>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "model.h"
#include "camera.h"
#include "device.h"
#include "pipeline.h"
#include "../object.h"

namespace yib {
	class RenderSystem {
	public:
		struct PushConstant {
			glm::mat4 transform{ 1.0f };
		};

		RenderSystem(
			Device& device,
			uint32_t width,
			uint32_t height,
			VkRenderPass render_pass
		);

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;

		void RenderModels(
			VkCommandBuffer command_buffer,
			std::vector<std::shared_ptr<Object>> objects,
			const Camera& camera
		);

		bool success;
	private:
		PipelineConfig CreatePipelineConfig() const;

		Device& device;
		VkRenderPass render_pass;
		std::shared_ptr<Pipeline> pipeline;
	};
}