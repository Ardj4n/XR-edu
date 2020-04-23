#pragma once

#include <GL/glew.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#define XR_USE_PLATFORM_XLIB
#define XR_USE_GRAPHICS_API_OPENGL
#include <openxr/openxr_platform.h>
#include <openxr/openxr.h>

#include "PlatformRenderer.h"

class OpenGLRenderer : public virtual PlatformRenderer
{
public:
    OpenGLRenderer();
    ~OpenGLRenderer();

	std::string getRenderExtensionName();
    bool initPlatformResources(int width, int height);
    bool free();

    bool beginEyeFrame(int eye, int textureIndex);
    bool endEyeRender(int eye, int textureIndex);

    void* getGraphicsBinding(XrInstance &xrInstance, XrSystemId &xrSystem);
    bool initSwapchains(XrSession &xrSession, std::vector<XrViewConfigurationView> &views);

    void renderTest(int eye, int textureIndex);

    XrSwapchain getSwapchian(int eye);

private:
    struct Swapchain {
        XrSwapchain								handle;
        int32_t									width;
        int32_t									height;
        std::vector<XrSwapchainImageOpenGLKHR>	surfaceImages;
        std::vector<unsigned int>               framebuffers;
    };

    std::vector<Swapchain> swapchains;
    int sizeX, sizeY;

    unsigned int depthbuffer;
};
