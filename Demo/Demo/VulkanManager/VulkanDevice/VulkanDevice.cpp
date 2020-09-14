#include "VulkanManager/VulkanDevice/VulkanDevice.h"

Demo::VulkanDevice::VulkanDevice(VulkanInstance* vulkanInstance)
{
	std::cout << "VulkanDevice:ctor()" << std::endl;
	this->instanceRef = vulkanInstance->getInstance();
	this->surfaceRef = vulkanInstance->getSurface();
	createDevice();
	createLocalCommandPool();
}

Demo::VulkanDevice::~VulkanDevice()
{
	if (commandPool)
	{
		vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
	}

	if (logicalDevice)
	{
		vkDestroyDevice(logicalDevice, nullptr);
	}
	else
	{
		std::cout << "VulkanDevice::Destructor()--logicalDevice was 0 at destruction?" << std::endl;
	}

	std::cout << "VulkanDevice::Destructor()" << std::endl;
}

VkPhysicalDevice Demo::VulkanDevice::getPhysicalDevice()
{
	if (physicalDevice)
		return physicalDevice;
	else
		return nullptr;
}

VkDevice Demo::VulkanDevice::getLogicalDevice()
{
	if (logicalDevice)
		return logicalDevice;
	else
		return nullptr;
}

VkQueue Demo::VulkanDevice::getPresentQueue()
{
	if (presentQueue)
		return presentQueue;
	else
		return nullptr;
}

VkQueue Demo::VulkanDevice::getGraphicsQueue()
{
	if (graphicsQueue)
		return graphicsQueue;
	else
		return nullptr;
}

VkCommandPool Demo::VulkanDevice::getLocalCommandPool()
{
	if (commandPool)
	{
		return commandPool;
	}
	else
	{
		return nullptr;
	}
}

uint32_t Demo::VulkanDevice::getQueueFamilyIndexGraphics()
{
	return queueFamilyIndices.graphicsFamily;
}

uint32_t Demo::VulkanDevice::getQueueFamilyIndexPresent()
{
	return queueFamilyIndices.presentFamily;
}

Demo::QueueFamilyIndices Demo::VulkanDevice::getQueueFamilyIndices()
{
	Demo::QueueFamilyIndices returner = queueFamilyIndices;
	return returner;
}

void Demo::VulkanDevice::createLocalCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
	if (queueFamilyIndex == 10) //Default 
	{
		queueFamilyIndex = getQueueFamilyIndexGraphics();
	}


	VkResult result;
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	commandPoolCreateInfo.flags = flags;
	result = vkCreateCommandPool(logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Failed to Create Local CommandPool" << std::endl;
	}

}

VkCommandBuffer Demo::VulkanDevice::allocCommandBufferFromLocalPool(VkCommandBufferLevel commandBufferLevel, bool begin)
{
	VkResult result;

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = commandBufferLevel;
	allocInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffer;
	result = vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Failed to allocate commandbuffer!" << std::endl;
		return nullptr;
	}
	else
	{
		if (begin)
		{
			VkCommandBufferBeginInfo commandBufferBeginInfo{};
			commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			VkResult result;
			result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
			if (result != VK_SUCCESS)
			{
				std::cout << "VulkanDevice: Failed to begin commandBuffer!!" << std::endl;
			}
		}
		return commandBuffer;
	}
}

VkCommandPool Demo::VulkanDevice::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
	if (queueFamilyIndex == 10) //Default
	{
		queueFamilyIndex = getQueueFamilyIndexGraphics();
	}

	VkResult result;
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	commandPoolCreateInfo.flags = flags;
	VkCommandPool newCommandPool;
	result = vkCreateCommandPool(logicalDevice, &commandPoolCreateInfo, nullptr, &newCommandPool);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Failed to Create CommandPool" << std::endl;
		return nullptr;
	}

	if (result == VK_SUCCESS)
	{
		return newCommandPool;
	}
}

VkCommandBuffer Demo::VulkanDevice::allocCommandBuffer(VkCommandBufferLevel level, VkCommandPool commandPool)
{
	VkResult result;

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = level;
	allocInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffer;
	result = vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Failed to allocate commandbuffer!" << std::endl;
		return nullptr;
	}
	else
	{
		return commandBuffer;
	}

}

