#include "Gameplay/Components/ItemBandageBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"

void BandageBehaviour::Awake()
{
}

void BandageBehaviour::RenderImGui() {
}

nlohmann::json BandageBehaviour::ToJson() const {
	return {
		
	};
}

BandageBehaviour::BandageBehaviour() :
	IComponent(),
	_impulse(10.0f)
{ }

BandageBehaviour::~BandageBehaviour() = default;

BandageBehaviour::Sptr BandageBehaviour::FromJson(const nlohmann::json& blob) {
	BandageBehaviour::Sptr result = std::make_shared<BandageBehaviour>();

	return result;
}
extern float playerX, playerY;
extern bool hasKey;
extern int ammoCount, playerHealth, bandageCount;

void BandageBehaviour::Update(float deltaTime) {
	bool pressed = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_E);
	if (pressed) 
	{
		if (_isPressed == false) 
		{
			if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y -  playerY, 2) * 2)) <= 2)
			{
				if (bandageCount <= 2)
				{
					GetGameObject()->GetScene()->RemoveGameObject(GetGameObject()->SelfRef());
					bandageCount += 1;
					std::cout << "Bandage count: " << bandageCount << std::endl;
				}
			}
		}
		_isPressed = pressed;
	} 
	else 
	{
		_isPressed = false;
	}
}

