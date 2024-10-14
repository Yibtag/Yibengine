#include "texture.h"

#include <cmath>

#include "buffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../shared/stb/stb_image.h"

namespace yib {
	Texture::Texture(
		Device& device,
		const std::string& file
	) : device(device), success(false) {
		int width, height, channels, bytes_per_pixel;
		stbi_uc* data = stbi_load(
			file.c_str(),
			&width,
			&height,
			&bytes_per_pixel,
			4
		);
		if (data == NULL) {
			return;
		}

		this->width = static_cast<uint32_t>(width);
		this->height = static_cast<uint32_t>(height);
		this->mip_levels = std::floor(std::log2(std::max(width, height))) + 1;

		Buffer staging_buffer = Buffer(
			this->device,
			4,
			width * height,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		if (!staging_buffer.success) {
			return;
		}

		staging_buffer.Map();
		staging_buffer.Write(data);

		this->format = VK_FORMAT_R8G8B8A8_SRGB;

		VkImageCreateInfo image_info = { };

		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.format = this->format;
		image_info.mipLevels = this->mip_levels;
		image_info.arrayLayers = 1;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.extent = { this->width, this->height, 1 };
		image_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		if (!this->device.CreateImageWithInfo(
			image_info,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			this->memory,
			this->image
		)) {
			return;
		}

		if (!TransitionImageLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		)) {
			return;
		}

		if (!this->device.CopyBufferToImage(
			staging_buffer.GetBuffer(),
			this->image,
			width,
			height,
			1
		)) {
			return;
		}

		if (!GenerateMipmaps()) {
			return;
		}

		VkSamplerCreateInfo sampler_info = { };

		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = VK_FILTER_NEAREST;
		sampler_info.minFilter = VK_FILTER_NEAREST;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.compareOp = VK_COMPARE_OP_NEVER;
		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = static_cast<float>(this->mip_levels);
		sampler_info.maxAnisotropy = 4.0f;
		sampler_info.anisotropyEnable = VK_TRUE;
		sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		if (vkCreateSampler(
			this->device.GetDevice(),
			&sampler_info,
			nullptr,
			&this->sampler
		) != VK_SUCCESS) {
			return;
		}

		VkImageViewCreateInfo view_info = { };

		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = this->format;
		view_info.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;
		view_info.subresourceRange.levelCount = this->mip_levels;
		view_info.image = this->image;

		if (vkCreateImageView(
			this->device.GetDevice(),
			&view_info,
			nullptr,
			&this->view
		) != VK_SUCCESS) {
			return;
		}

		stbi_image_free(data);

		this->success = true;
	}

	Texture::~Texture() {
		vkDestroyImage(
			this->device.GetDevice(),
			this->image,
			nullptr
		);

		vkFreeMemory(
			this->device.GetDevice(),
			this->memory,
			nullptr
		);

		vkDestroyImageView(
			this->device.GetDevice(),
			this->view,
			nullptr
		);

		vkDestroySampler(
			this->device.GetDevice(),
			this->sampler,
			nullptr
		);
	}


	uint32_t Texture::GetWidth() const {
		return this->width;
	}

	uint32_t Texture::GetHeight() const {
		return this->height;
	}

	VkImageView Texture::GetView() const {
		return this->view;
	}

	VkSampler Texture::GetSampler() const {
		return this->sampler;
	}

	uint32_t Texture::GetMipLevels() const {
		return this->mip_levels;
	}

	VkImageLayout Texture::GetImageLayout() const {
		return this->layout;
	}

	VkDescriptorImageInfo Texture::GetDescriptorInfo() const {
		VkDescriptorImageInfo image_info = { };

		image_info.sampler = this->sampler;
		image_info.imageView = this->view;
		image_info.imageLayout = this->layout;

		return image_info;
	}


	bool Texture::TransitionImageLayout(
		VkImageLayout layout
	) {
		VkCommandBuffer command_buffer = this->device.BeginSingleTimeCommands();
		if (command_buffer == VK_NULL_HANDLE) {
			return false;
		}

		VkImageMemoryBarrier barrier = { };
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = this->layout;
		barrier.newLayout = layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = this->mip_levels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags destination_stage;

		if (
			this->layout == VK_IMAGE_LAYOUT_UNDEFINED &&
			layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (
			this->layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} else {
			return false;
		}

		vkCmdPipelineBarrier(
			command_buffer,
			source_stage,
			destination_stage,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier
		);

		if (!this->device.EndSingleTimeCommands(command_buffer)) {
			return false;
		}

		return true;
	}

	bool Texture::GenerateMipmaps() {
		VkFormatProperties format_properties;
		vkGetPhysicalDeviceFormatProperties(
			this->device.GetPhysicalDevice(),
			this->format,
			&format_properties
		);

		if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			return false;
		}

		VkCommandBuffer command_buffer = this->device.BeginSingleTimeCommands();
		if (command_buffer == VK_NULL_HANDLE) {
			return false;
		}

		VkImageMemoryBarrier barrier = { };

		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = this->image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mip_width = width;
		int32_t mip_height = height;

		for (uint32_t i = 1; i < this->mip_levels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(
				command_buffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&barrier
			);

			VkImageBlit blit = { };
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mip_width, mip_height, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = {
				mip_width > 1 ? mip_width / 2 : 1,
				mip_height > 1 ? mip_height / 2 : 1, 1
			};
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(
				command_buffer,
				this->image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				this->image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&blit,
				VK_FILTER_LINEAR
			);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(
				command_buffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&barrier
			);

			if (mip_width > 1) mip_width /= 2;
			if (mip_height > 1) mip_height /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mip_levels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			command_buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier
		);

		this->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if (!this->device.EndSingleTimeCommands(command_buffer)) {
			return false;
		}

		return true;
	}
}