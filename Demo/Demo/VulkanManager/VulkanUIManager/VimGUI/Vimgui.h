#pragma once

#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <assert.h>
#include <vector>
#include <array>
#include <Math/math.hpp>
#include <Math/glm/glm.hpp>
#include <Math/glm/gtc/matrix_transform.hpp>
#include <Math/glm/gtc/type_ptr.hpp>

#define Turquoise 		glm::vec4(26.0, 188.0, 156.0, 255.0);
#define Emerald 		glm::vec4(46.0, 204.0, 113.0, 255.0);
#define PeterRiver		glm::vec4(52.0, 152.0, 219.0, 255.0);
#define Amethyst		glm::vec4(155.0, 89.0, 182.0, 255.0);
#define WetAsphalt		glm::vec4(52.0, 73.0, 94.0, 255.0);

#define GreenSea		glm::vec4(22.0, 160.0, 133.0, 255.0);
#define Nephritis		glm::vec4(39.0, 174.0, 96.0, 255.0);
#define BelizeHole		glm::vec4(41.0, 128.0, 185.0, 255.0);
#define Wisteria		glm::vec4(142.0, 68.0, 173.0, 255.0);
#define MidnightBlue 	glm::vec4(44.0, 62.0, 80.0, 255.0);

#define SunFlower		glm::vec4(241.0, 196.0, 15.0, 255.0);
#define Carrot			glm::vec4(230.0, 126.0, 34.0, 255.0);
#define Alizarin		glm::vec4(231.0, 76.0, 60.0, 255.0);
#define Clouds			glm::vec4(236.0, 240.0, 241.0, 255.0);
#define Concrete		glm::vec4(149.0, 165.0, 166.0, 255.0);

#define Orange			glm::vec4(243.0, 156.0, 18.0, 255.0);
#define Pumpkin			glm::vec4(211.0, 84.0, 0.0, 255.0);
#define Pomegranate		glm::vec4(192.0, 57.0, 43.0, 255.0);
#define Silver			glm::vec4(189.0, 195.0, 199.0, 255.0);
#define Asbestos		glm::vec4(127.0, 140.0, 141.0, 255.0);

namespace Demo
{
	//Buffer&Image---------------
	struct UITexture
	{
		UITexture(VkDevice logicalDevice)
		{
			this->logicalDevice = logicalDevice;
		}
		~UITexture()
		{
			if (!destroyed)
			{
				destroy();
			}
		}
		bool destroyed = false;
		const char* filepath;
		int textureWidth = 0;
		int textureHeight = 0;
		int textureChannels = 0;
		VkDevice logicalDevice = nullptr;
		VkImage textureImage = VK_NULL_HANDLE;
		VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
		VkDeviceSize textureImageMemorySize = 0;
		VkImageView textureImageView = VK_NULL_HANDLE;
		VkSampler textureSampler = VK_NULL_HANDLE;
		VkImageLayout textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkDescriptorImageInfo textureDescriptor;

		void destroy()
		{
			if (!destroyed)
			{
				if (logicalDevice != nullptr)
				{
					destroyed = true;
					if (textureSampler)
					{
						vkDestroySampler(logicalDevice, textureSampler, nullptr);
					}

					if (textureImageView)
					{
						vkDestroyImageView(logicalDevice, textureImageView, nullptr);
					}

					if (textureImage)
					{
						vkDestroyImage(logicalDevice, textureImage, nullptr);
					}

					if (textureImageMemory)
					{
						vkFreeMemory(logicalDevice, textureImageMemory, nullptr);
					}
				}
			}
			else
			{
				std::cout << "Demo::Texture: Destroy called, but device was nullptr?" << std::endl;
			}

		}
	};
	struct UIBuffer
	{
		UIBuffer(VkDevice device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize deviceSize)
		{
			this->device = device;
			this->usageFlags = usageFlags;
			this->memoryPropertyFlags = memoryPropertyFlags;
			this->size = deviceSize;
		}
		~UIBuffer()
		{
			if (buffer != VK_NULL_HANDLE || memory != VK_NULL_HANDLE && !destroyed)
			{
				destroy();
			}
		}
		VkDevice device;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo descriptor;
		VkDeviceSize vertexCount = 0;
		VkDeviceSize size = 0;
		VkDeviceSize alignment = 0;
		VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
		void* mapped = nullptr;
		bool destroyed = false;
		VkBufferUsageFlags usageFlags;
		VkMemoryPropertyFlags memoryPropertyFlags;

		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
		{
			return vkMapMemory(device, memory, offset, size, 0, &mapped);
		}

