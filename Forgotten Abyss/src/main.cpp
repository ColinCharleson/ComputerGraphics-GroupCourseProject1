#include <Logging.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <json.hpp>
#include <fstream>
#include <sstream>
#include <typeindex>
#include <optional>
#include <string>

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus) 

// Graphics
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture2D.h"
#include "Graphics/TextureCube.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include <Gameplay\Components\ItemKeyBehaviour.h>
#include <Gameplay\Components\ItemBandageBehaviour.h>
#include <Gameplay\Components\ItemAmmoBehaviour.h>
#include <Gameplay\Components\DoorBehaviour.h>
#include <Gameplay\Components\SlimeBehaviour.h>
#include <Gameplay\Components\PauseScreenBehaviour.h>
#include <Gameplay\Components\MenuScreenBehaviour.h>
#include <Gameplay\Components\HealthBar.h>
#include <Gameplay\Components\HealthBar2HP.h>
#include <Gameplay\Components\HealthBar1HP.h>
#include <Gameplay\Components\WinScreenBehaviour.h>
#include <Gameplay\Components\LoseScreenBehaviour.h>
#include <Gameplay\Components\NormalAmmoCount.h>
#include <Gameplay\Components\BandageCount.h>
#include <Gameplay\Components\SpikeBehaviour.h>
#include <Gameplay\Components\Bolt.h>
#include <Gameplay\Components\EnemyAI.h>
#include <Gameplay\Components\MorphMeshRenderer.h>

#include <Gameplay\Components\MorphAnimator.h>




// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/CapsuleCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Gameplay/Physics/EnemyPath.h"
#include "Gameplay/Physics/EnemyPathCatMull.h"
#include "Gameplay/Physics/EnemyPathBezeir.h"
#include "Graphics/DebugDraw.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream> //03
#include <string> //03

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <GLM/glm.hpp> //04
#include <glm/gtc/matrix_transform.hpp> //04
#include <time.h>

//#define STB_IMAGE_IMPLEMENTATION //07
//#include <stb_image.h> //07

using namespace std;

float playerX, playerY;
float boltX, boltY, boltZ;
bool arrowOut = false;
bool gameWin = false;
bool gamePaused = false;
bool onMenu = true;
bool canShoot = true;
bool hasKey = false, slimeSlow = false, levelComplete = false;
int ammoCount = 5, playerHealth = 3, bandageCount = 0;
int roomType, progressScore;
glm::quat currentRot;
//#define LOG_GL_NOTIFICATIONS

/*
	Handles debug messages from OpenGL
	https://www.khronos.org/opengl/wiki/Debug_Output#Message_Components
	@param source    Which part of OpenGL dispatched the message
	@param type      The type of message (ex: error, performance issues, deprecated behavior)
	@param id        The ID of the error or message (to distinguish between different types of errors, like nullref or index out of range)
	@param severity  The severity of the message (from High to Notification)
	@param length    The length of the message
	@param message   The human readable message from OpenGL
	@param userParam The pointer we set with glDebugMessageCallback (should be the game pointer)
*/
void GlDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	std::string sourceTxt;
	switch (source)
	{
	case GL_DEBUG_SOURCE_API: sourceTxt = "DEBUG"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceTxt = "WINDOW"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceTxt = "SHADER"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: sourceTxt = "THIRD PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION: sourceTxt = "APP"; break;
	case GL_DEBUG_SOURCE_OTHER: default: sourceTxt = "OTHER"; break;
	}
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW:          LOG_INFO("[{}] {}", sourceTxt, message); break;
	case GL_DEBUG_SEVERITY_MEDIUM:       LOG_WARN("[{}] {}", sourceTxt, message); break;
	case GL_DEBUG_SEVERITY_HIGH:         LOG_ERROR("[{}] {}", sourceTxt, message); break;
#ifdef LOG_GL_NOTIFICATIONS
	case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_INFO("[{}] {}", sourceTxt, message); break;
#endif
	default: break;
	}
}

// Stores our GLFW window in a global variable for now
GLFWwindow* window;
// The current size of our window in pixels
glm::ivec2 windowSize = glm::ivec2(1920, 1080);

// The title of our GLFW window
std::string windowTitle = "Forgotten Abyss";

// using namespace should generally be avoided, and if used, make sure it's ONLY in cpp files
using namespace Gameplay;
using namespace Gameplay::Physics;

// The scene that we will be rendering
Scene::Sptr scene = nullptr;

void GlfwWindowResizedCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	windowSize = glm::ivec2(width, height);
	if (windowSize.x * windowSize.y > 0)
	{
		scene->MainCamera->ResizeWindow(width, height);
	}
}

/// <summary>
/// Handles intializing GLFW, should be called before initGLAD, but after Logger::Init()
/// Also handles creating the GLFW window
/// </summary>
/// <returns>True if GLFW was initialized, false if otherwise</returns>
bool initGLFW()
{
	// Initialize GLFW
	if (glfwInit() == GLFW_FALSE)
	{
		LOG_ERROR("Failed to initialize GLFW");
		return false;
	}

	//Create a new GLFW window and make it current
	//window = glfwCreateWindow(windowSize.x, windowSize.y, windowTitle.c_str(), glfwGetPrimaryMonitor(), nullptr);
	window = glfwCreateWindow(windowSize.x, windowSize.y, windowTitle.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set our window resized callback
	glfwSetWindowSizeCallback(window, GlfwWindowResizedCallback);
	// Pass the window to the input engine and let it initialize itself
	InputEngine::Init(window);

	GuiBatcher::SetWindowSize(windowSize);

	return true;
}

/// <summary>
/// Handles initializing GLAD and preparing our GLFW window for OpenGL calls
/// </summary>
/// <returns>True if GLAD is loaded, false if there was an error</returns>
bool initGLAD()
{
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0)
	{
		LOG_ERROR("Failed to initialize Glad");
		return false;
	}
	return true;
}

/// <summary>
/// Draws a widget for saving or loading our scene
/// Draws a widget for saving or loading our scene
/// </summary>
/// <param name="scene">Reference to scene pointer</param>
/// <param name="path">Reference to path string storage</param>
/// <returns>True if a new scene has been loaded</returns>
bool DrawSaveLoadImGui(Scene::Sptr& scene, std::string& path)
{
	// Since we can change the internal capacity of an std::string,
	// we can do cool things like this!
	ImGui::InputText("Path", path.data(), path.capacity());

	// Draw a save button, and save when pressed
	if (ImGui::Button("Save"))
	{
		scene->Save(path);

		std::string newFilename = std::filesystem::path(path).stem().string() + "-manifest.json";
		ResourceManager::SaveManifest(newFilename);
	}
	ImGui::SameLine();

	// Load scene from file button
	if (ImGui::Button("Load"))
	{
		// Since it's a reference to a ptr, this will
		// overwrite the existing scene!
		scene = nullptr;

		std::string newFilename = std::filesystem::path(path).stem().string() + "-manifest.json";
		ResourceManager::LoadManifest(newFilename);
		scene = Scene::Load(path);

		return true;
	}
	return false;
}

/// <summary>
/// Draws some ImGui controls for the given light
/// </summary>
/// <param name="title">The title for the light's header</param>
/// <param name="light">The light to modify</param>
/// <returns>True if the parameters have changed, false if otherwise</returns>
bool DrawLightImGui(const Scene::Sptr& scene, const char* title, int ix)
{
	bool isEdited = false;
	bool result = false;
	Light& light = scene->Lights[ix];
	ImGui::PushID(&light); // We can also use pointers as numbers for unique IDs
	if (ImGui::CollapsingHeader(title))
	{
		isEdited |= ImGui::DragFloat3("Pos", &light.Position.x, 0.01f);
		isEdited |= ImGui::ColorEdit3("Col", &light.Color.r);
		isEdited |= ImGui::DragFloat("Range", &light.Range, 0.1f);

		result = ImGui::Button("Delete");
	}
	if (isEdited)
	{
		scene->SetShaderLight(ix);
	}

	ImGui::PopID();
	return result;
}

bool RoomFunction()
{

	double x = rand() / static_cast<double>(RAND_MAX + 1);

	roomType = 1 + static_cast<int>(x * (5.0f - 1.0f));

	if (progressScore < 1)
	{
		if (roomType == 1)
		{
			progressScore += 1;
			double x = rand() / static_cast<double>(RAND_MAX + 1);

			int roomVar = 1 + static_cast<int>(x * (4.0f - 1.0f));

			string levelToLoad;
			if (roomVar == 1)
			{
				levelToLoad = "One.json";
				std::cout << "trap 1 loaded";
			}
			else if (roomVar == 2)
			{
				levelToLoad = "One.json";
				std::cout << "trap 2 loaded";
			}
			else if (roomVar == 3)
			{
				levelToLoad = "One.json";
				std::cout << "trap 3 loaded";
			}

			scene = nullptr;

			std::string newFilename = std::filesystem::path(levelToLoad).stem().string() + "-manifest.json";
			ResourceManager::LoadManifest(newFilename);
			scene = Scene::Load(levelToLoad);

			scene->Window = window;
			scene->Awake();
			return true;
			// trap room
		}
		else if (roomType == 2)
		{
			progressScore += 2;

			double x = rand() / static_cast<double>(RAND_MAX + 1);

			int roomVar = 1 + static_cast<int>(x * (4.0f - 1.0f));

			string levelToLoad;
			if (roomVar == 1)
			{
				levelToLoad = "One.json";
				std::cout << "brawl 1 loaded";
			}
			else if (roomVar == 2)
			{
				levelToLoad = "One.json";
				std::cout << "brawl 2 loaded";
			}
			else if (roomVar == 3)
			{
				levelToLoad = "One.json";
				std::cout << "brawl 3 loaded";
			}

			scene = nullptr;

			std::string newFilename = std::filesystem::path(levelToLoad).stem().string() + "-manifest.json";
			ResourceManager::LoadManifest(newFilename);
			scene = Scene::Load(levelToLoad);

			scene->Window = window;
			scene->Awake();
			return true;
			// brawl room
		}
		else if (roomType == 3)
		{
			progressScore += 3;
			double x = rand() / static_cast<double>(RAND_MAX + 1);

			int roomVar = 1 + static_cast<int>(x * (4.0f - 1.0f));

			string levelToLoad;
			if (roomVar == 1)
			{
				levelToLoad = "One.json";
				std::cout << "combo 1 loaded";
			}
			else if (roomVar == 2)
			{
				levelToLoad = "One.json";
				std::cout << "combo 2 loaded";
			}
			else if (roomVar == 3)
			{
				levelToLoad = "One.json";
				std::cout << "combo 3 loaded";
			}

			scene = nullptr;

			std::string newFilename = std::filesystem::path(levelToLoad).stem().string() + "-manifest.json";
			ResourceManager::LoadManifest(newFilename);
			scene = Scene::Load(levelToLoad);

			scene->Window = window;
			scene->Awake();
			return true;
			// combo room
		}
		else if (roomType == 4)
		{
			progressScore += 0;
			double x = rand() / static_cast<double>(RAND_MAX + 1);

			int roomVar = 1 + static_cast<int>(x * (4.0f - 1.0f));

			string levelToLoad;
			if (roomVar == 1)
			{
				levelToLoad = "One.json";
				std::cout << "hall 1 loaded";
			}
			else if (roomVar == 2)
			{
				levelToLoad = "One.json";
				std::cout << "hall 2 loaded";
			}
			else if (roomVar == 3)
			{
				levelToLoad = "One.json";
				std::cout << "hall 3 loaded";
			}

			scene = nullptr;

			std::string newFilename = std::filesystem::path(levelToLoad).stem().string() + "-manifest.json";
			ResourceManager::LoadManifest(newFilename);
			scene = Scene::Load(levelToLoad);

			scene->Window = window;
			scene->Awake();
			return true;
			// hallway
		}
	}
	else
	{
		std::cout << "BOSS Room Loaded" << std::endl;

		string levelToLoad = "BossRoom.json";

		scene = nullptr;

		std::string newFilename = std::filesystem::path(levelToLoad).stem().string() + "-manifest.json";
		ResourceManager::LoadManifest(newFilename);
		scene = Scene::Load(levelToLoad);

		scene->Window = window;
		scene->Awake();
		return true;
		// load boss level
	}
	return false;
}





