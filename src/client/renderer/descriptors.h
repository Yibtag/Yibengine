#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "device.h"

namespace yib {
	class DescriptorSetLayout {
	public:
		class Builder {
		public:
			Builder(Device& device);

			bool AddBinding(
				uint32_t binding,
				VkDescriptorType descriptor_type,
				VkShaderStageFlags stage_flags,
				uint32_t count = 1
			);
			std::unique_ptr<DescriptorSetLayout> Build() const;
		private:
			Device& device;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
		};

		DescriptorSetLayout(
			Device &device,
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings
		);
		~DescriptorSetLayout();

		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

		VkDescriptorSetLayout GetDescriptorSetLayout() const;

		bool success;
	private:
		Device& device;
		VkDescriptorSetLayout desciptor_set_layout;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

		friend class DescriptorWriter;
	};

	class DescriptorPool {
	public:
		class Builder {
		public:
			Builder(Device& device);

			void AddPoolSize(VkDescriptorType descriptor_type, uint32_t count);
			void SetPoolFlags(VkDescriptorPoolCreateFlags flags);
			void SetMaxSets(uint32_t count);

			std::unique_ptr<DescriptorPool> Build() const;
		private:
			Device& device;

			uint32_t max_sets = 1000;
			VkDescriptorPoolCreateFlags flags = 0;
			std::vector<VkDescriptorPoolSize> sizes = {};
		};

		DescriptorPool(
			Device& device,
			uint32_t max_sets,
			VkDescriptorPoolCreateFlags flags,
			const std::vector<VkDescriptorPoolSize> &sizes
		);
		~DescriptorPool();

		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;

		bool AllocateDescriptor(
			const VkDescriptorSetLayout descriptor_set_layout,
			VkDescriptorSet& descriptor
		) const;
		void FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

		void ResetPool();

		bool success;
	private:
		Device& device;
		VkDescriptorPool descriptor_pool;

		friend class DescriptorWriter;
	};

	class DescriptorWriter {
	public:
		DescriptorWriter(
			DescriptorSetLayout& set_layout,
			DescriptorPool& pool
		);

		bool WriteBuffer(
			uint32_t binding,
			VkDescriptorBufferInfo* buffer_info
		);
		bool WriteImage(
			uint32_t binding,
			VkDescriptorImageInfo* image_info
		);

		bool Build(VkDescriptorSet& set);
		void Overwrite(VkDescriptorSet& set);
	private:
		DescriptorPool& pool;
		DescriptorSetLayout& set_layout;
		std::vector<VkWriteDescriptorSet> writes;
	};
}