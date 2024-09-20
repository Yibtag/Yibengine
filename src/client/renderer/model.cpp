#include "model.h"

#include <cstring>

namespace yib {
	std::vector<VkVertexInputBindingDescription> Vertex::GetBindingDescription() {
		std::vector<VkVertexInputBindingDescription> binding_descriptions(1);

		binding_descriptions.at(0).binding = 0;
		binding_descriptions.at(0).stride = sizeof(Vertex);
		binding_descriptions.at(0).inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return binding_descriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2);

		attribute_descriptions.at(0).binding = 0;
		attribute_descriptions.at(0).location = 0;
		attribute_descriptions.at(0).format = VK_FORMAT_R32G32_SFLOAT;
		attribute_descriptions.at(0).offset = offsetof(Vertex, position);

		attribute_descriptions.at(1).binding = 0;
		attribute_descriptions.at(1).location = 1;
		attribute_descriptions.at(1).format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions.at(1).offset = offsetof(Vertex, color);

		return attribute_descriptions;
	}


	Model::Model(
		Device& device,
		const std::vector<Vertex>& vertecies
	) : device(device), success(false) {
		if (!CreateVertexBuffers(vertecies)) {
			return;
		}

		this->success = true;
	}

	Model::~Model() {
		DestoryVertexBuffers();
	}


	void Model::Bind(VkCommandBuffer command_buffer) {
		VkBuffer buffers[] = { this->vertex_buffer };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(
			command_buffer,
			0,
			1,
			buffers,
			offsets
		);
	}

	void Model::Draw(VkCommandBuffer command_buffer) const {
		vkCmdDraw(
			command_buffer,
			this->vertex_count,
			1,
			0,
			0
		);
	}


	bool Model::CreateVertexBuffers(const std::vector<Vertex>& vertecies) {
		this->vertex_count = vertecies.size();
		if (this->vertex_count < 3) {
			return false;
		}

		VkDeviceSize buffer_size = sizeof(vertecies[0]) * this->vertex_count;

		if (!this->device.CreateBuffer(
			buffer_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			this->vertex_buffer,
			this->vertex_buffer_memory
		)) {
			return false;
		}

		void* data;
		if (vkMapMemory(
			this->device.GetDevice(),
			this->vertex_buffer_memory,
			0,
			buffer_size,
			0,
			&data
		) != VK_SUCCESS) {
			return false;
		}

		memcpy_s(
			data,
			buffer_size,
			vertecies.data(),
			buffer_size
		);

		vkUnmapMemory(
			this->device.GetDevice(),
			this->vertex_buffer_memory
		);

		return true;
	}

	void Model::DestoryVertexBuffers() {
		vkDestroyBuffer(
			this->device.GetDevice(),
			this->vertex_buffer,
			nullptr
		);

		vkFreeMemory(
			this->device.GetDevice(),
			this->vertex_buffer_memory,
			nullptr
		);
	}
}