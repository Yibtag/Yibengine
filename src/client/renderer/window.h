#pragma once

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace yib {
	class Window {
	public:
		Window(
			const std::string title,
			uint32_t width,
			uint32_t height
		);
		~Window();

		void Run();
		bool Running();

		void ResetResized();
		bool GetResized() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

		GLFWwindow* GetInternal();

		bool success;
	private:
		static void ResizeCallback(GLFWwindow* window, int width, int height);

		std::string title;
		bool resized;
		uint32_t width;
		uint32_t height;

		GLFWwindow* window = nullptr;
	};
}