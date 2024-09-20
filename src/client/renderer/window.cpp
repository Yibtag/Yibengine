#include "window.h"

namespace yib {
	Window::Window(
		const std::string title,
		uint32_t width,
		uint32_t height
	) : title(title), width(width), height(height), resized(false), success(false) {
		if (!glfwInit()) {
			return;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		this->window = glfwCreateWindow(
			this->width,
			this->height,
			this->title.c_str(),
			nullptr,
			nullptr
		);
		if (this->window == nullptr) {
			return;
		}

		if (glfwVulkanSupported() == GLFW_FALSE) {
			return;
		}

		glfwSetWindowUserPointer(this->window, this);
		glfwSetFramebufferSizeCallback(this->window, ResizeCallback);

		this->success = true;
	}

	Window::~Window() {
		glfwDestroyWindow(this->window);
		glfwTerminate();
	}

	void Window::Run() {
		glfwPollEvents();
	}

	bool Window::Running() {
		return !glfwWindowShouldClose(this->window);
	}

	void Window::ResetResized() {
		this->resized = false;
	}

	bool Window::GetResized() const {
		return this->resized;
	}

	uint32_t Window::GetWidth() const {
		return this->width;
	}

	uint32_t Window::GetHeight() const {
		return this->height;
	}

	GLFWwindow* Window::GetInternal() {
		return this->window;
	}


	void Window::ResizeCallback(GLFWwindow* window, int width, int height) {
		Window* engine_window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

		engine_window->resized = true;
		engine_window->width = width;
		engine_window->height = height;
	}
}