		void unmap()
		{
			if (mapped)
			{
				vkUnmapMemory(device, memory);
				mapped = nullptr;
			}
		}

		VkResult bind(VkDeviceSize offset = 0)
		{
			return vkBindBufferMemory(device, buffer, memory, offset);
		}

		void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
		{
			descriptor.offset = offset;
			descriptor.buffer = buffer;
			descriptor.range = size;
		}

		void copyTo(void* data, VkDeviceSize size)
		{
			assert(mapped);
			memcpy(mapped, data, size);
		}

		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
		{
			VkMappedMemoryRange mappedRange = {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = memory;
			mappedRange.offset = offset;
			mappedRange.size = size;
			return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
		}

		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
		{
			VkMappedMemoryRange mappedRange = {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = memory;
			mappedRange.offset = offset;
			mappedRange.size = size;
			return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);

		}

		void destroy()
		{
			destroyed = true;
			if (device != VK_NULL_HANDLE)
			{
				if (buffer != VK_NULL_HANDLE)
				{
					vkDestroyBuffer(device, buffer, nullptr);
					buffer = VK_NULL_HANDLE;
				}
				if (memory != VK_NULL_HANDLE)
				{
					vkFreeMemory(device, memory, nullptr);
					buffer = VK_NULL_HANDLE;
				}
			}
		}

	};
	//---------------------------

	//Base-Render-Elements-------
	struct DistanceFieldText
	{
		uint32_t elementID = 0;
		std::wstring text = L"";
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.001f);
		glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		float scale = 1.0f;
	};
	struct ColorQuad
	{
		uint32_t elementID = 0; //ID
		glm::vec3 position;		//POSX, POSY, LAYERZ
		glm::vec2 size;			//WIDTH, HEIGHT
		float scale = 1.0f;		//SCALE
		glm::vec4 color;		//RGBA
	};
	struct ColorLine
	{
		uint32_t elementID = 0;
		glm::vec3 p1Pos;
		glm::vec3 p2Pos;
		glm::vec4 p1Color;
		glm::vec4 p2Color;
	};
	//---------------------------

	//GUI-Core-Elements----------
	struct Panel
	{
		uint32_t panelID = 0;
		int width = 0, height = 0, posx = 0, posy = 0, headerHeight = 33;
		bool panelHeader = false, panelControls = false, panelBody = false, panelTitle = false, panelOutline = false;
		glm::vec3 panelbodyPos = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 panelheaderPos = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 panelTitlePos = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec4 headerColor = WetAsphalt;
		glm::vec4 bodyColor = MidnightBlue;
		glm::vec4 titleColor = Clouds;
		std::wstring panelTitleString = L"";
		float defaultLayer = 0.010f;
		uint32_t panelbodyid = 0;
		uint32_t panelheaderid = 0;
		uint32_t paneltitleid = 0;
		uint32_t outlinetopid = 0;
		uint32_t outlineleftid = 0;
		uint32_t outlinerightid = 0;
		uint32_t outlinebotid = 0;
		std::vector<uint32_t> colorQuadIDs;
		std::vector<uint32_t> distanceFieldTextIDs;
		std::vector<uint32_t> colorLineIDs;
	};
	//---------------------------

	//GUI-Components-------------
	struct DragFloat3
	{

	};
	struct CheckBox
	{

	};
	struct RadioButton 
	{

	};
	struct PushButton
	{

	};
	struct TextLabel
	{

	};
	struct TextEdit
	{

	};
	struct Slider
	{
	};
	struct Spinner
	{

	};
	//---------------------------

	//Vimgui---------------------
	class VimguiCore
	{
	public:
		VimguiCore(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t swapWidth, uint32_t swapHeight, VkRenderPass renderPass, uint32_t graphicsQueueIndex, VkQueue graphicsQueue);	//Standalone feature
		~VimguiCore();
		void buildCommandBuffer(VkCommandBuffer cmdBuf);
		void onResize(uint32_t width, uint32_t height);
		void regenUI(); //Involved calling all rebuildBuffer funcs.

