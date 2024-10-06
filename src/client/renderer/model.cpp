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
		attribute_descriptions.at(0).format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions.at(0).offset = offsetof(Vertex, position);

		attribute_descriptions.at(1).binding = 0;
		attribute_descriptions.at(1).location = 1;
		attribute_descriptions.at(1).format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions.at(1).offset = offsetof(Vertex, color);

		return attribute_descriptions;
	}


	Model::Model(
		Device& device,
		ModelData data
	) : device(device), success(false) {
		if (!CreateVertexBuffers(data.vertices)) {
			return;
		}

		if (!CreateIndexBuffers(data.indices)) {
			return;
		}

		this->success = true;
	}


	void Model::Bind(VkCommandBuffer command_buffer) const {
		VkBuffer buffers[] = { this->vertex_buffer->GetBuffer() };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(
			command_buffer,
			0,
			1,
			buffers,
			offsets
		);

		if (this->has_index_buffer) {
			vkCmdBindIndexBuffer(
				command_buffer,
				this->index_buffer->GetBuffer(),
				0,
				VK_INDEX_TYPE_UINT32
			);
		}
	}

	void Model::Draw(VkCommandBuffer command_buffer) const {
		if (this->has_index_buffer) {
			vkCmdDrawIndexed(
				command_buffer,
				this->index_count,
				1,
				0,
				0,
				0
			);
		} else {
			vkCmdDraw(
				command_buffer,
				this->vertex_count,
				1,
				0,
				0
			);
		}
	}


	bool Model::CreateVertexBuffers(const std::vector<Vertex>& vertices) {
		this->vertex_count = vertices.size();
		if (this->vertex_count < 3) {
			return false;
		}

		VkDeviceSize buffer_size = sizeof(vertices[0]) * this->vertex_count;
		VkDeviceSize vertex_size = sizeof(vertices[0]);

		Buffer staging_buffer = Buffer(
			this->device,
			vertex_size,
			this->vertex_count,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		if (!staging_buffer.success) {
			return false;
		}

		staging_buffer.Map();
		staging_buffer.Write(
			(void*)vertices.data()
		);

		this->vertex_buffer = std::make_unique<Buffer>(
			this->device,
			vertex_size,
			this->vertex_count,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		if (!this->vertex_buffer->success) {
			return false;
		}
		
		if (!this->device.CopyBuffer(
			staging_buffer.GetBuffer(),
			this->vertex_buffer->GetBuffer(),
			buffer_size
		)) {
			return false;
		}

		return true;
	}


	bool Model::CreateIndexBuffers(const std::vector<uint32_t>& indices) {
		this->index_count = indices.size();
		if (this->index_count < 1) {
			return false;
		}

		this->has_index_buffer = true;

		VkDeviceSize buffer_size = sizeof(indices[0]) * this->index_count;
		VkDeviceSize index_size = sizeof(indices[0]);

		Buffer staging_buffer = Buffer(
			this->device,
			index_size,
			this->index_count,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		if (!staging_buffer.success) {
			return false;
		}

		staging_buffer.Map();
		staging_buffer.Write(
			(void*)indices.data()
		);

		this->index_buffer = std::make_unique<Buffer>(
			this->device,
			index_size,
			this->index_count,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		if (!this->index_buffer->success) {
			return false;
		}

		if (!this->device.CopyBuffer(
			staging_buffer.GetBuffer(),
			this->index_buffer->GetBuffer(),
			buffer_size
		)) {
			return false;
		}

		return true;
	}
}