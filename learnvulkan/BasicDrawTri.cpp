#include "BasicDrawTri.h"


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void BasicDrawTri::initVulkan()
{
	createInstance();
	/*setupDebugCallback();*/
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();

	createSwapChain();
	createSwapChainImageViews();
	createRenderPass();

	createDescriptorSetLayout();
	createGraphicsPipeline();

	createCommandPool();

	createDepthResources();
	createFrameBuffers();

	createTextureImage();//use command buffer . should be after command pool
	createTextureImageView();
	createTextureSampler();

	createVertexBuffer();
	createIndexBuffer();

	createUniformBuffer();
	createDescriptorSet();

	createCommandBuffer();
	createSemaphores();

}

void BasicDrawTri::cleanup()
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

	vkDestroyImage(device, textureImg, nullptr);
	vkFreeMemory(device, textureDeviceBuffer, nullptr);
	vkDestroyImageView(device, textureImgView, nullptr);
	vkDestroySampler(device, textureSampler, nullptr);

	vkDestroyDescriptorSetLayout(device, descSetLayout, nullptr);
	vkDestroyDescriptorPool(device, descPool, nullptr);

	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(vkInst, surface, nullptr);
	//DestroyDebugReportCallbackEXT(vkInst, callback, nullptr);
	vkDestroyInstance(vkInst, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

void BasicDrawTri::createInstance()
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

void BasicDrawTri::pickPhysicalDevice()
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

void BasicDrawTri::createLogicalDevice()
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

void BasicDrawTri::createSwapChain()
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

void BasicDrawTri::createSwapChainImageViews()
{
	swapchainImageViews.resize(swapchainImages.size());
	for (size_t i = 0; i < swapchainImages.size(); i++)
	{
		swapchainImageViews[i] = createImageView(swapchainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void BasicDrawTri::createRenderPass()
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

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAattachRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttach, depthAttachment };

	VkRenderPassCreateInfo renderpassInfo = {};
	renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassInfo.attachmentCount = attachments.size();
	renderpassInfo.pAttachments = attachments.data();
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

void BasicDrawTri::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBind = {};
	uboLayoutBind.binding = 0;
	uboLayoutBind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBind.descriptorCount = 1;
	uboLayoutBind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	uboLayoutBind.pImmutableSamplers = nullptr;//for image

	VkDescriptorSetLayoutBinding samplerLayoutBind = {};
	samplerLayoutBind.binding = 1;
	samplerLayoutBind.descriptorCount = 1;
	samplerLayoutBind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBind.pImmutableSamplers = nullptr;
	samplerLayoutBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBind, samplerLayoutBind };
	VkDescriptorSetLayoutCreateInfo layoutinfo = {};
	layoutinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutinfo.bindingCount = bindings.size();
	layoutinfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutinfo, nullptr, &descSetLayout) != VK_SUCCESS)
	{
		throw runtime_error("fail to create desc set layout!");
	}

}

void BasicDrawTri::createGraphicsPipeline()
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
	//inputAassemInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAassemInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	//inputAassemInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
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

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional

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
	pipelineInfo.pDepthStencilState = &depthStencil;
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

void BasicDrawTri::createCommandPool()
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

void BasicDrawTri::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();
	createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}

void BasicDrawTri::createFrameBuffers()
{
	swapchainFrameBuffers.resize(swapchainImageViews.size());
	for (size_t i = 0; i < swapchainImageViews.size(); i++)
	{
		std::array<VkImageView, 2> attachments = { swapchainImageViews[i] , depthImageView };
		VkFramebufferCreateInfo fboInfo = {};
		fboInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fboInfo.renderPass = renderpass;
		fboInfo.attachmentCount = attachments.size();
		fboInfo.pAttachments = attachments.data();
		fboInfo.width = swapChainExtent.width;
		fboInfo.height = swapChainExtent.height;
		fboInfo.layers = 1;
		if (vkCreateFramebuffer(device, &fboInfo, nullptr, &swapchainFrameBuffers[i]) != VK_SUCCESS)
		{
			throw runtime_error("fail to create framebuffer!");
		}
	}
}

void BasicDrawTri::createTextureImage()
{
	int texW, texH, texC;
	stbi_uc* pixels = stbi_load("img.jpg", &texW, &texH, &texC, STBI_rgb_alpha);
	VkDeviceSize imageSize = texW*texH * 4;
	cout << texW << "," << texH << "," << imageSize << endl;

	VkBuffer stageBuffer;
	VkDeviceMemory stageDeviceBuffer;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stageBuffer, stageDeviceBuffer);

	void *data;
	vkMapMemory(device, stageDeviceBuffer, 0, imageSize, 0, &data);
	memcpy(data, pixels, imageSize);
	vkUnmapMemory(device, stageDeviceBuffer);

	//free pixels

	//transfer stage buffer to image buffer
	createImage(texW, texH, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		0,
		textureImg, textureDeviceBuffer);

	transitionImageLayout(textureImg, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stageBuffer, textureImg, static_cast<uint32_t>(texW), static_cast<uint32_t>(texH));
	transitionImageLayout(textureImg, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(device, stageBuffer, nullptr);
	vkFreeMemory(device, stageDeviceBuffer, nullptr);
}

void BasicDrawTri::createTextureImageView()
{
	textureImgView = createImageView(textureImg, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

void BasicDrawTri::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo = {};
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

	//w
	if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void BasicDrawTri::createVertexBuffer()
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
	createBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer, vertexBufferMemory);

	copyBuffer(stageBuffer, vertexBuffer, size);

	//clean stage buffer
	vkDestroyBuffer(device, stageBuffer, nullptr);
	vkFreeMemory(device, stageDeviceBuffer, nullptr);

}

void BasicDrawTri::createIndexBuffer()
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

