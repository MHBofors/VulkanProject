#define GLFW_INLCUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <stdlib.h>
#include <stdio.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

typedef struct
{
    GLFWwindow* window;
    VkInstance instance;
}Application;

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
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions,
        .enabledLayerCount = 0
    };

    VkResult result = vkCreateInstance(&createInfo, NULL, &pApp->instance);

    if (vkCreateInstance(&createInfo, NULL, &pApp->instance) != VK_SUCCESS)
    {
        printf("failed to create instance!\n");
    }
}

void initVulkan(Application *pApp)
{
    createInstance(pApp);
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