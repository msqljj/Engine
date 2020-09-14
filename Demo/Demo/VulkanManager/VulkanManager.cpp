#include "VulkanManager/VulkanManager.h"

Demo::VulkanManager::VulkanManager(InputManager* inputManager)
{
	std::cout << "VulkanManager::ctor()" << std::endl;
	if (inputManager)
	{
		this->inputManager = inputManager;
	}
}

Demo::VulkanManager::VulkanManager()
{
	std::cout << "VulkanManager::ctor()" << std::endl;
}

Demo::VulkanManager::~VulkanManager()
{
	vkDeviceWaitIdle(vulkanDevice->getLogicalDevice());

	delete vulkanUIManager;
	vulkanUIManager = 0;

	delete vulkanSync;
	vulkanSync = 0;

	delete vulkanTexture;
	vulkanTexture = 0;

	delete vulkanSwapchain;
	vulkanSwapchain = 0;

	delete vulkanDevice;
	vulkanDevice = 0;

	delete vulkanInstance;
	vulkanInstance = 0;

	std::cout << "VulkanManager::dtor()" << std::endl;
}

void Demo::VulkanManager::init(HWND hWnd)
{
	std::cout << "VulkanManager::ctor()" << std::endl;
	this->hWnd = hWnd;
	bool debug = true;
#ifdef NDEBUG 
	debug = false;
#endif

	vulkanInstance = new VulkanInstance(debug, this->hWnd);
	vulkanDevice = new VulkanDevice(vulkanInstance);
	vulkanSwapchain = new VulkanSwapchain(vulkanInstance, vulkanDevice);
	vulkanTexture = new VulkanTexture(vulkanDevice);
	vulkanSync = new VulkanSync(vulkanDevice->getLogicalDevice(), vulkanSwapchain->getSwapchainImagesSize());
	logicalDevice = vulkanDevice->getLogicalDevice();
	inFlightFences = vulkanSync->getInFlightFences();
	swapchain = vulkanSwapchain->getSwapchain();
	imageAvailableSemaphores = vulkanSync->getImageAvailableSemaphores();
	renderFinishedSemaphores = vulkanSync->getRenderFinishedSemaphores();
	maxFramesInFlight = vulkanSync->maxFramesInFlight;
	imagesInFlight = vulkanSync->getImagesInFlight();
	graphicsQueue = vulkanDevice->getGraphicsQueue();
	presentQueue = vulkanDevice->getPresentQueue();
	commandBuffers.resize(vulkanSwapchain->getSwapchainImagesSize());
	for (int i = 0; i < vulkanSwapchain->getSwapchainImagesSize(); ++i)
	{
		commandBuffers[i] = vulkanDevice->allocCommandBufferFromLocalPool(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	}
	vulkanUIManager = new VulkanUIManager(vulkanDevice, vulkanSwapchain, vulkanTexture, inputManager);
	buildCommandBuffer();
	prepared = true;
	//At this point, the vulkanManager is prepared. ui commandbuffer will not be submitted before ui i prepared.. So VulkanManager can start clearing the screen.


}

void Demo::VulkanManager::onResize(int32_t width, int32_t height)
{
	if (vulkanSwapchain)
	{
		prepared = false;
		vkDeviceWaitIdle(logicalDevice);
		vulkanSwapchain->resize(width, height);
		swapchain = vulkanSwapchain->getSwapchain();
		for (int i = 0; i < commandBuffers.size(); ++i)
		{
			vkFreeCommandBuffers(logicalDevice, vulkanDevice->getLocalCommandPool(), 1, &commandBuffers[i]);
			commandBuffers[i] = vulkanDevice->allocCommandBufferFromLocalPool(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		}
		vulkanUIManager->onResize(vulkanSwapchain->getSwapchainExtentWidth(), vulkanSwapchain->getSwapchainExtentHeight() );
		
		buildCommandBuffer();
		prepared = true;
		//Manager is now ready, since other commandBuffers wont be added to masterBuffer unless the module is ready, we can resize them now.
		//If this was multithreaded, then the other buffers would simply be added back whenever theyre ready, hence almost no wait time at all before resuming..
		//Though some things might not appear at once.
	}
}

void Demo::VulkanManager::render()
{
	//THe first time around this will not wait at all.
	//Wait for command buffer operations completion of the currentFrame
	vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	uint32_t imageIndex = 0; //Represents the acquired VkImage of our swapchainImages. 
	VkResult result = vkAcquireNextImageKHR(logicalDevice, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		if (exitFlag == false) { std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl; onResize(0, 0); }
		else
		{
			return;
		}
	}

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}
	// Mark the image as now being in use by this frame
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];


	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] }; //Present complete
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] }; //Render complete
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	//VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	//specifies the stage of the pipeline after blending where the final color values are output from the pipeline.
	//This stage also includes subpass loadand store operationsand multisample resolve operations for framebuffer attachments with a color or depth / stencil format.

	assembleCommandBuffer(imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;													// One Wait Semaphore
	submitInfo.pWaitSemaphores = waitSemaphores;										// Semaphore(s) to wait upon before the submitted command buffer starts executing
	submitInfo.pWaitDstStageMask = waitStages;											// Pointer to list of pipeline stages that semaphore waits will occur at
	submitInfo.commandBufferCount = static_cast<uint32_t>(masterCommandBuffer.size());  // One command buffer
	submitInfo.pCommandBuffers = masterCommandBuffer.data();							// Command buffer(s) to execute in this batch / submission
	submitInfo.signalSemaphoreCount = 1;												// One Signal Semaphore
	submitInfo.pSignalSemaphores = signalSemaphores;									// Semaphore(s) to be signaled when command buffers have completed

	vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}
	VkSwapchainKHR swapChains[] = { swapchain };
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;											//One Wait Semaphore
	presentInfo.pWaitSemaphores = signalSemaphores;								//Semaphore(s) to wait upon before image is presented
	presentInfo.swapchainCount = 1;												//Number of swapchains
	presentInfo.pSwapchains = swapChains;										//swapchain to present image from
	presentInfo.pImageIndices = &imageIndex;									//image index to image in the swapchain. 

	result = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		if (exitFlag == false)
		{
			std::cout << "VK_ERROR_OUT_OF_DATE_KHR! OR VK_SUBOPTIMAL_KHR! RESIZING!" << std::endl;
			onResize(0, 0);
		}
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}
	if (result == VK_SUCCESS)
	{
		renderCounter++;
	}
	currentFrame = (currentFrame + 1) % maxFramesInFlight;
}

