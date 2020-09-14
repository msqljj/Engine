#include "VulkanManager/VulkanInstance/VulkanInstance.h"

Demo::VulkanInstance::VulkanInstance(bool validationEnable, HWND hWnd)
{
	std::cout << "VulkanInstance:ctor()" << std::endl;
	validation = validationEnable;
	this->hWnd = hWnd;
	if (validation)
	{
		const char* debugLayer = "VK_LAYER_KHRONOS_validation";
		desired_instance_layers.push_back(debugLayer);
	}
	createInstance();
	createSurface();

}

Demo::VulkanInstance::~VulkanInstance()
{
	std::cout << "Demo::VulkanInstance::Destructor()" << std::endl;
	if (surface)
	{
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}
	if (validation)
	{
		vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
	if (instance)
	{
		vkDestroyInstance(instance, nullptr);
	}

}

VkInstance Demo::VulkanInstance::getInstance()
{
	if (instance)
		return instance;
	else
		return nullptr;
}

VkSurfaceKHR Demo::VulkanInstance::getSurface()
{
	if (surface)
	return surface;
}

void Demo::VulkanInstance::createInstance()
{
	if (checkAvailableInstanceExtensions(desired_instance_extensions))
	{
		std::cout << "VulkanInstance: Desired instance extensions found!" << std::endl;
	}
	if (checkAvailableInstanceLayers(desired_instance_layers))
	{
		std::cout << "VulkanInstance: Desired instance layers found!" << std::endl;
	}

	VkApplicationInfo applicationInfo;
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = "APPLICATION_NAME";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "ENGINE_NAME";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);


	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT = {};
	debugUtilsMessengerCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugUtilsMessengerCreateInfoEXT.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugUtilsMessengerCreateInfoEXT.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugUtilsMessengerCreateInfoEXT.pfnUserCallback = debugCallback;

	VkInstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledLayerCount = 0;
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(desired_instance_extensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = desired_instance_extensions.data();
	if (validation)
	{
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(desired_instance_layers.size());
		instanceCreateInfo.ppEnabledLayerNames = desired_instance_layers.data();
		instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugUtilsMessengerCreateInfoEXT;
	}


	VkResult result;
	result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanInstance: createInstance failure!" << std::endl;
	}
	if (result == VK_SUCCESS)
	{
		std::cout << "VulkanInstance: createInstance = VK_SUCCESS!" << std::endl;
	}

	if (validation)
	{
		result = vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCreateInfoEXT, nullptr, &debugMessenger);
		if (result == VK_SUCCESS)
		{
			std::cout << "VulkanInstance: createDebugMessenger = VK_SUCCESS!" << std::endl;
		}
	}
}

void Demo::VulkanInstance::createSurface()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

	VkWin32SurfaceCreateInfoKHR surfaceCreateInfoKHR;
	surfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfoKHR.hinstance = (HINSTANCE)hInstance;
	surfaceCreateInfoKHR.hwnd = (HWND)hWnd;
	surfaceCreateInfoKHR.flags = 0;
	surfaceCreateInfoKHR.pNext = NULL;

	VkResult result;

	result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfoKHR, nullptr, &surface);

	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanInstance: Win32 Surface creation failed!" << std::endl;
	}

	if (result == VK_SUCCESS)
	{
		std::cout << "VulkanInstance: createSurface = VK_SUCCESS!" << std::endl;
	}
}

bool Demo::VulkanInstance::isInstanceExtensionAvailable(const char* desiredExtension)
{
	char* extensionFound = NULL;
	uint32_t desiredExtensionsFound = 0;
	uint32_t extensions_count = 0;

	VkResult result;
	result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanRenderer: Could not get the number of istance extensions!" << std::endl;
	}

	std::vector<VkExtensionProperties> extensionProps(extensions_count);
	result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, extensionProps.data());
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanRenderer: Could not enumerate instance extensions!" << std::endl;
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

bool Demo::VulkanInstance::isInstanceLayerAvailable(const char* desiredLayer)
{
	uint32_t layer_count;

	VkResult result;
	result = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanRenderer: Could not the number of instance layers" << std::endl;
	}
	std::vector<VkLayerProperties> layerProps(layer_count);
	result = vkEnumerateInstanceLayerProperties(&layer_count, layerProps.data());
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanRenderer: Could not enumerate instance layers!" << std::endl;
	}

	for (uint32_t i = 0; i < layer_count; ++i)
	{
		if (strcmp(layerProps[i].layerName, desiredLayer) == 0)
		{
			return true;
		}
	}
	return false;
}

bool Demo::VulkanInstance::checkAvailableInstanceExtensions(std::vector<const char*> desiredInstanceExtensions)
{
	for (int i = 0; i < desiredInstanceExtensions.size(); i++)
	{
		if (!isInstanceExtensionAvailable(desiredInstanceExtensions[i]))
		{
			std::cout << "VulkanRenderer: Following Instance Extension not supported!  '" << desiredInstanceExtensions[i] << "'" << std::endl;
			return false;
		}
	}
	return true;
}

bool Demo::VulkanInstance::checkAvailableInstanceLayers(std::vector<const char*> desiredInstanceLayers)
{
	for (int i = 0; i < desiredInstanceLayers.size(); i++)
	{
		if (!isInstanceLayerAvailable(desiredInstanceLayers[i]))
		{
			std::cout << "VulkanRenderer: Following Instance Layer not supported!  '" << desiredInstanceLayers[i] << "'" << std::endl;
			return false;
		}
	}
	return true;
}

VkResult Demo::VulkanInstance::vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* callback)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, create_info, allocator, callback);
	}

	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void Demo::VulkanInstance::vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, debugMessenger, pAllocator);
	}

	else
	{
		std::cout << "VulkanRenderer: Error Destroying DebugUtilsMessengerEXT" << std::endl;
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL Demo::VulkanInstance::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cout << "VulkanRenderer: ~Validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}
