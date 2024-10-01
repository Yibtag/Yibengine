#include "pipeline.h"

#include <iostream>
#include <filesystem>

#include "model.h"
#include "../../shared/file.h"

namespace yib {
	Pipeline::Pipeline(
		Device& device,
		const uint32_t width,
		const uint32_t height,
		const std::string vertex_shader,
		const std::string fragmnet_sahder,
		PipelineConfig config
	) :
		device(device),
		width(width),
		height(height),
		vertex_shader(vertex_shader),
		fragment_shader(fragmnet_sahder),
		config(config),
		success(false)
	{
		if (!CreateShaderModules()) {
			return;
		}

		if (!CreatePipelineLayout()) {
			return;
		}

		if (!CreatePipeline()) {
			return;
		}

		this->success = true;
	}

	Pipeline::~Pipeline() {
		DestoryPipeline();
		DestroyPipelineLayout();
		DestroyShaderModules();
	}


	VkPipelineLayout Pipeline::GetPipelineLayout() const {
		return this->pipeline_layout;
	}


	PipelineConfig Pipeline::CreateDefaultConfig(VkRenderPass render_pass) {
		PipelineConfig config = {};

		config.input_assembly_info = {};

		config.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		config.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		config.input_assembly_info.primitiveRestartEnable = VK_FALSE;

		config.viewport_info = {};

		config.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		config.viewport_info.viewportCount = 1;
		config.viewport_info.pViewports = nullptr;
		config.viewport_info.scissorCount = 1;
		config.viewport_info.pScissors = nullptr;

		config.rasterization_info = {};

		config.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		config.rasterization_info.depthClampEnable = VK_FALSE;
		config.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
		config.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
		config.rasterization_info.lineWidth = 1.0f;
		config.rasterization_info.cullMode = VK_CULL_MODE_NONE;
		config.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		config.rasterization_info.depthBiasEnable = VK_FALSE;
		config.rasterization_info.depthBiasConstantFactor = 0.0f;
		config.rasterization_info.depthBiasClamp = 0.0f;
		config.rasterization_info.depthBiasSlopeFactor = 0.0f;

		config.multisample_info = {};

		config.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		config.multisample_info.sampleShadingEnable = VK_FALSE;
		config.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		config.multisample_info.minSampleShading = 1.0f;
		config.multisample_info.pSampleMask = nullptr;
		config.multisample_info.alphaToCoverageEnable = VK_FALSE;
		config.multisample_info.alphaToOneEnable = VK_FALSE;

		config.color_blend_attachment = {};

		config.color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		config.color_blend_attachment.blendEnable = VK_FALSE;
		config.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		config.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		config.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		config.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		config.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		config.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		config.color_blend_info = {};

		config.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		config.color_blend_info.logicOpEnable = VK_FALSE;
		config.color_blend_info.logicOp = VK_LOGIC_OP_COPY;
		config.color_blend_info.attachmentCount = 1;
		config.color_blend_info.pAttachments = &config.color_blend_attachment;
		config.color_blend_info.blendConstants[0] = 0.0f;
		config.color_blend_info.blendConstants[1] = 0.0f;
		config.color_blend_info.blendConstants[2] = 0.0f;
		config.color_blend_info.blendConstants[3] = 0.0f;

		config.depth_stencil_info = {};

		config.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		config.depth_stencil_info.depthTestEnable = VK_TRUE;
		config.depth_stencil_info.depthWriteEnable = VK_TRUE;
		config.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
		config.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
		config.depth_stencil_info.minDepthBounds = 0.0f;
		config.depth_stencil_info.maxDepthBounds = 1.0f;
		config.depth_stencil_info.stencilTestEnable = VK_FALSE;
		config.depth_stencil_info.front = {};
		config.depth_stencil_info.back = {};

		config.dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		config.dynamic_state_info = {};

		config.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		config.dynamic_state_info.dynamicStateCount = config.dynamic_states.size();
		config.dynamic_state_info.pDynamicStates = config.dynamic_states.data();
		config.dynamic_state_info.flags = 0;

		config.pipeline_layout_info = {};

		config.pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		config.pipeline_layout_info.setLayoutCount = 0;
		config.pipeline_layout_info.pSetLayouts = nullptr;
		config.pipeline_layout_info.pushConstantRangeCount = 0;
		config.pipeline_layout_info.pPushConstantRanges = nullptr;

		config.render_pass = render_pass;

		return config;
	}


	void Pipeline::BindCommandBuffer(VkCommandBuffer command_buffer) const {
		vkCmdBindPipeline(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->pipeline
		);
	}


