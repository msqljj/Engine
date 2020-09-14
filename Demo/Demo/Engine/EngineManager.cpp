#include <Engine/EngineManager.h>
using namespace Demo;

EngineManager::EngineManager()
{
	std::cout << "EngineManager::ctor()" << std::endl;
	audioManager = new AudioManager();
	fileManager = new FileManager();
	inputManager = new InputManager();
	physicsManager = new PhysicsManager();
	vulkanManager = new VulkanManager(inputManager);
	windowManager = new WindowManager(inputManager, vulkanManager);
	sceneManager = new SceneManager();
}

EngineManager::~EngineManager()
{
	delete audioManager;
	audioManager = 0;

	delete fileManager;
	fileManager = 0;

	delete inputManager;
	inputManager = 0;

	delete physicsManager;
	physicsManager = 0;

	delete sceneManager;
	sceneManager = 0;

	delete vulkanManager;
	vulkanManager = 0;

	delete windowManager;
	windowManager = 0;
}

void EngineManager::main()
{
	checkMessages(); //Check incoming messages from windows
	//Calc current time and update respectively
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = previousTime - currentTime;
	deltaTime = static_cast<float>(elapsed.count());
	deltaTime = deltaTime * -1;
	previousTime = currentTime;
	accumulator += deltaTime;
	if (accumulator > 1 / updateLoopFreq)
	{
		update();
		accumulator = 0.0f;
	}
	render();
	auto tEnd = std::chrono::high_resolution_clock::now();
	duration1s = (float)(std::chrono::duration<double, std::milli>(tEnd - timestamp1s).count());
	if (duration1s > 1000.0f) //Everything inside this happens every second.
	{
		eng1sTimer();
		timestamp1s = tEnd;
	}
}

void EngineManager::set1SecondCallback(void callback())
{
	if (callback != nullptr)
	{
		t_1sTimer = callback;
	}
}

void Demo::EngineManager::eng1sTimer()
{
	updateStatisticWindowTitle();
	if (t_1sTimer != nullptr)
	{
		t_1sTimer(); //callback
	}
}

void EngineManager::checkMessages()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void EngineManager::update()
{
	updateFrameCounter++;
}

void EngineManager::render()
{
	if (vulkanManager)
	{
		if (vulkanManager->prepared)
		{
			vulkanManager->render();
		}
		renderFrameCounter = vulkanManager->renderCounter;
	}


}

void EngineManager::enableStatisticWindowTitle(DemoWindow* window)
{
	if (window != nullptr)
	{
		windowStatistics = window;
	}
}

void Demo::EngineManager::updateStatisticWindowTitle()
{
	if (windowStatistics != nullptr)
	{
		std::stringstream ss;
		ss << (windowStatistics->getTitle()) << " Update loop: " << updateFrameCounter << ("hz") << " - Render loop: " << renderFrameCounter << "hz" << " - dT: " << deltaTime << " - fps(dt calc): " << 1 / deltaTime;
		std::string titleText = ss.str();
		SetWindowText(windowStatistics->getHWND(), titleText.c_str());
		updateFrameCounter = 0;
		renderFrameCounter = 0;
		if (vulkanManager != nullptr)
		{
			vulkanManager->renderCounter = 0;
		}

	}
}

