#include "device.h"

static const char* GetSeverityString(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
	switch (severity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		return "Verbose";
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		return "Info";
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		return "Warrning";
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		return "Error";
	default:
		return "Invalid";
	}
}

static const char* GetDebugTypeString(VkDebugUtilsMessageTypeFlagsEXT type) {
	switch (type) {
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		return "General";
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		return "Validator";
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		return "Performance";
	case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
		return "Device address binding";
	default:
		return "Invalid";
	}
}

static VKAPI_ATTR VkBool32 DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* user_data
) {
	printf("Debug callback: %s\n", callback_data->pMessage);
	printf("	Severity: %s\n", GetSeverityString(severity));
	printf("	Type: %s\n", GetDebugTypeString(type));
	printf("	Objects: \n");

	for (uint32_t i = 0; i < callback_data->objectCount; i++) {
		printf("		%llx\n", callback_data->pObjects[i].objectHandle);
	}

	return VK_FALSE;
}

namespace yib {
	Device::Device(const std::string name, Window* window) : name(name), window(window), success(false) {
		if (!CreateInstance()) {
			return;
		}

		if (this->enable_validation_layers) {
			if (!CreateDebugCallback()) {
				return;
			}
		}

		if (!CreateSurface()) {
			return;
		}

		if (!SelectPhysicalDevice()) {
			return;
		}

		if (!CreateLogicalDevice()) {
			return;
		}

		if (!CreateCommandPool()) {
			return;
		}

		this->success = true;
	}

	Device::~Device() {
		DestroyCommandPool();
		DestroyLogicalDevice();
		DestorySurface();

		if (this->enable_validation_layers) {
			DestroyDebugCallback();
		}

		DestoryInstance();
	}


	VkDevice Device::GetDevice() const {
		return this->device;
	}

	VkSurfaceKHR Device::GetSurface() const {
		return this->surface;
	}

	VkQueue Device::GetPresentQueue() const {
		return this->present_queue;
	}

	VkQueue Device::GetGraphicsQueue() const {
		return this->graphics_queue;
	}

	VkCommandPool Device::GetCommandPool() const {
		return this->command_pool;
	}

	VkPhysicalDevice Device::GetPhysicalDevice() const {
		return this->physical_device;
	}

	VkPhysicalDeviceProperties Device::GetPhysicalDeviceProperties() const {
		VkPhysicalDeviceProperties propeties;
		vkGetPhysicalDeviceProperties(this->physical_device, &propeties);

		return propeties;
	}


	std::optional<uint32_t> Device::FindMemoryType(
		uint32_t type_filter,
		VkMemoryPropertyFlags properties
	) const {
		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(this->physical_device, &memory_properties);
		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
			if (
				(type_filter & (1 << i)) &&
				(memory_properties.memoryTypes[i].propertyFlags & properties) == properties
				) {
				return i;
			}
		}

