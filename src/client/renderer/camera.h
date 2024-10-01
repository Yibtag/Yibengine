#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace yib {
	class Camera {
	public:
		Camera();

		const glm::mat4& GetViewMatrix() const;
		const glm::mat4& GetProjectionMatrix() const;

		void SetOrthographicProjection(
			float left,
			float right,
			float top,
			float bottom,
			float near,
			float far
		);
		bool SetPerspectiveProjection(
			float yfov,
			float aspect,
			float near,
			float far
		);

		void SetViewDirection(
			glm::vec3 position,
			glm::vec3 direction,
			glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f)
		);

		void SetViewYXZ(
			glm::vec3 position,
			glm::vec3 rotation
		);

		void SetViewTarget(
			glm::vec3 position,
			glm::vec3 target,
			glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f)
		);
	private:
		glm::mat4 view;
		glm::mat4 projection;
	};
}