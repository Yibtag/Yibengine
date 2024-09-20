#pragma once

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "device.h"

namespace yib {
	struct Vertex {
		glm::vec2 position;
		glm::vec3 color;

		static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	};

	class Model {
	public:
		Model(
			Device& device,
			const std::vector<Vertex>& vertecies
		);
		~Model();

		void Bind(VkCommandBuffer command_buffer);
		void Draw(VkCommandBuffer command_buffer) const;

		bool success;
	private:
		bool CreateVertexBuffers(const std::vector<Vertex>& vertecies);
		void DestoryVertexBuffers();

		Device& device;

		uint32_t vertex_count = 0;
		VkBuffer vertex_buffer = VK_NULL_HANDLE;
		VkDeviceMemory vertex_buffer_memory = VK_NULL_HANDLE;
	};
}