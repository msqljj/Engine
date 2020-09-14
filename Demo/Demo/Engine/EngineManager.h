#pragma once
#include <iostream>
#include <sstream>
#include "AudioManager/AudioManager.h"
#include "FileManager/FileManager.h"
#include "InputManager/InputManager.h"
#include "PhysicsManager/PhysicsManager.h"
#include "SceneManager/SceneManager.h"
#include "VulkanManager/VulkanManager.h"
#include "WindowManager/WindowManager.h"


#ifndef _ENGINEMANAGER_H_
#define _ENGINEMANAGER_H_
namespace Demo 
{
	class EngineManager
	{
	public:
		AudioManager* audioManager = nullptr;
		FileManager* fileManager = nullptr;
		InputManager* inputManager = nullptr;
		PhysicsManager* physicsManager = nullptr;
		SceneManager* sceneManager = nullptr;
		VulkanManager* vulkanManager = nullptr;
		WindowManager* windowManager = nullptr;
		double timeElapsed = 0.0f;
		float deltaTime = 0.0f;
	private:
		std::chrono::high_resolution_clock::time_point startTime;
		std::chrono::high_resolution_clock::time_point previousTime;
		float accumulator = 0.0f;
		void (*t_1sTimer)() = 0; //Callback func
		float duration1s = 0; //Duration since last reset
		std::chrono::time_point<std::chrono::high_resolution_clock> timestamp1s;
		uint32_t updateFrameCounter = 0;
		uint32_t renderFrameCounter = 0;
		float updateLoopFreq = 60.0f;
		DemoWindow* windowStatistics;

	public:
		EngineManager();
		~EngineManager();
		void main();
		void set1SecondCallback(void callback());
		void enableStatisticWindowTitle(DemoWindow* window);
	private:
		void eng1sTimer();
		void checkMessages();
		void update();
		void render();
		void updateStatisticWindowTitle();
	};
}
#endif