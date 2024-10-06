#include "buffer.h"

namespace yib {
	Buffer::Buffer(
		Device& device,
		VkDeviceSize instance_size,
		uint32_t instance_count,
		VkBufferUsageFlags flags,
		VkMemoryPropertyFlags memory_flags,
		VkDeviceSize min_offset_alignment
	) :
	device(device),
	buffer_size(instance_size * instance_count),
	instance_size(instance_size),
	instance_count(instance_count),
	flags(flags),
	memory_flags(memory_flags),
	success(false)
	{
		this->alignment_size = GetAlignment(
			this->instance_size,
			min_offset_alignment
		);

		if (!this->device.CreateBuffer(
			this->buffer_size,
			this->flags,
			this->memory_flags,
			this->buffer,
			this->memory
		)) {
			return;
		}

		this->success = true;
	}

	Buffer::~Buffer() {
		Unmap();

		vkDestroyBuffer(
			this->device.GetDevice(),
			this->buffer,
			nullptr
		);

		vkFreeMemory(
			this->device.GetDevice(),
			this->memory,
			nullptr
		);
	}


	VkBuffer Buffer::GetBuffer() const {
		return this->buffer;
	}

	void* Buffer::GetMappedMemory() const {
		return this->mapped;
	}

	uint32_t Buffer::GetInstanceCount() const {
		return this->instance_count;
	}

	VkDeviceSize Buffer::GetBufferSize() const {
		return this->buffer_size;
	}

	VkDeviceSize Buffer::GetInstanceSize() const {
		return this->instance_size;
	}

	VkDeviceSize Buffer::GetAlignmentSize() const {
		return this->alignment_size;
	}

	VkBufferUsageFlags Buffer::GetFlags() const {
		return this->flags;
	}

	VkMemoryPropertyFlags Buffer::GetMemoryFlags() const {
		return this->memory_flags;
	}


	bool Buffer::Map(
		VkDeviceSize size,
		VkDeviceSize offset
	) {
		if (buffer == VK_NULL_HANDLE || memory == VK_NULL_HANDLE) {
			return false;
		}

		return vkMapMemory(
			this->device.GetDevice(),
			this->memory,
			offset,
			size,
			0,
			&this->mapped
		) == VK_SUCCESS;
	}

	void Buffer::Unmap() {
		if (this->mapped == nullptr) {
			return;
		}

		vkUnmapMemory(
			this->device.GetDevice(),
			this->memory
		);

		this->mapped = nullptr;
	}


	bool Buffer::Write(
		void* data,
		VkDeviceSize size,
		VkDeviceSize offset
	) {
		if (this->mapped == nullptr) {
			return false;
		}

		if (size == VK_WHOLE_SIZE) {
			memcpy(
				this->mapped,
				data,
				this->buffer_size
			);

			return true;
		}

		char* memory_offset = (char*)this->mapped;
		memory_offset += offset;

		memcpy(
			memory_offset,
			data,
			size
		);

		return true;
	}

	bool Buffer::Flush(
		VkDeviceSize size,
		VkDeviceSize offset
	) {
		VkMappedMemoryRange memory_range = { };

		memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		memory_range.memory = this->memory;
		memory_range.offset = offset;
		memory_range.size = size;

		return vkFlushMappedMemoryRanges(
			this->device.GetDevice(),
			1,
			&memory_range
		);
	}

	bool Buffer::Invalidate(
		VkDeviceSize size,
		VkDeviceSize offset
	) {
		VkMappedMemoryRange memory_ranged = { };

		memory_ranged.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		memory_ranged.memory = this->memory;
		memory_ranged.offset = offset;
		memory_ranged.size = size;

		return vkInvalidateMappedMemoryRanges(
			this->device.GetDevice(),
			1,
			&memory_ranged
		) == VK_SUCCESS;
	}

	VkDescriptorBufferInfo Buffer::DescriptorInfo(
		VkDeviceSize size,
		VkDeviceSize offset
	) {
		return VkDescriptorBufferInfo{
			this->buffer,
			offset,
			size,
		};
	}


	bool Buffer::WriteToIndex(
		void* data,
		int index
	) {
		return Write(
			data,
			this->instance_size,
			index * this->alignment_size
		);
	}

	bool Buffer::FlushIndex(int index) {
		return Flush(
			this->alignment_size,
			index * this->alignment_size
		);
	}

	bool Buffer::InvalidateIndex(int index) {
		return Invalidate(
			this->alignment_size,
			index * this->alignment_size
		);
	}

	VkDescriptorBufferInfo Buffer::DescriptorInfoForIndex(int index) {
		return DescriptorInfo(
			this->alignment_size,
			index * this->alignment_size
		);
	}


	VkDeviceSize Buffer::GetAlignment(
		VkDeviceSize instance_size,
		VkDeviceSize min_offset_alignment
	) {
		if (min_offset_alignment > 0) {
			return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
		}

		return instance_size;
	}
}