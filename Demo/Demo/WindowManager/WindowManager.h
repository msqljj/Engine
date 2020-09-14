#pragma once
#include <Windows.h>
#include <string>
#include <chrono>
#include <Math/math.hpp>
#include <iostream>
#include <InputManager/InputManager.h>
#include <VulkanManager/VulkanManager.h>

#ifndef _WINDOWWMANAGER_H_
#define _WINDOWMANAGER_H_
namespace Demo
{
	struct DemoWindow
	{
		friend class WindowManager;
	public:
		HWND getHWND()
		{
			if (hWnd != nullptr)
			{
				return hWnd;
			}
			else
			{
				std::cout << "Failed to get HWND" << std::endl;
			}
		}
		const char* getTitle()
		{
			return windowTitle;
		}
	private:
		HINSTANCE hInstance = nullptr;
		HWND hWnd = nullptr;
		const char* windowClassName = "NEW_WND_CLASS";
		const char* windowTitle = "NEW_WINDOW";
		const char* textureFilePath = "";
		int width = 1280;
		int height = 800;
		int posX = 0;
		int posY = 0;
		bool windowShouldClose = false;

	};

	class WindowManager
	{
	public:
		VulkanManager* vulkanManagerRef = nullptr;
		InputManager* inputManagerRef = nullptr;
		WindowManager(InputManager* inputManager, VulkanManager* vulkanManager);
		WindowManager();
		~WindowManager();
		bool windowShouldClose(DemoWindow window);
		DemoWindow createStandardWindow(int width, int height, const char* applicationName);
		DemoWindow createSpashScreenWindow(int width, int height, const char* windowName, const char* bmpTexPath);
		static LRESULT CALLBACK WndProcRelay(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		bool exitFlag = false;
	private:

		bool customWindowFrame = false;
	};
}

#endif


