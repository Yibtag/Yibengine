#include "swapchain.h"

#include <array>
#include <thread>
#include <cassert>

namespace yib {
	SwapChain::SwapChain(
		Device& device,
		VkExtent2D window_extent
	) : 
		device(device),
		prev_swapchain(VK_NULL_HANDLE),
		success(false)
	{
		if (!Create()) {
			return;
		}

		this->success = true;
	}

	SwapChain::SwapChain(
		Device& device,
		VkExtent2D window_extent,
		std::shared_ptr<SwapChain> prev
	) :
		device(device),
		prev_swapchain(prev),
		success(false)
	{
		if (!Create()) {
			return;
		}

		this->success = true;
	}

	SwapChain::~SwapChain() {
		DestroySyncObjects();
		DestoryFrameBuffer();
		DestroyDepthResources();
		DestroyRenderPass();
		DestoryImageViews();
		DestorySwapChain();
	}


	VkExtent2D SwapChain::GetExtent() const {
		return this->extent;
	}

	uint32_t SwapChain::GetImageCount() const {
		return this->images.size();
	}

	VkRenderPass SwapChain::GetRenderPass() const {
		return this->render_pass;
	}

	VkSwapchainKHR SwapChain::GetSwapChain() const {
		return this->swap_chain;
	}

	VkFramebuffer SwapChain::GetFrameBuffer(uint32_t index) const {
		return this->frame_buffers.at(index);
	}

	bool SwapChain::CompareSwapChainFormats(const SwapChain& swap_chain) const {
		return swap_chain.image_format == this->image_format &&
				swap_chain.depth_format == this->depth_format;
	}


	VkResult SwapChain::GetNextImage(uint32_t* image_index) {
		vkWaitForFences(
			this->device.GetDevice(),
			1,
			&this->in_flight_fences.at(this->current_frame),
			VK_TRUE,
			std::numeric_limits<uint64_t>::max()
		);

		VkResult result = vkAcquireNextImageKHR(
			this->device.GetDevice(),
			this->swap_chain,
			std::numeric_limits<uint64_t>::max(),
			this->image_available_semaphores.at(this->current_frame),
			VK_NULL_HANDLE,
			image_index
		);

		return result;
	}

