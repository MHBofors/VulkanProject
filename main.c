#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

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
    #define VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME ""
    #define VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR 0
#else
    const uint32_t enableCompatibilityBit = 1;
#endif

typedef struct
{
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
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
VkSurfaceFormatKHR chooseSwapSurfaceFormat(SwapChainSupportDetails *details);
VkPresentModeKHR chooseSwapPresentMode(SwapChainSupportDetails *details);
VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR *capabilities, GLFWwindow *window);
void createSwapChain(Application *pApp);


void initWindow(Application *pApp)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

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

    
    printf("Available extensions:\n");
    for(int i = 0; i < extensionCount; i++)
    {
        printf("\t%s\n", pExtensions[i].extensionName);
    }

    //Extension support checking
    printf("\nRequired extension support:\n");
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
    
    for(int i = 0; i < requiredExtensionCount; i++)
    {
        uint32_t check = 0;
        for(int j = 0; j < extensionCount; j++)
        {
            if(strcmp(pAvailableExtensions[j].extensionName, deviceExtensions[i]))
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

VkSurfaceFormatKHR chooseSwapSurfaceFormat(SwapChainSupportDetails *details)//Selects color space, could add ranking
{
    VkSurfaceFormatKHR availableFormat;
    for(int i = 0; i < details->formatCount; i++)
    {
        availableFormat = details->formats[i];
        if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    return details->formats[0];
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

void createSwapChain(Application *pApp) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pApp->physicalDevice, pApp->surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(&swapChainSupport);
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
}

void initVulkan(Application *pApp)
{
    if(enableValidationLayers && !checkValidationLayerSupport())
    {
        printf("validation layers requested, but not available");
        exit(1);
    }
    createInstance(pApp);
    setupDebugMessenger(pApp);
    createSurface(pApp);
    pickPhysicalDevice(pApp);
    createLogicalDevice(pApp);
    createSwapChain(pApp);
}

void mainLoop(Application *pApp)
{
    while(!glfwWindowShouldClose(pApp->window))
    {
        glfwPollEvents();
    }
}

void cleanup(Application *pApp)
{
    vkDestroyDevice(pApp->device, NULL);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(pApp->instance, pApp->debugMessenger, NULL);
    }

    vkDestroySurfaceKHR(pApp->instance, pApp->surface, NULL);

    vkDestroyInstance(pApp->instance, NULL);

    glfwDestroyWindow(pApp->window);

    glfwTerminate();
    
    vkDestroySwapchainKHR(pApp->device, pApp->swapChain, NULL);
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
