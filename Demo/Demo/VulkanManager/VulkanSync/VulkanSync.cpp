#include "VulkanSync.h"

Demo::VulkanSync::VulkanSync(VkDevice logicalDevice, int32_t imagesInFlightSize)
{
	this->logicalDeviceRef = logicalDevice;
	/*  Pass in the size of swapchainImages */
	maxFramesInFlight = imagesInFlightSize;
	createSyncObjects(imagesInFlightSize);
}

Demo::VulkanSync::~VulkanSync()
{
	for (int i = 0; i < maxFramesInFlight; i++) {
		vkDestroySemaphore(logicalDeviceRef, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(logicalDeviceRef, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(logicalDeviceRef, inFlightFences[i], nullptr);
	}
	std::cout << "Demo::VulkanSync::Destructor()" << std::endl;
}

std::vector<VkFence> Demo::VulkanSync::getInFlightFences()
{
	return inFlightFences;
}

std::vector<VkFence> Demo::VulkanSync::getImagesInFlight()
{
	return imagesInFlight;
}



std::vector<VkSemaphore> Demo::VulkanSync::getImageAvailableSemaphores()
{
	return imageAvailableSemaphores;
}

std::vector<VkSemaphore> Demo::VulkanSync::getRenderFinishedSemaphores()
{
	return renderFinishedSemaphores;
}

void Demo::VulkanSync::createSyncObjects(int32_t imagesInFlightSize)
{
	//THERE IS ONLY 2 FRAMES IN FLIGHT
	imageAvailableSemaphores.resize(maxFramesInFlight);
	renderFinishedSemaphores.resize(maxFramesInFlight);
	inFlightFences.resize(maxFramesInFlight);
	imagesInFlight.resize(imagesInFlightSize, VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < maxFramesInFlight; i++)
	{
		if (vkCreateSemaphore(logicalDeviceRef, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(logicalDeviceRef, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(logicalDeviceRef, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			std::cout << "VulkanRenderer: Failed to create synchronization objects for current frame!" << std::endl;
		}
	}
}

