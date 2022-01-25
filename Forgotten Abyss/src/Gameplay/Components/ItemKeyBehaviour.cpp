#include "Gameplay/Components/ItemKeyBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"

void KeyBehaviour::Awake()
{
}

void KeyBehaviour::RenderImGui() {
}

nlohmann::json KeyBehaviour::ToJson() const {
	return {
		
	};
}

KeyBehaviour::KeyBehaviour() :
	IComponent(),
	_impulse(10.0f)
{ }

KeyBehaviour::~KeyBehaviour() = default;

KeyBehaviour::Sptr KeyBehaviour::FromJson(const nlohmann::json& blob) {
	KeyBehaviour::Sptr result = std::make_shared<KeyBehaviour>();

	return result;
}
extern float playerX, playerY;
extern bool hasKey;
extern float ammoCount, playerHealth, bandageCount;

void KeyBehaviour::Update(float deltaTime) {


	bool pressed = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_E);
	if (pressed) 
	{
		if (_isPressed == false) 
		{
			if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y -  playerY, 2) * 2)) <= 2)
			{
				if (hasKey == false)
				{
					std::cout << "Picked up key" << std::endl;
					GetGameObject()->GetScene()->RemoveGameObject(GetGameObject()->SelfRef());
					hasKey = true;
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

