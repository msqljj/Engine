#pragma once
#include <Windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <vector>
#include <iostream>

#ifndef _VULKANINSTANCE_H_
#define _VULKANINSTANCE_H_
namespace Demo 
{
	class VulkanInstance
	{
	public:
		VulkanInstance(bool validationEnable, HWND hwnd);
		~VulkanInstance();
		VkInstance getInstance();
		VkSurfaceKHR getSurface();

	private:
		HWND hWnd;
		VkInstance instance;
		VkSurfaceKHR surface;
		VkDebugUtilsMessengerEXT debugMessenger;
		bool validation = false;
		std::vector<const char*> desired_instance_layers = {};
		std::vector<const char*> desired_instance_extensions = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_utils" };

		void createInstance();
		void createSurface();
		bool isInstanceExtensionAvailable(const char* desiredExtension);
		bool isInstanceLayerAvailable(const char* desiredLayer);
		bool checkAvailableInstanceExtensions(std::vector<const char*> desiredInstanceExtensions);
		bool checkAvailableInstanceLayers(std::vector<const char*> desiredInstanceLayers);
		VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* callback);
		void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	};
}

#endif

