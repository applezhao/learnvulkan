#pragma once
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#define VK_USE_PLATFORM_WIN32_KHR
#include<vulkan/vulkan.h>


#include <iostream>
#include <stdexcept>
#include <functional>

#include <vector>
#include <Windows.h>
#include <set>
#include <limits>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <array>
using namespace std;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData)
{
	std::cerr << "validationLayer:" << msg << endl;
	return VK_FALSE;
}

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
	CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (CreateDebugReportCallback== VK_NULL_HANDLE)
		cerr << "fail to get extenstion! test" << endl;
	
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		cerr << "fail to get extenstion!" << endl;
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	size_t size = (size_t)file.tellg();
	vector<char> code(size);
	file.seekg(0);
	file.read(code.data(), size);
	file.close();
	return code;
}

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription ret = {};
		ret.binding = 0;
		ret.stride = sizeof(Vertex);
		ret.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;//or instance
		return ret;
	}

	static array<VkVertexInputAttributeDescription, 2> getAttributeDescription()
	{
		array<VkVertexInputAttributeDescription, 2> ret = {};
		ret[0].binding = 0;
		ret[0].location = 0;
		ret[0].format = VK_FORMAT_R32G32_SFLOAT;
		ret[0].offset = offsetof(Vertex, pos);

		ret[1].binding = 0;
		ret[1].location = 1;
		ret[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		ret[1].offset = offsetof(Vertex, color);

		return ret;
	}
};

const vector<Vertex> vertices =
{
	{ { -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f } },
	{ { 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } },
	{ { -0.5f, 0.5f },{ 1.0f, 1.0f, 1.0f } }
};

