#pragma once
#include <string>
#include <iostream>
#include <vulkan/vulkan.h>
#include <VulkanManager/VulkanDevice/vulkanDevice.h>


#ifndef _VULKANTEXTURE_H_
#define _VULKANTEXTURE_H_
namespace Demo
{
	struct Texture
	{
		Texture(VkDevice logicalDevice)
		{
			this->logicalDevice = logicalDevice;
		}
		~Texture()
		{
			if (!destroyed)
			{
				destroy();
			}
		}
		bool destroyed = false;
		const char* filepath;
		int textureWidth = 0;
		int textureHeight = 0;
		int textureChannels = 0;
		VkDevice logicalDevice = nullptr;
		VkImage textureImage = VK_NULL_HANDLE;
		VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
		VkDeviceSize textureImageMemorySize = 0;
		VkImageView textureImageView = VK_NULL_HANDLE;
		VkSampler textureSampler = VK_NULL_HANDLE;
		VkImageLayout textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkDescriptorImageInfo textureDescriptor;

		void destroy()
		{
			if (!destroyed)
			{
				if (logicalDevice != nullptr)
				{
					destroyed = true;
					if (textureSampler)
					{
						vkDestroySampler(logicalDevice, textureSampler, nullptr);
					}

					if (textureImageView)
					{
						vkDestroyImageView(logicalDevice, textureImageView, nullptr);
					}

					if (textureImage)
					{
						vkDestroyImage(logicalDevice, textureImage, nullptr);
					}

					if (textureImageMemory)
					{
						vkFreeMemory(logicalDevice, textureImageMemory, nullptr);
					}
				}
			}
			else
			{
				std::cout << "Demo::Texture: Destroy called, but device was nullptr?" << std::endl;
			}

		}
	};
	class VulkanTexture
	{
	public:
		VulkanTexture(VulkanDevice* vulkanDevice);
		~VulkanTexture();
		Texture* loadImage(const char* filepath);
	private:
		VulkanDevice* vulkanDevice = nullptr;
		VkDevice logicalDevice = nullptr;
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void setupTexture(Texture* texture);
	};

}

#endif