void Demo::VulkanManager::update()
{

}

void Demo::VulkanManager::setClearColor(float r, float g, float b)
{
	clrR = r;
	clrG = g;
	clrB = b;
	buildCommandBuffer();
}

void Demo::VulkanManager::buildCommandBuffer()
{
	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	VkClearValue clearValues[2];
	clearValues[0].color = { { clrR, clrG, clrB, 0.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderPass = vulkanSwapchain->renderPass;
	renderPassBeginInfo.renderArea.extent.width = vulkanSwapchain->getSwapchainExtentWidth();
	renderPassBeginInfo.renderArea.extent.height = vulkanSwapchain->getSwapchainExtentHeight();
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;
	for (int32_t i = 0; i < commandBuffers.size(); ++i)
	{
		renderPassBeginInfo.framebuffer = vulkanSwapchain->swapchainFramebuffers[i];
		vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo);
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport = {};
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.width = static_cast<float>(vulkanSwapchain->getSwapchainExtentWidth());
		viewport.height = static_cast<float>(vulkanSwapchain->getSwapchainExtentHeight());
		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent.width = vulkanSwapchain->getSwapchainExtentWidth();
		scissor.extent.height = vulkanSwapchain->getSwapchainExtentHeight();
		vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);
		VkDeviceSize offsets[1] = { 0 };

		vkCmdEndRenderPass(commandBuffers[i]);
		vkEndCommandBuffer(commandBuffers[i]);
	}
}

void Demo::VulkanManager::assembleCommandBuffer(uint32_t imageIndex)
{
	masterCommandBuffer.clear();
	masterCommandBuffer.push_back(commandBuffers[imageIndex]);
	if (vulkanUIManager->prepared)
	{
		masterCommandBuffer.push_back(vulkanUIManager->cmdBuffers[imageIndex]);
	}
}