void Demo::VulkanDevice::freeCommandBuffers(VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue queue)
{
	if (commandBuffer == VK_NULL_HANDLE)
	{
		return;
	}

	vkEndCommandBuffer(commandBuffer);
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;
	VkFence fence;
	vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence);
	// Submit to the queue
	vkQueueSubmit(queue, 1, &submitInfo, fence);
	// Wait for the fence to signal that command buffer has finished executing
	vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, 100000000000);
	vkDestroyFence(logicalDevice, fence, nullptr);
	vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

void Demo::VulkanDevice::createBuffer(Buffer* buffer, void* data)
{
	buffer->device = logicalDevice;
	VkResult result;

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = buffer->usageFlags;
	bufferCreateInfo.size = buffer->size;
	result = vkCreateBuffer(buffer->device, &bufferCreateInfo, nullptr, &buffer->buffer);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Failed to create buffer!" << std::endl;
	}

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(logicalDevice, buffer->buffer, &memoryRequirements);
	buffer->alignment = memoryRequirements.alignment;

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, buffer->memoryPropertyFlags);
	result = vkAllocateMemory(buffer->device, &memoryAllocateInfo, nullptr, &buffer->memory);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Failed to allocate buffer memory!" << std::endl;
	}

	if (data != nullptr)
	{
		VkResult result;
		result = buffer->map();
		if (result != VK_SUCCESS)
		{
			std::cout << "VulkanDevice: Failed to map memory in buffer" << std::endl;
		}
		memcpy(buffer->mapped, data, buffer->size);
		if ((buffer->memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
		{
			std::cout << "VulkanDevice: Flushed" << std::endl;
			buffer->flush();
		}

		buffer->unmap();
	}
	buffer->setupDescriptor();
	result = buffer->bind();
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Failed to bind buffer memory!" << std::endl;
	}
}

void Demo::VulkanDevice::createDevice()
{
	physicalDevice = selectDevice();
	if (!physicalDevice)
	{
		std::cout << "VulkanDevice: Device not selected! ERROR" << std::endl;
	}
	queueFamilyIndices = setQueueFamilyIndices(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = setDeviceQueueCreateInfos(queueFamilyIndices);

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(desired_device_extensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = desired_device_extensions.data();
	deviceCreateInfo.enabledLayerCount = 0;
	if (validation)
	{
		std::cout << "VulkanDevice: Validation requested" << std::endl;
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(desired_device_layers.size());
		deviceCreateInfo.ppEnabledLayerNames = desired_device_layers.data();
	}
	VkResult result;
	result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Failed to Create Device!" << std::endl;
	}
	if (result == VK_SUCCESS)
	{
		std::cout << "VulkanDevice: createDevice = VK_SUCCESS!" << std::endl;
	}


	vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, queueFamilyIndices.presentFamily, 0, &presentQueue);
}

