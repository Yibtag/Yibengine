#pragma once

#include <string>
#include <vector>
#include <cstring>
#include <optional>
#include <algorithm>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "window.h"

namespace yib {
	struct QueueFamiliyIndices {
		uint32_t graphics_index = 0;
		bool graphics = false;
		uint32_t present_index = 0;
		bool present = false;
	};

	struct SwapChainSupport {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	class Device {
	public:
		Device(const std::string name, Window* window);
		~Device();

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;

		VkDevice GetDevice() const;
		VkSurfaceKHR GetSurface() const;
		VkQueue GetPresentQueue() const;
		VkQueue GetGraphicsQueue() const;
		VkCommandPool GetCommandPool() const;
		VkPhysicalDevice GetPhysicalDevice() const;

		std::optional<uint32_t> FindMemoryType(
			uint32_t type_filter,
			VkMemoryPropertyFlags properties
		) const;
		VkFormat FindSupportedFormat(
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features
		) const;
		bool CreateImageWithInfo(
			const VkImageCreateInfo& image_info,
			VkMemoryPropertyFlags properties,
			VkDeviceMemory& image_memory,
			VkImage& image
		) const;
		bool CreateBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer& buffer,
			VkDeviceMemory& buffer_memory
		);
		bool IsPhysicalDeviceSuitable(const VkPhysicalDevice& device);
		bool SupportsRequiredExtentions(const VkPhysicalDevice& device);
		QueueFamiliyIndices GetFamilyIndices(const VkPhysicalDevice& device) const;
		SwapChainSupport GetSwapChainSupport(const VkPhysicalDevice& device) const;

		bool success;
	private:
		bool CreateInstance();
		void DestoryInstance() const;

		bool CreateDebugCallback();
		bool DestroyDebugCallback() const;

		bool CreateSurface();
		bool DestorySurface() const;

		bool SelectPhysicalDevice();

		bool CreateLogicalDevice();
		void DestroyLogicalDevice() const;

		bool CreateCommandPool();
		void DestroyCommandPool() const;

		std::vector<const char*> GetAvailExtentions() const;
		std::vector<const char*> GetRequiredExtentions() const;

		const std::string name;
		const bool enable_validation_layers = true;
		const std::vector<const char*> validation_layers = {
				"VK_LAYER_KHRONOS_validation"
		};
		const std::vector<const char*> device_extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		Window* window;
		VkDevice device = VK_NULL_HANDLE;
		VkInstance instance = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkQueue present_queue = VK_NULL_HANDLE;
		VkQueue graphics_queue = VK_NULL_HANDLE;
		VkCommandPool command_pool = VK_NULL_HANDLE;
		VkPhysicalDevice physical_device = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
	};
}