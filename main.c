#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "vkMath.h"
#include "utils.h"

#ifndef M_PI_2
# define M_PI_2		1.57079632679489661923	/* pi/2 */
# define M_PI_4		0.78539816339744830962	/* pi/4 */
#endif

#define indexC 6
#define vertexC 4

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

const uint32_t validationLayerCount = 1;
const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

const uint32_t requiredExtensionCount = 1;
const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
    const uint32_t enableValidationLayers = 0;
#else
    const uint32_t enableValidationLayers = 1;
#endif

#ifndef __APPLE__
    const uint32_t enableCompatibilityBit = 0;
#else
    const uint32_t enableCompatibilityBit = 1;
#endif

typedef struct{
    float x;
    float y;
} Vector2;

typedef struct{
    float x;
    float y;
    float z;
} Vector3;

typedef struct{
    Vector2 position;
    Vector3 color;
} Vertex;

const uint32_t vertexCount = vertexC;

Vertex vertices[vertexC] = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const uint32_t indexCount = indexC;

const uint16_t vertexIndices[indexC] = {0, 1, 2, 2, 3, 0};

uint32_t frameIndex = 0;

clock_t startTime;

typedef struct
{
    GLFWwindow *window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    uint32_t imageCount;
    VkImage *swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkImageView *swapChainImageViews;
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkFramebuffer *swapChainFramebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;
    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkBuffer *uniformBuffers;
    VkDeviceMemory *uniformBuffersMemory;
    void **uniformBuffersMapped;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet *descriptorSets;
} Application;

enum queueFamilyFlagBit{GRAPHICS_FAMILY_BIT = 1, PRESENT_FAMILY_BIT = 1<<1};

typedef struct {
    uint32_t flagBits;
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} QueueFamilyIndices;

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t formatCount;
    VkSurfaceFormatKHR *formats;
    uint32_t presentModeCount;
    VkPresentModeKHR *presentModes;
} SwapChainSupportDetails;

typedef struct {
    float model[4][4];
    float view[4][4];
    float projection[4][4];
} UniformBufferObject;

void initWindow(Application *pApp);
void initVulkan(Application *pApp);
void mainLoop(Application *pApp);
void cleanup(Application *pApp);
void run(Application *pApp);
void createInstance(Application *pApp);
uint32_t checkValidationLayerSupport(void);
void setupDebugMessenger(Application *pApp);
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo);
void pickPhysicalDevice(Application *pApp);
uint32_t isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
uint32_t checkDeviceExtensionSupport(VkPhysicalDevice device);
uint32_t isComplete(QueueFamilyIndices indices);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
void createLogicalDevice(Application *pApp);
void createSurface(Application *pApp);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface);//Ugly, clean up
VkPresentModeKHR chooseSwapPresentMode(SwapChainSupportDetails *details);
VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR *capabilities, GLFWwindow *window);
void createSwapChain(Application *pApp);
void createImageViews(Application *pApp);
VkShaderModule createShaderModule(Application *pApp, char *shaderFile);
void createDescriptorSetLayout(Application *pApp);
void createGraphicsPipeline(Application *pApp);
void createRenderPass(Application *pApp);
void createFramebuffers(Application *pApp);
void createCommandPool(Application *pApp);
void createCommandBuffer(Application *pApp);
void recordCommandBuffer(VkCommandBuffer commandBuffer, Application *pApp, uint32_t imageIndex);
void updateUniformBuffer(Application *pApp, uint32_t currentImage);
void drawFrame(Application *pApp);
void createSyncObjects(Application *pApp);
void recreateSwapChain(Application *pApp);
void cleanupSwapChain(Application *pApp);
VkVertexInputBindingDescription getBindingDescription(void);
VkVertexInputAttributeDescription *getAttributeDescriptions(void);
void copyBuffer(Application *pApp, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void createBuffer(Application *pApp, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *bufferMemory);
uint32_t findMemoryType(Application *pApp, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void createVertexBuffer(Application *pApp);
void createIndexBuffer(Application *pApp);
void createUniformBuffers(Application *pApp);
void createDescriptorPool(Application *pApp);
void createDescriptorSets(Application *pApp);

void initWindow(Application *pApp)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    pApp->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
}

void createInstance(Application *pApp)
{
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    
    
    
    const char *debugExtensions[glfwExtensionCount + enableValidationLayers + 2 * enableCompatibilityBit];

    for(int i = 0; i < glfwExtensionCount; i++)
    {
        debugExtensions[i] = glfwExtensions[i];
    }

    if(enableValidationLayers)
    {
        debugExtensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        glfwExtensionCount++;
    }
    
    if(enableCompatibilityBit)
    {
        debugExtensions[glfwExtensionCount] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
        glfwExtensionCount++;
        debugExtensions[glfwExtensionCount] = "VK_KHR_get_physical_device_properties2";
        glfwExtensionCount++;
    }

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = debugExtensions,
        .enabledLayerCount = enableValidationLayers
    };
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};
    if(enableValidationLayers)
    {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;

        populateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = NULL;
    }
    
    if(enableCompatibilityBit)
    {
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }

    if (vkCreateInstance(&createInfo, NULL, &pApp->instance) != VK_SUCCESS)
    {
        printf("failed to create instance!\n");
    }

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount,
    NULL);

    VkExtensionProperties pExtensions[extensionCount];
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount,
    pExtensions);

    
    printf("Available instance extensions:\n");
    for(int i = 0; i < extensionCount; i++)
    {
        printf("\t%s\n", pExtensions[i].extensionName);
    }

    //Extension support checking
    printf("\nRequired instance extension support:\n");
    uint32_t unsupportedExtensionCount = 0;

    for(int i = 0; i < glfwExtensionCount; i++)
    {
        int check = 0;
        for(int j = 0; j < extensionCount; j++)
        {
            if(strcmp(pExtensions[j].extensionName, createInfo.ppEnabledExtensionNames[i]))
            {
                check = 1;
                break;
            }
        }
        
        if(check)
        {
            printf("\t%s - supported\n", createInfo.ppEnabledExtensionNames[i]);
        }
        else
        {
            unsupportedExtensionCount++;
            printf("\t%s - not supported\n", createInfo.ppEnabledExtensionNames[i]);
        }
    }
    
    if(unsupportedExtensionCount)
    {
        printf("%d unsupported extensions", unsupportedExtensionCount);
        exit(1);
    }

}

