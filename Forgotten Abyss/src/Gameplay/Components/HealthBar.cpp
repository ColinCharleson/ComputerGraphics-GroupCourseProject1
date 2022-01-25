#include "Gameplay/Components/HealthBar.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"

void HealthBehaviour::Awake()
{
}

void HealthBehaviour::RenderImGui() {
}

nlohmann::json HealthBehaviour::ToJson() const {
	return {
		
	};
}

HealthBehaviour::HealthBehaviour() :
	IComponent(),
	_impulse(10.0f)
{ }

HealthBehaviour::~HealthBehaviour() = default;

HealthBehaviour::Sptr HealthBehaviour::FromJson(const nlohmann::json& blob) {
	HealthBehaviour::Sptr result = std::make_shared<HealthBehaviour>();

	return result;
}

extern int playerHealth;
void HealthBehaviour::Update(float deltaTime) {
	Gameplay::IComponent::Sptr ptr = threeHealth.lock();
	if (playerHealth == 3)
	{
		ptr->IsEnabled = true;
	}
	else
	{
		ptr->IsEnabled = false;
	}
}

