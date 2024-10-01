#include "camera.h"

#include <limits>

namespace yib {
	Camera::Camera() : view({1.0f}), projection({ 1.0f }) { }


	const glm::mat4& Camera::GetViewMatrix() const {
		return this->view;
	}

	const glm::mat4& Camera::GetProjectionMatrix() const {
		return this->projection;
	}


	void Camera::SetOrthographicProjection(
		float left,
		float right,
		float top,
		float bottom,
		float near,
		float far
	) {
		this->projection = glm::mat4{ 1.0f };

		this->projection[0][0] = 2.f / (right - left);
		this->projection[1][1] = 2.f / (bottom - top);
		this->projection[2][2] = 1.f / (far - near);
		this->projection[3][0] = -(right + left) / (right - left);
		this->projection[3][1] = -(bottom + top) / (bottom - top);
		this->projection[3][2] = -near / (far - near);
	}

	bool Camera::SetPerspectiveProjection(
		float yfov,
		float aspect,
		float near,
		float far
	) {
		if (!(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f)) {
			return false;
		}
		
		const float tan_half_fovy = tan(yfov / 2.f);

		this->projection = glm::mat4{ 0.0f };

		this->projection[0][0] = 1.f / (aspect * tan_half_fovy);
		this->projection[1][1] = 1.f / (tan_half_fovy);
		this->projection[2][2] = far / (far - near);
		this->projection[2][3] = 1.f;
		this->projection[3][2] = -(far * near) / (far - near);

		return true;
	}


	void Camera::SetViewDirection(
		glm::vec3 position,
		glm::vec3 direction,
		glm::vec3 up
	) {
		const glm::vec3 w =  glm::normalize(direction);
		const glm::vec3 u = glm::normalize(glm::cross(w, up));
		const glm::vec3 v = glm::cross(w, u);

		this->view = glm::mat4{ 1.f };

		this->view[0][0] = u.x;
		this->view[1][0] = u.y;
		this->view[2][0] = u.z;
		this->view[0][1] = v.x;
		this->view[1][1] = v.y;
		this->view[2][1] = v.z;
		this->view[0][2] = w.x;
		this->view[1][2] = w.y;
		this->view[2][2] = w.z;
		this->view[3][0] = -glm::dot(u, position);
		this->view[3][1] = -glm::dot(v, position);
		this->view[3][2] = -glm::dot(w, position);
	}

	void Camera::SetViewYXZ(
		glm::vec3 position,
		glm::vec3 rotation
	) {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);

		const glm::vec3 u = {
			(c1 * c3 + s1 * s2 * s3),
			(c2 * s3),
			(c1 * s2 * s3 - c3 * s1)
		};
		const glm::vec3 v = {
			(c3 * s1 * s2 - c1 * s3),
			(c2 * c3),
			(c1 * c3 * s2 + s1 * s3)
		};
		const glm::vec3 w = {
			(c2 * s1),
			(-s2),
			(c1 * c2)
		};

		this->view = glm::mat4{ 1.f };

		this->view[0][0] = u.x;
		this->view[1][0] = u.y;
		this->view[2][0] = u.z;
		this->view[0][1] = v.x;
		this->view[1][1] = v.y;
		this->view[2][1] = v.z;
		this->view[0][2] = w.x;
		this->view[1][2] = w.y;
		this->view[2][2] = w.z;
		this->view[3][0] = -glm::dot(u, position);
		this->view[3][1] = -glm::dot(v, position);
		this->view[3][2] = -glm::dot(w, position);
	}

	void Camera::SetViewTarget(
		glm::vec3 position,
		glm::vec3 target,
		glm::vec3 up
	) {
		SetViewDirection(position, target - position, up);
	}
}