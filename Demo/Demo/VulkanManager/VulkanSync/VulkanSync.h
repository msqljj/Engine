#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
namespace Demo
{
	class VulkanSync
	{
	public:
		VulkanSync(VkDevice logicalDevice, int32_t imagesInFlightSize);
		~VulkanSync();
		std::vector<VkFence> getInFlightFences();
		std::vector<VkFence> getImagesInFlight();
		std::vector<VkSemaphore> getImageAvailableSemaphores();
		std::vector<VkSemaphore> getRenderFinishedSemaphores();
		int maxFramesInFlight = 2;


	private:
		void createSyncObjects(int32_t imagesInFlightSize);

		VkDevice logicalDeviceRef = nullptr;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;

	};
}