void BasicDrawTri::createUniformBuffer()
{
	VkDeviceSize size = sizeof(UniformBufferObject);
	createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		uboBuffer, uboBufferMemory, true);
}


void BasicDrawTri::createDescriptorSet()
{
	createDescriptorPool();
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

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = textureImgView;
	imageInfo.sampler = textureSampler;

	std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

}

void BasicDrawTri::createCommandBuffer()
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
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkBuffer vertexbufs[] = { vertexBuffer };
		VkDeviceSize offset[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexbufs, offset);
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout, 0, 1, &descSet, 0, nullptr);

		vkCmdSetLineWidth(commandBuffers[i], 50);
		vkCmdDraw(commandBuffers[i], vertices.size(), 1, 0, 0);
		
		//vkCmdDrawIndexed(commandBuffers[i], indexes.size(), 1, 0, 0, 0);



		vkCmdEndRenderPass(commandBuffers[i]);
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
		{
			throw runtime_error("fail to record command buffer!");
		}
	}
}

void BasicDrawTri::createSemaphores()
{
	VkSemaphoreCreateInfo semphoreInfo = {};
	semphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(device, &semphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS
		|| vkCreateSemaphore(device, &semphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
	{
		throw runtime_error("fail to create semaphores!");
	}
}

//=======================================================================================
//help functions
//=======================================================================================

void BasicDrawTri::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolinfo = {};
	poolinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolinfo.poolSizeCount = poolSizes.size();
	poolinfo.pPoolSizes = poolSizes.data();
	poolinfo.maxSets = 1;

	if (vkCreateDescriptorPool(device, &poolinfo, nullptr, &descPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}

}

VkImageView BasicDrawTri::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageView ret;
	VkImageViewCreateInfo createinfo = {};
	createinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createinfo.image = image;
	createinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createinfo.format = format;
	createinfo.subresourceRange.aspectMask = aspectFlags;
	createinfo.subresourceRange.baseMipLevel = 0;
	createinfo.subresourceRange.levelCount = 1;
	createinfo.subresourceRange.baseArrayLayer = 0;
	createinfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device, &createinfo, nullptr, &ret) != VK_SUCCESS)
	{
		throw runtime_error("fail to create texture image view!");
	}
	return ret;
}

VkFormat BasicDrawTri::findDepthFormat()
{
	return findSupportedFormat(
	{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool BasicDrawTri::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat BasicDrawTri::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

void BasicDrawTri::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& outImage, VkDeviceMemory& outImageMemory)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device, &imageInfo, nullptr, &outImage) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, outImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &outImageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(device, outImage, outImageMemory, 0);
}

void BasicDrawTri::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags prop,
	VkBuffer& outBuffer, VkDeviceMemory& outDeviceBuffer, bool isUBO)
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

VkShaderModule BasicDrawTri::createShaderModule(const std::vector<char>& code)
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

bool BasicDrawTri::isDeviceSuitable(VkPhysicalDevice phydevice)
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

bool BasicDrawTri::checkDeviceExtensionSupport(VkPhysicalDevice phydevice)
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

SwapChainSupportDetails BasicDrawTri::querySwapChainSupport(VkPhysicalDevice phydevice)
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

QueueFamilyIndices BasicDrawTri::findQueueFamilies(VkPhysicalDevice phydevice, VkSurfaceKHR surface)
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
		cout << i << ":" << queueFamily.queueFlags << "," << (queueFamily.queueFlags^VK_QUEUE_GRAPHICS_BIT) << "," << (queueFamily.queueFlags&VK_QUEUE_GRAPHICS_BIT) << endl;
		if (indices.isComplete())
			break;
		i++;
	}
	cout << indices.graphicsFamily << "," << indices.presentFamily << "," << indices.transferFamily << endl;
	return indices;
}

uint32_t BasicDrawTri::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flag)
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


bool BasicDrawTri::checkValidationLayerSupport()
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


VkCommandBuffer BasicDrawTri::beginSingleCommandBuffer()
{
	//begin single time command buffer
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
	return cmdbuf;
}

void BasicDrawTri::endSingleCommandBuffer(VkCommandBuffer cmdbuf)
{
	//end single time command buffer
	vkEndCommandBuffer(cmdbuf);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdbuf;

	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue);
	vkFreeCommandBuffers(device, commandPool, 1, &cmdbuf);
}
void BasicDrawTri::copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
{

	VkCommandBuffer cmdbuf = beginSingleCommandBuffer();
	//sth todo
	VkBufferCopy copyregion = {};
	copyregion.size = size;
	vkCmdCopyBuffer(cmdbuf, src, dst, 1, &copyregion);

	endSingleCommandBuffer(cmdbuf);
}

//for copy buffer to image
void BasicDrawTri::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer cmdbuf = beginSingleCommandBuffer();
	//sth todo
	VkImageMemoryBarrier barrier = {};//to make sure read after write for sharing_mode_exclusive
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
	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0; // TODO

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
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(cmdbuf, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	endSingleCommandBuffer(cmdbuf);
}

void BasicDrawTri::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer cmdbuf = beginSingleCommandBuffer();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(cmdbuf, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


	endSingleCommandBuffer(cmdbuf);
}

//VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
//	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
//	PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
//	CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
//	if (CreateDebugReportCallback== VK_NULL_HANDLE)
//		cerr << "fail to get extenstion! test" << endl;
//	
//	if (func != nullptr) {
//		return func(instance, pCreateInfo, pAllocator, pCallback);
//	}
//	else {
//		cerr << "fail to get extenstion!" << endl;
//		return VK_ERROR_EXTENSION_NOT_PRESENT;
//	}
//}
//
//void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
//	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
//	if (func != nullptr) {
//		func(instance, callback, pAllocator);
//	}
//}