bool pausePressed;
int main()
{
	srand(time(NULL));
	Logger::Init(); // We'll borrow the logger from the toolkit, but we need to initialize it

	//Initialize GLFW
	if (!initGLFW())
		return 1;

	//Initialize GLAD
	if (!initGLAD())
		return 1;

	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GlDebugMessage, nullptr);

	// Initialize our ImGui helper
	ImGuiHelper::Init(window);

	// Initialize our resource manager
	ResourceManager::Init();

	// Register all our resource types so we can load them from manifest files
	ResourceManager::RegisterType<Texture2D>();
	ResourceManager::RegisterType<TextureCube>();
	ResourceManager::RegisterType<Shader>();
	ResourceManager::RegisterType<Material>();
	ResourceManager::RegisterType<MeshResource>();

	// Register all of our component types so we can load them from files
	ComponentManager::RegisterType<Camera>();
	ComponentManager::RegisterType<RenderComponent>();
	ComponentManager::RegisterType<RigidBody>();
	ComponentManager::RegisterType<TriggerVolume>();
	ComponentManager::RegisterType<RotatingBehaviour>();
	ComponentManager::RegisterType<JumpBehaviour>();
	ComponentManager::RegisterType<KeyBehaviour>();
	ComponentManager::RegisterType<AmmoBehaviour>();
	ComponentManager::RegisterType<Bolt>();
	ComponentManager::RegisterType<BandageBehaviour>();
	ComponentManager::RegisterType<SpikeBehaviour>();
	ComponentManager::RegisterType<SlimeBehaviour>();
	ComponentManager::RegisterType<DoorBehaviour>();
	ComponentManager::RegisterType<PauseScreen>();
	ComponentManager::RegisterType<MenuScreen>();
	ComponentManager::RegisterType<WinScreen>();
	ComponentManager::RegisterType<LoseScreen>();
	ComponentManager::RegisterType<MaterialSwapBehaviour>();
	ComponentManager::RegisterType<TriggerVolumeEnterBehaviour>();
	ComponentManager::RegisterType<SimpleCameraControl>();
	ComponentManager::RegisterType<EnemyBehaviour>();
	ComponentManager::RegisterType<EnemyPath>();
	ComponentManager::RegisterType<EnemyPathCatMull>();
	ComponentManager::RegisterType<EnemyPathBezeir>();
	ComponentManager::RegisterType<MorphMeshRenderer>();
	ComponentManager::RegisterType<Morphanimator>();

	ComponentManager::RegisterType<RectTransform>();
	ComponentManager::RegisterType<GuiPanel>();
	ComponentManager::RegisterType<GuiText>();
	ComponentManager::RegisterType<HealthBehaviour>();
	ComponentManager::RegisterType<HealthBehaviour2>();
	ComponentManager::RegisterType<HealthBehaviour1>();
	ComponentManager::RegisterType<NormalAmmo>();
	ComponentManager::RegisterType<BandageCount>();

	// GL states, we'll enable depth testing and backface fulling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene)
	{
		ResourceManager::LoadManifest("manifest.json");
		scene = Scene::Load("scene.json");

		// Call scene awake to start up all of our components
		scene->Window = window;
		scene->Awake();
	}
	else
	{
		// This time we'll have 2 different shaders, and share data between both of them using the UBO
		// This shader will handle reflective materials
		Shader::Sptr reflectiveShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shader.glsl" },
			{ ShaderPartType::Fragment, "shaders/frag_environment_reflective.glsl" }
		});

		// This shader handles our basic materials without reflections (cause they expensive)
		Shader::Sptr basicShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shader.glsl" },
			{ ShaderPartType::Fragment, "shaders/frag_blinn_phong_textured.glsl" }
		});

		MeshResource::Sptr spiderMesh = ResourceManager::CreateAsset<MeshResource>("SpiderMesh.obj");
		MeshResource::Sptr chestMesh = ResourceManager::CreateAsset<MeshResource>("Chest.obj"); //1
		Texture2D::Sptr    chestTexture = ResourceManager::CreateAsset<Texture2D>("textures/ChestTex.png");
		MeshResource::Sptr keyMesh = ResourceManager::CreateAsset<MeshResource>("Key.obj"); //2
		Texture2D::Sptr    keyTexture = ResourceManager::CreateAsset<Texture2D>("textures/KeyTex.png");
		MeshResource::Sptr bandageMesh = ResourceManager::CreateAsset<MeshResource>("Bandage.obj"); //3
		Texture2D::Sptr    bandageTexture = ResourceManager::CreateAsset<Texture2D>("textures/BandageTex.png");
		MeshResource::Sptr golemMesh = ResourceManager::CreateAsset<MeshResource>("GolemMesh.obj");
		MeshResource::Sptr bowMesh = ResourceManager::CreateAsset<MeshResource>("Crossbow.obj");
		Texture2D::Sptr    bowTexture = ResourceManager::CreateAsset<Texture2D>("textures/CrossbowTex.png");
		MeshResource::Sptr skeletonMesh = ResourceManager::CreateAsset<MeshResource>("SkeletonMesh.obj");
		Texture2D::Sptr    spiderTexture = ResourceManager::CreateAsset<Texture2D>("textures/SpiderTex.png");
		MeshResource::Sptr torchMesh = ResourceManager::CreateAsset<MeshResource>("Torch.obj");  //4
		Texture2D::Sptr    torchTexture = ResourceManager::CreateAsset<Texture2D>("textures/TorchTex.png");
		MeshResource::Sptr arrowMesh = ResourceManager::CreateAsset<MeshResource>("ArrowPick.obj");  // 5
		MeshResource::Sptr boltMesh = ResourceManager::CreateAsset<MeshResource>("ArrowBolt3.obj");  //6
		Texture2D::Sptr    arrowTexture = ResourceManager::CreateAsset<Texture2D>("textures/BakedArrow.png");
		Texture2D::Sptr    golemTexture = ResourceManager::CreateAsset<Texture2D>("textures/GolemTex.png");
		Texture2D::Sptr    skeletonTexture = ResourceManager::CreateAsset<Texture2D>("textures/SkeletonTex.png");
		MeshResource::Sptr finalBoss = ResourceManager::CreateAsset<MeshResource>("FinalBoss.obj");
		Texture2D::Sptr    bossTexture = ResourceManager::CreateAsset<Texture2D>("textures/FinalBoss.png");
		MeshResource::Sptr demonMesh = ResourceManager::CreateAsset<MeshResource>("demon.obj");
		Texture2D::Sptr    demonTexture = ResourceManager::CreateAsset<Texture2D>("textures/Demon2.png");

		MeshResource::Sptr wall1Mesh = ResourceManager::CreateAsset<MeshResource>("Wall1.obj");  //7
		MeshResource::Sptr wall2Mesh = ResourceManager::CreateAsset<MeshResource>("Wall2.obj"); //8
		MeshResource::Sptr wall3Mesh = ResourceManager::CreateAsset<MeshResource>("Wall3.obj");//9
		MeshResource::Sptr wall4Mesh = ResourceManager::CreateAsset<MeshResource>("Wall4our.obj");//9

		MeshResource::Sptr SpikeTrap = ResourceManager::CreateAsset<MeshResource>("Spike.obj"); //10
		Texture2D::Sptr    spikeTexture = ResourceManager::CreateAsset<Texture2D>("textures/Spike.png");
		MeshResource::Sptr slimeTrap = ResourceManager::CreateAsset<MeshResource>("Slime.obj"); //11
		Texture2D::Sptr    slimeTexture = ResourceManager::CreateAsset<Texture2D>("textures/Slime.png");

		Texture2D::Sptr    wallTexture = ResourceManager::CreateAsset<Texture2D>("textures/WallTexture.png");
		Texture2D::Sptr    wallTexture2 = ResourceManager::CreateAsset<Texture2D>("textures/WallTextureVar2.png");
		Texture2D::Sptr    wallTexture3 = ResourceManager::CreateAsset<Texture2D>("textures/WallTextureVar3.png");
		Texture2D::Sptr    wallTexture4 = ResourceManager::CreateAsset<Texture2D>("textures/WallTextureVar4.png");
		Texture2D::Sptr    wallTexture5 = ResourceManager::CreateAsset<Texture2D>("textures/WallTextureVar5.png");
		MeshResource::Sptr doormesh = ResourceManager::CreateAsset<MeshResource>("DoorModel.obj"); //12
		Texture2D::Sptr    doorTexture = ResourceManager::CreateAsset<Texture2D>("textures/DoorTexture.png");
		Texture2D::Sptr    floorTexture = ResourceManager::CreateAsset<Texture2D>("textures/Base.png");
		MeshResource::Sptr BedMesh = ResourceManager::CreateAsset<MeshResource>("Bed.obj");
		Texture2D::Sptr    Bed = ResourceManager::CreateAsset<Texture2D>("textures/BedTex.png");

		Texture2D::Sptr    healthBar = ResourceManager::CreateAsset<Texture2D>("textures/OperationHealthFull.png");
		Texture2D::Sptr    healthBar2 = ResourceManager::CreateAsset<Texture2D>("textures/Operation1ThirdHealth.png");
		Texture2D::Sptr    healthBar1 = ResourceManager::CreateAsset<Texture2D>("textures/Operation2thirdsHealth.png");
		Texture2D::Sptr    ammoHUD = ResourceManager::CreateAsset<Texture2D>("textures/AmmoHUD.png");

		MeshResource::Sptr ChainModel = ResourceManager::CreateAsset<MeshResource>("Chain.obj"); // 13
		Texture2D::Sptr    ChainTexture = ResourceManager::CreateAsset<Texture2D>("textures/ChainTex.png");
		MeshResource::Sptr Chain2Model = ResourceManager::CreateAsset<MeshResource>("Chain2.obj"); // 14
		Texture2D::Sptr    Chain2Texture = ResourceManager::CreateAsset<Texture2D>("textures/Chain2Tex.png");
		MeshResource::Sptr BonesModel = ResourceManager::CreateAsset<MeshResource>("Bones.obj"); // 15
		Texture2D::Sptr    BonesTexture = ResourceManager::CreateAsset<Texture2D>("textures/BoneTex.png");
		MeshResource::Sptr JailBarModel = ResourceManager::CreateAsset<MeshResource>("JailBar.obj"); // 16
		Texture2D::Sptr    JailBarTexture = ResourceManager::CreateAsset<Texture2D>("textures/JailTex.png");
		MeshResource::Sptr BarrelModel = ResourceManager::CreateAsset<MeshResource>("Barrel.obj"); // 17
		Texture2D::Sptr    BarrelTexture = ResourceManager::CreateAsset<Texture2D>("textures/BarrelTex.png");

		Texture2D::Sptr    pausePNG = ResourceManager::CreateAsset<Texture2D>("textures/UIPause2.png");
		Texture2D::Sptr    menuPNG = ResourceManager::CreateAsset<Texture2D>("textures/UIMenu2.png");
		Texture2D::Sptr    losePNG = ResourceManager::CreateAsset<Texture2D>("textures/UILose2.png");
		Texture2D::Sptr    winPNG = ResourceManager::CreateAsset<Texture2D>("textures/UIWin2.png");
		Texture2D::Sptr    Crosshair = ResourceManager::CreateAsset<Texture2D>("textures/CH.png");
		Texture2D::Sptr    Bandage = ResourceManager::CreateAsset<Texture2D>("textures/Bandaid.png");


		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/ocean/ocean.jpg");
		Shader::Sptr      skyboxShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/skybox_frag.glsl" }
		});

		// Create an empty scene
		scene = std::make_shared<Scene>();

		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap);
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		// Create our materials
		// This will be our box material, with no environment reflections

		
		Material::Sptr chainMaterial = ResourceManager::CreateAsset<Material>(reflectiveShader);
		{
			chainMaterial->Name = "chain1";
			chainMaterial->Set("u_Material.Diffuse", ChainTexture);
			chainMaterial->Set("u_Material.Shininess", 0.1f);
				
		}
		
		Material::Sptr chain2Material = ResourceManager::CreateAsset<Material>(reflectiveShader);
		{
			chainMaterial->Name = "chain2";
			chainMaterial->Set("u_Material.Diffuse", Chain2Texture);
			chainMaterial->Set("u_Material.Shininess", 0.1f);

		}

		
		Material::Sptr bonesMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			bonesMaterial->Name = "Bones";
			bonesMaterial->Set("u_Material.Diffuse", BonesTexture);
			bonesMaterial->Set("u_Material.Shininess", 0.1f);

		}
		

		Material::Sptr jailbarMaterial = ResourceManager::CreateAsset<Material>(reflectiveShader);
		{
			jailbarMaterial->Name = "JailBar";
			jailbarMaterial->Set("u_Material.Diffuse", JailBarTexture);
			jailbarMaterial->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr spikeMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			spikeMaterial->Name = "Spikes";
			spikeMaterial->Set("u_Material.Diffuse", spikeTexture);
			spikeMaterial->Set("u_Material.Shininess", 0.1f);

		}
		
		
		Material::Sptr barrelMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			barrelMaterial->Name = "Barrel";
			barrelMaterial->Set("u_Material.Diffuse", BarrelTexture);
			barrelMaterial->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr slimeMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			slimeMaterial->Name = "Barrel";
			slimeMaterial->Set("u_Material.Diffuse", slimeTexture);
			slimeMaterial->Set("u_Material.Shininess", 0.1f);

		}
	
		Material::Sptr wallMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			wallMaterial->Name = "Wall1";
			wallMaterial->Set("u_Material.Diffuse", wallTexture);
			wallMaterial->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr wallMaterial2 = ResourceManager::CreateAsset<Material>(basicShader);
		{
			wallMaterial2->Name = "Wall2";
			wallMaterial2->Set("u_Material.Diffuse", wallTexture2);
			wallMaterial2->Set("u_Material.Shininess", 0.1f);

		}
	
		Material::Sptr wallMaterial3 = ResourceManager::CreateAsset<Material>(basicShader);
		{
			wallMaterial3->Name = "Wall3";
			wallMaterial3->Set("u_Material.Diffuse", wallTexture3);
			wallMaterial3->Set("u_Material.Shininess", 0.1f);

		}

		Material::Sptr wallMaterial4 = ResourceManager::CreateAsset<Material>(basicShader);
		{
			wallMaterial4->Name = "Wall4";
			wallMaterial4->Set("u_Material.Diffuse", wallTexture4);
			wallMaterial4->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr wallMaterial5 = ResourceManager::CreateAsset<Material>(basicShader);
		{
			wallMaterial5->Name = "Wall5";
			wallMaterial5->Set("u_Material.Diffuse", wallTexture5);
			wallMaterial5->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr torchMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			torchMaterial->Name = "Torch";
			torchMaterial->Set("u_Material.Diffuse", torchTexture);
			torchMaterial->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr chestMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			chestMaterial->Name = "Chest";
			chestMaterial->Set("u_Material.Diffuse", chestTexture);
			chestMaterial->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr doorMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			doorMaterial->Name = "Door";
			doorMaterial->Set("u_Material.Diffuse", doorTexture);
			doorMaterial->Set("u_Material.Shininess", 0.1f);

		}
	
		Material::Sptr spiderMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			spiderMaterial->Name = "Spider";
			spiderMaterial->Set("u_Material.Diffuse", spiderTexture);
			spiderMaterial->Set("u_Material.Shininess", 0.1f);

		}
	
		Material::Sptr bossMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			bossMaterial->Name = "Boss";
			bossMaterial->Set("u_Material.Diffuse", bossTexture);
			bossMaterial->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr golemMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			golemMaterial->Name = "Golem";
			golemMaterial->Set("u_Material.Diffuse", golemTexture);
			golemMaterial->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr demonMaterial = ResourceManager::CreateAsset<Material>(reflectiveShader);
		{
			demonMaterial->Name = "Demon";
			demonMaterial->Set("u_Material.Diffuse", demonTexture);
			demonMaterial->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr arrowMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			arrowMaterial->Name = "Arrow";
			arrowMaterial->Set("u_Material.Diffuse", arrowTexture);
			arrowMaterial->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr keyMaterial = ResourceManager::CreateAsset<Material>(reflectiveShader);
		{
			keyMaterial->Name = "Key";
			keyMaterial->Set("u_Material.Diffuse", keyTexture);
			keyMaterial->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr bandageMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			bandageMaterial->Name = "Bandage";
			bandageMaterial->Set("u_Material.Diffuse", bandageTexture);
			bandageMaterial->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr bowMat = ResourceManager::CreateAsset<Material>(basicShader);
		{
			bowMat->Name = "Bow";
			bowMat->Set("u_Material.Diffuse", bowTexture);
			bowMat->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr skeletonMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			skeletonMaterial->Name = "Skeleton";
			skeletonMaterial->Set("u_Material.Diffuse", skeletonTexture);
			skeletonMaterial->Set("u_Material.Shininess", 0.1f);

		}
		
		Material::Sptr floorMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			floorMaterial->Name = "Floor";
			floorMaterial->Set("u_Material.Diffuse", floorTexture);
			floorMaterial->Set("u_Material.Shininess", 0.1f);

		}

		
		Material::Sptr BedMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			BedMaterial->Name = "Bed";
			BedMaterial->Set("u_Material.Diffuse", Bed);
			BedMaterial->Set("u_Material.Shininess", 0.1f);

		}



		// Create some lights for our scene
		scene->Lights.resize(4);
		scene->Lights[0].Position = glm::vec3(-1.0f, -9.0f, 2.0f);
		scene->Lights[0].Color = glm::vec3(0.8f, 0.2f, 0.0f);
		scene->Lights[0].Range = (0.3f);

		scene->Lights[1].Position = glm::vec3(-6.0f, -10.0f, 2.0f);
		scene->Lights[1].Color = glm::vec3(0.8f, 0.2f, 0.0f);
		scene->Lights[1].Range = (0.3f);

		scene->Lights[2].Position = glm::vec3(9.0f, -11.0f, 2.0f);
		scene->Lights[2].Color = glm::vec3(0.8f, 0.2f, 0.0f);
		scene->Lights[2].Range = (0.3f);


		scene->Lights[3].Position = glm::vec3(6.0f, 0.0f, 2.0f);
		scene->Lights[3].Color = glm::vec3(0.8f, 0.2f, 0.0f);
		scene->Lights[3].Range = (0.3f);

		// We'll create a mesh that is a simple plane that we can resize later
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

		// Set up the scene's camera
		GameObject::Sptr camera = scene->CreateGameObject("Main Camera");
		{
			camera->SetPostion(glm::vec3(1.0f, -1.0f, 1.0f));
			camera->LookAt(glm::vec3(1, -12, 1));

			camera->Add<SimpleCameraControl>();

			Camera::Sptr cam = camera->Add<Camera>();

			// Make sure that the camera is set as the scene's main camera!
			scene->MainCamera = cam;

			RenderComponent::Sptr renderer = camera->Add<RenderComponent>();
			renderer->SetMesh(bowMesh);
			renderer->SetMaterial(bowMat);

			RigidBody::Sptr PlayerTrigger = camera->Add<RigidBody>(RigidBodyType::Dynamic);
			PlayerTrigger->AddCollider(SphereCollider::Create(0.80));
			PlayerTrigger->SetLinearDamping(2.0f);

			//Crossbow Anims
			MorphMeshRenderer::Sptr camMorph1 = camera->Add<MorphMeshRenderer>();

			camMorph1->SetMorphMeshRenderer(bowMesh, bowMat);
			Morphanimator::Sptr camMorph2 = camera->Add<Morphanimator>();

			MeshResource::Sptr crossbowAnim1 = ResourceManager::CreateAsset<MeshResource>("Animations/Crossbow/CrossShoot_000001.obj");
			MeshResource::Sptr crossbowAnim2 = ResourceManager::CreateAsset<MeshResource>("Animations/Crossbow/CrossShoot_000018.obj");
			MeshResource::Sptr crossbowAnim3 = ResourceManager::CreateAsset<MeshResource>("Animations/Crossbow/CrossShoot_000020.obj");
			MeshResource::Sptr crossbowAnim4 = ResourceManager::CreateAsset<MeshResource>("Animations/Crossbow/CrossShoot_000030.obj");
			MeshResource::Sptr crossbowAnim5 = ResourceManager::CreateAsset<MeshResource>("Animations/Crossbow/CrossShoot_000050.obj");

			std::vector<MeshResource::Sptr> frames;
			frames.push_back(crossbowAnim1);
			frames.push_back(crossbowAnim2);
			frames.push_back(crossbowAnim3);
			frames.push_back(crossbowAnim4);
			frames.push_back(crossbowAnim5);

			camMorph2->SetInitial();
			camMorph2->SetFrameTime(0.9f);
			camMorph2->SetFrames(frames);

		}

		GameObject::Sptr playerBolt = scene->CreateGameObject("Bolt");
		{
			// Set position in the scene
			playerBolt->SetPostion(glm::vec3(0.89f, -1.17f, 0.97f));
			playerBolt->SetRotation(glm::vec3(90.0f, 0.0f, -91.0f));
			playerBolt->SetScale(glm::vec3(0.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = playerBolt->Add<RenderComponent>();
			renderer->SetMesh(boltMesh);
			renderer->SetMaterial(arrowMaterial);

			playerBolt->Add<Bolt>();
		}
		// Set up all our sample objects
		GameObject::Sptr floor = scene->CreateGameObject("Floor");
		{
			// Make a big tiled mesh
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(100.0f), glm::vec2(20.0f)));
			tiledMesh->GenerateMesh();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = floor->Add<RenderComponent>();
			renderer->SetMesh(tiledMesh);
			renderer->SetMaterial(floorMaterial);

			RigidBody::Sptr floorRB = floor->Add<RigidBody>(RigidBodyType::Static);
			floorRB->AddCollider(PlaneCollider::Create());
		}


		GameObject::Sptr Beding = scene->CreateGameObject("Bed");
		{

			Beding->SetPostion(glm::vec3(2.550f, -0.730f, -0.020f));
			Beding->SetRotation(glm::vec3(90.0f, 0.0f, -106.0f));
			Beding->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = Beding->Add<RenderComponent>();
			renderer->SetMesh(BedMesh);
			renderer->SetMaterial(BedMaterial);

		}
		GameObject::Sptr roof = scene->CreateGameObject("Roof");
		{
			roof->SetPostion(glm::vec3(0.0f, 0.0f, 2.5f));
			roof->SetRotation(glm::vec3(180.0f, 0.0f, 0.0f));
			// Make a big tiled mesh
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(100.0f), glm::vec2(20.0f)));
			tiledMesh->GenerateMesh();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = roof->Add<RenderComponent>();
			renderer->SetMesh(tiledMesh);
			renderer->SetMaterial(floorMaterial);
		}
		GameObject::Sptr chestKey = scene->CreateGameObject("Chest1");
		{
			// Set position in the scene
			chestKey->SetPostion(glm::vec3(-4.0f, -9.0f, 0.0f));
			chestKey->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
			chestKey->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chestKey->Add<RenderComponent>();
			renderer->SetMesh(chestMesh);
			renderer->SetMaterial(chestMaterial);

			RigidBody::Sptr floorRB = chestKey->Add<RigidBody>(RigidBodyType::Static);
			floorRB->AddCollider(BoxCollider::Create(glm::vec3(0.1f, 1, 0.1f)));
		}
		GameObject::Sptr keyObj = scene->CreateGameObject("key");
		{
			// Set position in the scene
			keyObj->SetPostion(glm::vec3(-4.0f, -9.0f, 0.0f));
			keyObj->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
			keyObj->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = keyObj->Add<RenderComponent>();
			renderer->SetMesh(keyMesh);
			renderer->SetMaterial(keyMaterial);

			keyObj->Add<KeyBehaviour>();
		}
		GameObject::Sptr chestBandage = scene->CreateGameObject("Chest2");
		{
			// Set position in the scene
			chestBandage->SetPostion(glm::vec3(8.0f, -11.0f, 0.0f));
			chestBandage->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
			chestBandage->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chestBandage->Add<RenderComponent>();
			renderer->SetMesh(chestMesh);
			renderer->SetMaterial(chestMaterial);

			RigidBody::Sptr floorRB = chestBandage->Add<RigidBody>(RigidBodyType::Static);
			floorRB->AddCollider(BoxCollider::Create(glm::vec3(0.1f, 1, 0.1f)));
		}
		GameObject::Sptr bandageObj = scene->CreateGameObject("bandage");
		{
			// Set position in the scene
			bandageObj->SetPostion(glm::vec3(8.0f, -11.0f, 0.0f));
			bandageObj->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
			bandageObj->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = bandageObj->Add<RenderComponent>();
			renderer->SetMesh(bandageMesh);
			renderer->SetMaterial(bandageMaterial);

			bandageObj->Add<BandageBehaviour>();
		}
		GameObject::Sptr chestAmmo1 = scene->CreateGameObject("Chest3");
		{
			// Set position in the scene
			chestAmmo1->SetPostion(glm::vec3(-4.0f, -3.0f, 0.0f));
			chestAmmo1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			chestAmmo1->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chestAmmo1->Add<RenderComponent>();
			renderer->SetMesh(chestMesh);
			renderer->SetMaterial(chestMaterial);

			RigidBody::Sptr floorRB = chestAmmo1->Add<RigidBody>(RigidBodyType::Static);
			floorRB->AddCollider(BoxCollider::Create(glm::vec3(0.1f, 1, 0.1f)));
		}
		GameObject::Sptr Ammo1 = scene->CreateGameObject("ammo pickup 1");
		{
			// Set position in the scene
			Ammo1->SetPostion(glm::vec3(-4.0f, -3.0f, 0.0f));
			Ammo1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			Ammo1->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = Ammo1->Add<RenderComponent>();
			renderer->SetMesh(arrowMesh);
			renderer->SetMaterial(arrowMaterial);

			Ammo1->Add<AmmoBehaviour>();
		}
		GameObject::Sptr chestAmmo2 = scene->CreateGameObject("Chest4");
		{
			// Set position in the scene
			chestAmmo2->SetPostion(glm::vec3(6.0f, -1.0f, 0.0f));
			chestAmmo2->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			chestAmmo2->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chestAmmo2->Add<RenderComponent>();
			renderer->SetMesh(chestMesh);
			renderer->SetMaterial(chestMaterial);

			RigidBody::Sptr floorRB = chestAmmo2->Add<RigidBody>(RigidBodyType::Static);
			floorRB->AddCollider(BoxCollider::Create(glm::vec3(0.3f, 1, 0.3f)));
		}
		GameObject::Sptr Ammo2 = scene->CreateGameObject("ammo pickup2");
		{
			// Set position in the scene
			Ammo2->SetPostion(glm::vec3(6.0f, -1.0f, 0.0f));
			Ammo2->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			Ammo2->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = Ammo2->Add<RenderComponent>();
			renderer->SetMesh(arrowMesh);
			renderer->SetMaterial(arrowMaterial);

			Ammo2->Add<AmmoBehaviour>();
		}
		GameObject::Sptr barrel1 = scene->CreateGameObject("Barrel");
		{
			// Set position in the scene
			barrel1->SetPostion(glm::vec3(5.5f, -11.5f, 0.0f));
			barrel1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = barrel1->Add<RenderComponent>();
			renderer->SetMesh(BarrelModel);
			renderer->SetMaterial(barrelMaterial);

			RigidBody::Sptr floorRB = barrel1->Add<RigidBody>(RigidBodyType::Static);
			floorRB->AddCollider(BoxCollider::Create(glm::vec3(0.1f, 1, 0.1f)));
		}
		GameObject::Sptr door = scene->CreateGameObject("Door");
		{
			// Set position in the scene
			door->SetPostion(glm::vec3(1.0f, -12.0f, 0.0f));
			door->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = door->Add<RenderComponent>();
			renderer->SetMesh(doormesh);
			renderer->SetMaterial(doorMaterial);

			door->Add<DoorBehaviour>();
		}
		GameObject::Sptr spike1 = scene->CreateGameObject("Spike Trap");
		{
			// Set position in the scene
			spike1->SetPostion(glm::vec3(0.0f, -3.0f, 0.0f));
			spike1->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = spike1->Add<RenderComponent>();
			renderer->SetMesh(SpikeTrap);
			renderer->SetMaterial(spikeMaterial);

			spike1->Add<SpikeBehaviour>();
		}
		GameObject::Sptr slime1 = scene->CreateGameObject("Slime Trap");
		{
			// Set position in the scene
			slime1->SetPostion(glm::vec3(2.0f, -9.0f, 0.0f));
			slime1->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = slime1->Add<RenderComponent>();
			renderer->SetMesh(slimeTrap);
			renderer->SetMaterial(slimeMaterial);

			slime1->Add<SlimeBehaviour>();
		}
		GameObject::Sptr chain1 = scene->CreateGameObject("chain1");
		{
			// Set position in the scene
			chain1->SetPostion(glm::vec3(0.0f, -7.0f, -0.1f));
			chain1->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chain1->Add<RenderComponent>();
			renderer->SetMesh(ChainModel);
			renderer->SetMaterial(chainMaterial);

		}
		GameObject::Sptr chain12 = scene->CreateGameObject("chain1");
		{
			// Set position in the scene

			chain12->SetPostion(glm::vec3(6.1f, -7.f, -0.1f));
			chain12->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chain12->Add<RenderComponent>();
			renderer->SetMesh(ChainModel);
			renderer->SetMaterial(chainMaterial);

		}
		GameObject::Sptr chain2 = scene->CreateGameObject("chain2");
		{
			// Set position in the scene
			chain2->SetPostion(glm::vec3(-6.0f, -7.0f, -0.1f));
			chain2->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chain2->Add<RenderComponent>();
			renderer->SetMesh(Chain2Model);
			renderer->SetMaterial(chain2Material);

		}

		GameObject::Sptr chain3 = scene->CreateGameObject("chain3");
		{
			// Set position in the scene
			chain3->SetPostion(glm::vec3(6.1f, -0.61f, -0.1f));
			chain3->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chain3->Add<RenderComponent>();
			renderer->SetMesh(Chain2Model);
			renderer->SetMaterial(chain2Material);

		}
		GameObject::Sptr chain22 = scene->CreateGameObject("chain22");
		{
			// Set position in the scene
			chain22->SetPostion(glm::vec3(4.0f, -7.0f, -0.1f));
			chain22->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chain22->Add<RenderComponent>();
			renderer->SetMesh(Chain2Model);
			renderer->SetMaterial(chain2Material);

		}
		GameObject::Sptr ribcage = scene->CreateGameObject("ribcage");
		{
			// Set position in the scene
			ribcage->SetPostion(glm::vec3(-6.5f, -5.0f, 0.0f));
			ribcage->SetRotation(glm::vec3(90.0f, 0.0f, 67.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = ribcage->Add<RenderComponent>();
			renderer->SetMesh(BonesModel);
			renderer->SetMaterial(bonesMaterial);

		}
		GameObject::Sptr ribcage2 = scene->CreateGameObject("ribcage2");
		{
			// Set position in the scene
			ribcage2->SetPostion(glm::vec3(6.5f, -2.0f, 0.0f));
			ribcage2->SetRotation(glm::vec3(90.0f, 0.0f, 67.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = ribcage2->Add<RenderComponent>();
			renderer->SetMesh(BonesModel);
			renderer->SetMaterial(bonesMaterial);


		}
		GameObject::Sptr ribcage3 = scene->CreateGameObject("ribcage3");
		{
			// Set position in the scene
			ribcage3->SetPostion(glm::vec3(6.5f, -6.0f, 0.0f));
			ribcage3->SetRotation(glm::vec3(90.0f, 0.0f, 10.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = ribcage3->Add<RenderComponent>();
			renderer->SetMesh(BonesModel);
			renderer->SetMaterial(bonesMaterial);

		}
		GameObject::Sptr ribcage4 = scene->CreateGameObject("ribcage4");
		{
			// Set position in the scene
			ribcage4->SetPostion(glm::vec3(6.5f, -10.0f, 0.0f));
			ribcage4->SetRotation(glm::vec3(90.0f, 0.0f, 30.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = ribcage4->Add<RenderComponent>();
			renderer->SetMesh(BonesModel);
			renderer->SetMaterial(bonesMaterial);

		}
		GameObject::Sptr jailbar1 = scene->CreateGameObject("jailbar1");
		{
			// Set position in the scene
			jailbar1->SetPostion(glm::vec3(3.16f, -7.85f, 0.0f));
			jailbar1->SetRotation(glm::vec3(90.0f, 0.0f, -7.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = jailbar1->Add<RenderComponent>();
			renderer->SetMesh(JailBarModel);
			renderer->SetMaterial(jailbarMaterial);
		}
		GameObject::Sptr jailbar2 = scene->CreateGameObject("jailbar2");
		{
			// Set position in the scene
			jailbar2->SetPostion(glm::vec3(5.05f, -7.98f, 0.0f));
			jailbar2->SetRotation(glm::vec3(90.0f, 0.0f, 67.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = jailbar2->Add<RenderComponent>();
			renderer->SetMesh(JailBarModel);
			renderer->SetMaterial(jailbarMaterial);
		}
		{//walls start///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			GameObject::Sptr wall1 = scene->CreateGameObject("wall");
			{
				// Set position in the scene
				wall1->SetPostion(glm::vec3(0.0f, 0.0f, 0.0f));
				wall1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall1->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB = wall1->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall2 = scene->CreateGameObject("Wall2");
			{
				// Set position in the scene
				wall2->SetPostion(glm::vec3(2.0f, 0.0f, 0.0f));
				wall2->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall2->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB1 = wall2->Add<RigidBody>(RigidBodyType::Static);
				floorRB1->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall3 = scene->CreateGameObject("Wall3");
			{
				// Set position in the scene
				wall3->SetPostion(glm::vec3(3.0f, -1.0f, 0.0f));
				wall3->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall3->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial5);

				RigidBody::Sptr floorRB3 = wall3->Add<RigidBody>(RigidBodyType::Static);
				floorRB3->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall4 = scene->CreateGameObject("Wall4");
			{
				// Set position in the scene
				wall4->SetPostion(glm::vec3(3.0f, -3.0f, 0.0f));
				wall4->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall4->Add<RenderComponent>();
				renderer->SetMesh(wall3Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB4 = wall4->Add<RigidBody>(RigidBodyType::Static);
				floorRB4->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall5 = scene->CreateGameObject("Wall5");
			{
				// Set position in the scene
				wall5->SetPostion(glm::vec3(3.0f, -5.0f, 0.0f));
				wall5->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall5->Add<RenderComponent>();
				renderer->SetMesh(wall4Mesh);
				renderer->SetMaterial(wallMaterial);


				RigidBody::Sptr floorRB5 = wall5->Add<RigidBody>(RigidBodyType::Static);
				floorRB5->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}

			GameObject::Sptr wall6 = scene->CreateGameObject("Wall6");
			{
				// Set position in the scene
				wall6->SetPostion(glm::vec3(-1.0f, -1.0f, 0.0f));
				wall6->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall6->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB6 = wall6->Add<RigidBody>(RigidBodyType::Static);
				floorRB6->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall7 = scene->CreateGameObject("Wall7");
			{
				// Set position in the scene
				wall7->SetPostion(glm::vec3(-1.0f, -3.0f, 0.0f));
				wall7->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall7->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial5);

				RigidBody::Sptr floorRB7 = wall7->Add<RigidBody>(RigidBodyType::Static);
				floorRB7->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall8 = scene->CreateGameObject("Wall8");
			{
				// Set position in the scene
				wall8->SetPostion(glm::vec3(-1.0f, -7.0f, 0.0f));
				wall8->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall8->Add<RenderComponent>();
				renderer->SetMesh(wall4Mesh);
				renderer->SetMaterial(wallMaterial);


				RigidBody::Sptr floorRB8 = wall8->Add<RigidBody>(RigidBodyType::Static);
				floorRB8->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall9 = scene->CreateGameObject("Wall9");
			{
				// Set position in the scene
				wall9->SetPostion(glm::vec3(0.0f, -12.0f, 0.0f));
				wall9->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall9->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB9 = wall9->Add<RigidBody>(RigidBodyType::Static);
				floorRB9->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall10 = scene->CreateGameObject("Wall10");
			{
				// Set position in the scene
				wall10->SetPostion(glm::vec3(2.0f, -12.0f, 0.0f));
				wall10->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall10->Add<RenderComponent>();
				renderer->SetMesh(wall4Mesh);
				renderer->SetMaterial(wallMaterial);


				RigidBody::Sptr floorRB10 = wall10->Add<RigidBody>(RigidBodyType::Static);
				floorRB10->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall11 = scene->CreateGameObject("Wall11");
			{
				// Set position in the scene
				wall11->SetPostion(glm::vec3(-1.0f, -9.0f, 0.0f));
				wall11->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall11->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial5);


				RigidBody::Sptr floorRB11 = wall11->Add<RigidBody>(RigidBodyType::Static);
				floorRB11->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr torchWall11 = scene->CreateGameObject("Torch 11 ");
			{
				// Set position in the scene
				torchWall11->SetPostion(glm::vec3(-1.0f, -9.0f, 0.0f));
				torchWall11->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = torchWall11->Add<RenderComponent>();
				renderer->SetMesh(torchMesh);
				renderer->SetMaterial(torchMaterial);

			}
			GameObject::Sptr torchWall32 = scene->CreateGameObject("Torch 32 ");
			{
				// Set position in the scene
				torchWall32->SetPostion(glm::vec3(-6.0f, -10.0f, 0.0f));
				torchWall32->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = torchWall32->Add<RenderComponent>();
				renderer->SetMesh(torchMesh);
				renderer->SetMaterial(torchMaterial);

			}
			GameObject::Sptr torchWall36 = scene->CreateGameObject("Torch 36 ");
			{
				// Set position in the scene
				torchWall36->SetPostion(glm::vec3(9.0f, -11.0f, 0.0f));
				torchWall36->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = torchWall36->Add<RenderComponent>();
				renderer->SetMesh(torchMesh);
				renderer->SetMaterial(torchMaterial);

			}
			GameObject::Sptr torchWall41 = scene->CreateGameObject("Torch 41 ");
			{
				// Set position in the scene
					// Set position in the scene
				torchWall41->SetPostion(glm::vec3(6.0f, 0.0f, 0.0f));
				torchWall41->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = torchWall41->Add<RenderComponent>();
				renderer->SetMesh(torchMesh);
				renderer->SetMaterial(torchMaterial);

			}
			GameObject::Sptr wall12 = scene->CreateGameObject("Wall12");
			{
				// Set position in the scene
				wall12->SetPostion(glm::vec3(-1.0f, -11.0f, 0.0f));
				wall12->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall12->Add<RenderComponent>();
				renderer->SetMesh(wall4Mesh);
				renderer->SetMaterial(wallMaterial);


				RigidBody::Sptr floorRB12 = wall12->Add<RigidBody>(RigidBodyType::Static);
				floorRB12->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));

			}
			GameObject::Sptr wall13 = scene->CreateGameObject("Wall13");
			{
				// Set position in the scene
				wall13->SetPostion(glm::vec3(3.0f, -11.0f, 0.0f));
				wall13->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall13->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial5);


				RigidBody::Sptr floorRB13 = wall13->Add<RigidBody>(RigidBodyType::Static);
				floorRB13->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall14 = scene->CreateGameObject("Wall14");
			{
				// Set position in the scene
				wall14->SetPostion(glm::vec3(3.0f, -9.0f, 0.0f));
				wall14->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall14->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);



				RigidBody::Sptr floorRB14 = wall14->Add<RigidBody>(RigidBodyType::Static);
				floorRB14->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall15 = scene->CreateGameObject("Wall15");
			{
				// Set position in the scene
				wall15->SetPostion(glm::vec3(-2.0f, -6.0f, 0.0f));
				wall15->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall15->Add<RenderComponent>();
				renderer->SetMesh(wall4Mesh);
				renderer->SetMaterial(wallMaterial2);


				RigidBody::Sptr floorRB15 = wall15->Add<RigidBody>(RigidBodyType::Static);
				floorRB15->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall16 = scene->CreateGameObject("Wall16");
			{
				// Set position in the scene
				wall16->SetPostion(glm::vec3(4.0f, -8.0f, 0.0f));
				wall16->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall16->Add<RenderComponent>();
				renderer->SetMesh(wall2Mesh);
				renderer->SetMaterial(wallMaterial);


				RigidBody::Sptr floorRB16 = wall16->Add<RigidBody>(RigidBodyType::Static);
				floorRB16->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall17 = scene->CreateGameObject("Wall17");
			{
				// Set position in the scene
				wall17->SetPostion(glm::vec3(-2.0f, -4.0f, 0.0f));
				wall17->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall17->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);


				RigidBody::Sptr floorRB17 = wall17->Add<RigidBody>(RigidBodyType::Static);
				floorRB17->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall18 = scene->CreateGameObject("Wall18");
			{
				// Set position in the scene
				wall18->SetPostion(glm::vec3(4.0f, -6.0f, 0.0f));
				wall18->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall18->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB18 = wall18->Add<RigidBody>(RigidBodyType::Static);
				floorRB18->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall19 = scene->CreateGameObject("Wall19");
			{
				// Set position in the scene
				wall19->SetPostion(glm::vec3(7.0f, -7.0f, 0.0f));
				wall19->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall19->Add<RenderComponent>();
				renderer->SetMesh(wall4Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB19 = wall19->Add<RigidBody>(RigidBodyType::Static);
				floorRB19->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall20 = scene->CreateGameObject("Wall20");
			{
				// Set position in the scene
				wall20->SetPostion(glm::vec3(7.0f, -9.0f, 0.0f));
				wall20->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall20->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB20 = wall20->Add<RigidBody>(RigidBodyType::Static);
				floorRB20->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall21 = scene->CreateGameObject("Wall21");
			{
				// Set position in the scene
				wall21->SetPostion(glm::vec3(7.0f, -5.0f, 0.0f));
				wall21->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall21->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);


				RigidBody::Sptr floorRB21 = wall21->Add<RigidBody>(RigidBodyType::Static);
				floorRB21->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall22 = scene->CreateGameObject("Wall22");
			{
				// Set position in the scene
				wall22->SetPostion(glm::vec3(-4.0f, -6.0f, 0.0f));
				wall22->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall22->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);


				RigidBody::Sptr floorRB22 = wall22->Add<RigidBody>(RigidBodyType::Static);
				floorRB22->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall23 = scene->CreateGameObject("wall23");
			{
				// Set position in the scene
				wall23->SetPostion(glm::vec3(-4.0f, -2.0f, 0.0f));
				wall23->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall23->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);


				RigidBody::Sptr floorRB23 = wall23->Add<RigidBody>(RigidBodyType::Static);
				floorRB23->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall24 = scene->CreateGameObject("wall24");
			{
				// Set position in the scene
				wall24->SetPostion(glm::vec3(-5.0f, -3.0f, 0.0f));
				wall24->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall24->Add<RenderComponent>();
				renderer->SetMesh(wall4Mesh);
				renderer->SetMaterial(wallMaterial);


				RigidBody::Sptr floorRB24 = wall24->Add<RigidBody>(RigidBodyType::Static);
				floorRB24->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall25 = scene->CreateGameObject("wall25");
			{
				// Set position in the scene
				wall25->SetPostion(glm::vec3(-3.0f, -3.0f, 0.0f));
				wall25->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall25->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);


				RigidBody::Sptr floorRB25 = wall25->Add<RigidBody>(RigidBodyType::Static);
				floorRB25->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall26 = scene->CreateGameObject("wall26");
			{
				// Set position in the scene
				wall26->SetPostion(glm::vec3(-6.0f, -4.0f, 0.0f));
				wall26->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall26->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);


				RigidBody::Sptr floorRB26 = wall26->Add<RigidBody>(RigidBodyType::Static);
				floorRB26->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall27 = scene->CreateGameObject("wall27");
			{
				// Set position in the scene
				wall27->SetPostion(glm::vec3(-7.0f, -5.0f, 0.0f));
				wall27->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall27->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial2);

				RigidBody::Sptr floorRB27 = wall27->Add<RigidBody>(RigidBodyType::Static);
				floorRB27->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall28 = scene->CreateGameObject("wall28");
			{
				// Set position in the scene
				wall28->SetPostion(glm::vec3(-7.0f, -7.0f, 0.0f));
				wall28->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall28->Add<RenderComponent>();
				renderer->SetMesh(wall4Mesh);
				renderer->SetMaterial(wallMaterial);


				RigidBody::Sptr floorRB28 = wall28->Add<RigidBody>(RigidBodyType::Static);
				floorRB28->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall29 = scene->CreateGameObject("wall29");
			{
				// Set position in the scene
				wall29->SetPostion(glm::vec3(-7.0f, -9.0f, 0.0f));
				wall29->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall29->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB29 = wall29->Add<RigidBody>(RigidBodyType::Static);
				floorRB29->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall30 = scene->CreateGameObject("wall30");
			{
				// Set position in the scene
				wall30->SetPostion(glm::vec3(-5.0f, -7.0f, 0.0f));
				wall30->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall30->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB30 = wall30->Add<RigidBody>(RigidBodyType::Static);
				floorRB30->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall31 = scene->CreateGameObject("wall31");
			{
				// Set position in the scene
				wall31->SetPostion(glm::vec3(-4.0f, -8.0f, 0.0f));
				wall31->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall31->Add<RenderComponent>();
				renderer->SetMesh(wall4Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB31 = wall31->Add<RigidBody>(RigidBodyType::Static);
				floorRB31->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall32 = scene->CreateGameObject("wall32");
			{
				// Set position in the scene
				wall32->SetPostion(glm::vec3(-6.0f, -10.0f, 0.0f));
				wall32->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall32->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB32 = wall32->Add<RigidBody>(RigidBodyType::Static);
				floorRB32->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall33 = scene->CreateGameObject("wall33");
			{
				// Set position in the scene
				wall33->SetPostion(glm::vec3(-4.0f, -10.0f, 0.0f));
				wall33->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall33->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB33 = wall33->Add<RigidBody>(RigidBodyType::Static);
				floorRB33->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall34 = scene->CreateGameObject("wall34");
			{
				// Set position in the scene
				wall34->SetPostion(glm::vec3(-3.0f, -9.0f, 0.0f));
				wall34->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall34->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial3);

				RigidBody::Sptr floorRB34 = wall34->Add<RigidBody>(RigidBodyType::Static);
				floorRB34->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall35 = scene->CreateGameObject("wall35");
			{
				// Set position in the scene
				wall35->SetPostion(glm::vec3(5.0f, -9.0f, 0.0f));
				wall35->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall35->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB35 = wall35->Add<RigidBody>(RigidBodyType::Static);
				floorRB35->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall36 = scene->CreateGameObject("wall36");
			{
				// Set position in the scene
				wall36->SetPostion(glm::vec3(5.0f, -11.0f, 0.0f));
				wall36->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall36->Add<RenderComponent>();
				renderer->SetMesh(wall4Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB36 = wall36->Add<RigidBody>(RigidBodyType::Static);
				floorRB36->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall37 = scene->CreateGameObject("wall37");
			{
				// Set position in the scene
				wall37->SetPostion(glm::vec3(6.0f, -12.0f, 0.0f));
				wall37->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall37->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial3);

				RigidBody::Sptr floorRB37 = wall37->Add<RigidBody>(RigidBodyType::Static);
				floorRB37->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall38 = scene->CreateGameObject("wall38");
			{
				// Set position in the scene
				wall38->SetPostion(glm::vec3(8.0f, -12.0f, 0.0f));
				wall38->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall38->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB38 = wall38->Add<RigidBody>(RigidBodyType::Static);
				floorRB38->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall39 = scene->CreateGameObject("wall39");
			{
				// Set position in the scene
				wall39->SetPostion(glm::vec3(9.0f, -11.0f, 0.0f));
				wall39->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall39->Add<RenderComponent>();
				renderer->SetMesh(wall4Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB39 = wall39->Add<RigidBody>(RigidBodyType::Static);
				floorRB39->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall40 = scene->CreateGameObject("wall40");
			{
				// Set position in the scene
				wall40->SetPostion(glm::vec3(8.0f, -10.0f, 0.0f));
				wall40->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall40->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB40 = wall40->Add<RigidBody>(RigidBodyType::Static);
				floorRB40->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall41 = scene->CreateGameObject("wall41");
			{
				// Set position in the scene
				wall41->SetPostion(glm::vec3(6.0f, 0.0f, 0.0f));
				wall41->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall41->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB41 = wall41->Add<RigidBody>(RigidBodyType::Static);
				floorRB41->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall42 = scene->CreateGameObject("wall42");
			{
				// Set position in the scene
				wall42->SetPostion(glm::vec3(7.0f, -1.0f, 0.0f));
				wall42->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall42->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial5);

				RigidBody::Sptr floorRB42 = wall42->Add<RigidBody>(RigidBodyType::Static);
				floorRB42->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall43 = scene->CreateGameObject("wall43");
			{
				// Set position in the scene
				wall43->SetPostion(glm::vec3(5.0f, -1.0f, 0.0f));
				wall43->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall43->Add<RenderComponent>();
				renderer->SetMesh(wall4Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB43 = wall43->Add<RigidBody>(RigidBodyType::Static);
				floorRB43->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall44 = scene->CreateGameObject("wall44");
			{
				// Set position in the scene
				wall44->SetPostion(glm::vec3(5.0f, -5.0f, 0.0f));
				wall44->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall44->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial5);

				RigidBody::Sptr floorRB44 = wall44->Add<RigidBody>(RigidBodyType::Static);
				floorRB44->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall45 = scene->CreateGameObject("wall45");
			{
				// Set position in the scene
				wall45->SetPostion(glm::vec3(5.0f, -3.0f, 0.0f));
				wall45->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall45->Add<RenderComponent>();
				renderer->SetMesh(wall1Mesh);
				renderer->SetMaterial(wallMaterial4);

				RigidBody::Sptr floorRB45 = wall45->Add<RigidBody>(RigidBodyType::Static);
				floorRB45->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
			GameObject::Sptr wall46 = scene->CreateGameObject("wall46");
			{
				// Set position in the scene
				wall46->SetPostion(glm::vec3(7.0f, -3.0f, 0.0f));
				wall46->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = wall46->Add<RenderComponent>();
				renderer->SetMesh(wall4Mesh);
				renderer->SetMaterial(wallMaterial);

				RigidBody::Sptr floorRB46 = wall46->Add<RigidBody>(RigidBodyType::Static);
				floorRB46->AddCollider(BoxCollider::Create(glm::vec3(1, 1, 0)));
			}
		} // walls end

		{ // ememies start
			//GameObject::Sptr demon1 = scene->CreateGameObject("demon");
			//{
			//	// Set position in the scene
			//	demon1->SetPostion(glm::vec3(2.0f, -20.0f, 0.0f));
			//	demon1->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
			//	demon1->SetScale(glm::vec3(0.07f));

			//	// Create and attach a renderer for the model
			//	RenderComponent::Sptr renderer = demon1->Add<RenderComponent>();
			//	renderer->SetMesh(demonMesh);
			//	renderer->SetMaterial(demonMaterial);

			//	demon1->Add<EnemyBehaviour>();


			//	RigidBody::Sptr demon1RB = demon1->Add<RigidBody>(RigidBodyType::Dynamic);
			//	BoxCollider::Sptr collider1 = BoxCollider::Create(glm::vec3(0.4f, 0.4f, 0.3f));
			//	collider1->SetPosition(glm::vec3(0.0f, 0.4f, 0.0f));
			//	demon1RB->AddCollider(collider1);
			//	demon1RB->SetLinearDamping(1.f);

			//	demon1RB->SetLinearDamping(1.0f);

			//	//spider walk setup
			//	MorphMeshRenderer::Sptr demonmorph1 = demon1->Add<MorphMeshRenderer>();

			//	demonmorph1->SetMorphMeshRenderer(spiderMesh, spiderMaterial);
			//	Morphanimator::Sptr demonmorph2 = demon1->Add<Morphanimator>();

			//	MeshResource::Sptr demonMeshWalk1 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000000.obj");
			//	MeshResource::Sptr demonMeshWalk2 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000003.obj");
			//	MeshResource::Sptr demonMeshWalk3 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000004.obj");
			//	MeshResource::Sptr demonMeshWalk4 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000005.obj");
			//	MeshResource::Sptr demonMeshWalk5 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000007.obj");
			//	MeshResource::Sptr demonMeshWalk6 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000008.obj");
			//	MeshResource::Sptr demonMeshWalk7 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000009.obj");
			//	MeshResource::Sptr demonMeshWalk8 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000010.obj");
			//	MeshResource::Sptr demonMeshWalk9 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000011.obj");
			//	MeshResource::Sptr demonMeshWalk10 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000013.obj");
			//	MeshResource::Sptr demonMeshWalk11 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000014.obj");
			//	MeshResource::Sptr demonMeshWalk12 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000015.obj");
			//	MeshResource::Sptr demonMeshWalk13 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000016.obj");
			//	MeshResource::Sptr demonMeshWalk14 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000017.obj");
			//	MeshResource::Sptr demonMeshWalk15 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000019.obj");
			//	MeshResource::Sptr demonMeshWalk16 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000020.obj");
			//	MeshResource::Sptr demonMeshWalk17 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000021.obj");
			//	MeshResource::Sptr demonMeshWalk18 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000022.obj");
			//	MeshResource::Sptr demonMeshWalk19 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000023.obj");
			//	MeshResource::Sptr demonMeshWalk20 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000024.obj");
			//	MeshResource::Sptr demonMeshWalk21 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000025.obj");
			//	MeshResource::Sptr demonMeshWalk22 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000026.obj");
			//	MeshResource::Sptr demonMeshWalk23 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000027.obj");
			//	MeshResource::Sptr demonMeshWalk24 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000028.obj");
			//	MeshResource::Sptr demonMeshWalk25 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000030.obj");
			//	MeshResource::Sptr demonMeshWalk26 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000031.obj");
			//	MeshResource::Sptr demonMeshWalk27 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000032.obj");
			//	MeshResource::Sptr demonMeshWalk28 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000033.obj");
			//	MeshResource::Sptr demonMeshWalk29 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000034.obj");
			//	MeshResource::Sptr demonMeshWalk30 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000035.obj");
			//	MeshResource::Sptr demonMeshWalk31 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000036.obj");
			//	MeshResource::Sptr demonMeshWalk32 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000038.obj");
			//	MeshResource::Sptr demonMeshWalk33 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000039.obj");
			//	MeshResource::Sptr demonMeshWalk34 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000040.obj");
			//	MeshResource::Sptr demonMeshWalk35 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000041.obj");
			//	MeshResource::Sptr demonMeshWalk36 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000042.obj");
			//	MeshResource::Sptr demonMeshWalk37 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000043.obj");
			//	MeshResource::Sptr demonMeshWalk38 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000044.obj");
			//	MeshResource::Sptr demonMeshWalk39 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000045.obj");
			//	MeshResource::Sptr demonMeshWalk40 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000046.obj");
			//	MeshResource::Sptr demonMeshWalk41 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000047.obj");
			//	MeshResource::Sptr demonMeshWalk42 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000048.obj");
			//	MeshResource::Sptr demonMeshWalk43 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000049.obj");
			//	MeshResource::Sptr demonMeshWalk44 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000050.obj");
			//	MeshResource::Sptr demonMeshWalk45 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000052.obj"); 
			//	MeshResource::Sptr demonMeshWalk46 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000054.obj");
			//	MeshResource::Sptr demonMeshWalk47 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000055.obj");
			//	MeshResource::Sptr demonMeshWalk48 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000056.obj");
			//	MeshResource::Sptr demonMeshWalk49 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000057.obj");
			//	MeshResource::Sptr demonMeshWalk50 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000059.obj");
			//	MeshResource::Sptr demonMeshWalk51 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000060.obj");
			//	MeshResource::Sptr demonMeshWalk52 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000061.obj");
			//	MeshResource::Sptr demonMeshWalk53 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000063.obj");
			//	MeshResource::Sptr demonMeshWalk54 = ResourceManager::CreateAsset<MeshResource>("Animations/Demon2/DemonWalk_000065.obj");

			//	std::vector<MeshResource::Sptr> frames;
			//	frames.push_back(demonMeshWalk1);
			//	frames.push_back(demonMeshWalk2);
			//	frames.push_back(demonMeshWalk3);
			//	frames.push_back(demonMeshWalk4);
			//	frames.push_back(demonMeshWalk5);
			//	frames.push_back(demonMeshWalk6);
			//	frames.push_back(demonMeshWalk7);
			//	frames.push_back(demonMeshWalk8);
			//	frames.push_back(demonMeshWalk9);
			//	frames.push_back(demonMeshWalk10);
			//	frames.push_back(demonMeshWalk11);
			//	frames.push_back(demonMeshWalk12);
			//	frames.push_back(demonMeshWalk13);
			//	frames.push_back(demonMeshWalk14);
			//	frames.push_back(demonMeshWalk15);
			//	frames.push_back(demonMeshWalk16);
			//	frames.push_back(demonMeshWalk17);
			//	frames.push_back(demonMeshWalk18);
			//	frames.push_back(demonMeshWalk19);
			//	frames.push_back(demonMeshWalk20);
			//	frames.push_back(demonMeshWalk21);
			//	frames.push_back(demonMeshWalk22);
			//	frames.push_back(demonMeshWalk23);
			//	frames.push_back(demonMeshWalk24);
			//	frames.push_back(demonMeshWalk25);
			//	frames.push_back(demonMeshWalk26);
			//	frames.push_back(demonMeshWalk27);
			//	frames.push_back(demonMeshWalk28);
			//	frames.push_back(demonMeshWalk29);
			//	frames.push_back(demonMeshWalk30);
			//	frames.push_back(demonMeshWalk31);
			//	frames.push_back(demonMeshWalk32);
			//	frames.push_back(demonMeshWalk33);
			//	frames.push_back(demonMeshWalk34);
			//	frames.push_back(demonMeshWalk35);
			//	frames.push_back(demonMeshWalk36);
			//	frames.push_back(demonMeshWalk37);
			//	frames.push_back(demonMeshWalk38);
			//	frames.push_back(demonMeshWalk39);
			//	frames.push_back(demonMeshWalk40);
			//	frames.push_back(demonMeshWalk41);
			//	frames.push_back(demonMeshWalk42);
			//	frames.push_back(demonMeshWalk43);
			//	frames.push_back(demonMeshWalk44);
			//	frames.push_back(demonMeshWalk45);
			//	frames.push_back(demonMeshWalk46);
			//	frames.push_back(demonMeshWalk47);
			//	frames.push_back(demonMeshWalk48);
			//	frames.push_back(demonMeshWalk49);
			//	frames.push_back(demonMeshWalk50);
			//	frames.push_back(demonMeshWalk51);
			//	frames.push_back(demonMeshWalk52);
			//	frames.push_back(demonMeshWalk53);
			//	frames.push_back(demonMeshWalk54);

			//	demonmorph2->SetInitial();
			//	demonmorph2->SetFrameTime(0.1f);
			//	demonmorph2->SetFrames(frames);

			//}
			GameObject::Sptr spider1 = scene->CreateGameObject("Spider");
			{
				// Set position in the scene
				spider1->SetPostion(glm::vec3(-4.0f, -5.0f, 0.0f));
				spider1->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				spider1->SetScale(glm::vec3(0.6f, 0.6f, 0.6f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = spider1->Add<RenderComponent>();
				renderer->SetMesh(spiderMesh);
				renderer->SetMaterial(spiderMaterial);

				spider1->Add<EnemyBehaviour>();


				RigidBody::Sptr spider1RB = spider1->Add<RigidBody>(RigidBodyType::Dynamic);
				BoxCollider::Sptr collider1 = BoxCollider::Create(glm::vec3(0.4f, 0.4f, 0.3f));
				collider1->SetPosition(glm::vec3(0.0f, 0.4f, 0.f));
				spider1RB->AddCollider(collider1);
				spider1RB->SetLinearDamping(1.f);

				spider1RB->SetLinearDamping(1.0f);

				//spider walk setup
				MorphMeshRenderer::Sptr spidermorph1 = spider1->Add<MorphMeshRenderer>();

				spidermorph1->SetMorphMeshRenderer(spiderMesh, spiderMaterial);
				Morphanimator::Sptr spidermorph2 = spider1->Add<Morphanimator>();

				MeshResource::Sptr spiderMeshWalk1 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000000.obj");
				MeshResource::Sptr spiderMeshWalk2 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000005.obj");
				MeshResource::Sptr spiderMeshWalk3 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000010.obj");
				MeshResource::Sptr spiderMeshWalk4 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000015.obj");
				MeshResource::Sptr spiderMeshWalk5 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000020.obj");

				std::vector<MeshResource::Sptr> frames;
				frames.push_back(spiderMeshWalk1);
				frames.push_back(spiderMeshWalk2);
				frames.push_back(spiderMeshWalk3);
				frames.push_back(spiderMeshWalk4);
				frames.push_back(spiderMeshWalk5);

				spidermorph2->SetInitial();
				spidermorph2->SetFrameTime(0.1f);
				spidermorph2->SetFrames(frames);


			}
			GameObject::Sptr spider2 = scene->CreateGameObject("spider2");
			{
				// Set position in the scene
				spider2->SetPostion(glm::vec3(-6.0f, -9.0f, 0.0f));
				spider2->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				spider2->SetScale(glm::vec3(0.6f, 0.6f, 0.6f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = spider2->Add<RenderComponent>();
				renderer->SetMesh(spiderMesh);
				renderer->SetMaterial(spiderMaterial);

				spider2->Add<EnemyBehaviour>();

				RigidBody::Sptr spider2RB = spider2->Add<RigidBody>(RigidBodyType::Dynamic);
				BoxCollider::Sptr collider2 = BoxCollider::Create(glm::vec3(0.4f, 0.4f, 0.3f));
				collider2->SetPosition(glm::vec3(0.0f, 0.4f, 0.f));
				spider2RB->AddCollider(collider2);
				spider2RB->SetLinearDamping(1.f);

				spider2RB->SetLinearDamping(1.0f);

				//spider walk setup
				MorphMeshRenderer::Sptr spidermorph1 = spider2->Add<MorphMeshRenderer>();
	
				Morphanimator::Sptr spidermorph2 = spider2->Add<Morphanimator>();

				MeshResource::Sptr spiderMeshWalk1 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000000.obj");
				MeshResource::Sptr spiderMeshWalk2 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000005.obj");
				MeshResource::Sptr spiderMeshWalk3 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000010.obj");
				MeshResource::Sptr spiderMeshWalk4 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000015.obj");
				MeshResource::Sptr spiderMeshWalk5 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000020.obj");

				std::vector<MeshResource::Sptr> frames;
				frames.push_back(spiderMeshWalk1);
				frames.push_back(spiderMeshWalk2);
				frames.push_back(spiderMeshWalk3);
				frames.push_back(spiderMeshWalk4);
				frames.push_back(spiderMeshWalk5);

				spidermorph2->SetInitial();
				spidermorph2->SetFrameTime(0.1f);
				spidermorph2->SetFrames(frames);

			}
			GameObject::Sptr golem1 = scene->CreateGameObject("Golem");
			{
				// Set position in the scene
				golem1->SetPostion(glm::vec3(6.0f, -7.0f, 0.0f));
				golem1->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				golem1->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the model
				RenderComponent::Sptr renderer = golem1->Add<RenderComponent>();
				renderer->SetMesh(golemMesh);
				renderer->SetMaterial(golemMaterial);
				golem1->Add<EnemyPath>();


				RigidBody::Sptr golem1RB = golem1->Add<RigidBody>(RigidBodyType::Dynamic);
				BoxCollider::Sptr collider3 = BoxCollider::Create(glm::vec3(0.4f, 0.8f, 0.4f));
				collider3->SetPosition(glm::vec3(0.0f, 0.8f, 0.f));
				golem1RB->AddCollider(collider3);
				golem1RB->SetLinearDamping(1.f);

				//Golem walk setup
				MorphMeshRenderer::Sptr golemMorph1 = golem1->Add<MorphMeshRenderer>();

				golemMorph1->SetMorphMeshRenderer(golemMesh, golemMaterial);
				Morphanimator::Sptr golemMorph2 = golem1->Add<Morphanimator>();

				MeshResource::Sptr golemWalkMesh1 = ResourceManager::CreateAsset<MeshResource>("Animations/Golem/GolemWalk_000001.obj");
				MeshResource::Sptr golemWalkMesh2 = ResourceManager::CreateAsset<MeshResource>("Animations/Golem/GolemWalk_000020.obj");
				MeshResource::Sptr golemWalkMesh3 = ResourceManager::CreateAsset<MeshResource>("Animations/Golem/GolemWalk_000040.obj");
				MeshResource::Sptr golemWalkMesh4 = ResourceManager::CreateAsset<MeshResource>("Animations/Golem/GolemWalk_000060.obj");
				MeshResource::Sptr golemWalkMesh5 = ResourceManager::CreateAsset<MeshResource>("Animations/Golem/GolemWalk_000080.obj");

				std::vector<MeshResource::Sptr> frames;
				frames.push_back(golemWalkMesh1);
				frames.push_back(golemWalkMesh2);
				frames.push_back(golemWalkMesh3);
				frames.push_back(golemWalkMesh4);
				frames.push_back(golemWalkMesh5);

				golemMorph2->SetInitial();
				golemMorph2->SetFrameTime(0.5f);
				golemMorph2->SetFrames(frames);

			}
		}

		GameObject::Sptr skeleton1 = scene->CreateGameObject("Skeleton");
		{
			// Set position in the scene
			skeleton1->SetPostion(glm::vec3(1.0f, -5.0f, 0.1f));
			skeleton1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			skeleton1->SetScale(glm::vec3(0.8f, 0.8f, 0.8f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = skeleton1->Add<RenderComponent>();
			renderer->SetMesh(skeletonMesh);
			renderer->SetMaterial(skeletonMaterial);


			skeleton1->Add<EnemyBehaviour>();
			RigidBody::Sptr skeletonRB = skeleton1->Add<RigidBody>(RigidBodyType::Dynamic);
			BoxCollider::Sptr collider4 = BoxCollider::Create(glm::vec3(0.35f, 1.2f, 0.35f));
			collider4->SetPosition(glm::vec3(0.0f, 1.2f, 0.f));
			skeletonRB->AddCollider(collider4);
			skeletonRB->SetLinearDamping(1.f);

			//skeleton walk setup
			MorphMeshRenderer::Sptr skeletonMorph1 = skeleton1->Add<MorphMeshRenderer>();

			skeletonMorph1->SetMorphMeshRenderer(skeletonMesh, skeletonMaterial);
			Morphanimator::Sptr skeletonMorph2 = skeleton1->Add<Morphanimator>();

			MeshResource::Sptr skeletonWalkMesh1 = ResourceManager::CreateAsset<MeshResource>("Animations/Skeleton/SkeletonWalk_000001.obj");
			MeshResource::Sptr skeletonWalkMesh2 = ResourceManager::CreateAsset<MeshResource>("Animations/Skeleton/SkeletonWalk_000010.obj");
			MeshResource::Sptr skeletonWalkMesh3 = ResourceManager::CreateAsset<MeshResource>("Animations/Skeleton/SkeletonWalk_000020.obj");
			MeshResource::Sptr skeletonWalkMesh4 = ResourceManager::CreateAsset<MeshResource>("Animations/Skeleton/SkeletonWalk_000030.obj");
			MeshResource::Sptr skeletonWalkMesh5 = ResourceManager::CreateAsset<MeshResource>("Animations/Skeleton/SkeletonWalk_000040.obj");

			std::vector<MeshResource::Sptr> frames;
			frames.push_back(skeletonWalkMesh1);
			frames.push_back(skeletonWalkMesh2);
			frames.push_back(skeletonWalkMesh3);
			frames.push_back(skeletonWalkMesh4);
			frames.push_back(skeletonWalkMesh5);

			skeletonMorph2->SetInitial();
			skeletonMorph2->SetFrameTime(0.1f);
			skeletonMorph2->SetFrames(frames);

		}
		GameObject::Sptr boss = scene->CreateGameObject("Boss");
		{
			// Set position in the scene
			boss->SetPostion(glm::vec3(-4.0f, -5.0f, 10.0f));
			boss->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
			boss->SetScale(glm::vec3(0.6f, 0.6f, 0.6f));


			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = boss->Add<RenderComponent>();
			renderer->SetMesh(finalBoss);
			renderer->SetMaterial(bossMaterial);

			//Boss walk setup
			MorphMeshRenderer::Sptr bossMorph1 = boss->Add<MorphMeshRenderer>();

			bossMorph1->SetMorphMeshRenderer(finalBoss, bossMaterial);
			Morphanimator::Sptr bossMorph2 = boss->Add<Morphanimator>();

			MeshResource::Sptr bossAnim1 = ResourceManager::CreateAsset<MeshResource>("Animations/Boss/Boss_000001.obj");
			MeshResource::Sptr bossAnim2 = ResourceManager::CreateAsset<MeshResource>("Animations/Boss/Boss_000040.obj");
			MeshResource::Sptr bossAnim3 = ResourceManager::CreateAsset<MeshResource>("Animations/Boss/Boss_000080.obj");
			
			std::vector<MeshResource::Sptr> frames;
			frames.push_back(bossAnim1);
			frames.push_back(bossAnim2);
			frames.push_back(bossAnim3);

			bossMorph2->SetInitial();
			bossMorph2->SetFrameTime(0.5f);
			bossMorph2->SetFrames(frames);


		}

		// enemies end
		// Create a trigger volume for testing how we can detect collisions with objects!
		GameObject::Sptr trigger = scene->CreateGameObject("Trigger");
		{
			TriggerVolume::Sptr volume = trigger->Add<TriggerVolume>();
			BoxCollider::Sptr collider = BoxCollider::Create(glm::vec3(3.0f, 3.0f, 1.0f));
			collider->SetPosition(glm::vec3(0.0f, 0.0f, 0.f));
			volume->AddCollider(collider);

			trigger->Add<TriggerVolumeEnterBehaviour>();
		}

		/////////////////////////// UI //////////////////////////////

		GameObject::Sptr MenScreen = scene->CreateGameObject("Menu Screen");
		{
			RectTransform::Sptr transform = MenScreen->Add<RectTransform>();
			transform->SetMin({ 1920, 1080 });
			transform->SetMax({ 0, 0 });
			transform->SetPosition(glm::vec2(960, 540));

			GuiPanel::Sptr testPanel = MenScreen->Add<GuiPanel>();
			//testPanel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			testPanel->SetTexture(menuPNG);
			testPanel->SetBorderRadius(1920); //Tinker with 

			MenScreen->Add<MenuScreen>();
		}


		GameObject::Sptr pausScreen = scene->CreateGameObject("Pause Screen");
		{
			RectTransform::Sptr transform = pausScreen->Add<RectTransform>();
			transform->SetMin({ 1920, 1080 });
			transform->SetMax({ 0, 0 });
			transform->SetPosition(glm::vec2(960, 540));

			GuiPanel::Sptr testPanel = pausScreen->Add<GuiPanel>();
			//testPanel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			testPanel->SetTexture(pausePNG);
			testPanel->SetBorderRadius(1920); //Tinker with 
			testPanel->IsEnabled = false;

			pausScreen->Add<PauseScreen>();
			pausScreen->Get<PauseScreen>()->testPanel = testPanel;
		}

		GameObject::Sptr WinnerScreen = scene->CreateGameObject("Winner Screen");
		{
			RectTransform::Sptr transform = WinnerScreen->Add<RectTransform>();
			transform->SetMin({ 1920, 1080 });
			transform->SetMax({ 0, 0 });
			transform->SetPosition(glm::vec2(960, 540));

			GuiPanel::Sptr testPanel = WinnerScreen->Add<GuiPanel>();
			//testPanel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			testPanel->SetTexture(winPNG);
			testPanel->SetBorderRadius(1920); //Tinker with 
			testPanel->IsEnabled = false;

			WinnerScreen->Add<WinScreen>();
			WinnerScreen->Get<WinScreen>()->testPanel = testPanel;
		}

		GameObject::Sptr LostScreen = scene->CreateGameObject("Pause Screen");
		{
			RectTransform::Sptr transform = LostScreen->Add<RectTransform>();
			transform->SetMin({ 1920, 1080 });
			transform->SetMax({ 0, 0 });
			transform->SetPosition(glm::vec2(960, 540));

			GuiPanel::Sptr imageLose = LostScreen->Add<GuiPanel>();
			//testPanel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			imageLose->SetTexture(losePNG);
			imageLose->SetBorderRadius(1920); //Tinker with 
			imageLose->IsEnabled = false;

			LostScreen->Add<LoseScreen>();
			LostScreen->Get<LoseScreen>()->imageLose = imageLose;

		}
		GameObject::Sptr BandageText = scene->CreateGameObject("Bandage txt");
		{
			RectTransform::Sptr transform = BandageText->Add<RectTransform>();
			transform->SetPosition(glm::vec2(220, 1000));
			transform->SetMin({ 10, 10 });
			transform->SetMax({ 446, 2062 });

			GuiPanel::Sptr panel = BandageText->Add<GuiPanel>();
			panel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

			Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 20.0f);
			font->Bake();

			GuiText::Sptr text = BandageText->Add<GuiText>();
			text->SetText(std::to_string(bandageCount));
			text->SetFont(font);
			text->SetColor(glm::vec4(0, 1, 0, 0));

			BandageText->Add<BandageCount>();
		}

		GameObject::Sptr BandagePack = scene->CreateGameObject("Bandage UI");
		{

			BandagePack->RenderGUI();
			//glEnable(GL_BLEND);
			RectTransform::Sptr transform = BandagePack->Add<RectTransform>();

			transform->SetPosition(glm::vec2(220, 920));
			transform->SetSize(glm::vec2(140, -40));

			GuiPanel::Sptr testPanel = BandagePack->Add<GuiPanel>();

			testPanel->SetTexture(Bandage);
			testPanel->SetBorderRadius(18); //Tinker with
		}
		GameObject::Sptr AmmoText = scene->CreateGameObject("Ammo txt");
		{
			RectTransform::Sptr transform = AmmoText->Add<RectTransform>();
			transform->SetPosition(glm::vec2(1720, 540));
			transform->SetMin({ 1700, 10 });
			transform->SetMax({ 1720, 2062 });

			GuiPanel::Sptr panel = AmmoText->Add<GuiPanel>();
			panel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

			Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 20.0f);
			font->Bake();

			GuiText::Sptr text = AmmoText->Add<GuiText>();
			text->SetText(std::to_string(ammoCount));
			text->SetFont(font);
			text->SetColor(glm::vec4(0, 1, 0, 0));

			AmmoText->Add<NormalAmmo>();
		}

		GameObject::Sptr AmmoPack = scene->CreateGameObject("Ammo UI");
		{
			AmmoPack->RenderGUI();
			//glEnable(GL_BLEND);
			RectTransform::Sptr transform = AmmoPack->Add<RectTransform>();

			transform->SetPosition(glm::vec2(1700, 920));
			transform->SetSize(glm::vec2(99, -50));

			GuiPanel::Sptr testPanel = AmmoPack->Add<GuiPanel>();

			testPanel->SetTexture(ammoHUD);
			testPanel->SetBorderRadius(0); //Tinker with

			Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 20.0f);
			font->Bake();

			GuiText::Sptr text = AmmoPack->Add<GuiText>();
			text->SetText("Hello World");
			text->SetFont(font);
		}

		GameObject::Sptr ThreeHealth = scene->CreateGameObject("ThreeHealth");
		{

			ThreeHealth->RenderGUI();
			//glEnable(GL_BLEND);
			RectTransform::Sptr transform = ThreeHealth->Add<RectTransform>();

			transform->SetPosition(glm::vec2(260, 90));
			transform->SetSize(glm::vec2(130, 0));

			GuiPanel::Sptr threeHealth = ThreeHealth->Add<GuiPanel>();
			threeHealth->SetTexture(healthBar);
			threeHealth->SetBorderRadius(52);
			threeHealth->IsEnabled = true;

			ThreeHealth->Add<HealthBehaviour>();
			ThreeHealth->Get<HealthBehaviour>()->threeHealth = threeHealth;
		}
		GameObject::Sptr TwoHealth = scene->CreateGameObject("TwoHealth");
		{

			TwoHealth->RenderGUI();
			//glEnable(GL_BLEND);
			RectTransform::Sptr transform = TwoHealth->Add<RectTransform>();

			transform->SetPosition(glm::vec2(260, 90));
			transform->SetSize(glm::vec2(130, 0));

			GuiPanel::Sptr twoHealth = TwoHealth->Add<GuiPanel>();
			twoHealth->SetTexture(healthBar1);
			twoHealth->SetBorderRadius(52);
			twoHealth->IsEnabled = false;


			TwoHealth->Add<HealthBehaviour2>();
			TwoHealth->Get<HealthBehaviour2>()->twoHealth = twoHealth;
		}
		GameObject::Sptr OneHealth = scene->CreateGameObject("OneHealth");
		{

			OneHealth->RenderGUI();
			//glEnable(GL_BLEND);
			RectTransform::Sptr transform = OneHealth->Add<RectTransform>();

			transform->SetPosition(glm::vec2(260, 90));
			transform->SetSize(glm::vec2(130, 0));

			GuiPanel::Sptr oneHealth = OneHealth->Add<GuiPanel>();
			oneHealth->SetTexture(healthBar2);
			oneHealth->SetBorderRadius(52);
			oneHealth->IsEnabled = false;

			OneHealth->Add<HealthBehaviour1>();
			OneHealth->Get<HealthBehaviour1>()->oneHealth = oneHealth;
		}

		GameObject::Sptr CrossHair = scene->CreateGameObject("Crosshair");
		{
			RectTransform::Sptr transform = CrossHair->Add<RectTransform>();
			transform->SetMin({ 50 ,50 });
			transform->SetMax({ 50, 50 });
			transform->SetSize(glm::vec2(5, 5));
			transform->SetPosition(glm::vec2(960, 350));

			GuiPanel::Sptr CrossHair2 = CrossHair->Add<GuiPanel>();

			CrossHair2->SetColor(glm::vec4(0.5f, 0.0f, 0.0f, 1.0f));
			CrossHair2->SetTexture(Crosshair);
			CrossHair2->SetBorderRadius(19); //Tinker with 
		}



		// Call scene awake to start up all of our components
		scene->Window = window;
		scene->Awake();

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");
	}



	// We'll use this to allow editing the save/load path
	// via ImGui, note the reserve to allocate extra space
	// for input!
	std::string scenePath = "Corner.json";
	scenePath.reserve(256);

	bool isRotating = true;
	float rotateSpeed = 90.0f;
	// Our high-precision timer
	double lastFrame = glfwGetTime();


	BulletDebugMode physicsDebugMode = BulletDebugMode::None;
	float playbackSpeed = 1.0f;

	nlohmann::json editorSceneState;

	///// Game loop gameloop/////
	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		{
			if (bandageCount > 0)
				if (playerHealth < 3)
				{
					playerHealth += 1;
					bandageCount -= 1;
					cout << "Player Health: " << playerHealth << endl;
					cout << "Bandage Count: " << bandageCount << endl;
				}
		}


		glfwPollEvents();
		ImGuiHelper::StartFrame();





		if (levelComplete)
		{/*
			if (progressScore < 1)
			{
				levelComplete = false;
				hasKey = false;
				RoomFunction();
			}
			else*/
			{
				gameWin = true;
			}
		}

		bool pressed = glfwGetKey(window, GLFW_KEY_ESCAPE);
		if (pressed)
		{
			if (gameWin == false)
			{
				if (pausePressed == false)
				{
					gamePaused = !gamePaused;
				}
				pausePressed = pressed;
			}
		}
		else
		{
			pausePressed = false;
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			if (onMenu == true)
			{
				onMenu = false;

				scene->IsPlaying = true;
			}
		}
		if (gameWin == true)
		{
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			{
				arrowOut = false;
				gamePaused = false;
				onMenu = true;
				canShoot = true;
				hasKey = false;
				levelComplete = false;
				ammoCount = 5;
				playerHealth = 3;
				bandageCount = 0;

				gameWin = false;
				scene = nullptr;

				std::string newFilename = std::filesystem::path("scene.json").stem().string() + "-manifest.json";
				ResourceManager::LoadManifest(newFilename);
				scene = Scene::Load("scene.json");

				scene->Window = window;
				scene->Awake();

			}
		}
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR);

		// Calculate the time since our last frame (dt)
		double thisFrame = glfwGetTime();
		float dt = static_cast<float>(thisFrame - lastFrame);
		// Showcasing how to use the imGui library!
		bool isDebugWindowOpen = ImGui::Begin("Debugging");
		if (isDebugWindowOpen)
		{
			// Draws a button to control whether or not the game is currently playing
			static char buttonLabel[64];
			sprintf_s(buttonLabel, "%s###playmode", scene->IsPlaying ? "Exit Play Mode" : "Enter Play Mode");
			void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods);

			if (ImGui::Button(buttonLabel))
			{
				// Save scene so it can be restored when exiting play mode
				if (!scene->IsPlaying)
				{
					editorSceneState = scene->ToJson();
				}

				// Toggle state
				scene->IsPlaying = !scene->IsPlaying;

				// If we've gone from playing to not playing, restore the state from before we started playing
				if (!scene->IsPlaying)
				{
					scene = nullptr;
					// We reload to scene from our cached state
					scene = Scene::FromJson(editorSceneState);
					// Don't forget to reset the scene's window and wake all the objects!
					scene->Window = window;
					scene->Awake();
				}
			}

			// Make a new area for the scene saving/loading
			ImGui::Separator();
			if (DrawSaveLoadImGui(scene, scenePath))
			{
				// C++ strings keep internal lengths which can get annoying
				// when we edit it's underlying datastore, so recalcualte size
				scenePath.resize(strlen(scenePath.c_str()));

				// We have loaded a new scene, call awake to set
				// up all our components
				scene->Window = window;
				scene->Awake();
			}
			ImGui::Separator();
			// Draw a dropdown to select our physics debug draw mode
			if (BulletDebugDraw::DrawModeGui("Physics Debug Mode:", physicsDebugMode))
			{
				scene->SetPhysicsDebugDrawMode(physicsDebugMode);
			}
			LABEL_LEFT(ImGui::SliderFloat, "Playback Speed:    ", &playbackSpeed, 0.0f, 10.0f);
			ImGui::Separator();
		}

		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update our application level uniforms every frame

		// Draw some ImGui stuff for the lights
		if (isDebugWindowOpen)
		{
			for (int ix = 0; ix < scene->Lights.size(); ix++)
			{
				char buff[256];
				sprintf_s(buff, "Light %d##%d", ix, ix);
				// DrawLightImGui will return true if the light was deleted
				if (DrawLightImGui(scene, buff, ix))
				{
					// Remove light from scene, restore all lighting data
					scene->Lights.erase(scene->Lights.begin() + ix);
					scene->SetupShaderAndLights();
					// Move back one element so we don't skip anything!
					ix--;
				}
			}
			// As long as we don't have max lights, draw a button
			// to add another one
			if (scene->Lights.size() < scene->MAX_LIGHTS)
			{
				if (ImGui::Button("Add Light"))
				{
					scene->Lights.push_back(Light());
					scene->SetupShaderAndLights();
				}
			}
			// Split lights from the objects in ImGui
			ImGui::Separator();
		}

		dt *= playbackSpeed;

		// Perform updates for all components
		scene->Update(dt);

		// Grab shorthands to the camera and shader from the scene
		Camera::Sptr camera = scene->MainCamera;

		// Cache the camera's viewprojection
		glm::mat4 viewProj = camera->GetViewProjection();
		DebugDrawer::Get().SetViewProjection(viewProj);

		// Update our worlds physics!
		scene->DoPhysics(dt);

		// Draw object GUIs
		if (isDebugWindowOpen)
		{
			scene->DrawAllGameObjectGUIs();
		}

		// The current material that is bound for rendering
		Material::Sptr currentMat = nullptr;
		Shader::Sptr shader = nullptr;

		TextureCube::Sptr environment = scene->GetSkyboxTexture();
		if (environment) environment->Bind(0);

		// Render all our objects
		ComponentManager::Each<RenderComponent>([&](const RenderComponent::Sptr& renderable) {
			// Early bail if mesh not set
			if (renderable->GetMesh() == nullptr)
			{
				return;
			}

			// If we don't have a material, try getting the scene's fallback material
			// If none exists, do not draw anything
			if (renderable->GetMaterial() == nullptr)
			{
				if (scene->DefaultMaterial != nullptr)
				{
					renderable->SetMaterial(scene->DefaultMaterial);
				}
				else
				{
					return;
				}
			}
			playerX = scene->MainCamera->GetGameObject()->GetPosition().x;
			playerY = scene->MainCamera->GetGameObject()->GetPosition().y;
			// If the material has changed, we need to bind the new shader and set up our material and frame data
			// Note: This is a good reason why we should be sorting the render components in ComponentManager
			if (renderable->GetMaterial() != currentMat)
			{
				currentMat = renderable->GetMaterial();
				shader = currentMat->GetShader();

				shader->Bind();
				shader->SetUniform("u_CamPos", scene->MainCamera->GetGameObject()->GetPosition());
				currentMat->Apply();
			}

			// Grab the game object so we can do some stuff with it
			GameObject* object = renderable->GetGameObject();

			// Set vertex shader parameters
			shader->SetUniformMatrix("u_ModelViewProjection", viewProj * object->GetTransform());
			shader->SetUniformMatrix("u_Model", object->GetTransform());
			shader->SetUniformMatrix("u_NormalMatrix", glm::mat3(glm::transpose(glm::inverse(object->GetTransform()))));

			// Draw the object
			renderable->GetMesh()->Draw();
			});

		// Use our cubemap to draw our skybox
		//scene->DrawSkybox();
// Disable culling

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Enable the scissor test;
		glEnable(GL_SCISSOR_TEST);
		glEnable(GL_BLEND);


		// Our projection matrix will be our entire window for now
		glm::mat4 proj = glm::ortho(0.0f, (float)windowSize.x, (float)windowSize.y, 0.0f, -1.0f, 1.0f);
		GuiBatcher::SetProjection(proj);

		// Iterate over and render all the GUI objects
		scene->RenderGUI();

		// Flush the Gui Batch renderer
		GuiBatcher::Flush();

		// Disable alpha blending
		glDisable(GL_BLEND);
		// Disable scissor testing
		glDisable(GL_SCISSOR_TEST);
		// Re-enable depth writing
		glDepthMask(GL_TRUE);

		// End our ImGui window
		ImGui::End();

		VertexArrayObject::Unbind();

		lastFrame = thisFrame;
		ImGuiHelper::EndFrame();
		InputEngine::EndFrame();
		glfwSwapBuffers(window);
	}

	// Clean up the ImGui library
	ImGuiHelper::Cleanup();

	// Clean up the resource manager
	ResourceManager::Cleanup();

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}