uint32_t checkValidationLayerSupport(void)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    
    VkLayerProperties pAvailableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, pAvailableLayers);

    for(int i = 0; i < validationLayerCount; i++)
    {
        uint32_t check = 0;
        for(int j = 0; j < layerCount; j++)
        {
            if(strcmp(pAvailableLayers[j].layerName, validationLayers[i]))
            {
                check = 1;
                break;
            }
        }
        
        if(!check)
        {
            return 0;
        }
    }
    
    return 1;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
void* pUserData)
{
    printf("validation layer: %s", pCallbackData->pMessage);
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    //createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
}

void setupDebugMessenger(Application *pApp)
{
    if(!enableValidationLayers)
    {
        return;
    }
    
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
    populateDebugMessengerCreateInfo(&createInfo);

    if (CreateDebugUtilsMessengerEXT(pApp->instance, &createInfo, NULL, &pApp->debugMessenger) != VK_SUCCESS) 
    {
        perror("failed to set up debug messenger!");
    }
}

void pickPhysicalDevice(Application *pApp)
{
    pApp->physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(pApp->instance, &deviceCount, NULL);

    if(deviceCount == 0)
    {
        printf("Failed to find GPUs with Vulkan support");
        exit(1);
    }

    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(pApp->instance, &deviceCount, devices);

    for(int i = 0; i < deviceCount; i++)
    {
        if(isDeviceSuitable(devices[i], pApp->surface))
        {
            pApp->physicalDevice = devices[i];
            break;
        }
    }

    if(pApp->physicalDevice == VK_NULL_HANDLE)
    {
        printf("Failed to find suitable GPU");
        exit(1);
    }
}

uint32_t isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    
    uint32_t extensionSupport = checkDeviceExtensionSupport(device);
    uint32_t swapChainAdequate = 0;
    if (extensionSupport) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = (swapChainSupport.formatCount != 0) && (swapChainSupport.presentModeCount != 0);
    }
    return isComplete(indices) && extensionSupport && swapChainAdequate;
}

uint32_t checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    
    VkExtensionProperties pAvailableExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, pAvailableExtensions);
    
    printf("\nRequired device extension support:\n");
    for(int i = 0; i < requiredExtensionCount; i++)
    {
        uint32_t check = 0;
        for(int j = 0; j < extensionCount; j++)
        {
            if(strcmp(pAvailableExtensions[j].extensionName, deviceExtensions[i]))
            {
                printf("\t%s - supported\n", deviceExtensions[i]);
                check = 1;
                break;
            }
        }
        
        if(!check)
        {
            printf("\t%s - not supported\n", deviceExtensions[i]);
            return 0;
        }
    }
    return 1;
}