		//ColorQuad-------------------------------
		uint32_t addColorQuad(ColorQuad cq);
		float findColorQuad(uint32_t posx, uint32_t posy);
		void positionColorQuad(uint32_t id, uint32_t posx, uint32_t posy);
		void colorEditColorQuad(uint32_t id, float r, float g, float b, float a);
		void removeColorQuad(uint32_t id);
		void removeColorQuad(uint32_t posx, uint32_t posy);
		void removeColorQuadElement(uint32_t id);
		void addColorQuadElement(ColorQuad cq);
		void rebuildColorQuadBuffer();
		void updateColorQuads();
		//-----------------------------------------

		//DistanceFieldText------------------------
		uint32_t addDistanceFieldText(DistanceFieldText dft);
		float findDistanceFieldText(uint32_t posx, uint32_t posy);
		void positionDistanceFieldText(uint32_t id, uint32_t posx, uint32_t posy);
		void colorEditDistanceFieldText(uint32_t id, float r, float g, float b, float a);
		void removeDistanceFieldText(uint32_t id);
		void removeDistanceFieldText(uint32_t posx, uint32_t posy);
		void removeDistanceFieldTextElement(uint32_t id);
		void addDistanceFieldTextElement(DistanceFieldText dft);
		void rebuildDistanceFieldTextBuffer();
		void updateDistanceFieldText();
		//-----------------------------------------

		//ColorLine--------------------------------
		uint32_t addColorLine(ColorLine cl);
		float findColorLine(uint32_t posx, uint32_t posy);
		void positionColorLine(uint32_t id, uint32_t posx, uint32_t posy);
		void colorEditColorLine(uint32_t id, float r, float g, float b, float a);
		void removeColorLine(uint32_t id);
		void removeColorLine(uint32_t posx, uint32_t posy);
		void removeColorLineElement(uint32_t id);
		void addColorLineElement(ColorLine cl);
		void rebuildColorLineBuffer();
		void updateColorLines();
		//-----------------------------------------

