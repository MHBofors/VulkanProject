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
}Application;

void initWindow(Application *pApp);
void initVulkan(Application *pApp);
void mainLoop(Application *pApp);
void cleanup(Application *pApp);
void run(Application *pApp);
void createInstance(Application *pApp);
uint32_t checkValidationLayerSupport();
void setupDebugMessenger(Application *pApp);
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo);

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

void initVulkan(Application *pApp)
{
    if(enableValidationLayers && !checkValidationLayerSupport())
    {
        printf("validation layers requested, but not available");
        exit(1);
    }
    createInstance(pApp);
    setupDebugMessenger(pApp);
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