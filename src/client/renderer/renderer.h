#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

#include "model.h"
#include "window.h"
#include "device.h"
#include "pipeline.h"
#include "swapchain.h"

namespace yib {
	class Renderer {
	public:
		Renderer(
			const std::string name,
			uint32_t width,
			uint32_t height,
			Window& window,
			Device& device
		);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		bool HasFrameBegan() const;
		VkRenderPass GetRenderPass() const;
		VkCommandBuffer GetCurrentCommandBuffer();
		std::optional<uint32_t> GetFrameIndex() const;

		std::optional<VkCommandBuffer> BeginFrame();
		bool EndFrame();

		bool BeginRenderPass(VkCommandBuffer command_buffer);
		bool EndRenderPass(VkCommandBuffer command_buffer);

		bool success;
	private:
		bool CreateCommandBuffers();
		void DestroyCommandBuffers();
	
		bool RecreateSwapChain();

		uint32_t image_index;
		uint32_t frame_index;
		bool frame_began;

		Window& window;
		Device& device;
		std::unique_ptr<SwapChain> swap_chain;
		std::vector<VkCommandBuffer> command_buffers;
	};
}