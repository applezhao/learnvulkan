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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


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
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription ret = {};
		ret.binding = 0;
		ret.stride = sizeof(Vertex);
		ret.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;//or instance
		return ret;
	}

	static array<VkVertexInputAttributeDescription, 3> getAttributeDescription()
	{
		array<VkVertexInputAttributeDescription, 3> ret = {};
		ret[0].binding = 0;
		ret[0].location = 0;
		ret[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		ret[0].offset = offsetof(Vertex, pos);

		ret[1].binding = 0;
		ret[1].location = 1;
		ret[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		ret[1].offset = offsetof(Vertex, color);

		ret[2].binding = 0;
		ret[2].location = 2;
		ret[2].format = VK_FORMAT_R32G32_SFLOAT;
		ret[2].offset = offsetof(Vertex, texCoord);

		return ret;
	}
};

const vector<Vertex> vertices =
{
	{ { -0.5f, -0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	{ { 0.5f, -0.5f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, 0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
	{ { -0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

	{ { -0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	{ { 0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, 0.5f, -0.5f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
	{ { -0.5f, 0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } }
};

const std::vector<uint16_t> indexes = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
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