VkPhysicalDevice Demo::VulkanDevice::selectDevice()
{
	VkPhysicalDevice vk_physicalDevice = nullptr;
	std::vector<VkPhysicalDevice> devices = enumeratePhysicalDevices(instanceRef);
	VkResult result;
	VkBool32 deviceFound = VK_FALSE;
	//Foreach device in devices
	for (int i = 0; i < devices.size(); ++i)
	{
		std::vector<VkQueueFamilyProperties> queueFamilyProperties = getPhysicalDeviceQueueFamilyProperties(devices[i]);

		//Foreach queueFamily in queueFamilies
		// if this? then : else
		VkBool32 presentSupport = false;
		for (int j = 0; j < queueFamilyProperties.size(); ++j)
		{
			VkBool32 graphicsQueue = false;
			VkBool32 transferQueue = false;
			VkBool32 computeQueue = false;
			VkBool32 present = false;

			const char* empty = "        ";
			const char* graphics_bit = "GRAPHICS";
			const char* transfer_bit = "TRANSFER";
			const char* compute_bit = "COMPUTE ";
			const char* present_support = "PRESENT ";

			if ((queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 1)
			{
				graphicsQueue = true;
			}
			if ((queueFamilyProperties[j].queueFlags & VK_QUEUE_TRANSFER_BIT) == 4)
			{
				transferQueue = true;
			}
			if ((queueFamilyProperties[j].queueFlags & VK_QUEUE_COMPUTE_BIT) == 2)
			{
				computeQueue = true;
			}
			//Checks if the GPU supports presenting data to the Surface.
			result = vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, surfaceRef, &presentSupport);
			if (result != VK_SUCCESS)
			{
				std::cout << "VulkanDevice: Device does NOT support presenting to Surface" << std::endl;
			}

			if (result == VK_SUCCESS)
			{
				present = true;
			}
			//VkPhysicalDeviceProperties physicalDeviceProperties;
			//vkGetPhysicalDeviceProperties(devices[i], &physicalDeviceProperties);
			//std::cout << "VulkanDevice: Physical device selected: " << physicalDeviceProperties.deviceName << std::endl;
			//std::cout << "DEVICE: " << i << " | Queue Index: " << j  << " | Nr of Queues: " << queueFamilyProperties[j].queueCount << " | Flags: " << (graphicsQueue ? graphics_bit : empty) << " | " << (transferQueue ? transfer_bit : empty) << " | " << (computeQueue ? compute_bit : empty) << " | " << (present ? present_support : empty) << std::endl;;
		}

		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(devices[i], &physicalDeviceProperties);
		//Query device Memory Properties
		uint64_t randomAccessMemory; //Memory heaps can return a very large number. uint32_t will only work for up to 4gb RAM.
		uint32_t byteMB = 1048576; //Conversion factor for bytes to MB
		uint32_t selectedCardMB = 0;
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(devices[i], &deviceMemoryProperties);
		randomAccessMemory = deviceMemoryProperties.memoryHeaps->size;
		randomAccessMemory = randomAccessMemory / byteMB;

		if (randomAccessMemory > selectedCardMB&& presentSupport&& physicalDeviceProperties.deviceType == 2 && checkAvailableDeviceExtensions(devices[i], desired_device_extensions) && checkAvailableDeviceLayers(devices[i], desired_device_layers))
		{
			//This function checks is the card found has more ram than previous, or 0 ram.
			//Then it checks if the card has present to surface support
			//Then it checks if its a discrete graphics card
			//Then it checks if the device supports the enabled device extensions, and layers.
			selectedCardMB = static_cast<uint32_t>(randomAccessMemory);
			vk_physicalDevice = devices[i];
			deviceFound = VK_TRUE;
		}

		if (deviceFound == VK_FALSE && randomAccessMemory > selectedCardMB&& presentSupport&& checkAvailableDeviceExtensions(devices[i], desired_device_extensions) && checkAvailableDeviceLayers(devices[i], desired_device_layers))
		{
			//If no discrete device was found then do this check. 
			selectedCardMB = static_cast<uint32_t>(randomAccessMemory);
			vk_physicalDevice = devices[i];
			deviceFound = VK_TRUE;
		}


	}
	//We were first looking for a dedicated GPU, if this wasnt found, & we're at the end of our
	if (vk_physicalDevice == nullptr)
	{
		std::cout << "VulkanDevice: Dedicated GPU NOT FOUND! Picking the first availiable device" << std::endl;
		vk_physicalDevice = devices[0];
	}
	if (vk_physicalDevice == nullptr)
	{
		std::cout << "VulkanDevice: NO GPU SUPPORTED!" << std::endl;
	}
	if (vk_physicalDevice)
	{
		vkGetPhysicalDeviceProperties(vk_physicalDevice, &physicalDeviceProperties);
		vkGetPhysicalDeviceProperties(vk_physicalDevice, &physicalDeviceProperties);
		vkGetPhysicalDeviceMemoryProperties(vk_physicalDevice, &physicalDeviceMemoryProperties);
		std::cout << "VulkanDevice: Physical device selected: " << physicalDeviceProperties.deviceName << std::endl;
		return vk_physicalDevice;
	}
}

std::vector<VkPhysicalDevice> Demo::VulkanDevice::enumeratePhysicalDevices(VkInstance instance)
{
	//Find number of devices
	uint32_t deviceCount = 0;
	VkResult result;
	result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Could not get the number of physical Devices." << std::endl;
		return{};
	}

	//Query device information, store it in a vector
	std::vector<VkPhysicalDevice> devices(deviceCount);
	result = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Could not enumerate physical Devices." << std::endl;
		return{};
	}
	return devices;
}

std::vector<VkQueueFamilyProperties> Demo::VulkanDevice::getPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice)
{
	//Find number of queueFamilies
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	//Query device information, store it in a vector
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	return queueFamilies;
}