const std::vector<uint16_t> indexes = {
	0, 1, 2, 2, 3, 0
};

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;
	int transferFamily = -1;
	bool isComplete()
	{
		return graphicsFamily >= 0 && presentFamily >= 0 && transferFamily >= 0;
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	vector<VkSurfaceFormatKHR> formats;
	vector<VkPresentModeKHR> presentModes;

	//choose surface format(color depth)
	VkSurfaceFormatKHR chooseSwapSurfaceFormat()
	{
		if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
			return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
		for (const auto& format : formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_UNORM&&format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
				return format;
		}
		return formats[0];
	}
	//choose present mode
	VkPresentModeKHR choosePresentMode()
	{
		for (const auto& mode : presentModes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return mode;
			else if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				return mode;
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}


};


class BasicDrawTri
{
	
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}


private:
	void initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(width, height, "drawTri", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, BasicDrawTri::onWindowResize);

	}
	static void onWindowResize(GLFWwindow* window, int width, int height)
	{
		if (width == 0 && height == 0)
			return;
		BasicDrawTri* app = reinterpret_cast<BasicDrawTri*>(glfwGetWindowUserPointer(window));
		app->recreateSwapChain();
	}

	//choose swap extent(resolution)
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
			return capabilities.currentExtent;
		else
		{
			int w, h;
			glfwGetWindowSize(window, &w, &h);
			this->width = w;
			this->height = h;
			VkExtent2D ext = { width, height };
			ext.width = max(capabilities.minImageExtent.width, min(capabilities.maxImageExtent.width, ext.width));
			ext.height = max(capabilities.minImageExtent.height, min(capabilities.maxImageExtent.height, ext.height));
			return ext;
		}
	}

	void initVulkan()
	{
		createInstance();
		/*setupDebugCallback();*/
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createFrameBuffers();
		createCommandPool();
		createVertexBuffer();
		createIndexBuffer();

		createUniformBuffer();
		createDescriptorPool();
		createDescriptorSet();

		createCommandBuffer();
		createSemaphores();

	}
	void mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();

			updateUniformBuffer();
			drawFrame();
		}
		vkDeviceWaitIdle(device);
	}
	void cleanup()
	{
		cleanupSwapChain();

		vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
		vkDestroyCommandPool(device, commandPool, nullptr);

		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);
		vkDestroyBuffer(device, indexBuffer, nullptr);
		vkFreeMemory(device, indexBufferMemory, nullptr);
		vkDestroyBuffer(device, uboBuffer, nullptr);
		vkFreeMemory(device, uboBufferMemory, nullptr);
		
		vkDestroyDescriptorSetLayout(device, descSetLayout, nullptr);
		vkDestroyDescriptorPool(device, descPool, nullptr);

		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(vkInst, surface, nullptr);
		//DestroyDebugReportCallbackEXT(vkInst, callback, nullptr);
		vkDestroyInstance(vkInst, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void updateUniformBuffer()
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

		UniformBufferObject ubo = {};
		ubo.model= glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;//y flip

		void* data;
		vkMapMemory(device, uboBufferMemory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, uboBufferMemory);
	}

	void drawFrame()
	{
		//update app state.....

		vkQueueWaitIdle(presentQueue);
		//acquire a swapchain image
		uint32_t imageIndex;
		VkResult status = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		if (status == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return;
		}
		else if (status != VK_SUCCESS&&status != VK_SUBOPTIMAL_KHR)
		{
			throw runtime_error("fail to acquire swap chain image!");
		}
		//execute command buffer
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		{
			throw runtime_error("fail to submit draw command buffer!");
		}
		
		// return image to swapchain for present
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		status = vkQueuePresentKHR(presentQueue, &presentInfo);
		
		if (status == VK_ERROR_OUT_OF_DATE_KHR || status == VK_SUBOPTIMAL_KHR)
			recreateSwapChain();
		else if(status!=VK_SUCCESS)
			throw runtime_error("fail to present swap chain image!");
		vkQueueWaitIdle(presentQueue);
	}

	void createInstance()
	{
		//check ext
		uint32_t instExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &instExtensionCount, nullptr);
		cout << instExtensionCount << endl;
		vector<VkExtensionProperties> instExts(instExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &instExtensionCount, instExts.data());

		for (const auto& exts : instExts)
		{
			cout << exts.extensionName << endl;
		}

		if (enableValidationLayers && !checkValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		//...
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkDebugReportCallbackCreateInfoEXT dbg_info;
		memset(&dbg_info, 0, sizeof(dbg_info));
		dbg_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		dbg_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT/* | VK_DEBUG_REPORT_INFORMATION_BIT_EXT*/;
		dbg_info.pfnCallback = &debugCallback;

		VkInstanceCreateInfo instInfo;
		instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instInfo.pApplicationInfo = &appInfo;
		instInfo.pNext = &dbg_info;
		//...
		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		instInfo.enabledExtensionCount = glfwExtensionCount;
		instInfo.ppEnabledExtensionNames = glfwExtensions;
		

		if (enableValidationLayers)
		{
			instInfo.enabledLayerCount = validationLayers.size();
			instInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			instInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&instInfo, nullptr, &vkInst) != VK_SUCCESS)
		{
			throw runtime_error("fail to create instance");
		}


	}

	void pickPhysicalDevice()
	{
		physicalDevice = VK_NULL_HANDLE;
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(vkInst, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw runtime_error("fail to find GPUs supporting vulkan!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(vkInst, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (isDeviceSuitable(device))
			{
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE)
		{
			throw runtime_error("fail to find GPUs supporting vulkan - suitable!");
		}

		QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
		if (!indices.isComplete())
		{
			throw std::runtime_error("fail to find a device with required queue family!");
		}
	}

	void createLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
		if (!indices.isComplete())
		{
			throw runtime_error("fail to find queue family!");
		}

		vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily, indices.transferFamily };
		float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		//createInfo.pQueueCreateInfos = &queueCreateInfo;
		//createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		createInfo.queueCreateInfoCount = queueCreateInfos.size();
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		{
			throw runtime_error("fail to create logical device!");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
		vkGetDeviceQueue(device, indices.transferFamily, 0, &transferQueue);
	}

	void createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
		VkSurfaceFormatKHR surfaceFormat = swapChainSupport.chooseSwapSurfaceFormat();
		VkPresentModeKHR presentMode = swapChainSupport.choosePresentMode();
		VkExtent2D ext = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = ext;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
		uint32_t queueFamiliesIndices[] = { indices.graphicsFamily, indices.presentFamily };
		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamiliesIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//best performance
		}
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
		{
			throw runtime_error("fail to create swap chain!");
		}
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = ext;
		uint32_t imageCount_1;
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount_1, nullptr);
		swapchainImages.resize(imageCount_1);
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount_1, swapchainImages.data());

	}

	void createImageViews()
	{
		swapchainImageViews.resize(swapchainImages.size());
		for (size_t i = 0; i < swapchainImages.size(); i++)
		{
			VkImageViewCreateInfo createinfo = {};
			createinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createinfo.image = swapchainImages[i];
			createinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createinfo.format = swapChainImageFormat;
			createinfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createinfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createinfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createinfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createinfo.subresourceRange.baseMipLevel = 0;
			createinfo.subresourceRange.levelCount = 1;
			createinfo.subresourceRange.baseArrayLayer = 0;
			createinfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createinfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS)
			{
				throw runtime_error("fail to create image view!");
			}
		}
	}

	void createRenderPass()
	{
		VkAttachmentDescription colorAttach = {};
		colorAttach.format = swapChainImageFormat;
		colorAttach.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAattachRef = {};
		colorAattachRef.attachment = 0;
		colorAattachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAattachRef;

		VkRenderPassCreateInfo renderpassInfo = {};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderpassInfo.attachmentCount = 1;
		renderpassInfo.pAttachments = &colorAttach;
		renderpassInfo.subpassCount = 1;
		renderpassInfo.pSubpasses = &subpass;

		//subpass dependencies
		VkSubpassDependency depend = {};
		depend.srcSubpass = VK_SUBPASS_EXTERNAL;
		depend.dstSubpass = 0;
		depend.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		depend.srcAccessMask = 0;
		depend.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		depend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		renderpassInfo.dependencyCount = 1;
		renderpassInfo.pDependencies = &depend;

		if (vkCreateRenderPass(device, &renderpassInfo, nullptr, &renderpass) != VK_SUCCESS)
		{
			throw runtime_error("fail to create render pass!");
		}
	}

	void createDescriptorPool()
	{
		VkDescriptorPoolSize poolsize = {};
		poolsize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolsize.descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolinfo = {};
		poolinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolinfo.poolSizeCount = 1;
		poolinfo.pPoolSizes = &poolsize;
		poolinfo.maxSets = 1;

		if(vkCreateDescriptorPool(device, &poolinfo, nullptr, &descPool)!=VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}

	}

	void createDescriptorSet()
	{
		VkDescriptorSetLayout layouts[] = { descSetLayout };
		VkDescriptorSetAllocateInfo allocinfo = {};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.descriptorPool = descPool;
		allocinfo.descriptorSetCount = 1;
		allocinfo.pSetLayouts = layouts;
		if (vkAllocateDescriptorSets(device, &allocinfo, &descSet) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor set!");
		}

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uboBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

	}

	void createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBind = {};
		uboLayoutBind.binding = 0;
		uboLayoutBind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBind.descriptorCount = 1;
		uboLayoutBind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBind.pImmutableSamplers = nullptr;//for image

		VkDescriptorSetLayoutCreateInfo layoutinfo = {};
		layoutinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutinfo.bindingCount = 1;
		layoutinfo.pBindings = &uboLayoutBind;

		if (vkCreateDescriptorSetLayout(device, &layoutinfo, nullptr, &descSetLayout) != VK_SUCCESS)
		{
			throw runtime_error("fail to create desc set layout!");
		}

	}

	void createGraphicsPipeline()
	{
		vector<char> vertCode = readFile("vert.spv");
		vector<char> fragCode = readFile("frag.spv");
		vertShaderModule = createShaderModule(vertCode);
		fragShaderModule = createShaderModule(fragCode);

		VkPipelineShaderStageCreateInfo vertStageCreateinfo = {};
		vertStageCreateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertStageCreateinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertStageCreateinfo.module = vertShaderModule;
		vertStageCreateinfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragStageCreateinfo = {};
		fragStageCreateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragStageCreateinfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragStageCreateinfo.module = fragShaderModule;
		fragStageCreateinfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] = { vertStageCreateinfo, fragStageCreateinfo };

		//fixed stages:
		//vertx input, input assembly, viewport and scissors,
		//rasterizer, multisampling, depth and stencil
		//color blending, dynamic state, pipeline layout
		auto vertexBindingDesc = Vertex::getBindingDescription();
		auto vertexAttriDescs = Vertex::getAttributeDescription();
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = vertexAttriDescs.size();
		vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDesc;
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttriDescs.data();

		VkPipelineInputAssemblyStateCreateInfo inputAassemInfo = {};
		inputAassemInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAassemInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAassemInfo.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = swapChainExtent.width;
		viewport.height = swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportInfo = {};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.pViewports = &viewport;
		viewportInfo.scissorCount = 1;
		viewportInfo.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizeInfo = {};
		rasterizeInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizeInfo.depthClampEnable = VK_FALSE;// for shadow map, set it true
		rasterizeInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizeInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizeInfo.lineWidth = 1.0f;
		rasterizeInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizeInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizeInfo.depthBiasEnable = VK_FALSE;//offset bias is useful for shadow map

		VkPipelineMultisampleStateCreateInfo multiSamInfo = {};
		multiSamInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multiSamInfo.sampleShadingEnable = VK_FALSE;
		multiSamInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachState = {};
		colorBlendAttachState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachState.blendEnable = VK_TRUE;
		colorBlendAttachState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachState.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
		colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendInfo.logicOpEnable = VK_FALSE;
		colorBlendInfo.attachmentCount = 1;
		colorBlendInfo.pAttachments = &colorBlendAttachState;

		/*VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };
		VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = 2;
		dynamicStateInfo.pDynamicStates = dynamicStates;*/

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descSetLayout;
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw runtime_error("fail to create pipeline layout!");
		}

		//finally graphics pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStageCreateInfos;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAassemInfo;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState = &rasterizeInfo;
		pipelineInfo.pMultisampleState = &multiSamInfo;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlendInfo;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderpass;
		pipelineInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw runtime_error("fail to create graphics pipeline!");
		}


	}

	void createFrameBuffers()
	{
		swapchainFrameBuffers.resize(swapchainImageViews.size());
		for (size_t i = 0; i < swapchainImageViews.size(); i++)
		{
			VkImageView attachments[] = { swapchainImageViews[i] };
			VkFramebufferCreateInfo fboInfo = {};
			fboInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fboInfo.renderPass = renderpass;
			fboInfo.attachmentCount = 1;
			fboInfo.pAttachments = attachments;
			fboInfo.width = swapChainExtent.width;
			fboInfo.height = swapChainExtent.height;
			fboInfo.layers = 1;
			if (vkCreateFramebuffer(device, &fboInfo, nullptr, &swapchainFrameBuffers[i]) != VK_SUCCESS)
			{
				throw runtime_error("fail to create framebuffer!");
			}
		}
	}

	void createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndice = findQueueFamilies(physicalDevice, surface);
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndice.graphicsFamily;
		poolInfo.flags = 0;
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			throw runtime_error("fail to create command pool!");
		}
	}

	void createVertexBuffer()
	{
		VkDeviceSize size = sizeof(vertices[0])*vertices.size();
		//create stage buffer
		VkBuffer stageBuffer;
		VkDeviceMemory stageDeviceBuffer;
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stageBuffer, stageDeviceBuffer);

		//filling stage device buffer
		void* data;
		vkMapMemory(device, stageDeviceBuffer, 0, size, 0, &data);//map to cpu
		memcpy(data, vertices.data(), size);
		vkUnmapMemory(device, stageDeviceBuffer);

		//create usual vertex buffer
		createBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT| VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ,
			vertexBuffer, vertexBufferMemory);

		copyBuffer(stageBuffer, vertexBuffer, size);

		//clean stage buffer
		vkDestroyBuffer(device, stageBuffer, nullptr);
		vkFreeMemory(device, stageDeviceBuffer, nullptr);

	}

	void createIndexBuffer()
	{
		VkDeviceSize size = sizeof(indexes[0])*indexes.size();
		//create stage buffer
		VkBuffer stageBuffer;
		VkDeviceMemory stageDeviceBuffer;
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stageBuffer, stageDeviceBuffer);

		//filling stage device buffer
		void* data;
		vkMapMemory(device, stageDeviceBuffer, 0, size, 0, &data);//map to cpu
		memcpy(data, indexes.data(), size);
		vkUnmapMemory(device, stageDeviceBuffer);

		//create usual vertex buffer
		createBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer, indexBufferMemory);

		copyBuffer(stageBuffer, indexBuffer, size);

		//clean stage buffer
		vkDestroyBuffer(device, stageBuffer, nullptr);
		vkFreeMemory(device, stageDeviceBuffer, nullptr);
	}

	void createUniformBuffer()
	{
		VkDeviceSize size = sizeof(UniformBufferObject);
		createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uboBuffer, uboBufferMemory, true);
	}

	void createCommandBuffer()
	{
		commandBuffers.resize(swapchainFrameBuffers.size());
		VkCommandBufferAllocateInfo cmdBufferAlloInfo = {};
		cmdBufferAlloInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAlloInfo.commandPool = commandPool;
		cmdBufferAlloInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAlloInfo.commandBufferCount = commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &cmdBufferAlloInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw runtime_error("fail to create command buffer!");
		}

		for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr;
			vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

			//start a render pass
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderpass;
			renderPassInfo.framebuffer = swapchainFrameBuffers[i];
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = swapChainExtent;
			VkClearValue clearColor = { 0,0,0,1 };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			VkBuffer vertexbufs[] = { vertexBuffer };
			VkDeviceSize offset[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexbufs, offset);
			vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout, 0, 1, &descSet, 0, nullptr);

			//vkCmdDraw(commandBuffers[i], vertices.size(), 1, 0, 0);
			vkCmdDrawIndexed(commandBuffers[i], indexes.size(), 1, 0, 0, 0);
			
			

			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			{
				throw runtime_error("fail to record command buffer!");
			}
		}
	}

	void createSemaphores()
	{
		VkSemaphoreCreateInfo semphoreInfo = {};
		semphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		if (vkCreateSemaphore(device, &semphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS
			|| vkCreateSemaphore(device, &semphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
		{
			throw runtime_error("fail to create semaphores!");
		}
	}

	void recreateSwapChain()
	{
		vkDeviceWaitIdle(device);
		cleanupSwapChain();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFrameBuffers();
		createCommandBuffer();
	}

	void cleanupSwapChain()
	{
		for (size_t i = 0; i < swapchainFrameBuffers.size(); i++)
		{
			vkDestroyFramebuffer(device, swapchainFrameBuffers[i], nullptr);
		}
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderpass, nullptr);
		for (size_t i = 0; i < swapchainImageViews.size(); i++)
		{
			vkDestroyImageView(device, swapchainImageViews[i], nullptr);
		}

		
		vkFreeCommandBuffers(device, commandPool, commandBuffers.size(), commandBuffers.data());
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroySwapchainKHR(device, swapchain, nullptr);
	}

	VkShaderModule createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createinfo = {};
		createinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createinfo.codeSize = code.size();
		createinfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		VkShaderModule ret;
		if (vkCreateShaderModule(device, &createinfo, nullptr, &ret) != VK_SUCCESS)
		{
			throw runtime_error("fail to create shader module!");
		}
		return ret;

	}

	bool isDeviceSuitable(VkPhysicalDevice phydevice)
	{
		QueueFamilyIndices indices = findQueueFamilies(phydevice, surface);
		bool extensionsSupported = checkDeviceExtensionSupport(phydevice);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapDetails = querySwapChainSupport(phydevice);
			swapChainAdequate = !swapDetails.formats.empty() &&
				!swapDetails.presentModes.empty();
		}
		return indices.isComplete() && extensionsSupported &&swapChainAdequate;
		/*VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(phydevice, &deviceProperties);
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(phydevice, &deviceFeatures);

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;*/
	}
	
	bool checkDeviceExtensionSupport(VkPhysicalDevice phydevice)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(phydevice, nullptr, &extensionCount, nullptr);
		vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(phydevice, nullptr, &extensionCount, availableExtensions.data());

		set<string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice phydevice)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phydevice, surface, &details.capabilities);

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(phydevice, surface, &formatCount, nullptr);
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(phydevice, surface, &formatCount, details.formats.data());

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(phydevice, surface, &presentModeCount, nullptr);
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(phydevice, surface, &presentModeCount, details.presentModes.data());
		return details;
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice phydevice, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices;
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(phydevice, &queueFamilyCount, nullptr);
		vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(phydevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto queueFamily : queueFamilies)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(phydevice, i, surface, &presentSupport);
			if (presentSupport)
			{
				indices.presentFamily = i;
			}
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags&VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags&VK_QUEUE_TRANSFER_BIT /*&& !(queueFamily.queueFlags^VK_QUEUE_GRAPHICS_BIT) && !(queueFamily.queueFlags^VK_QUEUE_COMPUTE_BIT)*/)
			{
				indices.transferFamily = i;
			}
			cout << i<<":"<<queueFamily.queueFlags<<","<< (queueFamily.queueFlags^VK_QUEUE_GRAPHICS_BIT) <<","<<(queueFamily.queueFlags&VK_QUEUE_GRAPHICS_BIT)<< endl;
			if (indices.isComplete())
				break;
			i++;
		}
		cout << indices.graphicsFamily << "," << indices.presentFamily << "," << indices.transferFamily << endl;
		return indices;
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flag)
	{
		VkPhysicalDeviceMemoryProperties availProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &availProperties);
		for (size_t i = 0; i < availProperties.memoryTypeCount; i++)
		{
			if ((typeFilter&(1 << i)) && (availProperties.memoryTypes[i].propertyFlags&flag) == flag)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void createSurface()
	{
		if (glfwCreateWindowSurface(vkInst, window, nullptr, &surface) != VK_SUCCESS)
		{
			throw runtime_error("fail to create surface!");
		}
	}

	bool checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;
			for (const auto& layerAvail : availableLayers)
			{
				if (strcmp(layerName, layerAvail.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}
			if (!layerFound)
				return false;
		}
		return true;
	}

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags prop,
		VkBuffer& outBuffer, VkDeviceMemory& outDeviceBuffer, bool isUBO=false)
	{
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
		uint32_t sharedFamily[] = { indices.graphicsFamily,  indices.transferFamily };
		VkBufferCreateInfo createinfo = {};
		createinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createinfo.size = size;
		createinfo.usage = usage;
		if (!isUBO)
		{
			createinfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
			createinfo.queueFamilyIndexCount = 2;
			createinfo.pQueueFamilyIndices = sharedFamily;
		}
		else
			createinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		

		if (vkCreateBuffer(device, &createinfo, nullptr, &outBuffer) != VK_SUCCESS)
		{
			throw runtime_error("fail to create buffer!");
		}

		VkMemoryRequirements memRequire;
		vkGetBufferMemoryRequirements(device, outBuffer, &memRequire);
		uint32_t typeIndex = findMemoryType(memRequire.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequire.size;
		allocInfo.memoryTypeIndex = typeIndex;

		//vkAllocateMemory is not recommand. max=4096. ??????
		if (vkAllocateMemory(device, &allocInfo, nullptr, &outDeviceBuffer) != VK_SUCCESS)
		{
			throw ("fail to allocate device buffer!");
		}
		vkBindBufferMemory(device, outBuffer, outDeviceBuffer, 0);

	}

	void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo cmdBufferAlloInfo = {};
		cmdBufferAlloInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAlloInfo.commandPool = commandPool;
		cmdBufferAlloInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAlloInfo.commandBufferCount = 1;
		VkCommandBuffer cmdbuf;
		vkAllocateCommandBuffers(device, &cmdBufferAlloInfo, &cmdbuf);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;//only once

		vkBeginCommandBuffer(cmdbuf, &beginInfo);
		VkBufferCopy copyregion = {};
		copyregion.size = size;
		vkCmdCopyBuffer(cmdbuf, src, dst, 1, &copyregion);
		vkEndCommandBuffer(cmdbuf);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdbuf;

		vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(transferQueue);
		vkFreeCommandBuffers(device, commandPool, 1, &cmdbuf);

	}

	GLFWwindow* window;
	int width = 800;
	int height = 600;

	const bool enableValidationLayers = true;

	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};
	const std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkInstance vkInst;
	VkPhysicalDevice physicalDevice;
	VkDevice device;

	
	VkSurfaceKHR surface;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;

	VkSwapchainKHR swapchain;
	vector<VkImage> swapchainImages;
	vector<VkImageView> swapchainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	VkRenderPass renderpass;

	VkDescriptorSet descSet;
	VkDescriptorPool descPool;
	VkDescriptorSetLayout descSetLayout;

	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	vector<VkFramebuffer> swapchainFrameBuffers;

	VkCommandPool commandPool;
	vector<VkCommandBuffer> commandBuffers;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkBuffer uboBuffer;
	VkDeviceMemory uboBufferMemory;

	VkDebugReportCallbackEXT callback;

	
};