	bool Pipeline::CreateShaderModules() {
		std::vector<char> vertex_shader_code = File::Read(this->vertex_shader.c_str());
		if (vertex_shader_code.empty()) {
			return false;
		}

		std::vector<char> fragment_shader_code = File::Read(this->fragment_shader.c_str());
		if (fragment_shader_code.empty()) {
			return false;
		}

		this->vertex_shader_module = CreateShaderModule(vertex_shader_code);
		if (this->vertex_shader_module == VK_NULL_HANDLE) {
			return false;
		}

		this->fragment_shader_module = CreateShaderModule(fragment_shader_code);
		if (this->fragment_shader_module == VK_NULL_HANDLE) {
			return false;
		}

		return true;
	}

	void Pipeline::DestroyShaderModules() {
		if (this->vertex_shader_module != VK_NULL_HANDLE) {
			vkDestroyShaderModule(
				this->device.GetDevice(),
				this->vertex_shader_module,
				nullptr
			);
		}
		
		if (this->fragment_shader_module != VK_NULL_HANDLE) {
			vkDestroyShaderModule(
				this->device.GetDevice(),
				this->fragment_shader_module,
				nullptr
			);
		}
	}


	bool Pipeline::CreatePipelineLayout() {
		if (vkCreatePipelineLayout(
			this->device.GetDevice(),
			&this->config.pipeline_layout_info,
			nullptr,
			&this->pipeline_layout
		) != VK_SUCCESS) {
			return false;
		}

		return true;
	}

	void Pipeline::DestroyPipelineLayout() {
		if (this->pipeline_layout == VK_NULL_HANDLE) {
			return;
		}

		vkDestroyPipelineLayout(
			this->device.GetDevice(),
			this->pipeline_layout,
			nullptr
		);
	}


	bool Pipeline::CreatePipeline() {
		VkPipelineShaderStageCreateInfo shader_stages[2] = {};

		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader_stages[0].module = this->vertex_shader_module;
		shader_stages[0].pName = "main";
		shader_stages[0].flags = NULL;
		shader_stages[0].pNext = nullptr;
		shader_stages[0].pSpecializationInfo = nullptr;

		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_stages[1].module = this->fragment_shader_module;
		shader_stages[1].pName = "main";
		shader_stages[1].flags = NULL;
		shader_stages[1].pNext = nullptr;
		shader_stages[1].pSpecializationInfo = nullptr;

		std::vector<VkVertexInputAttributeDescription> attribute_descriptions = Vertex::GetAttributeDescriptions();
		std::vector<VkVertexInputBindingDescription> binding_descriptions = Vertex::GetBindingDescription();

		VkPipelineVertexInputStateCreateInfo vertex_input_info = {};

		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexAttributeDescriptionCount = attribute_descriptions.size();
		vertex_input_info.vertexBindingDescriptionCount = binding_descriptions.size();
		vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();
		vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();

		VkGraphicsPipelineCreateInfo pipeline_info = {};

		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_stages;
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &this->config.input_assembly_info;
		pipeline_info.pViewportState = &this->config.viewport_info;
		pipeline_info.pRasterizationState = &this->config.rasterization_info;
		pipeline_info.pMultisampleState = &this->config.multisample_info;
		pipeline_info.pColorBlendState = &this->config.color_blend_info;
		pipeline_info.pDepthStencilState = &this->config.depth_stencil_info;
		pipeline_info.pDynamicState = &this->config.dynamic_state_info;

		pipeline_info.layout = this->pipeline_layout;
		pipeline_info.renderPass = this->config.render_pass;
		pipeline_info.subpass = this->config.subpass;

		pipeline_info.basePipelineIndex = -1;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(
			this->device.GetDevice(),
			VK_NULL_HANDLE,
			1,
			&pipeline_info,
			nullptr,
			&this->pipeline
		) != VK_SUCCESS) {
			return false;
		}

		return true;
	}

	void Pipeline::DestoryPipeline() {
		if (this->pipeline == VK_NULL_HANDLE) {
			return;
		}

		vkDestroyPipeline(
			this->device.GetDevice(),
			this->pipeline,
			nullptr
		);
	}


	VkShaderModule Pipeline::CreateShaderModule(const std::vector<char>& data) {
		VkShaderModule shader_module;

		VkShaderModuleCreateInfo create_info = {};

		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.pCode = reinterpret_cast<const uint32_t*>(data.data());
		create_info.codeSize = data.size();

		if (vkCreateShaderModule(
			this->device.GetDevice(),
			&create_info,
			nullptr,
			&shader_module
		) != VK_SUCCESS) {
			return VK_NULL_HANDLE;
		}

		return shader_module;
	}
}