Demo::QueueFamilyIndices Demo::VulkanDevice::setQueueFamilyIndices(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices vk_queueFamilyIndices;
	vk_queueFamilyIndices.graphicsFamily = 0;
	vk_queueFamilyIndices.transferFamily = 0;
	vk_queueFamilyIndices.computeFamily = 0;
	vk_queueFamilyIndices.presentFamily = 0;
	std::vector<VkQueueFamilyProperties> queueFamilyProperties = getPhysicalDeviceQueueFamilyProperties(physicalDevice);
	VkResult result;
	VkBool32 presentSupport = false;

	for (int j = 0; j < queueFamilyProperties.size(); ++j)
	{
		VkBool32 graphicsQueue = false;
		VkBool32 transferQueue = false;
		VkBool32 computeQueue = false;
		VkBool32 present = false;

		const char* empty = "    x   ";
		const char* graphics_bit = "GRAPHICS";
		const char* transfer_bit = "TRANSFER";
		const char* compute_bit = "COMPUTE ";
		const char* present_support = "PRESENT ";

		if ((queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 1)
		{
			graphicsQueue = true;
		}
		if ((queueFamilyProperties[j].queueFlags & VK_QUEUE_TRANSFER_BIT) == 4)
		{
			transferQueue = true;
		}
		if ((queueFamilyProperties[j].queueFlags & VK_QUEUE_COMPUTE_BIT) == 2)
		{
			computeQueue = true;
		}
		//Checks if the GPU supports presenting data to the Surface.
		result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, j, surfaceRef, &presentSupport);
		if (result != VK_SUCCESS)
		{
			std::cout << "VulkanDevice: Device does NOT support presenting to Surface" << std::endl;
		}

		if (result == VK_SUCCESS)
		{
			present = true;
		}

		//To take this a step further, you may want to search for dedicated queues, and flag them if found, then only search for whats not found.
		//NVIDIA usually keeps 16 queue of everything in the first fam
		//AMD usually has 4 fams with a bunch of different stuff
		//Intel usually has 1 fam

		if ((queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 1 && (queueFamilyProperties[j].queueFlags & VK_QUEUE_TRANSFER_BIT) == 4 && (queueFamilyProperties[j].queueFlags & VK_QUEUE_COMPUTE_BIT) == 2 && present)
		{
			vk_queueFamilyIndices.graphicsFamily = j;
			vk_queueFamilyIndices.transferFamily = j;
			vk_queueFamilyIndices.computeFamily = j;
			vk_queueFamilyIndices.presentFamily = j;
		}
		else if ((queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 1 && present)
		{
			vk_queueFamilyIndices.graphicsFamily = j;
			std::cout << "VulkanDevice: You may run into issues because of queue families." << std::endl;
		}


		std::cout << "VulkanDevice: Queue Index: " << j << " | Nr of Queues: " << queueFamilyProperties[j].queueCount << "\t | Flags: " << (graphicsQueue ? graphics_bit : empty) << " | " << (transferQueue ? transfer_bit : empty) << " | " << (computeQueue ? compute_bit : empty) << " | " << (present ? present_support : empty) << std::endl;;

	}
	return vk_queueFamilyIndices;
}

std::vector<VkDeviceQueueCreateInfo> Demo::VulkanDevice::setDeviceQueueCreateInfos(QueueFamilyIndices queFam)
{
	std::set<uint32_t> uniqueQueueFamilies = { queFam.graphicsFamily, queFam.presentFamily, queFam.computeFamily, queFam.transferFamily };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	static float queuePriority = 1.0f;

	if (uniqueQueueFamilies.size() == 4)
	{
		VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
		deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
		deviceQueueCreateInfo.queueFamilyIndex = queFam.graphicsFamily;
		deviceQueueCreateInfo.queueCount = 1;

		queueCreateInfos.push_back(deviceQueueCreateInfo);
		deviceQueueCreateInfo.queueFamilyIndex = queFam.presentFamily;
		queueCreateInfos.push_back(deviceQueueCreateInfo);
		deviceQueueCreateInfo.queueFamilyIndex = queFam.transferFamily;
		queueCreateInfos.push_back(deviceQueueCreateInfo);
		deviceQueueCreateInfo.queueFamilyIndex = queFam.computeFamily;
		queueCreateInfos.push_back(deviceQueueCreateInfo);

		return queueCreateInfos;
	}

	if (uniqueQueueFamilies.size() == 3)
	{
		VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
		deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
		deviceQueueCreateInfo.queueFamilyIndex = queFam.graphicsFamily;
		deviceQueueCreateInfo.queueCount = 1;

		queueCreateInfos.push_back(deviceQueueCreateInfo);
		deviceQueueCreateInfo.queueFamilyIndex = queFam.presentFamily;
		queueCreateInfos.push_back(deviceQueueCreateInfo);
		deviceQueueCreateInfo.queueFamilyIndex = queFam.transferFamily;
		queueCreateInfos.push_back(deviceQueueCreateInfo);
		return queueCreateInfos;
	}

	if (uniqueQueueFamilies.size() == 2)
	{
		VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
		deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
		deviceQueueCreateInfo.queueFamilyIndex = queFam.graphicsFamily;
		deviceQueueCreateInfo.queueCount = 1;

		queueCreateInfos.push_back(deviceQueueCreateInfo);
		deviceQueueCreateInfo.queueFamilyIndex = queFam.presentFamily;
		queueCreateInfos.push_back(deviceQueueCreateInfo);

		return queueCreateInfos;
	}


	if (uniqueQueueFamilies.size() == 1)
	{
		VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
		deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
		deviceQueueCreateInfo.queueFamilyIndex = queFam.graphicsFamily;
		deviceQueueCreateInfo.queueCount = 1;

		queueCreateInfos.push_back(deviceQueueCreateInfo);
		return queueCreateInfos;
	}
}

bool Demo::VulkanDevice::isDeviceExtensionAvailable(VkPhysicalDevice physicalDevice, const char* desiredExtension)
{
	char* extensionFound = NULL;
	uint32_t desiredExtensionsFound = 0;
	uint32_t extensions_count = 0;

	VkResult result;
	result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensions_count, nullptr);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Could not get the number of device extensions!" << std::endl;
	}

	std::vector<VkExtensionProperties> extensionProps(extensions_count);
	result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensions_count, extensionProps.data());
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Could not enumerate device extensions!" << std::endl;
	}

	for (uint32_t i = 0; i < extensions_count; ++i)
	{
		if (strcmp(extensionProps[i].extensionName, desiredExtension) == 0)
		{
			return true;
		}
	}
	return false;
}

