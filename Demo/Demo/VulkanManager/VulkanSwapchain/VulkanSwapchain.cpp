#include "VulkanManager/VulkanSwapchain/VulkanSwapchain.h"

Demo::VulkanSwapchain::VulkanSwapchain(VulkanInstance* vulkanInstance, VulkanDevice* vulkanDevice)
{
	std::cout << "VulkanSwapchain::ctor()" << std::endl;
	this->surface = vulkanInstance->getSurface();
	this->physicalDevice = vulkanDevice->getPhysicalDevice();
	this->logicalDevice = vulkanDevice->getLogicalDevice();
	this->queueFamilyIndices = vulkanDevice->getQueueFamilyIndices();

	createSwapchain();
	createRenderPass();
	createDepthResources();
	createFramebuffer();

}

Demo::VulkanSwapchain::VulkanSwapchain()
{
	std::cout << "VulkanSwapchain::ctor()" << std::endl;
}

Demo::VulkanSwapchain::~VulkanSwapchain()
{
	destroyFramebuffer();
	destroyDepthResources();
	destroyRenderPass();
	destroySwapchain();
	vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
	std::cout << "VulkanSwapchain::dtor()" << std::endl;
}

void Demo::VulkanSwapchain::resize(uint32_t width, uint32_t height)
{
	destroyFramebuffer();
	destroyDepthResources();
	destroySwapchain();
	recreateSwapchain();
	createDepthResources();
	createFramebuffer();

}

VkSwapchainKHR Demo::VulkanSwapchain::getSwapchain()
{
	if (swapchain)
	{
		return swapchain;
	}
}

uint32_t Demo::VulkanSwapchain::getSwapchainImagesSize()
{
	return swapchainImagesCount;
}

uint32_t Demo::VulkanSwapchain::getSwapchainExtentWidth()
{
	return swapchainExtent.width;
}

uint32_t Demo::VulkanSwapchain::getSwapchainExtentHeight()
{
	return swapchainExtent.height;
}

VkFormat Demo::VulkanSwapchain::getColorFormat()
{
	return swapchainImageFormat;
}

VkFormat Demo::VulkanSwapchain::getDepthFormat()
{
	return depthFormat;
}

void Demo::VulkanSwapchain::createSwapchain()
{
	VkResult result;
	surfaceFormat = getSurfaceFormat(physicalDevice, surface);
	presentMode = getPresentMode(physicalDevice, surface);
	swapchainExtent = getSwapchainExtent(physicalDevice, surface);
	swapchainImagesCount = getSwapchainImagesCount(physicalDevice, surface);
	surfaceCapabilities = getSurfaceCapabilities(physicalDevice, surface);
	uint32_t queueFamIndices[] = {queueFamilyIndices.graphicsFamily, queueFamilyIndices.presentFamily};

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = swapchainImagesCount;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageExtent = swapchainExtent;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.presentMode = presentMode;
	if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamIndices;
	}
	result = vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, nullptr, &swapchain);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanRenderer: Failed to create Swapchain!" << std::endl;
	}

	//swapchainImages---------------------------------------------------------------------------------
	result = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImagesCount, nullptr);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanRenderer: Failed to get SwapchainImages" << std::endl;
	}

	swapchainImages.resize(swapchainImagesCount);

	result = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImagesCount, swapchainImages.data());
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanRenderer: Failed to enumerate SwapchainImages" << std::endl;
	}

	swapchainImageFormat = surfaceFormat.format;
	//------------------------------------------------------------------------------------------------

	//swapchainImageViews-----------------------------------------------------------------------------
	swapchainImageViews.resize(swapchainImages.size());

	for (uint32_t i = 0; i < swapchainImages.size(); ++i)
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.image = swapchainImages[i];
		imageViewCreateInfo.format = swapchainImageFormat;


		result = vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, &swapchainImageViews[i]);
		if (result != VK_SUCCESS)
		{
			std::cout << "VulkanRenderer: Failed to create image views!" << std::endl;
		}
	}


}

