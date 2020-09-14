#pragma once
#include <string>
#include <iostream>
#include <Windows.h>
#include <InputManager/InputManager.h>
#include <VulkanManager/VulkanDevice/VulkanDevice.h>
#include <VulkanManager/VulkanInstance/VulkanInstance.h>
#include <VulkanManager/VulkanSwapchain/VulkanSwapchain.h>
#include <VulkanManager/VulkanTexture/VulkanTexture.h>
#include <VulkanManager/VulkanSync/VulkanSync.h>
#include <VulkanManager/VulkanUIManager/VulkanUIManager.h>

#ifndef _VULKANMANAGER_H_
#define _VULKANMANAGER_H_
namespace Demo
{
	class VulkanManager
	{
	public:
		VulkanManager(InputManager* inputManager);
		VulkanManager();
		~VulkanManager();
		void init(HWND hWnd);
		void onResize(int32_t width, int32_t height);
		void render();
		void update();
		bool prepared = false;
		bool exitFlag = false;
		void setClearColor(float r, float g, float b);
		uint32_t renderCounter = 0;
		VulkanUIManager* vulkanUIManager = nullptr;
	private:
		void buildCommandBuffer();
		void assembleCommandBuffer(uint32_t imageIndex);
		size_t currentFrame = 0;
		HWND hWnd;
		InputManager* inputManager = nullptr;
		VulkanInstance* vulkanInstance = nullptr;
		VulkanDevice* vulkanDevice = nullptr;
		VulkanSwapchain* vulkanSwapchain = nullptr;
		VulkanTexture* vulkanTexture = nullptr;
		VulkanSync* vulkanSync = nullptr;

		//Renderloop-Variables---------------------------------------
		std::vector<VkCommandBuffer> masterCommandBuffer;
		std::vector<VkCommandBuffer> commandBuffers;
		int maxFramesInFlight = 2;
		VkDevice logicalDevice;
		std::vector<VkFence> inFlightFences;
		VkSwapchainKHR swapchain;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> imagesInFlight;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		//------------------------------------------------------------
		float clrR = 0.0f, clrG = 0.0f, clrB = 0.0f;
	};
}

#endif



