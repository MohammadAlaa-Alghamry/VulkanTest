#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <vector>
#include <map>

#include <optional>
#include <iomanip>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
	const bool isValidationLayersEnabled = false;
#else
	const bool isValidationLayersEnabled = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;

	bool isComplete() {
		return graphicsFamily.has_value();
	}
};

class HelloTriangleApplication {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	GLFWwindow* window;

	VkInstance instance;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	VkQueue graphicsQueue;

	VkDebugUtilsMessengerEXT debugMessenger;

	void initWindow() {
		glfwInit();

		std::cout << "Initializing my first GLFW window\n";
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		if (isValidationLayersEnabled) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroyInstance(instance, nullptr);
		vkDestroyDevice(device, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	void createInstance() {
		if (isValidationLayersEnabled && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto glfwExtensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
		createInfo.ppEnabledExtensionNames = glfwExtensions.data();
		// Messing
		for (int i = 0; i < glfwExtensions.size(); i++) {
			std::cout << "extension name: " << glfwExtensions[i] << '\n';
		}
		for (const char* extension : glfwExtensions) {
			std::cout << "const char* extension name: " << extension << '\n';
		} // End messing

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (isValidationLayersEnabled) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
		} else {
			createInfo.enabledLayerCount = 0;
		}

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!\n");
		}
		
		std::cout << "Done initialising Volkan instance.\n"; // Me
	}

	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;
			std::cout << "Required validation layer: " << layerName << '\n';

			for (const auto& layerProperties : availableLayers) {
				//std::cout << "current layer: " << layerProperties.layerName << '\n'; // Messing around
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					std::cout << "Layer found: " << layerProperties.layerName << '\n';
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (isValidationLayersEnabled) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr; // Optional You can optionally pass a pointer to the pUserData field which will be passed along to the callback function via the pUserData parameter. You could use this to pass a pointer to the HelloTriangleApplication class, for example.

	}

	void setupDebugMessenger() {
		if (!isValidationLayersEnabled) return;
		
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void pickPhysicalDevice() {
		std::cout << "picking physical device..." << std::endl;

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// Tutorila extra
		std::multimap<int, VkPhysicalDevice> candidates; // end

		for (VkPhysicalDevice device : devices) {
			if (!isDeviceSuitable(device)) {
				continue;
			} 
			// extra
			int score = rateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device)); // end
		}

		// Tutorial extra
		if (candidates.rbegin()->first > 0) {
			physicalDevice = candidates.rbegin()->second;
		}
		else {
			throw std::runtime_error("failed to find suitable GPU!");
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		} // End tutorial extra
	}
	// Tutorial Extra
	int rateDeviceSuitability(VkPhysicalDevice device) {
		int score = 0;

		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;

		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
		score += deviceProperties.limits.maxImageDimension2D;
		if (!deviceFeatures.geometryShader) return 0;

		std::cout << "device: " << deviceProperties.deviceName << std::endl;
		std::cout << "score: " << score << std::endl;
		return score;
	} // End tutorial extra;

	bool isDeviceSuitable(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		QueueFamilyIndices indices = findQueueFamilies(device);

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader && indices.isComplete();
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
		
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				auto bit = queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
				std::cout << "queueFamily.queueFlags: 0x" << std::setw(8) << std::setfill('0') << queueFamily.queueFlags << std::endl;
				std::cout << "queueFamily.queueFlags: 0X" << std::setw(8) << std::setfill('0') << VK_QUEUE_GRAPHICS_BIT << std::endl;
				std::cout << "queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT: " << bit << std::endl;
				std::cout << "i: " << i << std::endl;
				indices.graphicsFamily = i;
			}
			if (indices.isComplete()) { break; };
			i++;
		}

		return indices;
	}

	VkDevice createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		float queuePriority = 1.0f;

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		
		createInfo.enabledExtensionCount = 0;
		if (isValidationLayersEnabled) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);

	}
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