uint32_t isComplete(QueueFamilyIndices indices)
{
    return (indices.flagBits & GRAPHICS_FAMILY_BIT) && (indices.flagBits & PRESENT_FAMILY_BIT);
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;
    
    indices.flagBits = 0;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for(int i = 0; i < queueFamilyCount; i++)
    {
        
        if((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            indices.graphicsFamily = i;
            indices.flagBits |= GRAPHICS_FAMILY_BIT;
        }

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if(presentSupport)
        {
            indices.presentFamily = i;
            indices.flagBits |= PRESENT_FAMILY_BIT;
        }

        if(isComplete(indices))
        {
            break;
        }
    }
    
    return indices;
}

void createLogicalDevice(Application *pApp)
{
    QueueFamilyIndices indices = findQueueFamilies(pApp->physicalDevice, pApp->surface);
    float queuePriority = 1.0f;
    uint32Tree *queueSet = allocTree();
    insert(queueSet, indices.graphicsFamily);
    insert(queueSet, indices.presentFamily);
    
    uint32_t queueCount = queueSet->size;
    
    VkDeviceQueueCreateInfo queueCreateInfos[queueCount];
    uint32_t uniqueIndices[queueCount];
    toArray(queueSet, uniqueIndices);
    
    for (int i = 0; i < queueCount; i++) {
        VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = uniqueIndices[i],
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos[i] = queueCreateInfo;
    }


    VkPhysicalDeviceFeatures deviceFeatures = {
        0
    };

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queueCreateInfos,
        .queueCreateInfoCount = queueCount,
        .pEnabledFeatures = &deviceFeatures,
        .enabledExtensionCount = 0
    };
    
    if (enableValidationLayers) 
    {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
    } 
    else 
    {
        createInfo.enabledLayerCount = 0;
    }
    const char *requiredDeviceExtensions[requiredExtensionCount + enableCompatibilityBit];
    createInfo.enabledExtensionCount = requiredExtensionCount;
    for(int i = 0; i < requiredExtensionCount; i++)
    {
        requiredDeviceExtensions[i] = deviceExtensions[i];
    }
    
    if(enableCompatibilityBit)
    {
        requiredDeviceExtensions[createInfo.enabledExtensionCount] = "VK_KHR_portability_subset";
        createInfo.enabledExtensionCount++;
    }
    
    createInfo.ppEnabledExtensionNames = requiredDeviceExtensions;
    
    if (vkCreateDevice(pApp->physicalDevice, &createInfo, NULL, &pApp->device) != VK_SUCCESS) {
        printf("failed to create logical device!");
        exit(1);
    }

    vkGetDeviceQueue(pApp->device, indices.graphicsFamily, 0, &pApp->graphicsQueue);
    vkGetDeviceQueue(pApp->device, indices.presentFamily, 0, &pApp->presentQueue);
}

void createSurface(Application *pApp)
{
    if (glfwCreateWindowSurface(pApp->instance, pApp->window, NULL, &pApp->surface) != VK_SUCCESS) 
    {
        printf("failed to create window surface!");
        exit(1);
    }
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, NULL);

    if (details.formatCount != 0) {
        VkSurfaceFormatKHR formats[details.formatCount];
        details.formats = formats;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, details.formats);
    }
    
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, NULL);

    if (details.presentModeCount != 0) {
        VkPresentModeKHR presentModes[details.presentModeCount];
        details.presentModes = presentModes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, details.presentModes);
    }
    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface)//Selects color space, could add ranking
{
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);
    VkSurfaceFormatKHR availableFormats[formatCount];
    if(formatCount != 0)
    {
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, availableFormats);
        for(int i = 0; i < formatCount; i++)
            {
                VkSurfaceFormatKHR availableFormat = availableFormats[i];
                if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    return availableFormat;
                }
            }
        return availableFormats[0];
    }
    else
    {
        printf("No formats found!\n");
        exit(1);
    }
}

VkPresentModeKHR chooseSwapPresentMode(SwapChainSupportDetails *details)//Selects how the swap chain displays images to the screen, VK_PRESENT_MODE_FIFO_KHR -> display takes images in front of queue, inserts rendered images to the back
{
    VkPresentModeKHR availablePresentMode;
    for (int i = 0; i < details->presentModeCount; i++) {
        availablePresentMode = details->presentModes[i];
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR *capabilities, GLFWwindow *window)//Translates between pixels and screen coordinates, so swap chain images are correct resolution
{
    uint32_t max = ~0;
    if(capabilities->currentExtent.width != max)
    {
        return capabilities->currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        
        VkExtent2D actualExtent = {
            .width = (uint32_t)width,
            .height = (uint32_t)height
        };
        
        actualExtent.width = boundU32(actualExtent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
        actualExtent.height = boundU32(actualExtent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);
        return actualExtent;
    }
}

void createSwapChain(Application *pApp) 
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pApp->physicalDevice, pApp->surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(pApp->physicalDevice, pApp->surface);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(&swapChainSupport);
    VkExtent2D extent = chooseSwapExtent(&swapChainSupport.capabilities, pApp->window);
    
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    //maxImageCount = 0 -> no maximum #images
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = pApp->surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,//Specifies what kinds of operations the images will be used for, currently rendering directly to them
        .preTransform = swapChainSupport.capabilities.currentTransform,//Can specify transforms to apply to images in swap chain
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,//Opaque -> alpha channel should not be used to blend with other windows
        .presentMode = presentMode,
        .clipped = VK_TRUE,//True -> we don't care about color of obscured pixel
        .oldSwapchain = VK_NULL_HANDLE//If swap chain must be replaced, then reference to old one must be given here
    };
    
    QueueFamilyIndices indices = findQueueFamilies(pApp->physicalDevice, pApp->surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//Several families can use images without transfer of membership
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//Only one family can use images at a time
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = NULL; // Optional
    }
    
    if (vkCreateSwapchainKHR(pApp->device, &createInfo, NULL, &pApp->swapChain) != VK_SUCCESS) {
        printf("failed to create swap chain!");
        exit(1);
    }
    
    pApp->imageCount = imageCount;
    
    vkGetSwapchainImagesKHR(pApp->device, pApp->swapChain, &pApp->imageCount, NULL);
    pApp->swapChainImages = malloc(pApp->imageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(pApp->device, pApp->swapChain, &pApp->imageCount, pApp->swapChainImages);
    
    pApp->swapChainImageFormat = surfaceFormat.format;
    pApp->swapChainExtent = extent;
}

void createImageViews(Application *pApp)
{
    pApp->swapChainImageViews = malloc(pApp->imageCount * sizeof(VkImageView));
    for (size_t i = 0; i < pApp->imageCount; i++)
    {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = pApp->swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,//Determines how data will be interpreted, whether images should be treated as 1-,2-, or 3d images
            .format = pApp->swapChainImageFormat,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,//Allows you to "swizzle" colors arount
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,//Determines the purpose of the image, and which part should be accessed
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1
        };
        
        if (vkCreateImageView(pApp->device, &createInfo, NULL, &pApp->swapChainImageViews[i]) != VK_SUCCESS) {
            printf("failed to create image views!");
            exit(1);
        }
    }
}

