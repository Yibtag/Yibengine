#pragma once

#include <stdint.h>

#include <vulkan/vulkan.h>

#include "device.h"

namespace yib {
	class Buffer {
	public:
		Buffer(
			Device& device,
			VkDeviceSize instance_size,
			uint32_t instance_count,
			VkBufferUsageFlags flags,
			VkMemoryPropertyFlags memory_flags,
			VkDeviceSize min_offset_alignment = 1
		);
		~Buffer();

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;

		VkBuffer GetBuffer() const;
		void* GetMappedMemory() const;
		uint32_t GetInstanceCount() const;
		VkDeviceSize GetBufferSize() const;
		VkDeviceSize GetInstanceSize() const;
		VkDeviceSize GetAlignmentSize() const;
		VkBufferUsageFlags GetFlags() const;
		VkMemoryPropertyFlags GetMemoryFlags() const;

		bool Map(
			VkDeviceSize size = VK_WHOLE_SIZE,
			VkDeviceSize offset = 0
		);
		void Unmap();
		
		bool Write(
			void* data,
			VkDeviceSize size = VK_WHOLE_SIZE,
			VkDeviceSize offset = 0
		);
		bool Flush(
			VkDeviceSize size = VK_WHOLE_SIZE,
			VkDeviceSize offset = 0
		);
		bool Invalidate(
			VkDeviceSize size = VK_WHOLE_SIZE,
			VkDeviceSize offset = 0
		);
		VkDescriptorBufferInfo DescriptorInfo(
			VkDeviceSize size = VK_WHOLE_SIZE,
			VkDeviceSize offset = 0
		);


		bool WriteToIndex(
			void* data,
			int index
		);
		bool FlushIndex(int index);
		bool InvalidateIndex(int index);
		VkDescriptorBufferInfo DescriptorInfoForIndex(int index);

		bool success;
	private:
		static VkDeviceSize GetAlignment(
			VkDeviceSize instance_size,
			VkDeviceSize min_offset_alignment
		);

		Device& device;

		void* mapped = nullptr;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;

		VkDeviceSize buffer_size;
		uint32_t instance_count;
		VkDeviceSize instance_size;
		VkDeviceSize alignment_size;

		VkBufferUsageFlags flags;
		VkMemoryPropertyFlags memory_flags;
	};
}