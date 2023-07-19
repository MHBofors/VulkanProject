#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>



const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const uint32_t validationLayerCount = 1;
const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};


#ifdef NDEBUG
    const uint32_t enableValidationLayers = 0;
#else
    const uint32_t enableValidationLayers = 1;
#endif

typedef struct
{
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
}Application;

enum queueFamilyFlagBit{GRAPHICS_FAMILY_BIT = 1};

typedef struct {
    uint32_t flagBits;
    uint32_t graphicsFamily;
}QueueFamilyIndices;

void initWindow(Application *pApp);
void initVulkan(Application *pApp);
void mainLoop(Application *pApp);
void cleanup(Application *pApp);
void run(Application *pApp);
void createInstance(Application *pApp);
uint32_t checkValidationLayerSupport();
void setupDebugMessenger(Application *pApp);
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo);
void pickPhysicalDevice(Application *pApp);
uint32_t isDeviceSuitable(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
void createLogicalDevice(Application *pApp);


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
    
    const char *debugExtensions[glfwExtensionCount + 1];

    for(int i = 0; i < glfwExtensionCount; i++)
    {
        debugExtensions[i] = glfwExtensions[i];
    }

    if(enableValidationLayers)
    {
        debugExtensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        glfwExtensionCount++;
    }


    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = debugExtensions,
        .enabledLayerCount = 0
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
        createInfo.enabledExtensionCount = 0;

        createInfo.pNext = NULL;
    }

    VkResult result = vkCreateInstance(&createInfo, NULL, &pApp->instance);

    if (vkCreateInstance(&createInfo, NULL, &pApp->instance) != VK_SUCCESS)
    {
        printf("failed to create instance!\n");
    }

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount,
    NULL);

    VkExtensionProperties *pExtensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
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

    free(pExtensions);
}

uint32_t checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties *pAvailableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
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

    free(pAvailableLayers);
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
        if(isDeviceSuitable(devices[i]))
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

uint32_t isDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices indices = findQueueFamilies(device);

    return deviceFeatures.geometryShader && indices.flagBits & GRAPHICS_FAMILY_BIT;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for(int i = 0; i < queueFamilyCount; i++)
    {
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
            indices.flagBits |= GRAPHICS_FAMILY_BIT;
            break;
        }
    }
    return indices;
}

void createLogicalDevice(Application *pApp)
{
    QueueFamilyIndices indices = findQueueFamilies(pApp->physicalDevice);
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = indices.graphicsFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    VkPhysicalDeviceFeatures deviceFeatures = {
        0
    };

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = &queueCreateInfo,
        .queueCreateInfoCount = 1,
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

    if (vkCreateDevice(pApp->physicalDevice, &createInfo, NULL, &pApp->device) != VK_SUCCESS) {
        printf("failed to create logical device!");
        exit(1);
    }

    vkGetDeviceQueue(pApp->device, indices.graphicsFamily, 0, &pApp->graphicsQueue);
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
    pickPhysicalDevice(pApp);
    createLogicalDevice(pApp);
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

int main()
{
    Application app = {0};

    run(&app);


    return 0;
}