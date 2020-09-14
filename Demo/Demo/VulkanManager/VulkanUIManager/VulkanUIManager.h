#pragma once
#include <InputManager/InputManager.h>
#include <VulkanManager/VulkanDevice/VulkanDevice.h>
#include <VulkanManager/VulkanSwapchain/VulkanSwapchain.h>
#include <VulkanManager/VulkanTexture/VulkanTexture.h>
#include <InputManager/Keyboard/keyboard.h>
#include <InputManager/Mouse/mouse.h>
#include <VulkanManager/VulkanUIManager/VimGUI/Vimgui.h>

namespace Demo
{

	class VulkanUIManager
	{
	public:
		VulkanUIManager(VulkanDevice* vulkanDevice, VulkanSwapchain* vulkanSwapchain, VulkanTexture* vulkanTexture, InputManager* inputManager);
		~VulkanUIManager();
		void onResize(uint32_t width, uint32_t height);
		void buildCommandBuffer();
		void keyboardEvent(uint32_t action, uint32_t key); 
		void mouseMoveEvent(int32_t xpos, int32_t ypos); 
		void mouseKeyEvent(uint32_t action, uint32_t key); 
		void mouseScrollEvent(int32_t delta);

		std::vector<VkCommandBuffer> cmdBuffers;
		Vimgui* vimgui = nullptr;
		VkRenderPass guiRenderPass = nullptr;
		InputManager* inputManager = nullptr;
		bool prepared = false;

	private:
		void initGUI();
		void prepRenderPass(VkDevice logicalDevice, VkFormat colorFormat, VkFormat depthFormat);
		void prepCommandPool();
		void prepCommandBuffer();
		void update(float dt);

		VulkanDevice* vulkanDevice = nullptr;
		VulkanSwapchain* vulkanSwapchain = nullptr;
		VulkanTexture* vulkanTexture = nullptr;
		VkPhysicalDevice physialDevice = nullptr;
		VkDevice logicalDevice = nullptr;
		VkQueue graphicsQueue = nullptr;
		bool resizing = false;

	};

}

