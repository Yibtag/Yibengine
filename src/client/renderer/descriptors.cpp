#include "descriptors.h"

namespace yib {
	DescriptorSetLayout::Builder::Builder(Device& device) : device(device) { }

	bool DescriptorSetLayout::Builder::AddBinding(
		uint32_t binding,
		VkDescriptorType descriptor_type,
		VkShaderStageFlags stage_flags,
		uint32_t count
	) {
		if (bindings.count(binding) != 0) {
			return false;
		}

		VkDescriptorSetLayoutBinding layout_binding = { };

		layout_binding.binding = binding;
		layout_binding.descriptorType = descriptor_type;
		layout_binding.descriptorCount = count;
		layout_binding.stageFlags = stage_flags;

		this->bindings[binding] = layout_binding;

		return true;
	}

	std::unique_ptr<DescriptorSetLayout>  DescriptorSetLayout::Builder::Build() const {
		return std::make_unique<DescriptorSetLayout>(this->device, this->bindings);
	}


	DescriptorSetLayout::DescriptorSetLayout(
		Device& device,
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings
	) : device(device), bindings(bindings), success(success) {
		std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings = { };

		for (std::pair<uint32_t, VkDescriptorSetLayoutBinding> binding : bindings) {
			set_layout_bindings.push_back(binding.second);
		}

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info = { };

		descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_info.bindingCount = set_layout_bindings.size();
		descriptor_set_layout_info.pBindings = set_layout_bindings.data();

		if (vkCreateDescriptorSetLayout(
			this->device.GetDevice(),
			&descriptor_set_layout_info,
			nullptr,
			&desciptor_set_layout
		) != VK_SUCCESS) {
			return;
		}

		this->success = true;
	}

	DescriptorSetLayout::~DescriptorSetLayout() {
		vkDestroyDescriptorSetLayout(
			this->device.GetDevice(),
			this->desciptor_set_layout,
			nullptr
		);
	}

	VkDescriptorSetLayout DescriptorSetLayout::GetDescriptorSetLayout() const {
		return this->desciptor_set_layout;
	}


	DescriptorPool::Builder::Builder(Device& device) : device(device) { }

	void DescriptorPool::Builder::AddPoolSize(VkDescriptorType descriptor_type, uint32_t count) {
		this->sizes.push_back({ descriptor_type, count });
	}

	void DescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags) {
		this->flags = flags;
	}

	void DescriptorPool::Builder::SetMaxSets(uint32_t count) {
		this->max_sets = count;
	}

	std::unique_ptr<DescriptorPool> DescriptorPool::Builder::Build() const {
		return std::make_unique<DescriptorPool>(
			this->device,
			this->max_sets,
			this->flags,
			this->sizes
		);
	}


	DescriptorPool::DescriptorPool(
		Device& device,
		uint32_t max_sets,
		VkDescriptorPoolCreateFlags flags,
		const std::vector<VkDescriptorPoolSize>& sizes
	) : device(device), success(false) {
		VkDescriptorPoolCreateInfo create_info = { };

		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.poolSizeCount = sizes.size();
		create_info.pPoolSizes = sizes.data();
		create_info.maxSets = max_sets;
		create_info.flags = flags;

		if (vkCreateDescriptorPool(
			this->device.GetDevice(),
			&create_info,
			nullptr,
			&this->descriptor_pool
		) != VK_SUCCESS) {
			return;
		}

		this->success = true;
	}

	DescriptorPool::~DescriptorPool() {
		vkDestroyDescriptorPool(
			this->device.GetDevice(),
			this->descriptor_pool,
			nullptr
		);
	}

	bool DescriptorPool::AllocateDescriptor(
		const VkDescriptorSetLayout descriptor_set_layout,
		VkDescriptorSet& descriptor
	) const {
		VkDescriptorSetAllocateInfo allocate_info = { };

		allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocate_info.descriptorPool = this->descriptor_pool;
		allocate_info.pSetLayouts = &descriptor_set_layout;
		allocate_info.descriptorSetCount = 1;

		if (vkAllocateDescriptorSets(
			this->device.GetDevice(),
			&allocate_info,
			&descriptor
		) != VK_SUCCESS) {
			return false;
		}

		return true;
	}

	void DescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
		vkFreeDescriptorSets(
			this->device.GetDevice(),
			this->descriptor_pool,
			descriptors.size(),
			descriptors.data()
		);
	}

	void DescriptorPool::ResetPool() {
		vkResetDescriptorPool(
			this->device.GetDevice(),
			this->descriptor_pool,
			0
		);
	}


	DescriptorWriter::DescriptorWriter(
		DescriptorSetLayout& set_layout,
		DescriptorPool& pool
	) : set_layout(set_layout), pool(pool) { }

	bool DescriptorWriter::WriteBuffer(
		uint32_t binding,
		VkDescriptorBufferInfo* buffer_info
	) {
		if (this->set_layout.bindings.count(binding) != 1) {
			return false;
		}

		VkDescriptorSetLayoutBinding& binding_description = this->set_layout.bindings[binding];

		if (binding_description.descriptorCount != 1) {
			return false;
		}

		VkWriteDescriptorSet write = { };

		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = binding_description.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = buffer_info;
		write.descriptorCount = 1;

		this->writes.push_back(write);

		return true;
	}

	bool DescriptorWriter::WriteImage(
		uint32_t binding,
		VkDescriptorImageInfo* image_info
	) {
		if (this->set_layout.bindings.count(binding) != 1) {
			return false;
		}

		VkDescriptorSetLayoutBinding& binding_description = this->set_layout.bindings[binding];

		if (binding_description.descriptorCount != 1) {
			return false;
		}

		VkWriteDescriptorSet write = { };

		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = binding_description.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = image_info;
		write.descriptorCount = 1;

		this->writes.push_back(write);

		return true;
	}

	bool DescriptorWriter::Build(VkDescriptorSet& set) {
		if (!this->pool.AllocateDescriptor(
			this->set_layout.GetDescriptorSetLayout(),
			set
		)) {
			return false;
		}

		Overwrite(set);

		return true;
	}

	void DescriptorWriter::Overwrite(VkDescriptorSet& set) {
		for (VkWriteDescriptorSet& write : this->writes) {
			write.dstSet = set;
		}

		vkUpdateDescriptorSets(
			this->pool.device.GetDevice(),
			this->writes.size(),
			this->writes.data(),
			0,
			nullptr
		);
	}
}