		return std::nullopt;
	}

	VkFormat Device::FindSupportedFormat(
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features
	) const {
		for (VkFormat format : candidates) {
			VkFormatProperties propeties;
			vkGetPhysicalDeviceFormatProperties(
				this->physical_device,
				format,
				&propeties
			);

			if (
				tiling == VK_IMAGE_TILING_LINEAR &&
				(propeties.linearTilingFeatures & features) == features
				) {
				return format;
			}
			else if (
				tiling == VK_IMAGE_TILING_OPTIMAL &&
				(propeties.optimalTilingFeatures & features) == features
				) {
				return format;
			}
		}

		return VK_FORMAT_UNDEFINED;
	}

	bool Device::CreateImageWithInfo(
		const VkImageCreateInfo& image_info,
		VkMemoryPropertyFlags properties,
		VkDeviceMemory& image_memory,
		VkImage& image
	) const {
		if (vkCreateImage(
			this->device,
			&image_info,
			nullptr,
			&image
		) != VK_SUCCESS) {
			return false;
		}

		VkMemoryRequirements memory_requirements;
		vkGetImageMemoryRequirements(
			this->device,
			image,
			&memory_requirements
		);

		std::optional<uint32_t> memory_type_index = FindMemoryType(
			memory_requirements.memoryTypeBits,
			properties
		);
		if (!memory_type_index.has_value()) {
			return false;
		}

		VkMemoryAllocateInfo allocation_info{};

		allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocation_info.allocationSize = memory_requirements.size;
		allocation_info.memoryTypeIndex = memory_type_index.value();

		if (vkAllocateMemory(
			this->device,
			&allocation_info,
			nullptr,
			&image_memory
		) != VK_SUCCESS) {
			return false;
		}

		if (vkBindImageMemory(
			this->device,
			image,
			image_memory,
			NULL
		) != VK_SUCCESS) {
			return false;
		}

		return true;
	}

	bool Device::CreateBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& buffer_memory
	) {
		VkBufferCreateInfo buffer_info = {};

		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(
			this->device,
			&buffer_info,
			nullptr,
			&buffer
		) != VK_SUCCESS) {
			return false;
		}

		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements(
			this->device,
			buffer,
			&memory_requirements
		);

		std::optional<uint32_t> memory_type = FindMemoryType(memory_requirements.memoryTypeBits, properties);
		if (!memory_type.has_value()) {
			return false;
		}

		VkMemoryAllocateInfo allocation_info = {};

		allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocation_info.allocationSize = memory_requirements.size;
		allocation_info.memoryTypeIndex = memory_type.value();

		if (vkAllocateMemory(
			this->device,
			&allocation_info,
			nullptr,
			&buffer_memory
		) != VK_SUCCESS) {
			return false;
		}

		if (vkBindBufferMemory(
			this->device,
			buffer,
			buffer_memory,
			0
		) != VK_SUCCESS) {
			return false;
		}

		return true;
	}

	bool Device::CopyBuffer(
		VkBuffer source,
		VkBuffer destination,
		VkDeviceSize size
	) {
		VkCommandBuffer command_buffer = BeginSingleTimeCommands();
		if (command_buffer == VK_NULL_HANDLE) {
			return false;
		}

		VkBufferCopy copy_region = {};

		copy_region.size = size;
		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;

		vkCmdCopyBuffer(
			command_buffer,
			source,
			destination,
			1,
			&copy_region
		);

		EndSingleTimeCommands(command_buffer);

		return true;
	}

	VkCommandBuffer Device::BeginSingleTimeCommands() const {
		VkCommandBufferAllocateInfo allocation_info = {};

		allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocation_info.commandPool = this->command_pool;
		allocation_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer;
		if (vkAllocateCommandBuffers(
			this->device,
			&allocation_info,
			&command_buffer
		) != VK_SUCCESS) {
			return VK_NULL_HANDLE;
		}

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(
			command_buffer,
			&begin_info
		) != VK_SUCCESS) {
			return VK_NULL_HANDLE;
		}

		return command_buffer;
	}

	bool Device::EndSingleTimeCommands(VkCommandBuffer command_buffer) {
		if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
			return false;
		}

		VkSubmitInfo submit_info = {};

		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		if (vkQueueSubmit(
			this->graphics_queue,
			1,
			&submit_info,
			VK_NULL_HANDLE
		) != VK_SUCCESS) {
			return false;
		}

		if (vkQueueWaitIdle(this->graphics_queue) != VK_SUCCESS) {
			return false;
		}

		vkFreeCommandBuffers(
			this->device,
			this->command_pool,
			1,
			&command_buffer
		);

		return true;
	}

	bool Device::IsPhysicalDeviceSuitable(const VkPhysicalDevice& device) {
		VkPhysicalDeviceProperties propeties;
		vkGetPhysicalDeviceProperties(device, &propeties);

		if (propeties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			return false;
		}

		QueueFamiliyIndices indices = GetFamilyIndices(device);
		if (!indices.graphics || !indices.present) {
			return false;
		}

		if (!SupportsRequiredExtentions(device)) {
			return false;
		}

		SwapChainSupport swapchain_support = GetSwapChainSupport(device);
		if (
			swapchain_support.formats.empty() ||
			swapchain_support.present_modes.empty()
			) {
			return false;
		}

		VkPhysicalDeviceFeatures supported_features;
		vkGetPhysicalDeviceFeatures(device, &supported_features);
		if (supported_features.samplerAnisotropy == VK_FALSE) {
			return false;
		}

		return true;
	}

	bool Device::SupportsRequiredExtentions(const VkPhysicalDevice& device) {
		uint32_t extension_count = 0;
		vkEnumerateDeviceExtensionProperties(
			device,
			nullptr,
			&extension_count,
			nullptr
		);
		if (extension_count == 0) {
			return false;
		}

		std::vector<VkExtensionProperties> available_extension_properties(extension_count);
		vkEnumerateDeviceExtensionProperties(
			device,
			nullptr,
			&extension_count,
			available_extension_properties.data()
		);

		std::vector<const char*> avail_extentions = {};
		for (const VkExtensionProperties& extention : available_extension_properties) {
			uint32_t extention_name_size = strlen(extention.extensionName) + 1;
			char* extention_name = new char[extention_name_size];

			strcpy_s(
				extention_name,
				sizeof(char) * extention_name_size,
				extention.extensionName
			);

			avail_extentions.push_back(extention_name);
		}

		for (const char* required_extention : device_extensions) {
			bool found = false;

			for (const char* avail_extention : avail_extentions) {
				if (!strcmp(required_extention, avail_extention)) {
					found = true;
				}
			}

			if (!found) {
				return false;
			}
		}

		return true;
	}

	QueueFamiliyIndices Device::GetFamilyIndices(const VkPhysicalDevice& device) const {
		QueueFamiliyIndices indices = QueueFamiliyIndices();

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(
			device,
			&queue_family_count,
			nullptr
		);
		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(
			device,
			&queue_family_count,
			queue_families.data()
		);

		for (uint32_t i = 0; i < queue_family_count; i++) {
			VkQueueFamilyProperties queue_family = queue_families.at(i);

			if (!indices.graphics) {
				if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indices.graphics_index = i;
					indices.graphics = true;
				}
			}

			if (!indices.present) {
				VkBool32 present_support = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(
					device,
					i,
					this->surface,
					&present_support
				);

				if (present_support) {
					indices.present_index = i;
					indices.present = true;
				}
			}

			if (indices.graphics && indices.present) {
				break;
			}
		}

		return indices;
	}

	SwapChainSupport Device::GetSwapChainSupport(const VkPhysicalDevice& device) const {
		SwapChainSupport swapchain_support;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			device,
			this->surface,
			&swapchain_support.capabilities
		);

		uint32_t format_count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			device,
			this->surface,
			&format_count,
			nullptr
		);
		if (format_count != 0) {
			swapchain_support.formats.resize(format_count);

			vkGetPhysicalDeviceSurfaceFormatsKHR(
				device,
				this->surface,
				&format_count,
				swapchain_support.formats.data()
			);
		}

		uint32_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device,
			this->surface,
			&present_mode_count,
			nullptr
		);
		if (present_mode_count != 0) {
			swapchain_support.present_modes.resize(present_mode_count);

			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device,
				this->surface,
				&present_mode_count,
				swapchain_support.present_modes.data()
			);
		}

		return swapchain_support;
	}


	bool Device::CreateInstance() {
		VkApplicationInfo app_info = {};
		
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.apiVersion = VK_API_VERSION_1_0;

		app_info.pApplicationName = this->name.c_str();
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

		app_info.pEngineName = "Yibengine";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		
		VkInstanceCreateInfo create_info = {};

		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;

		std::vector<const char*> required_extentions = GetRequiredExtentions();

		create_info.enabledExtensionCount = required_extentions.size();
		create_info.ppEnabledExtensionNames = required_extentions.data();

		if (this->enable_validation_layers) {
			create_info.enabledLayerCount = this->validation_layers.size();
			create_info.ppEnabledLayerNames = this->validation_layers.data();

			create_info.pNext = nullptr;
		} else {
			create_info.enabledLayerCount = 0;
			create_info.pNext = nullptr;
		}

		if (vkCreateInstance(
			&create_info,
			nullptr,
			&this->instance
		) != VK_SUCCESS) {
			return false;
		}

		std::vector<const char*> avail_extentions = GetAvailExtentions();
		for (const char* required_extention : required_extentions) {
			bool found = false;

			for (const char* avail_extention : avail_extentions) {
				if (!strcmp(required_extention, avail_extention)) {
					found = true;
					break;
				}
			}

			if (!found) {
				return false;
			}
		}

		return true;
	}

	void Device::DestoryInstance() const {
		if (this->instance == VK_NULL_HANDLE) {
			return;
		}

		vkDestroyInstance(this->instance, NULL);
	}


	bool Device::CreateDebugCallback() {
		VkDebugUtilsMessengerCreateInfoEXT create_info = {};

		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity = 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = &DebugCallback;
		create_info.pUserData = nullptr;

		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(this->instance, "vkCreateDebugUtilsMessengerEXT");
		if (vkCreateDebugUtilsMessengerEXT == VK_NULL_HANDLE) {
			return false;
		}

		if (vkCreateDebugUtilsMessengerEXT(
			this->instance,
			&create_info,
			nullptr,
			&this->debug_messenger
		) != VK_SUCCESS) {
			return false;
		}

		return true;
	}

	bool Device::DestroyDebugCallback() const {
		if (this->debug_messenger == VK_NULL_HANDLE) {
			return false;
		}

		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->instance, "vkDestroyDebugUtilsMessengerEXT");
		if (vkDestroyDebugUtilsMessengerEXT == VK_NULL_HANDLE) {
			return false;
		}

		vkDestroyDebugUtilsMessengerEXT(
			this->instance,
			this->debug_messenger,
			nullptr
		);

		return true;
	}


	bool Device::CreateSurface() {
		if (glfwCreateWindowSurface(
			this->instance,
			this->window->GetInternal(),
			nullptr,
			&this->surface
		) != VK_SUCCESS) {
			return false;
		}

		return true;
	}

	bool Device::DestorySurface() const {
		if (this->surface == VK_NULL_HANDLE) {
			return false;
		}

		PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(this->instance, "vkDestroySurfaceKHR");
		if (vkDestroySurfaceKHR == VK_NULL_HANDLE) {
			return false;
		}

		vkDestroySurfaceKHR(
			this->instance,
			this->surface,
			nullptr
		);

		return true;
	}


	bool Device::SelectPhysicalDevice() {
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(
			this->instance,
			&device_count,
			nullptr
		);
		if (device_count == 0) {
			return false;
		}

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(
			this->instance,
			&device_count,
			devices.data()
		);

		for (const VkPhysicalDevice& device : devices) {
			if (!IsPhysicalDeviceSuitable(device)) {
				continue;
			}

			this->physical_device = device;
			break;
		}

		if (this->physical_device == VK_NULL_HANDLE) {
			return false;
		}

		return true;
	}


	bool Device::CreateLogicalDevice() {
		QueueFamiliyIndices indices = GetFamilyIndices(this->physical_device);

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {};
		std::vector<uint32_t> unique_queue_families = {
			indices.graphics_index,
			indices.present
		};

		float queue_priority = 0.0f;
		for (uint32_t queue_family : unique_queue_families) {
			VkDeviceQueueCreateInfo queue_create_info = {};

			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;

			queue_create_infos.push_back(queue_create_info);
		}

		VkPhysicalDeviceFeatures device_features = {};
		device_features.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		create_info.queueCreateInfoCount = queue_create_infos.size();
		create_info.pQueueCreateInfos = queue_create_infos.data();

		create_info.pEnabledFeatures = &device_features;
		create_info.enabledExtensionCount = this->device_extensions.size();
		create_info.ppEnabledExtensionNames = this->device_extensions.data();

		if (this->enable_validation_layers) {
			create_info.enabledLayerCount = validation_layers.size();
			create_info.ppEnabledLayerNames = validation_layers.data();
		} else {
			create_info.enabledLayerCount = 0;
		}

		if (vkCreateDevice(
			this->physical_device,
			&create_info,
			nullptr,
			&this->device
		) != VK_SUCCESS) {
			return false;
		}

		vkGetDeviceQueue(
			this->device,
			indices.graphics_index,
			NULL,
			&this->graphics_queue
		);

		vkGetDeviceQueue(
			this->device,
			indices.present_index,
			NULL,
			&this->present_queue
		);

		return true;
	}

	void Device::DestroyLogicalDevice() const {
		if (this->device == VK_NULL_HANDLE) {
			return;
		}

		vkDestroyDevice(this->device, nullptr);
	}


	bool Device::CreateCommandPool() {
		QueueFamiliyIndices indices = GetFamilyIndices(this->physical_device);

		VkCommandPoolCreateInfo pool_info = {};

		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = indices.graphics_index;
		pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(
			this->device,
			&pool_info,
			nullptr,
			&this->command_pool
		) != VK_SUCCESS) {
			return false;
		}

		return true;
	}

	void Device::DestroyCommandPool() const {
		if (this->command_pool == VK_NULL_HANDLE) {
			return;
		}

		vkDestroyCommandPool(
			this->device,
			this->command_pool,
			nullptr
		);
	}


	std::vector<const char*> Device::GetAvailExtentions() const {
		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(
			nullptr,
			&extension_count,
			nullptr
		);

		std::vector<VkExtensionProperties> extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(
			nullptr,
			&extension_count,
			extensions.data()
		);

		std::vector<const char*> result = {};
		for (const VkExtensionProperties& extention : extensions) {
			uint32_t extention_name_size = strlen(extention.extensionName) + 1;
			char* extention_name = new char[extention_name_size];

			strcpy_s(
				extention_name,
				sizeof(char) * extention_name_size,
				extention.extensionName
			);

			result.push_back(extention_name);
		}

		return result;
	}

	std::vector<const char*> Device::GetRequiredExtentions() const {
		uint32_t glfw_extention_count = 0;
		const char** glfw_extentions = glfwGetRequiredInstanceExtensions(&glfw_extention_count);

		std::vector<const char*> extantions = std::vector<const char*>(
			glfw_extentions,
			glfw_extentions + glfw_extention_count
		);

		if (enable_validation_layers) {
			extantions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extantions;
	}
}