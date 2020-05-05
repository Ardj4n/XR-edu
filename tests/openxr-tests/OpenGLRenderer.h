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


/**
 * @brief This class enables rendering from OpenGL to an OpenXR application in a Linux-based enviroment (currently).
 *
 * @paragrpah This class provides the mechanisms necessary for an application to generate a valid
 * XrSwapchainImageOpenGLKHR structure in order to create a OpenGL-based XrSession.
 * This class handles the creation of all the required Framebuffer objects, including a graphics device to be used for rendering.
 * This class requires a OpenGL context initialized previously.
 * This class manages indipendently the rendering objects by resetting the viewport and binding the required framebuffer.
 */
class OpenGLRenderer : public virtual PlatformRenderer
{
public:
    OpenGLRenderer();
    ~OpenGLRenderer();

	std::string getRenderExtensionName();
    bool initPlatformResources(int width, int height);
    bool free();

    bool beginEyeFrame(int eye, int textureIndex);
    bool endEyeFrame(int eye, int textureIndex);

    void* getGraphicsBinding(XrInstance &xrInstance, XrSystemId &xrSystem);
    bool initSwapchains(XrSession &xrSession, std::vector<XrViewConfigurationView> &views);

    XrSwapchain getSwapchain(int eye);

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