VkShaderModule createShaderModule(Application *pApp, char *shaderFile)
{
    char *binary;
    
    size_t codeSize = readFile(shaderFile, &binary);

    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = codeSize,
        .pCode = (uint32_t*)binary//Probably not safe, and should be fixed
    };
    
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(pApp->device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        printf("Failed to create shader module!");
        exit(1);
    }
    free(binary);
    return shaderModule;
}

void createRenderPass(Application *pApp)
{
    VkAttachmentDescription colorAttachment = {
        .format = pApp->swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,//Specifies what to do with the data in the attachment before and after rendering
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    
    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    
    VkSubpassDescription subpass = {
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef
    };
    
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,//Specifies the indices of the dependency in the dependent subpass
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,//Specifies the operations to wait for, and in which stage they occur
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,//Specifies the operations waiting for the previous operations, and in which stage they occur
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };
    
    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };
    
    
    
    if (vkCreateRenderPass(pApp->device, &renderPassInfo, NULL, &pApp->renderPass) != VK_SUCCESS) {
        printf("Failed to create render pass!");
        exit(1);
    }
}

void createDescriptorSetLayout(Application *pApp) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0, //Which binding is used
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, //Type of descriptor
        .descriptorCount = 1, //Number of value in the array
        
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, //Which shader stage the descriptor is used
        .pImmutableSamplers = NULL, // - relevant to image sampling descriptors -
    };
    
    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding
    };
    
    if (vkCreateDescriptorSetLayout(pApp->device, &layoutInfo, NULL, &pApp->descriptorSetLayout) != VK_SUCCESS) {
        printf("failed to create descriptor set layout!");
        exit(1);
    }
}