bool Demo::VulkanDevice::isDeviceLayerAvailable(VkPhysicalDevice physicalDevice, const char* desiredLayer)
{
	char* layerFound = NULL;
	uint32_t desiredLayersFound = 0;
	uint32_t layers_count = 0;

	VkResult result;
	result = vkEnumerateDeviceLayerProperties(physicalDevice, &layers_count, nullptr);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Could not get the number of device layers!" << std::endl;
	}

	std::vector<VkLayerProperties> layerProps(layers_count);
	result = vkEnumerateDeviceLayerProperties(physicalDevice, &layers_count, layerProps.data());
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Could not enumerate device layers!" << std::endl;
	}

	for (uint32_t i = 0; i < layers_count; ++i)
	{
		if (strcmp(layerProps[i].layerName, desiredLayer) == 0)
		{
			return true;
		}
	}
	return false;
}

bool Demo::VulkanDevice::checkAvailableDeviceExtensions(VkPhysicalDevice physicalDevice, std::vector<const char*> desiredDeviceExtensions)
{
	for (int i = 0; i < desiredDeviceExtensions.size(); i++)
	{
		if (!isDeviceExtensionAvailable(physicalDevice, desiredDeviceExtensions[i]))
		{
			std::cout << "VulkanDevice: Following Device Extension not supported!  '" << desiredDeviceExtensions[i] << "'" << std::endl;
			return false;
		}
	}
	return true;
}

bool Demo::VulkanDevice::checkAvailableDeviceLayers(VkPhysicalDevice physicalDevice, std::vector<const char*> desiredDeviceLayers)
{
	for (int i = 0; i < desiredDeviceLayers.size(); i++)
	{
		if (!isDeviceLayerAvailable(physicalDevice, desiredDeviceLayers[i]))
		{
			std::cout << "VulkanDevice: Following Device Layer not supported!  '" << desiredDeviceLayers[i] << "'" << std::endl;
			return false;
		}
	}
	return true;
}

uint32_t Demo::VulkanDevice::findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
{
	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				if (memTypeFound)
				{
					*memTypeFound = true;
				}
				return i;
			}
		}
		typeBits >>= 1;
	}

	if (memTypeFound)
	{
		*memTypeFound = false;
		return 0;
	}
	else
	{
		std::cout << "VulkanDevice: Could not find memory type!" << std::endl;
	}
}

