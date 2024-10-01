#pragma once

#include <string>

#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"

namespace yib {
	struct PipelineConfig {
		PipelineConfig() = default;

		VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
		VkPipelineViewportStateCreateInfo viewport_info;
		VkPipelineRasterizationStateCreateInfo rasterization_info;
		VkPipelineMultisampleStateCreateInfo multisample_info;
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		VkPipelineColorBlendStateCreateInfo color_blend_info;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
		VkPipelineDynamicStateCreateInfo dynamic_state_info;
		std::vector<VkPushConstantRange> push_constant_ranges;
		VkPipelineLayoutCreateInfo pipeline_layout_info;

		uint32_t subpass;
		VkRenderPass render_pass;
		std::vector<VkDynamicState> dynamic_states;
	};

	class Pipeline {
	public:
		Pipeline(
			Device& device,
			const uint32_t width,
			const uint32_t height,
			const std::string vertex_shader,
			const std::string fragmnet_sahder,
			PipelineConfig config
		);
		~Pipeline();

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		VkPipelineLayout GetPipelineLayout() const;

		static PipelineConfig CreateDefaultConfig(VkRenderPass render_pass);
		void BindCommandBuffer(VkCommandBuffer command_buffer) const;

		bool success;
	private:
		bool CreateShaderModules();
		void DestroyShaderModules();

		bool CreatePipelineLayout();
		void DestroyPipelineLayout();

		bool CreatePipeline();
		void DestoryPipeline();

		VkShaderModule CreateShaderModule(const std::vector<char>& data);

		const uint32_t width;
		const uint32_t height;
		const std::string vertex_shader;
		const std::string fragment_shader;
		PipelineConfig config;

		Device& device;
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
		VkShaderModule vertex_shader_module = VK_NULL_HANDLE;
		VkShaderModule fragment_shader_module = VK_NULL_HANDLE;
	};
}