void createGraphicsPipeline(Application *pApp)
{
    VkShaderModule vertexModule = createShaderModule(pApp, "shaders/vert.spv");
    VkShaderModule fragmentModule = createShaderModule(pApp, "shaders/frag.spv");
    
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertexModule,//Determines the module containing the code
        .pName = "main",//Determines which function will invoke the shader
        .pSpecializationInfo = NULL//Optional member specifying values for shader constants
    };
    
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragmentModule,
        .pName = "main",
        .pSpecializationInfo = NULL
    };
    
    VkPipelineShaderStageCreateInfo shaderStages[2] = {vertShaderStageInfo, fragShaderStageInfo};
    
    uint32_t stateCount = 2;
    VkDynamicState dynamicStates[stateCount];
    dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;

    //Specifies which part of the state will be dynamic
    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = stateCount,
        .pDynamicStates = dynamicStates
    };
    
    VkVertexInputBindingDescription bindingDescription = getBindingDescription();
    VkVertexInputAttributeDescription *attributeDescriptions = getAttributeDescriptions();
    
    //Specifies the bindings and attribute descriptions
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription, // Optional
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = attributeDescriptions
    };
    
    //Specifies which type of geometry will be rendered
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };
    
    //Specifies which region of the framebuffer that will be rendered to
    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) pApp->swapChainExtent.width,
        .height = (float) pApp->swapChainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    
    //Specifies in which regions pixels will be stored in the framebuffer
    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = pApp->swapChainExtent
    };

    //Needs pointers to viewport and scissor if created statically
    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };
    
    //Specifies how fragments are created from the geometry generated by the vertex shader
    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,//If enabled, fragments beyond the near and far plane are clamped to them
        .rasterizerDiscardEnable = VK_FALSE,//If enabled, disables the rasterization stage
        .polygonMode = VK_POLYGON_MODE_FILL,//Specifies how fragments are generated for geometry
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,//Specifies which type of face culling to use
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,//Specifies the vertex order for faces to be considered front-facing
        .depthBiasEnable = VK_FALSE,//If enabled, can add depth values by adding a constant, or biasing them based on fragment slope
        .depthBiasConstantFactor = 0.0f, // Optional
        .depthBiasClamp = 0.0f, // Optional
        .depthBiasSlopeFactor = 0.0f // Optional
    };
    
    //Specifies how to use multisampling for anti-aliasing
    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f, // Optional
        .pSampleMask = NULL, // Optional
        .alphaToCoverageEnable = VK_FALSE, // Optional
        .alphaToOneEnable = VK_FALSE // Optional
    };
    
    //Specifies the color blending configuration per attached framebuffer
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
        .colorBlendOp = VK_BLEND_OP_ADD, // Optional
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
        .alphaBlendOp = VK_BLEND_OP_ADD // Optional
    };
    
    //Specifies the global color blending configuration
    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY, // Optional
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants[0] = 0.0f, // Optional
        .blendConstants[1] = 0.0f, // Optional
        .blendConstants[2] = 0.0f, // Optional
        .blendConstants[3] = 0.0f // Optional
    };
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1, // Optional
        .pSetLayouts = &pApp->descriptorSetLayout, // Optional
        .pushConstantRangeCount = 0, // Optional
        .pPushConstantRanges = NULL // Optional
    };
    
    if (vkCreatePipelineLayout(pApp->device, &pipelineLayoutInfo, NULL, &pApp->pipelineLayout) != VK_SUCCESS)
    {
        printf("Failed to create pipeline layout!");
        exit(1);
    }
    
    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = NULL,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = pApp->pipelineLayout,
        .renderPass = pApp->renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,//Handle for if pipeline is recreated
        .basePipelineIndex = -1
    };
    
    if (vkCreateGraphicsPipelines(pApp->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pApp->graphicsPipeline) != VK_SUCCESS) {
        printf("failed to create graphics pipeline!");
    }
    
    vkDestroyShaderModule(pApp->device, vertexModule, NULL);
    vkDestroyShaderModule(pApp->device, fragmentModule, NULL);
}

void createFramebuffers(Application *pApp)
{
    pApp->swapChainFramebuffers = malloc(pApp->imageCount * sizeof(VkFramebuffer));
    for(int i = 0; i < pApp->imageCount; i++)
    {
        VkImageView attachments = pApp->swapChainImageViews[i];
        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = pApp->renderPass,
            .attachmentCount = 1,
            .pAttachments = &attachments,
            .width = pApp->swapChainExtent.width,
            .height = pApp->swapChainExtent.height,
            .layers = 1
        };
        
        if (vkCreateFramebuffer(pApp->device, &framebufferInfo, NULL, &pApp->swapChainFramebuffers[i]) != VK_SUCCESS) {
                printf("Failed to create framebuffer!");
        }
    }
}

void createCommandPool(Application *pApp)
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(pApp->physicalDevice, pApp->surface);
    
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,//Specifies how commandbuffers are recorded and reser
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily
    };
    
    if (vkCreateCommandPool(pApp->device, &poolInfo, NULL, &pApp->commandPool) != VK_SUCCESS) {
        printf("Failed to create command pool!");
        exit(1);
    }
}

void createCommandBuffer(Application *pApp)
{
    pApp->commandBuffers = malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);
    
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pApp->commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,//Specifies if the command buffers are primary or secondary
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT
    };
    
    if(vkAllocateCommandBuffers(pApp->device, &allocInfo, pApp->commandBuffers) != VK_SUCCESS) {
        printf("Failed to allocate command buffers!");
        exit(1);
    }
}

void recordCommandBuffer(VkCommandBuffer commandBuffer, Application *pApp, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,//Specifies how the command buffer will be used
        .pInheritanceInfo = NULL//Specifies which state to inherit from primary calling command buffer
    };
    
    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        printf("Failed to begin recording command buffer");
        exit(1);
    }
    
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    
    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = pApp->renderPass,
        .framebuffer = pApp->swapChainFramebuffers[imageIndex],
        .renderArea.offset = {0,0},//Specifies the size of the render area
        .renderArea.extent = pApp->swapChainExtent,
        .clearValueCount = 1,//Specifies the clear values for the attachment clear operation to use
        .pClearValues = &clearColor
    };
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pApp->graphicsPipeline);
    
    
    
    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)pApp->swapChainExtent.width,
        .height = (float)pApp->swapChainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    
    VkRect2D scissor = {
        .offset = {0,0},
        .extent = pApp->swapChainExtent
    };
    
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    
    VkBuffer vertexBuffers[] = {pApp->vertexBuffer};
    
    VkDeviceSize offsets[] = {0};
        
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);//Parameters 2, 3 specifies the offsets and how many vertex buffers to bind. Parameters 4, 5 specifies the array of vertex buffers to write and what offset to start reading from
        
    vkCmdBindIndexBuffer(commandBuffer, pApp->indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pApp->pipelineLayout, 0, 1, &pApp->descriptorSets[frameIndex], 0, NULL);
    
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    
    vkCmdEndRenderPass(commandBuffer);
    
    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        printf("Failed to record command buffer!");
        exit(1);
    }
}

