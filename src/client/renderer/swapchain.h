#pragma once

#include <vector>
#include <memory>

#include <vulkan/vulkan.h>

#include "device.h"

namespace yib {
	class SwapChain {
	public:
		SwapChain(
			Device& device,
			VkExtent2D window_extent
		);
		SwapChain(
			Device& device,
			VkExtent2D window_extent,
			std::shared_ptr<SwapChain> prev
		);
		~SwapChain();

		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;

		VkExtent2D GetExtent() const;
		uint32_t GetImageCount() const;
		VkRenderPass GetRenderPass() const;
		VkSwapchainKHR GetSwapChain() const;
		VkFramebuffer GetFrameBuffer(uint32_t index) const;
		bool CompareSwapChainFormats(const SwapChain& swap_chain) const;

		VkResult GetNextImage(uint32_t* image_index);
		VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* image_index);

		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		bool success;
	private:
		bool Create();

		bool CreateSwapChain();
		void DestorySwapChain();

		bool CreateImageViews();
		void DestoryImageViews();

		bool CreateRenderPass();
		void DestroyRenderPass();

		bool CreateDepthResources();
		void DestroyDepthResources();

		bool CreateFrameBuffer();
		void DestoryFrameBuffer();

		bool CreateSyncObjects();
		void DestroySyncObjects();

		VkFormat ChooseDepthFormat();
		VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avail_formats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& avail_present_modes);

		VkExtent2D extent = {};
		VkFormat image_format = {};
		std::vector<VkImage> images = {};
		std::vector<VkImageView> image_views = {};
		std::vector<VkFramebuffer> frame_buffers = {};

		VkFormat depth_format = {};
		std::vector<VkImage> depth_images = {};
		std::vector<VkImageView> depth_image_views = {};
		std::vector<VkDeviceMemory> depth_image_memorys = {};

		size_t current_frame = 0;
		std::vector<VkFence> in_flight_fences = {};
		std::vector<VkFence> images_in_flight = {};
		std::vector<VkSemaphore> render_finished_semaphores = {};
		std::vector<VkSemaphore> image_available_semaphores = {};

		Device& device;
		std::shared_ptr<SwapChain> prev_swapchain;
		VkRenderPass render_pass = VK_NULL_HANDLE;
		VkSwapchainKHR swap_chain = VK_NULL_HANDLE;
	};
};