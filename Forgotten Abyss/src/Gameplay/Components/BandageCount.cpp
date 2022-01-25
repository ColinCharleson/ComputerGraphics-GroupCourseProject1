#include "Gameplay/Components/BandageCount.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"

#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
void BandageCount::Awake()
{
}

void BandageCount::RenderImGui() {
}

nlohmann::json BandageCount::ToJson() const {
	return {
		
	};
}

BandageCount::BandageCount() :
	IComponent(),
	_impulse(10.0f)
{ }

BandageCount::~BandageCount() = default;

BandageCount::Sptr BandageCount::FromJson(const nlohmann::json& blob) {
	BandageCount::Sptr result = std::make_shared<BandageCount>();

	return result;
}

extern int bandageCount;
void BandageCount::Update(float deltaTime) {

	if (bandageCount == 2)
		GetGameObject()->Get<GuiText>()->SetText("2");
	if (bandageCount == 1)
		GetGameObject()->Get<GuiText>()->SetText("1");
	if (bandageCount == 0)
		GetGameObject()->Get<GuiText>()->SetText("0");

}