void updateUniformBuffer(Application *pApp, uint32_t currentFrame)
{
    clock_t currentTime = clock();
    float dTime = (float)(currentTime - startTime)/CLOCKS_PER_SEC;
    
    UniformBufferObject ubo;
    
    vector axis = {0.0f, 0.0f, 1.0f};

    rotationMatrix(ubo.model, dTime * 2*M_PI, axis);
    
    vector new_axis = {cos(dTime), sin(dTime), 0};
    
    float rot[4][4];
    rotationMatrix(rot, M_PI_4, new_axis);
    
    matmul(rot, ubo.model);
    
    //matcpy(rot, ubo.model);
    
    float d = 2.0f;
    
    vector camera = {d, d, d};
    vector up = {0.0f, 0.0f, 1.0f};
    vector object = {0.0f, 0.0f, 0.0f};
    
    cameraMatrix(ubo.view, camera, object, up);

    float r = pApp->swapChainExtent.width/((float) pApp->swapChainExtent.height);
    
    perspectiveMatrix(ubo.projection, M_PI_2, r, 0.1f, 10.0f);

    memcpy(pApp->uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}

void drawFrame(Application *pApp)
{
    vkWaitForFences(pApp->device, 1, &pApp->inFlightFences[frameIndex], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(pApp->device, pApp->swapChain, UINT64_MAX, pApp->imageAvailableSemaphores[frameIndex], VK_NULL_HANDLE, &imageIndex);
    
    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain(pApp);
        return;
    }
    
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        printf("Failed to acquire swap chain image!");
        exit(1);
    }

    updateUniformBuffer(pApp, frameIndex);

    vkResetFences(pApp->device, 1, &pApp->inFlightFences[frameIndex]);
    
    vkResetCommandBuffer(pApp->commandBuffers[frameIndex], 0);

    recordCommandBuffer(pApp->commandBuffers[frameIndex], pApp, imageIndex);

    VkSemaphore waitSemaphores[] = {pApp->imageAvailableSemaphores[frameIndex]};
    VkSemaphore signalSemaphores[] = {pApp->renderFinishedSemaphores[frameIndex]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,//Specifies which semaphores to wait for, before execution
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &pApp->commandBuffers[frameIndex],
        .signalSemaphoreCount = 1,//Specifies which semaphores to signal, after execution
        .pSignalSemaphores = signalSemaphores
    };

    if (vkQueueSubmit(pApp->graphicsQueue, 1, &submitInfo, pApp->inFlightFences[frameIndex]) != VK_SUCCESS) {
        printf("Failed to submit draw command buffer!");
        exit(1);
    }
    
    VkSwapchainKHR swapChains[] = {pApp->swapChain};
    

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,//Specifies which semaphores to wait for before presentation can happen
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,//Specifies which swap chains to present images to, as well as the index of the image for each swap chain
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,
        .pResults = NULL//Specifies an array of VK_RESULT, checking that presentation was successful for each swap chain
    };

    result = vkQueuePresentKHR(pApp->presentQueue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        recreateSwapChain(pApp);
        return;
    }
    
    else if (result != VK_SUCCESS)
    {
        printf("Failed to present swap chain image");
        exit(1);
    }

    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void createSyncObjects(Application *pApp)
{
    pApp->imageAvailableSemaphores = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    pApp->renderFinishedSemaphores = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    pApp->inFlightFences = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);
    
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if(vkCreateSemaphore(pApp->device, &semaphoreInfo, NULL, &pApp->imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(pApp->device, &semaphoreInfo, NULL, &pApp->renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(pApp->device, &fenceInfo, NULL, &pApp->inFlightFences[i]) != VK_SUCCESS){
            printf("Failed to create sync objects!");
            exit(1);
        }
    }
}

