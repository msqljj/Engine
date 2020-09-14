#include "VulkanManager/VulkanTexture/VulkanTexture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Demo::VulkanTexture::VulkanTexture(VulkanDevice* vulkanDevice)
{
	if (vulkanDevice != nullptr)
	{
		this->vulkanDevice = vulkanDevice;
		this->logicalDevice = vulkanDevice->getLogicalDevice();
	}
	else
	{
		std::cout << "VulkanTexture: vulkanDevice was nullptr?" << std::endl;
	}
	std::cout << "VulkanTexture::ctor()" << std::endl;
}

Demo::VulkanTexture::~VulkanTexture()
{
	std::cout << "VulkanTexture::dtor()" << std::endl;
}

Demo::Texture* Demo::VulkanTexture::loadImage(const char* filepath)
{
	Texture* texture;
	texture = new Texture(logicalDevice);
	texture->filepath = filepath;
	std::cout << "VulkanTexture: Loading texture from filepath: " << filepath << std::endl;
	stbi_uc* image = stbi_load(filepath, &texture->textureWidth, &texture->textureHeight, &texture->textureChannels, STBI_rgb_alpha);
	if (!image)
	{
		std::cout << "VulkanTexture: Failed to load image at filepath: " << filepath << std::endl;
		MessageBox(NULL, "Failed to load texture...", "VulkanTexture", NULL);
	}
	else
	{
		std::cout << "VulkanTexture: Texture Loaded!" << " Width: " << texture->textureWidth << " Height: " << texture->textureHeight << " Channels: " << texture->textureChannels << std::endl;
	}
	texture->textureImageMemorySize = texture->textureWidth * texture->textureHeight * 4;
	Demo::Buffer* stagingBuffer;
	stagingBuffer = new Buffer(vulkanDevice->getLogicalDevice(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, texture->textureImageMemorySize);
	vulkanDevice->createBuffer(stagingBuffer, image);
	stbi_image_free(image);
	createImage(texture->textureWidth, texture->textureHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture->textureImage, texture->textureImageMemory);
	transitionImageLayout(texture->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer->buffer, texture->textureImage, static_cast<uint32_t>(texture->textureWidth), static_cast<uint32_t>(texture->textureHeight));
	transitionImageLayout(texture->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	stagingBuffer->destroy();
	delete stagingBuffer;
	stagingBuffer = 0;
	setupTexture(texture);
	return texture;
}

void Demo::VulkanTexture::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = usage;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result;
	result = vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, &image);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = vulkanDevice->findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(logicalDevice, image, imageMemory, 0);
}

void Demo::VulkanTexture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = vulkanDevice->allocCommandBufferFromLocalPool(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else 
	{
		throw std::invalid_argument("unsupported layout transition!");
	}
	vkCmdPipelineBarrier(commandBuffer,sourceStage, destinationStage,0,0, nullptr,0, nullptr,1, &barrier);
	vulkanDevice->freeCommandBuffers(commandBuffer, vulkanDevice->getLocalCommandPool(), vulkanDevice->getGraphicsQueue());
}

void Demo::VulkanTexture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = vulkanDevice->allocCommandBufferFromLocalPool(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {width, height, 1 };
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	vulkanDevice->freeCommandBuffers(commandBuffer, vulkanDevice->getLocalCommandPool(), vulkanDevice->getGraphicsQueue());
}

void Demo::VulkanTexture::setupTexture(Texture* texture)
{
	//TextureImageView------------------------------------------------------------------------
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = texture->textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	VkResult result;
	result = vkCreateImageView(logicalDevice, &viewInfo, nullptr, &texture->textureImageView);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanTexture: Failed to create image view for texture!" << std::endl;
	}
	//----------------------------------------------------------------------------------------

	//TextureSampler--------------------------------------------------------------------------
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	result = vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &texture->textureSampler);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanTexture: Failed to create image sampler for texture!" << std::endl;
	}
	//----------------------------------------------------------------------------------------

	//TextureDescriptor-----------------------------------------------------------------------
	texture->textureDescriptor.imageLayout = texture->textureImageLayout;
	texture->textureDescriptor.imageView = texture->textureImageView;
	texture->textureDescriptor.sampler = texture->textureSampler;
	//----------------------------------------------------------------------------------------
}
