#pragma once
#include "BasicStruct.h"

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

	void initVulkan();
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
	void cleanup();
	void updateUniformBuffer()
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = 0;// std::chrono::high_resolution_clock::now();
		float time = 0.3; //std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

		UniformBufferObject ubo = {};
		ubo.model= glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;//y flip
		ubo.gLineColor = glm::vec4(1.0, 0.0, 0.0, 1.0);
		ubo.gLineWidth = 30;
		ubo.gHalfFilterWidth = 10;

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

	void createInstance();

	void pickPhysicalDevice();

	void createLogicalDevice();

	void createSwapChain();

	void createSwapChainImageViews();

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	void createTextureImageView();

	void createTextureSampler();

	void createRenderPass();

	void createDescriptorPool();

	void createDescriptorSet();

	void createDescriptorSetLayout();

	void createGraphicsPipeline();

	void createFrameBuffers();

	void createCommandPool();

	void createDepthResources();

	VkFormat findDepthFormat();

	bool hasStencilComponent(VkFormat format);
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& outImage, VkDeviceMemory& outImageMemory);

	void createTextureImage();

	void createVertexBuffer();

	void createIndexBuffer();

	void createUniformBuffer();

	void createCommandBuffer();

	void createSemaphores();

	void recreateSwapChain()
	{
		vkDeviceWaitIdle(device);
		cleanupSwapChain();
		createSwapChain();
		createSwapChainImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createDepthResources();
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

		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthImageMemory, nullptr);
	}

	VkShaderModule createShaderModule(const std::vector<char>& code);

	bool isDeviceSuitable(VkPhysicalDevice phydevice);
	
	bool checkDeviceExtensionSupport(VkPhysicalDevice phydevice);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice phydevice);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice phydevice, VkSurfaceKHR surface);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flag);

	void createSurface()
	{
		if (glfwCreateWindowSurface(vkInst, window, nullptr, &surface) != VK_SUCCESS)
		{
			throw runtime_error("fail to create surface!");
		}
	}

	bool checkValidationLayerSupport();

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags prop,
		VkBuffer& outBuffer, VkDeviceMemory& outDeviceBuffer, bool isUBO = false);

	VkCommandBuffer beginSingleCommandBuffer();
	void endSingleCommandBuffer(VkCommandBuffer cmdbuf);
	void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

	//for copy buffer to image
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

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

	VkImage textureImg;
	VkDeviceMemory textureDeviceBuffer;
	VkImageView textureImgView;
	VkSampler textureSampler;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkBuffer uboBuffer;
	VkDeviceMemory uboBufferMemory;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkDebugReportCallbackEXT callback;

	
};