void Demo::VulkanSwapchain::destroySwapchain()
{
	oldSwapchain = swapchain; // MUCH FASTER
	for (uint32_t i = 0; i < swapchainImageViews.size(); ++i)
	{
		vkDestroyImageView(logicalDevice, swapchainImageViews[i], nullptr);
	}
}

void Demo::VulkanSwapchain::recreateSwapchain()
{
	VkResult result;
	swapchainExtent = getSwapchainExtent(physicalDevice, surface);

	uint32_t queueFamIndices[] = { queueFamilyIndices.graphicsFamily, queueFamilyIndices.presentFamily };

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = oldSwapchain;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = swapchainImagesCount;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageExtent = swapchainExtent;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.presentMode = presentMode;

	if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamIndices;
	}

	result = vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, nullptr, &swapchain);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanRenderer: Failed to create Swapchain!" << std::endl;
	}

	vkDestroySwapchainKHR(logicalDevice, oldSwapchain, nullptr);


	//swapchainImages---------------------------------------------------------------------------------
	result = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImagesCount, nullptr);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanRenderer: Failed to get SwapchainImages" << std::endl;
	}
	swapchainImages.clear();
	swapchainImages.resize(swapchainImagesCount);

	result = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImagesCount, swapchainImages.data());
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanRenderer: Failed to enumerate SwapchainImages" << std::endl;
	}

	swapchainImageFormat = surfaceFormat.format;
	//------------------------------------------------------------------------------------------------

	//swapchainImageViews-----------------------------------------------------------------------------
	swapchainImageViews.clear();
	swapchainImageViews.resize(swapchainImages.size());

	for (uint32_t i = 0; i < swapchainImages.size(); ++i)
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.image = swapchainImages[i];
		imageViewCreateInfo.format = swapchainImageFormat;


		result = vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, &swapchainImageViews[i]);
		if (result != VK_SUCCESS)
		{
			std::cout << "VulkanRenderer: Failed to create image views!" << std::endl;
		}
	}
	//------------------------------------------------------------------------------------------------
}

VkSurfaceFormatKHR Demo::VulkanSwapchain::getSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	VkSurfaceFormatKHR surfaceFormat;
	bool formatSelected = false;
	uint32_t formatCount = 0;
	VkResult result;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanRenderer: Could not get the number of surface formats!" << std::endl;
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);

	if (formatCount != 0)
	{
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());
		for (const auto& availableFormat : surfaceFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				//Check for format and colorspace
				surfaceFormat = availableFormat;
				formatSelected = true;
			}
			if (formatSelected == false && availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				//Check for format Only
				surfaceFormat = availableFormat;
				formatSelected = true;
			}

			else if (formatSelected == false)
			{
				surfaceFormat = surfaceFormats[0]; //If the specified format wast found, just pick one..Throw a warning though
				std::cout << "swapChain error: Specified surface format not found!" << std::endl;
			}
		}

	}
	return surfaceFormat;
}

VkPresentModeKHR Demo::VulkanSwapchain::getPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	VkPresentModeKHR present_mode;
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount); // What if present mode is 0??

	if (presentModeCount != 0)
	{
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

		//Try to find VK_PRESENT_MODE_MAILBOX_KHR (no VSYNC, max fps)
		for (const auto& availablePresentMode : presentModes)
		{
			if (availablePresentMode == 1)
			{
				present_mode = availablePresentMode;

			}
		}
		//If VK_PRESENT_MODE_MAILBOX_KHR was not found, then just take whatever's available.
		if (present_mode == !VK_PRESENT_MODE_MAILBOX_KHR)
		{
			for (const auto& availablePresentMode : presentModes)
			{
				present_mode = availablePresentMode;
			}
		}

	}

	return present_mode;
}

VkExtent2D Demo::VulkanSwapchain::getSwapchainExtent(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
		VkExtent2D swapchain_extent;
	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surface_capabilities);

	if (surface_capabilities.currentExtent.width != UINT32_MAX)
	{
		swapchain_extent = surface_capabilities.currentExtent;
		screen_width = surface_capabilities.currentExtent.width;
		screen_height = surface_capabilities.currentExtent.height;
	}
	else
	{
		swapchain_extent.width = screen_width;
		swapchain_extent.height = screen_height;
		if (screen_width == 0)
		{
			swapchain_extent.width = 800;
			swapchain_extent.height = 600;
		}
	}
	return swapchain_extent;
}