void recreateSwapChain(Application *pApp)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(pApp->window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(pApp->window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(pApp->device);
    
    cleanupSwapChain(pApp);
    
    createSwapChain(pApp);
    createImageViews(pApp);
    createFramebuffers(pApp);
}

void cleanupSwapChain(Application *pApp)
{
    for(int i = 0; i < pApp->imageCount; i++)
    {
        vkDestroyFramebuffer(pApp->device, pApp->swapChainFramebuffers[i], NULL);
    }
    free(pApp->swapChainFramebuffers);
    
    for(int i = 0; i < pApp->imageCount; i++)
    {
        vkDestroyImageView(pApp->device, pApp->swapChainImageViews[i], NULL);
    }
    free(pApp->swapChainImageViews);
    
    vkDestroySwapchainKHR(pApp->device, pApp->swapChain, NULL);
    free(pApp->swapChainImages);
}

VkVertexInputBindingDescription getBindingDescription(void)
{
    VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,//Specifies the index in the array of bindings
        .stride = sizeof(Vertex),//Specifies the size of the binding, giving the length between the start of the binding to the next one
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX//Specifies whether to move to the next data entry after each vertex or each instance
    };
    
    return bindingDescription;
}

VkVertexInputAttributeDescription *getAttributeDescriptions(void)
{
    VkVertexInputAttributeDescription *attributeDescriptions = malloc(sizeof(VkVertexInputAttributeDescription) * 2);
    
    attributeDescriptions[0].binding = 0;//Specifies from which binding the data comes
    attributeDescriptions[0].location = 0;//Specifies which 'location' the data will have in the vertex shader
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;//Specifies the type of the data of the attribute, which uses the same enum as color formats
    attributeDescriptions[0].offset = offsetof(Vertex, position);//Specifies how far from the start of the data to read from
    
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    return attributeDescriptions;
}

void createBuffer(Application *pApp, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    
    if(vkCreateBuffer(pApp->device, &bufferInfo, NULL, buffer) != VK_SUCCESS){
        printf("Failed to create buffer!");
        exit(1);
    }
    
    VkMemoryRequirements memoryReq;
    vkGetBufferMemoryRequirements(pApp->device, *buffer, &memoryReq);
    
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryReq.size,
        .memoryTypeIndex = findMemoryType(pApp, memoryReq.memoryTypeBits, properties)
    };
    
    if(vkAllocateMemory(pApp->device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS){
        printf("Failed to allocate buffer memory");
        exit(1);
    }
    
    vkBindBufferMemory(pApp->device, *buffer, *bufferMemory, 0);
}

void copyBuffer(Application *pApp, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = pApp->commandPool,
        .commandBufferCount = 1
    };
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(pApp->device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    VkBufferCopy copyRegion = {
        copyRegion.srcOffset = 0, // Optional
        copyRegion.dstOffset = 0, // Optional
        copyRegion.size = size
    };
    
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };
    
    vkQueueSubmit(pApp->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(pApp->graphicsQueue);
    
    vkFreeCommandBuffers(pApp->device, pApp->commandPool, 1, &commandBuffer);
}

void createVertexBuffer(Application *pApp)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(pApp, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(pApp->device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices, (size_t) bufferSize);
    vkUnmapMemory(pApp->device, stagingBufferMemory);

    createBuffer(pApp, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &pApp->vertexBuffer, &pApp->vertexBufferMemory);

    copyBuffer(pApp, stagingBuffer, pApp->vertexBuffer, bufferSize);
    
    vkDestroyBuffer(pApp->device, stagingBuffer, NULL);
    vkFreeMemory(pApp->device, stagingBufferMemory, NULL);
}

void createIndexBuffer(Application *pApp)
{
    VkDeviceSize bufferSize = sizeof(vertexIndices[0]) * indexCount;
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(pApp, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(pApp->device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertexIndices, (size_t) bufferSize);
    vkUnmapMemory(pApp->device, stagingBufferMemory);

    createBuffer(pApp, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &pApp->indexBuffer, &pApp->indexBufferMemory);

    copyBuffer(pApp, stagingBuffer, pApp->indexBuffer, bufferSize);
    
    vkDestroyBuffer(pApp->device, stagingBuffer, NULL);
    vkFreeMemory(pApp->device, stagingBufferMemory, NULL);
}

uint32_t findMemoryType(Application *pApp, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(pApp->physicalDevice, &memoryProperties);
    
    for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if(typeFilter & (1 << i) && memoryProperties.memoryTypes[i].propertyFlags & properties)
        {
            return i;
        }
    }
    
    printf("Failed to find suitable memory type");
    exit(1);
}

void createUniformBuffers(Application *pApp)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    
    pApp->uniformBuffers = malloc(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
    pApp->uniformBuffersMemory = malloc(sizeof(VkDeviceMemory) * MAX_FRAMES_IN_FLIGHT);
    pApp->uniformBuffersMapped = malloc(sizeof(void*) * MAX_FRAMES_IN_FLIGHT);
    
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(pApp, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &pApp->uniformBuffers[i], &pApp->uniformBuffersMemory[i]);
        
        vkMapMemory(pApp->device, pApp->uniformBuffersMemory[i], 0, bufferSize, 0, &pApp->uniformBuffersMapped[i]);
    }
}

