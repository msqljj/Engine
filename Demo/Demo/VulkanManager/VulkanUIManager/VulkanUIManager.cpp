#include "VulkanUIManager.h"

Demo::VulkanUIManager::VulkanUIManager(VulkanDevice* vulkanDevice, VulkanSwapchain* vulkanSwapchain, VulkanTexture* vulkanTexture, InputManager* inputManager)
{
	if (vulkanDevice != nullptr)
	{
		this->vulkanDevice = vulkanDevice;
	}
	else
	{
		std::cout << "VulkanUI: vulkanDevice was nullptr?" << std::endl;
		return;
	}
	if (vulkanSwapchain != nullptr)
	{
		this->vulkanSwapchain = vulkanSwapchain;
	}
	else
	{
		std::cout << "VulkanUI: vulkanSwapchain was nullptr?" << std::endl;
		return;
	}
	if (vulkanTexture != nullptr)
	{
		this->vulkanTexture = vulkanTexture;
	}
	else
	{
		std::cout << "VulkanUI: vulkanTexture was nullptr?" << std::endl;
		return;
	}
	if (inputManager != nullptr)
	{
		this->inputManager = inputManager;
	}
	else
	{
		std::cout << "VulkanUI: inputManager was nullptr?" << std::endl;
		return;
	}
	cmdBuffers.resize(vulkanSwapchain->getSwapchainImagesSize());
	physialDevice = vulkanDevice->getPhysicalDevice();
	logicalDevice = vulkanDevice->getLogicalDevice();
	graphicsQueue = vulkanDevice->getGraphicsQueue();
	prepRenderPass(logicalDevice, vulkanSwapchain->getColorFormat(), vulkanSwapchain->getDepthFormat());
	prepCommandPool();
	prepCommandBuffer();
	vimgui = new Vimgui(vulkanDevice->getPhysicalDevice(), vulkanDevice->getLogicalDevice(), vulkanSwapchain->getSwapchainExtentWidth(), vulkanSwapchain->getSwapchainExtentHeight(), guiRenderPass, vulkanDevice->getQueueFamilyIndexGraphics(), vulkanDevice->getGraphicsQueue());
	initGUI();
}

Demo::VulkanUIManager::~VulkanUIManager()
{
	delete vimgui;
	vkDestroyRenderPass(logicalDevice, guiRenderPass, nullptr);
	std::cout << "VulkanUI:dtor()" << std::endl;
}

void Demo::VulkanUIManager::onResize(uint32_t width, uint32_t height)
{
	prepared = false;
	vkDeviceWaitIdle(vulkanDevice->getLogicalDevice());
	for (int i = 0; i < cmdBuffers.size(); ++i)
	{
		vkFreeCommandBuffers(logicalDevice, vulkanDevice->getLocalCommandPool(), 1, &cmdBuffers[i]);
		cmdBuffers[i] = vulkanDevice->allocCommandBufferFromLocalPool(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	}

	vimgui->vimguiCore->onResize(width, height);
	buildCommandBuffer();
	prepared = true;
}

void Demo::VulkanUIManager::initGUI()
{
	buildCommandBuffer();
}

void Demo::VulkanUIManager::prepRenderPass(VkDevice logicalDevice, VkFormat colorFormat, VkFormat depthFormat)
{
	VkAttachmentDescription attachments[2] = {};

	// Color attachment
	attachments[0].format = colorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; //Can also be load_op_clear
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE; //If you set this to dont care, then renderer doesnt give a fuck where your results from this RP will go
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Depth attachment
	attachments[1].format = depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Use subpass dependencies for image layout transitions
	VkSubpassDependency subpassDependencies[1] = {};
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = 0;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = NULL;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = subpassDependencies;

	VkResult result;
	result = vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &guiRenderPass);

}

void Demo::VulkanUIManager::prepCommandPool()
{
	//SHOULD HAVE ITS OWN POOL IF ITS ON ITS OWN THREAD!!!!!
}

void Demo::VulkanUIManager::prepCommandBuffer()
{
	for (int i = 0; i < cmdBuffers.size(); ++i)
	{
		cmdBuffers[i] = vulkanDevice->allocCommandBufferFromLocalPool(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	}
}

void Demo::VulkanUIManager::buildCommandBuffer()
{
	prepared = false;
	vkDeviceWaitIdle(logicalDevice);
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkClearValue clearValues[2];
	clearValues[1].color = { {0.0f, 0.0f, 0.0f, 0.0f} };

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderPass = guiRenderPass;
	renderPassBeginInfo.renderArea.extent.width = vulkanSwapchain->getSwapchainExtentWidth();
	renderPassBeginInfo.renderArea.extent.height = vulkanSwapchain->getSwapchainExtentHeight();
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (int32_t i = 0; i < cmdBuffers.size(); i++)
	{
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = vulkanSwapchain->swapchainFramebuffers[i];

		VkResult result;
		result = vkBeginCommandBuffer(cmdBuffers[i], &commandBufferBeginInfo);
		vkCmdBeginRenderPass(cmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewPort{};
		viewPort.minDepth = 0.0f;
		viewPort.maxDepth = 1.0f;
		viewPort.width = vulkanSwapchain->getSwapchainExtentWidth();
		viewPort.height = vulkanSwapchain->getSwapchainExtentHeight();
		vkCmdSetViewport(cmdBuffers[i], 0, 1, &viewPort);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent.width = vulkanSwapchain->getSwapchainExtentWidth();
		scissor.extent.height = vulkanSwapchain->getSwapchainExtentHeight();
		vkCmdSetScissor(cmdBuffers[i], 0, 1, &scissor);

		//Add module cmds
		vimgui->vimguiCore->buildCommandBuffer(cmdBuffers[i]);

		vkCmdEndRenderPass(cmdBuffers[i]);
		result = vkEndCommandBuffer(cmdBuffers[i]);
	}
	prepared = true;
}

void Demo::VulkanUIManager::keyboardEvent(uint32_t action, uint32_t key)
{
}

void Demo::VulkanUIManager::mouseMoveEvent(int32_t xpos, int32_t ypos)
{
}

void Demo::VulkanUIManager::mouseKeyEvent(uint32_t action, uint32_t key)
{
}

void Demo::VulkanUIManager::mouseScrollEvent(int32_t delta)
{
}

void Demo::VulkanUIManager::update(float dt)
{
}