uint32_t Demo::VulkanSwapchain::getSwapchainImagesCount(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	uint32_t swapchain_images_count;
	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surface_capabilities);
	swapchain_images_count = surface_capabilities.minImageCount + 1; //On Quadro 1000, min is 2+(1), max is 8.

	if (surface_capabilities.maxImageCount > 0 && swapchain_images_count > surface_capabilities.maxImageCount)
	{
		swapchain_images_count = surface_capabilities.maxImageCount;
	}

	return swapchain_images_count;
}

VkSurfaceCapabilitiesKHR Demo::VulkanSwapchain::getSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surface_capabilities);
	return surface_capabilities;
}

void Demo::VulkanSwapchain::createRenderPass()
{
	VkResult result;
	depthFormat = getSupportedDepthFormat(physicalDevice);

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = depthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassCreateInfo.pAttachments = attachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &dependency;

	result = vkCreateRenderPass(logicalDevice, &renderPassCreateInfo, nullptr, &renderPass);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanRenderer: Failed to create RenderPass!" << std::endl;
	}
}

void Demo::VulkanSwapchain::destroyRenderPass()
{
	if (renderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
	}
}

VkFormat Demo::VulkanSwapchain::getSupportedDepthFormat(VkPhysicalDevice physicalDevice)
{
	VkFormat depth_format;
	std::vector<VkFormat> reqDepthFormats{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkFormatFeatureFlags formatFeatureFlags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	for (VkFormat format : reqDepthFormats)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
		{
			depth_format = format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
		{
			depth_format = format;
		}

	}

	if (depth_format == 0)
	{
		std::cout << "VulkanRenderer: depthFormat not found!" << std::endl;
	}

	return depth_format;
}

void Demo::VulkanSwapchain::createDepthResources()
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.extent.width = swapchainExtent.width;
	imageCreateInfo.extent.height = swapchainExtent.height;
	imageCreateInfo.format = depthFormat;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkResult result = vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, &depthImage);

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(logicalDevice, depthImage, &memReq);

	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

	uint32_t memTypeIndex = 0;
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
	{
		if ((memReq.memoryTypeBits & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			memTypeIndex = i;
		}
	}

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memReq.size;
	memoryAllocateInfo.memoryTypeIndex = memTypeIndex;

	result = vkAllocateMemory(logicalDevice, &memoryAllocateInfo, nullptr, &depthImageMemory);
	result = vkBindImageMemory(logicalDevice, depthImage, depthImageMemory, 0);

	//Depth-Image-View
	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.image = depthImage;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = depthFormat;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	result = vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, &depthImageView);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanRenderer: Failed to create depthImageView" << std::endl;
	}
}

void Demo::VulkanSwapchain::destroyDepthResources()
{
	vkDestroyImageView(logicalDevice, depthImageView, nullptr);
	vkDestroyImage(logicalDevice, depthImage, nullptr);
	vkFreeMemory(logicalDevice, depthImageMemory, nullptr);
}

void Demo::VulkanSwapchain::createFramebuffer()
{
	VkResult result;
	swapchainFramebuffers.clear();
	swapchainFramebuffers.resize(swapchainImageViews.size());

	for (size_t i = 0; i < swapchainImageViews.size(); i++)
	{
		std::vector<VkImageView> attachments = { swapchainImageViews[i], depthImageView };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapchainExtent.width;
		framebufferInfo.height = swapchainExtent.height;
		framebufferInfo.layers = 1;

		result = vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapchainFramebuffers[i]);
		if (result != VK_SUCCESS)
		{
			std::cout << "VulkanRenderer: Failed to create framebuffer!" << std::endl;
		}
	}
}

void Demo::VulkanSwapchain::destroyFramebuffer()
{
	for (uint32_t i = 0; i < swapchainFramebuffers.size(); ++i)
	{
		vkDestroyFramebuffer(logicalDevice, swapchainFramebuffers[i], nullptr);
	}
}

