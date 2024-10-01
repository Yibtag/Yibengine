#include "renderer.h"

#include <array>

namespace yib {
	Renderer::Renderer(
		const std::string name,
		uint32_t width,
		uint32_t height,
		Window& window,
		Device& device
	) :
		window(window),
		device(device),
		swap_chain(std::make_unique<SwapChain>(
			device,
			VkExtent2D(
				window.GetWidth(),
				window.GetHeight()
			)
		)),
		image_index(0),
		frame_index(0),
		frame_began(false),
		success(false)
	{
		if (!this->swap_chain->success) {
			return;
		}

		if (!CreateCommandBuffers()) {
			return;
		}

		this->success = true;
	}

	Renderer::~Renderer() {
		DestroyCommandBuffers();

		vkDeviceWaitIdle(this->device.GetDevice());
	}


	bool Renderer::HasFrameBegan() const {
		return this->frame_began;
	}

	VkExtent2D Renderer::GetExtent() const {
		return this->swap_chain->GetExtent();
	}

	VkRenderPass Renderer::GetRenderPass() const {
		return this->swap_chain->GetRenderPass();
	}

	VkCommandBuffer Renderer::GetCurrentCommandBuffer() {
		if (!this->frame_began) {
			return VK_NULL_HANDLE;
		}

		return this->command_buffers.at(this->frame_index);
	}

	std::optional<uint32_t> Renderer::GetFrameIndex() const {
		if (!this->frame_began) {
			return std::nullopt;
		}

		return this->frame_index;
	}


	std::optional<VkCommandBuffer> Renderer::BeginFrame() {
		if (this->frame_began) {
			return std::nullopt;
		}

		VkResult result = this->swap_chain->GetNextImage(&this->image_index);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			if (!RecreateSwapChain()) {
				return std::nullopt;
			}

			return VK_NULL_HANDLE;
		}
		if (result != VK_SUCCESS) {
			return std::nullopt;
		}

		this->frame_began = true;

		VkCommandBuffer command_buffer = GetCurrentCommandBuffer();
		
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(
			command_buffer,
			&begin_info
		) != VK_SUCCESS) {
			return std::nullopt;
		}

		return command_buffer;
	}

	bool Renderer::EndFrame() {
		if (!this->frame_began) {
			return false;
		}

		VkCommandBuffer command_buffer = GetCurrentCommandBuffer();

		if (vkEndCommandBuffer(
			command_buffer
		) != VK_SUCCESS) {
			return false;
		}

		VkResult result = this->swap_chain->SubmitCommandBuffers(
			&command_buffer,
			&this->image_index
		);
		if (
			result == VK_ERROR_OUT_OF_DATE_KHR ||
			result == VK_SUBOPTIMAL_KHR ||
			this->window.GetResized()
		) {
			if (!RecreateSwapChain()) {
				return false;
			}
			this->window.ResetResized();
			this->frame_began = false;

			return true;
		}

		if (result != VK_SUCCESS) {
			return false;
		}

		this->frame_began = false;
		this->frame_index = (this->frame_index + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;

		return true;
	}


	bool Renderer::BeginRenderPass(VkCommandBuffer command_buffer) {
		if (
			!this->frame_began ||
			command_buffer != GetCurrentCommandBuffer()
		) {
			return false;
		}

		VkRenderPassBeginInfo render_pass_begin_info = {};

		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = this->swap_chain->GetRenderPass();
		render_pass_begin_info.framebuffer = this->swap_chain->GetFrameBuffer(this->image_index);

		render_pass_begin_info.renderArea.offset = { 0, 0 };
		render_pass_begin_info.renderArea.extent = this->swap_chain->GetExtent();

		std::array<VkClearValue, 2> clear_values = {};

		clear_values.at(0).color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clear_values.at(1).depthStencil = { 1.0f, 0 };

		render_pass_begin_info.clearValueCount = clear_values.size();
		render_pass_begin_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(
			command_buffer,
			&render_pass_begin_info,
			VK_SUBPASS_CONTENTS_INLINE
		);

		VkViewport viewport = {};

		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.width = this->swap_chain->GetExtent().width;
		viewport.height = this->swap_chain->GetExtent().height;

		vkCmdSetViewport(
			command_buffer,
			0,
			1,
			&viewport
		);

		VkRect2D scissor = {};

		scissor.offset = { 0, 0 };
		scissor.extent = this->swap_chain->GetExtent();

		vkCmdSetScissor(
			command_buffer,
			0,
			1,
			&scissor
		);

		return true;
	}

	bool Renderer::EndRenderPass(VkCommandBuffer command_buffer) {
		if (
			!this->frame_began ||
			command_buffer != GetCurrentCommandBuffer()
			) {
			return false;
		}

		vkCmdEndRenderPass(command_buffer);

		return true;
	}


	bool Renderer::CreateCommandBuffers() {
		this->command_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocation_info = {};

		allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocation_info.commandPool = this->device.GetCommandPool();
		allocation_info.commandBufferCount = this->command_buffers.size();

		if (vkAllocateCommandBuffers(
			this->device.GetDevice(),
			&allocation_info,
			this->command_buffers.data()
		) != VK_SUCCESS) {
			return false;
		}

		return true;
	}

	void Renderer::DestroyCommandBuffers() {
		vkFreeCommandBuffers(
			this->device.GetDevice(),
			this->device.GetCommandPool(),
			this->command_buffers.size(),
			this->command_buffers.data()
		);
	}

	bool Renderer::RecreateSwapChain() {
		VkExtent2D extent = VkExtent2D(
			this->window.GetWidth(),
			this->window.GetHeight()
		);

		while (extent.width == 0 || extent.height == 0) {
			extent = VkExtent2D(
				this->window.GetWidth(),
				this->window.GetHeight()
			);

			glfwWaitEvents();
		}

		vkDeviceWaitIdle(this->device.GetDevice());

		if (this->swap_chain == VK_NULL_HANDLE) {
			this->swap_chain = std::make_unique<SwapChain>(
				this->device,
				extent
			);

			if (!this->swap_chain->success) {
				return false;
			}
		} else {
			std::shared_ptr<SwapChain> old_swap_chain = std::move(this->swap_chain);

			this->swap_chain = std::make_unique<SwapChain>(
				this->device,
				extent,
				old_swap_chain
			);

			if (!this->swap_chain->success) {
				return false;
			}

			if (!this->swap_chain->CompareSwapChainFormats(*old_swap_chain.get())) {
				return false;
			}
		}

		return true;
	}
}