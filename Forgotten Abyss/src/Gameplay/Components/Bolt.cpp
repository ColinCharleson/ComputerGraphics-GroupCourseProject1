#include "Gameplay/Components/Bolt.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"

void Bolt::Awake()
{
}

void Bolt::RenderImGui()
{
}

nlohmann::json Bolt::ToJson() const
{
	return {

	};
}

Bolt::Bolt() :
	IComponent(),
	_impulse(10.0f)
{
}

Bolt::~Bolt() = default;

Bolt::Sptr Bolt::FromJson(const nlohmann::json & blob)
{
	Bolt::Sptr result = std::make_shared<Bolt>();

	return result;
}
extern int ammoCount;
extern float playerX, playerY;
extern int playerHealth;
extern float boltX, boltY, boltZ;
extern bool arrowOut;
extern glm::quat currentRot;
float fireTime = 0;
glm::quat firedRot;

extern bool onMenu;
extern bool canShoot;
extern bool gamePaused;
void Bolt::Update(float deltaTime)
{
	if (gamePaused == false)
	{
		boltX = GetGameObject()->GetPosition().x;
		boltY = GetGameObject()->GetPosition().y;
		boltZ = GetGameObject()->GetPosition().z;
		if (playerHealth > 0)
		{
			bool pressed = glfwGetMouseButton(GetGameObject()->GetScene()->Window, GLFW_MOUSE_BUTTON_1);
			if (ammoCount > 0)
			{
				if (canShoot == true)
				{
					if (arrowOut == false)
					{
						if (pressed)
						{
							if (_isPressed == false)
							{
								arrowOut = true;
								firedRot = currentRot;
								fireTime = 3;
							}
							_isPressed = pressed;
						}
						else
						{
							_isPressed = false;
						}
					}
				}
			}


			if (arrowOut == false)
			{
				GetGameObject()->SetPostion(glm::vec3(playerX, playerY, 0.6f));
				GetGameObject()->SetRotation(currentRot);
			}
			else if (arrowOut == true)
			{
				glm::vec3 worldMovement = firedRot * glm::vec3(0, 30 * deltaTime, 0);
				worldMovement.z = 0;
				GetGameObject()->SetPostion(GetGameObject()->GetPosition() + worldMovement);
			}
			if (fireTime > 0)
			{
				fireTime -= 1 * deltaTime;
			}
			else
			{
				if (arrowOut == true)
					ammoCount -= 1;

				arrowOut = false;
			}

			if (gamePaused == true)
			{
				GetGameObject()->SetScale(glm::vec3(0));
			}
			else if (gamePaused == false)
			{
				GetGameObject()->SetScale(glm::vec3(0.3));
			}


			if (ammoCount == 0)
			{
				GetGameObject()->SetScale(glm::vec3(0));
			}
			else
			{
				GetGameObject()->SetScale(glm::vec3(0.3));
			}
		}
	}
}