	VkResult SwapChain::SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* image_index) {
		if (this->images_in_flight.at(*image_index) != VK_NULL_HANDLE) {
			vkWaitForFences(
				this->device.GetDevice(),
				1,
				&this->images_in_flight[*image_index],
				VK_TRUE,
				UINT64_MAX
			);
		}
		this->images_in_flight.at(*image_index) = this->in_flight_fences.at(this->current_frame);

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore wait_semaphores[] = {
			this->image_available_semaphores.at(this->current_frame)
		};
		VkPipelineStageFlags wait_stages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = buffers;

		VkSemaphore signal_semaphores[] = {
			this->render_finished_semaphores.at(this->current_frame)
		};

		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		vkResetFences(
			this->device.GetDevice(),
			1,
			&this->in_flight_fences.at(this->current_frame)
		);

		VkResult result = vkQueueSubmit(
			this->device.GetGraphicsQueue(),
			1,
			&submit_info,
			this->in_flight_fences.at(this->current_frame)
		);
		if (result != VK_SUCCESS) {
			return result;
		}

		VkPresentInfoKHR present_info = {};

		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;

		VkSwapchainKHR swap_chains[] = {
			this->swap_chain
		};

		present_info.swapchainCount = 1;
		present_info.pSwapchains = swap_chains;

		present_info.pImageIndices = image_index;

		result = vkQueuePresentKHR(
			this->device.GetPresentQueue(),
			&present_info
		);

		this->current_frame = (this->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

		return result;
	}


	bool SwapChain::Create() {
		if (!CreateSwapChain()) {
			return false;
		}

		if (!CreateImageViews()) {
			return false;
		}

		if (!CreateRenderPass()) {
			return false;
		}

		if (!CreateDepthResources()) {
			return false;
		}

		if (!CreateFrameBuffer()) {
			return false;
		}

		if (!CreateSyncObjects()) {
			return false;
		}

		return true;
	}


	bool SwapChain::CreateSwapChain() {
		SwapChainSupport support = this->device.GetSwapChainSupport(this->device.GetPhysicalDevice());

		VkSurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(support.formats);
		VkPresentModeKHR present_mode = ChooseSwapPresentMode(support.present_modes);
		VkExtent2D extent = ChooseSwapExtent(support.capabilities);

		uint32_t image_count = support.capabilities.minImageCount + 1;
		if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount) {
			image_count = support.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR create_info = {};

		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = this->device.GetSurface();

		create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamiliyIndices indices = this->device.GetFamilyIndices(this->device.GetPhysicalDevice());
		uint32_t queue_family_indices[] = {
			indices.graphics_index,
			indices.present_index
		};

		if (indices.graphics_index != indices.present_index) {
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		} else {
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices = nullptr;
		}

		create_info.preTransform = support.capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;

		create_info.oldSwapchain = this->prev_swapchain == VK_NULL_HANDLE ? VK_NULL_HANDLE : this->prev_swapchain->GetSwapChain();

		VkResult result = vkCreateSwapchainKHR(
			this->device.GetDevice(),
			&create_info,
			nullptr,
			&this->swap_chain
		);
		if (result != VK_SUCCESS) {
			return false;
		}

		vkGetSwapchainImagesKHR(
			this->device.GetDevice(),
			this->swap_chain,
			&image_count,
			nullptr
		);
		this->images.resize(image_count);

		vkGetSwapchainImagesKHR(
			this->device.GetDevice(),
			this->swap_chain,
			&image_count,
			this->images.data()
		);

		this->extent = extent;
		this->image_format = surface_format.format;

		return true;
	}

	void SwapChain::DestorySwapChain() {
		if (this->swap_chain == VK_NULL_HANDLE) {
			return;
		}

		vkDestroySwapchainKHR(
			this->device.GetDevice(),
			this->swap_chain,
			nullptr
		);
	}


	bool SwapChain::CreateImageViews() {
		this->image_views.resize(this->images.size());

		for (uint32_t i = 0; i < this->images.size(); i++) {
			VkImageViewCreateInfo view_info = {};

			view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.image = this->images[i];
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format = this->image_format;
			view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view_info.subresourceRange.baseMipLevel = 0;
			view_info.subresourceRange.levelCount = 1;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(
				this->device.GetDevice(),
				&view_info,
				nullptr,
				&this->image_views[i]
			) !=VK_SUCCESS) {
				return false;
			}
		}

		return true;
	}

	void SwapChain::DestoryImageViews() {
		for (VkImageView image_view : this->image_views) {
			vkDestroyImageView(
				this->device.GetDevice(),
				image_view,
				nullptr
			);
		}

		this->image_views.clear();
	}


	bool SwapChain::CreateRenderPass() {
		VkAttachmentDescription depth_attachment = {};

		this->depth_format = ChooseDepthFormat();

		depth_attachment.format = this->depth_format;
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_referance = {};

		depth_attachment_referance.attachment = 1;
		depth_attachment_referance.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription color_attachment = {};

		color_attachment.format = this->image_format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_referance = {};

		color_attachment_referance.attachment = 0;
		color_attachment_referance.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};

		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;

		subpass.pResolveAttachments = nullptr;

		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;

		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_referance;
		subpass.pDepthStencilAttachment = &depth_attachment_referance;

		VkSubpassDependency dependency = {};

		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::vector<VkAttachmentDescription> attachments = {
			color_attachment,
			depth_attachment
		};

		VkRenderPassCreateInfo render_pass_info = {};

		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = attachments.size();
		render_pass_info.pAttachments = attachments.data();
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		if (vkCreateRenderPass(
			this->device.GetDevice(),
			&render_pass_info,
			nullptr,
			&this->render_pass
		) != VK_SUCCESS) {
			return false;
		}

		return true;
	}

	void SwapChain::DestroyRenderPass() {
		if (this->swap_chain == VK_NULL_HANDLE) {
			return;
		}

		vkDestroyRenderPass(
			this->device.GetDevice(),
			this->render_pass,
			nullptr
		);
	}


	bool SwapChain::CreateDepthResources() {
		this->depth_images.resize(this->images.size());
		this->depth_image_views.resize(this->images.size());
		this->depth_image_memorys.resize(this->images.size());

		for (int i = 0; i < this->depth_images.size(); i++) {
			VkImageCreateInfo image_info{};

			image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_info.imageType = VK_IMAGE_TYPE_2D;
			image_info.extent.width = this->extent.width;
			image_info.extent.height = this->extent.height;
			image_info.extent.depth = 1;
			image_info.mipLevels = 1;
			image_info.arrayLayers = 1;
			image_info.format = this->depth_format;
			image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
			image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			image_info.samples = VK_SAMPLE_COUNT_1_BIT;
			image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			image_info.flags = 0;

			this->device.CreateImageWithInfo(
				image_info,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				this->depth_image_memorys[i],
				this->depth_images[i]
			);

			VkImageViewCreateInfo view_info = {};

			view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.image = this->depth_images[i];
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format = this->depth_format;
			view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			view_info.subresourceRange.baseMipLevel = 0;
			view_info.subresourceRange.levelCount = 1;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(
				this->device.GetDevice(),
				&view_info,
				nullptr,
				&depth_image_views[i]
			) != VK_SUCCESS) {
				return false;
			}
		}

		return true;
	}

	void SwapChain::DestroyDepthResources() {
		for (int i = 0; i < this->depth_images.size(); i++) {
			vkDestroyImageView(
				this->device.GetDevice(),
				this->depth_image_views[i],
				nullptr
			);

			vkDestroyImage(
				this->device.GetDevice(),
				this->depth_images[i],
				nullptr
			);

			vkFreeMemory(
				this->device.GetDevice(),
				this->depth_image_memorys[i],
				nullptr
			);
		}
	}


	bool SwapChain::CreateFrameBuffer() {
		this->frame_buffers.resize(this->images.size());

		for (size_t i = 0; i < this->images.size(); i++) {
			std::array<VkImageView, 2> attachments = {
				this->image_views.at(i),
				this->depth_image_views.at(i)
			};

			VkFramebufferCreateInfo frame_buffer_info = {};

			frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frame_buffer_info.renderPass = render_pass;
			frame_buffer_info.attachmentCount = attachments.size();
			frame_buffer_info.pAttachments = attachments.data();
			frame_buffer_info.width = this->extent.width;
			frame_buffer_info.height = this->extent.height;
			frame_buffer_info.layers = 1;

			if (vkCreateFramebuffer(
				this->device.GetDevice(),
				&frame_buffer_info,
				nullptr,
				&this->frame_buffers.at(i)
			) != VK_SUCCESS) {
				return false;
			}
		}

		return true;
	}

	void SwapChain::DestoryFrameBuffer() {
		for (VkFramebuffer framebuffer : this->frame_buffers) {
			vkDestroyFramebuffer(
				this->device.GetDevice(),
				framebuffer,
				nullptr
			);
		}
	}


	bool SwapChain::CreateSyncObjects() {
		this->image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		this->render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		this->in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

		this->images_in_flight.resize(
			this->images.size(),
			VK_NULL_HANDLE
		);

		VkSemaphoreCreateInfo semaphore_info = {};

		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info = {};

		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(
				this->device.GetDevice(),
				&semaphore_info,
				nullptr,
				&this->image_available_semaphores[i]
			) != VK_SUCCESS) {
				return false;
			}

			if (vkCreateSemaphore(
				this->device.GetDevice(),
				&semaphore_info,
				nullptr,
				&this->render_finished_semaphores[i]
			) != VK_SUCCESS) {
				return false;
			}

			if (vkCreateFence(
				this->device.GetDevice(),
				&fence_info,
				nullptr,
				&this->in_flight_fences[i]
			) != VK_SUCCESS) {
				return false;
			}
		}

		return true;
	}

	void SwapChain::DestroySyncObjects() {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(
				this->device.GetDevice(),
				this->render_finished_semaphores[i],
				nullptr
			);

			vkDestroySemaphore(
				this->device.GetDevice(),
				this->image_available_semaphores[i],
				nullptr
			);

			vkDestroyFence(
				this->device.GetDevice(),
				this->in_flight_fences[i],
				nullptr
			);
		}
	}


	VkFormat SwapChain::ChooseDepthFormat() {
		return this->device.FindSupportedFormat(
			std::vector<VkFormat>{
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT
			},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	VkExtent2D SwapChain::ChooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}

		VkExtent2D actual_extent = {};

		actual_extent.width = std::max(
			capabilities.minImageExtent.width,
			std::min(
				capabilities.maxImageExtent.width,
				actual_extent.width
			)
		);

		actual_extent.height = std::max(
			capabilities.minImageExtent.height,
			std::min(
				capabilities.maxImageExtent.height,
				actual_extent.height
			)
		);

		return actual_extent;
	}

	VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avail_formats) {
		for (const VkSurfaceFormatKHR& avail_format : avail_formats) {
			if (
				avail_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
				avail_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
			) {
				return avail_format;
			}
		}

		return avail_formats[0];
	}

	VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& avail_present_modes) {
		for (const VkPresentModeKHR& avail_present_mode : avail_present_modes) {
			if (avail_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return avail_present_mode; 
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}
}