#pragma once

#include <string>
#include <vector>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include "object.h"
#include "renderer/model.h"
#include "renderer/camera.h"
#include "renderer/device.h"
#include "renderer/window.h"
#include "renderer/renderer.h"
#include "renderer/render_system.h"

namespace yib {
	class Client {
	public:
		Client(
			const std::string name,
			const uint32_t width,
			const uint32_t height
		);

		Client(const Client&) = delete;
		Client& operator=(const Client&) = delete;

		void Run();
		bool Running() const;
		
		bool success;
	private:
		// TODO: Remove this
		bool CreateObjects();

		bool running;
		const std::string name;
		const uint32_t width;
		const uint32_t height;

		Window window;
		Device device;
		Renderer renderer;
		RenderSystem render_system;

		// TODO: Remove this
		std::vector<std::shared_ptr<Object>> objects;
	};
}