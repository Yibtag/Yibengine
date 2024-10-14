#pragma once

#include <string>
#include <cstdint>

#include <vulkan/vulkan.h>

#include "device.h"

namespace yib {
	class Texture {
	public:
		Texture(
			Device& device,
			const std::string& file
		);
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		VkImageView GetView() const;
		VkSampler GetSampler() const;
		uint32_t GetMipLevels() const;
		VkImageLayout GetImageLayout() const;
		VkDescriptorImageInfo GetDescriptorInfo() const;

		bool success;
	private:
		bool TransitionImageLayout(
			VkImageLayout layout
		);
		bool GenerateMipmaps();

		Device& device;

		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t mip_levels = 0;

		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
		VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	};
}