void createDescriptorPool(Application *pApp)
{
    VkDescriptorPoolSize poolSize = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = (uint32_t)MAX_FRAMES_IN_FLIGHT,
    };
    
    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
        
        .maxSets = (uint32_t)MAX_FRAMES_IN_FLIGHT
    };
    
    if(vkCreateDescriptorPool(pApp->device, &poolInfo, NULL, &pApp->descriptorPool) != VK_SUCCESS) {
        printf("Failed to create descriptor pool!");
        exit(1);
    }
}

void createDescriptorSets(Application *pApp)
{
    VkDescriptorSetLayout *layouts = malloc(sizeof(VkDescriptorSetLayout) * MAX_FRAMES_IN_FLIGHT);
    for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        layouts[i] = pApp->descriptorSetLayout;
    }
    
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pApp->descriptorPool,
        .descriptorSetCount = (uint32_t)(MAX_FRAMES_IN_FLIGHT),
        .pSetLayouts = layouts
    };
    
    pApp->descriptorSets = malloc(sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);
    
    if(vkAllocateDescriptorSets(pApp->device, &allocInfo, pApp->descriptorSets) != VK_SUCCESS) {
        printf("Failed to create descriptor sets!");
        exit(1);
    }
    
    for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = pApp->uniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject)
        };
        
        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = pApp->descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            
            .pBufferInfo = &bufferInfo,
            .pImageInfo = NULL,
            .pTexelBufferView = NULL
        };
        
        vkUpdateDescriptorSets(pApp->device, 1, &descriptorWrite, 0, NULL);
    }
    
    free(layouts);
}

void initVulkan(Application *pApp)
{
    if(enableValidationLayers && !checkValidationLayerSupport())
    {
        printf("Validation layers requested, but not available");
        exit(1);
    }
    createInstance(pApp);
    setupDebugMessenger(pApp);
    createSurface(pApp);
    pickPhysicalDevice(pApp);
    createLogicalDevice(pApp);
    createSwapChain(pApp);
    createImageViews(pApp);
    createRenderPass(pApp);
    createDescriptorSetLayout(pApp);
    createGraphicsPipeline(pApp);
    createFramebuffers(pApp);
    createCommandPool(pApp);
    createVertexBuffer(pApp);
    createIndexBuffer(pApp);
    createUniformBuffers(pApp);
    createDescriptorPool(pApp);
    createDescriptorSets(pApp);
    createCommandBuffer(pApp);
    createSyncObjects(pApp);
}

void mainLoop(Application *pApp)
{
    while(!glfwWindowShouldClose(pApp->window))
    {
        glfwPollEvents();
        drawFrame(pApp);
    }

    vkDeviceWaitIdle(pApp->device);
}

void cleanup(Application *pApp)
{
    cleanupSwapChain(pApp);
    
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(pApp->device, pApp->uniformBuffers[i], NULL);
        vkFreeMemory(pApp->device, pApp->uniformBuffersMemory[i], NULL);
    }
    
    free(pApp->uniformBuffers);
    free(pApp->uniformBuffersMemory);
    free(pApp->uniformBuffersMapped);
    
    vkDestroyDescriptorPool(pApp->device, pApp->descriptorPool, NULL);
    
    vkDestroyDescriptorSetLayout(pApp->device, pApp->descriptorSetLayout, NULL);
    
    free(pApp->descriptorSets);
    
    vkDestroyBuffer(pApp->device, pApp->vertexBuffer, NULL);
    vkFreeMemory(pApp->device, pApp->vertexBufferMemory, NULL);
    
    vkDestroyBuffer(pApp->device, pApp->indexBuffer, NULL);
    vkFreeMemory(pApp->device, pApp->indexBufferMemory, NULL);
    
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(pApp->device, pApp->imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(pApp->device, pApp->renderFinishedSemaphores[i], NULL);
        vkDestroyFence(pApp->device, pApp->inFlightFences[i], NULL);
    }
    
    free(pApp->imageAvailableSemaphores);
    free(pApp->renderFinishedSemaphores);
    free(pApp->inFlightFences);
    
    vkDestroyCommandPool(pApp->device, pApp->commandPool, NULL);
    
    vkDestroyPipeline(pApp->device, pApp->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(pApp->device, pApp->pipelineLayout, NULL);
    vkDestroyRenderPass(pApp->device, pApp->renderPass, NULL);
    
    vkDestroyDevice(pApp->device, NULL);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(pApp->instance, pApp->debugMessenger, NULL);
    }

    vkDestroySurfaceKHR(pApp->instance, pApp->surface, NULL);

    vkDestroyInstance(pApp->instance, NULL);

    glfwDestroyWindow(pApp->window);

    glfwTerminate();
}

void run(Application *pApp)
{
    initWindow(pApp);
    initVulkan(pApp);
    mainLoop(pApp);
    cleanup(pApp);
}

int main(void)
{
    startTime = clock();

    if(enableCompatibilityBit)
    {
        printf("Compatibility bit enabled\n");
    }
    else
    {
        printf("Compatibility bit NOT enabled\n");
    }
    Application app = {0};

    run(&app);

    return 0;
}
