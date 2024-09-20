#pragma once

#include <memory>
#include <vector>

#include "model.h"
#include "device.h"
#include "pipeline.h"

namespace yib {
	class RenderSystem {
	public:
		RenderSystem(
			Device& device,
			uint32_t width,
			uint32_t height,
			VkRenderPass render_pass
		);

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;

		void RenderModels(VkCommandBuffer command_buffer, std::vector<std::shared_ptr<Model>> models);

		bool success;
	private:
		Device& device;
		VkRenderPass render_pass;
		std::shared_ptr<Pipeline> pipeline;
	};
}