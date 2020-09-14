#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <VulkanManager/VulkanInstance/VulkanInstance.h>
#include <VulkanManager/VulkanDevice/VulkanDevice.h>

#ifndef _VULKANSWAPCHAIN_H_
#define _VULKANSWAPCHAIN_H_

namespace Demo
{
	class VulkanSwapchain
	{
	public:
		
		VulkanSwapchain(VulkanInstance* vulkanInstance, VulkanDevice* vulkanDevice);
		VulkanSwapchain();
		~VulkanSwapchain();
		void resize(uint32_t width, uint32_t height);
		VkSwapchainKHR getSwapchain();
		uint32_t getSwapchainImagesSize();
		uint32_t getSwapchainExtentWidth();
		uint32_t getSwapchainExtentHeight();
		VkFormat getColorFormat();
		VkFormat getDepthFormat();
		//Framebuffer------------------------------------
		std::vector<VkFramebuffer> swapchainFramebuffers;
		//-----------------------------------------------

		//RenderPass-------------------------------------
		VkRenderPass renderPass = nullptr;
		//-----------------------------------------------
	private:
		//Ref
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
		VkQueue graphicsQueue;
		QueueFamilyIndices queueFamilyIndices;
		//---

		//Swapchain-Variables----------------------------
		uint32_t screen_width = 0;
		uint32_t screen_height = 0;
		VkSurfaceKHR surface;
		VkSurfaceFormatKHR surfaceFormat;
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		VkPresentModeKHR presentMode;
		VkSwapchainKHR oldSwapchain = nullptr;
		VkSwapchainKHR swapchain = nullptr;
		VkExtent2D swapchainExtent;
		std::vector<VkImage> swapchainImages;
		VkFormat swapchainImageFormat; //or colorFormat
		uint32_t swapchainImagesCount = 0;
		std::vector<VkImageView> swapchainImageViews;
		//-----------------------------------------------



		//DepthResources---------------------------------
		VkFormat depthFormat;
		VkImage depthImage;
		VkImageView depthImageView;
		VkDeviceMemory depthImageMemory;
		//-----------------------------------------------



		//Swapchain---------------------------------------------------------------------------------
		void createSwapchain();
		void destroySwapchain();
		void recreateSwapchain();
		VkSurfaceFormatKHR getSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		VkPresentModeKHR getPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		VkExtent2D getSwapchainExtent(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		uint32_t getSwapchainImagesCount(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		//------------------------------------------------------------------------------------------

		//RenderPass--------------------------------------------------------------------------------
		void createRenderPass();
		void destroyRenderPass();
		VkFormat getSupportedDepthFormat(VkPhysicalDevice physicalDevice);
		//------------------------------------------------------------------------------------------

		//DepthResources----------------------------------------------------------------------------
		void createDepthResources();
		void destroyDepthResources();
		//------------------------------------------------------------------------------------------

		//Framebuffer-------------------------------------------------------------------------------
		void createFramebuffer();
		void destroyFramebuffer();
		//------------------------------------------------------------------------------------------


	};
}

#endif

