#pragma once

// TODO: Remove this entire class and reimplement as an ecs(entity component system)

#include <memory>

#include "renderer/model.h"

namespace yib {
	class Transform {
	public:
		Transform();

		glm::mat4 GetMatrix() const;

		glm::vec3 scale;
		glm::vec3 rotation;
		glm::vec3 translation;
	};

	class Object {
	public:
		std::shared_ptr<Model> model;
		Transform transform;
	};
}