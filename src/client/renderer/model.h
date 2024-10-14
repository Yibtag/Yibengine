#pragma once

#include <memory>
#include <cstdint>
#include <optional>

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "device.h"
#include "buffer.h"

namespace yib {
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;

		static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	};

	struct ModelData {
		std::vector<Vertex> vertices = {};
		std::vector<uint32_t> indices = {};

		static std::optional<ModelData> LoadModel(const std::string& file);
	};

	class Model {
	public:
		Model(
			Device& device,
			ModelData data
		);

		void Bind(VkCommandBuffer command_buffer) const;
		void Draw(VkCommandBuffer command_buffer) const;

		bool success;
	private:
		bool CreateVertexBuffers(const std::vector<Vertex>& vertices);
		bool CreateIndexBuffers(const std::vector<uint32_t>& indices);

		Device& device;

		uint32_t vertex_count = 0;
		std::unique_ptr<Buffer> vertex_buffer;

		bool has_index_buffer = false;
		uint32_t index_count = 0;
		std::unique_ptr<Buffer> index_buffer;
	};
}