	private:
		//General----------------------------------------------------
		uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr);	//Standalone feature
		void createBuffer(UIBuffer* buffer, void* data);	//Standalone feature
		void loadPNG(std::vector<unsigned char>& buffer, const char* filename);
		int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32);
		UITexture* loadImage(const char* filepath);
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void setupTexture(UITexture* texture);

		void prepare();
		void prepUniformBuffers();
		void updateUniformBuffers();
		VkShaderModule loadShader(const char* filepath);

		
		VkPhysicalDevice physicalDevice = nullptr; //Standalone feature
		VkDevice logicalDevice = nullptr; //Standalone feature
		VkRenderPass renderPass; //Standalone feature
		uint32_t graphicsQueueIndex; //Standalone feature
		VkQueue graphicsQueue;
		VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties; //Standalone feature
		uint32_t swapchainExtentWidth = 0;
		uint32_t swapchainExtentHeight = 0;
		struct UBO
		{
			glm::mat4 projectionMatrix;
			glm::mat4 modelMatrix;
		};
		UBO ubo;
		UIBuffer* uboBuffer;
		bool prepared = false;
		uint32_t vertexBufferBindID = 0;
		//------------------------------------------------------------

		//ColorQuad---------------------------------------------------
		void cqPrepPipelineCache();
		void cqPrepVertexInputDescription();
		void cqPrepDescriptorSetLayout();
		void cqPrepPipelineLayout();
		void cqPrepPipeline();
		void cqPrepDescriptorPool();
		void cqPrepDescriptorSet();
		void cqPrepColorQuadBuffer();

		float cqIDMatch = -31239.0;
		size_t cqCurrentBufferSize = 0;
		uint32_t cqMaxElements = 1'920'000; //Which equals to a maximum of 10'000 squares.  1'920'000 / 8floats per vert / 6verts / sizeof(float) = 10.000, 
		uint32_t cqElementID = 0;
		UIBuffer* cqBuffer;
		std::vector<ColorQuad> cqElementBuffer;
		std::vector<float> cqVertices;

		VkPipeline cqPipeline;
		VkDescriptorSet cqDescriptor;
		VkDescriptorPool cqDescriptorPool;
		VkDescriptorSetLayout cqDescriptorSetLayout;
		VkPipelineLayout cqPipelineLayout;
		VkPipelineCache cqPipelineCache;
		std::vector<VkVertexInputBindingDescription> cpBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> cpAttributeDescriptions;
		//------------------------------------------------------------

		//DistanceFieldText-------------------------------------------
		void parseFontFile(const char* filepath);
		void loadImagea(const char* filepath);
		void dftPrepPipelineCache();
		void dftPrepVertexInputDescription();
		void dftPrepDescriptorSetLayout();
		void dftPrepPipelineLayout();
		void dftPrepPipeline();
		void dftPrepDescriptorPool();
		void dftPrepDescriptorSet();
		void dftPrepDistanceFieldTextBuffer();

		struct fontChar {
			uint32_t x, y;
			uint32_t width;
			uint32_t height;
			int32_t xoffset;
			int32_t yoffset;
			int32_t xadvance;
		};
		std::array<fontChar, 256> fontChars;
		int32_t charCount = 0;
		float dftIDMatch = -31239.0;
		size_t dftCurrentBufferSize = 0;
		uint32_t dftMaxElements = 1'920'000;
		uint32_t dftElementID = 0;
		UIBuffer* dftBuffer;
		UITexture* fontTexture = nullptr;
		std::vector<DistanceFieldText> dftElementBuffer;
		std::vector<float> dftVertices;

		VkPipeline dftNormalPipeline;
		VkPipeline dftOutlinePipeline;
		VkPipeline dftDropshadowPipeline;
		VkDescriptorSet dftDescriptor;
		VkDescriptorPool dftDescriptorPool;
		VkDescriptorSetLayout dftDescriptorSetLayout;
		VkPipelineLayout dftPipelineLayout;
		VkPipelineCache dftPipelineCache;
		std::vector<VkVertexInputBindingDescription> dftBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> dftAttributeDescriptions;
		//------------------------------------------------------------

		//ColorLine---------------------------------------------------
		void clPrepPipelineCache();
		void clPrepVertexInputDescription();
		void clPrepDescriptorSetLayout();
		void clPrepPipelineLayout();
		void clPrepPipeline();
		void clPrepDescriptorPool();
		void clPrepDescriptorSet();
		void clPrepColorLineBuffer();

		float clIDMatch = -31239.0;
		size_t clCurrentBufferSize = 0;
		uint32_t clMaxElements = 1'680'000;
		uint32_t clElementID = 0;
		UIBuffer* clBuffer;
		std::vector<ColorLine> clElementBuffer;
		std::vector<float> clVertices;

		VkPipeline clPipeline;
		VkDescriptorSet clDescriptor;
		VkDescriptorPool clDescriptorPool;
		VkDescriptorSetLayout clDescriptorSetLayout;
		VkPipelineLayout clPipelineLayout;
		VkPipelineCache clPipelineCache;
		std::vector<VkVertexInputBindingDescription> clBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> clAttributeDescriptions;
		//------------------------------------------------------------
	};

	class Vimgui
	{
	public:
		Vimgui(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t swapWidth, uint32_t swapHeight, VkRenderPass renderPass, uint32_t graphicsQueueIndex, VkQueue graphicsQueue);
		~Vimgui();

		void kbInput(uint32_t key, uint32_t additional);
		void mouseInput(int32_t posx, int32_t posy);
		void mouseKeyInput(uint32_t key, uint32_t additional);

		uint32_t genPanel(Panel panel);
		void destroyPanel(uint32_t panelID);
		void destroyAllPanels();
		uint32_t findPanelbody(uint32_t posx, uint32_t posy);
		uint32_t findPanelheader(uint32_t posx, uint32_t posy);
		void resizePanel(uint32_t panelID, uint32_t width, uint32_t height);
		void movePanel(uint32_t panelID, int32_t x, int32_t y);
		void dragPanel(int32_t x, int32_t y);
		void addComponent(uint32_t COMPONENT, uint32_t panelID);
		void addComponent(DragFloat3 df3Comp, uint32_t panelID);
		void updatePanel(uint32_t panelID);
		void hightLightPanel(uint32_t panelID);
		void focusPanel(uint32_t panelID);
		void hidePanel(uint32_t panelID);
		void setPanelOpacity(uint32_t panelID);
		void pushForwards(uint32_t panelID);
		void pushBackwards(uint32_t panelID);
		void regenerateUserInterface();

		void itsShowtime(float dt);

		VimguiCore* vimguiCore = nullptr;
		std::vector<Panel> panels;
		uint32_t panelID = 0;
	};
	//---------------------------
}
