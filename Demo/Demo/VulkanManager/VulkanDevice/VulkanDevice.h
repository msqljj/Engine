#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <cassert>
#include <vulkan/vulkan.h>
#include <VulkanManager/VulkanInstance/VulkanInstance.h>

#ifndef _VULKANDEVICE_H_
#define _VULKANDEVICE_H_

namespace Demo
{

	struct QueueFamilyIndices
	{
		uint32_t graphicsFamily;
		uint32_t transferFamily;
		uint32_t computeFamily;
		uint32_t presentFamily;
	};
	struct Buffer
	{
		Buffer(VkDevice device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize deviceSize) 
		{
			this->device = device;
			this->usageFlags = usageFlags;
			this->memoryPropertyFlags = memoryPropertyFlags;
			this->size = deviceSize;
		}
		~Buffer()
		{
			if (buffer != VK_NULL_HANDLE || memory != VK_NULL_HANDLE && !destroyed)
			{
				destroy();
			}
		}
		VkDevice device;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo descriptor;
		VkDeviceSize vertexCount = 0;
		VkDeviceSize size = 0;
		VkDeviceSize alignment = 0;
		VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
		void* mapped = nullptr;
		bool destroyed = false;
		VkBufferUsageFlags usageFlags;
		VkMemoryPropertyFlags memoryPropertyFlags;

		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
		{
			return vkMapMemory(device, memory, offset, size, 0, &mapped);
		}

		void unmap()
		{
			if (mapped)
			{
				vkUnmapMemory(device, memory);
				mapped = nullptr;
			}
		}

		VkResult bind(VkDeviceSize offset = 0)
		{
			return vkBindBufferMemory(device, buffer, memory, offset);
		}

		void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
		{
			descriptor.offset = offset;
			descriptor.buffer = buffer;
			descriptor.range = size;
		}

		void copyTo(void* data, VkDeviceSize size)
		{
			assert(mapped);
			memcpy(mapped, data, size);
		}

		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
		{
			VkMappedMemoryRange mappedRange = {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = memory;
			mappedRange.offset = offset;
			mappedRange.size = size;
			return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
		}

		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
		{
			VkMappedMemoryRange mappedRange = {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = memory;
			mappedRange.offset = offset;
			mappedRange.size = size;
			return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);

		}
		void destroy()
		{
			destroyed = true;
			if (device != VK_NULL_HANDLE)
			{
				if (buffer != VK_NULL_HANDLE)
				{
					vkDestroyBuffer(device, buffer, nullptr);
					buffer = VK_NULL_HANDLE;
				}
				if (memory != VK_NULL_HANDLE)
				{
					vkFreeMemory(device, memory, nullptr);
					buffer = VK_NULL_HANDLE;
				}
			}
		}

	};
	class VulkanDevice
	{
	public:
		VulkanDevice(VulkanInstance* vulkanInstance);
		~VulkanDevice();
		VkPhysicalDevice getPhysicalDevice();
		VkDevice getLogicalDevice();
		VkQueue getPresentQueue();
		VkQueue getGraphicsQueue();
		VkCommandPool getLocalCommandPool();
		uint32_t getQueueFamilyIndexGraphics();
		uint32_t getQueueFamilyIndexPresent();
		QueueFamilyIndices getQueueFamilyIndices();
		void createLocalCommandPool(uint32_t queueFamilyIndex = 10, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		VkCommandBuffer allocCommandBufferFromLocalPool(VkCommandBufferLevel commandBufferLevel, bool begin = false);
		VkCommandPool createCommandPool(uint32_t queueFamilyIndex = 10, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		VkCommandBuffer allocCommandBuffer(VkCommandBufferLevel level, VkCommandPool commandPool);
		void freeCommandBuffers(VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue queue);
		void createBuffer(Buffer* buffer, void* data = nullptr);
		uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr);
	private:
		void createDevice();
		VkPhysicalDevice selectDevice();
		std::vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance instance);
		std::vector<VkQueueFamilyProperties> getPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice);
		QueueFamilyIndices setQueueFamilyIndices(VkPhysicalDevice physicalDevice);
		std::vector<VkDeviceQueueCreateInfo> setDeviceQueueCreateInfos(QueueFamilyIndices queFam);
		bool isDeviceExtensionAvailable(VkPhysicalDevice physicalDevice, const char* desiredExtension);
		bool isDeviceLayerAvailable(VkPhysicalDevice physicalDevice, const char* desiredLayer);
		bool checkAvailableDeviceExtensions(VkPhysicalDevice physicalDevice, std::vector<const char*> desiredDeviceExtensions);
		bool checkAvailableDeviceLayers(VkPhysicalDevice physicalDevice, std::vector<const char*> desiredDeviceLayers);
		bool validation = false;
		VkInstance instanceRef = nullptr;
		VkSurfaceKHR surfaceRef = nullptr;
		VkPhysicalDevice physicalDevice = nullptr;
		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		VkPhysicalDeviceProperties physicalDeviceProperties;
		VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
		VkDevice logicalDevice = nullptr;
		VkQueue presentQueue = nullptr;
		VkQueue graphicsQueue = nullptr;
		std::vector<const char*> desired_device_layers = {};
		std::vector<const char*> desired_device_extensions = { "VK_KHR_swapchain" };
		VkCommandPool commandPool = nullptr;
		QueueFamilyIndices queueFamilyIndices;
	};
}

#endif


