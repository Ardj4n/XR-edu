#include "OpenGLRenderer.h"

#include <iostream>

void screenshot_ppm(const char *filename, unsigned int width,
    unsigned int height, GLubyte **pixels) {
    size_t i, j, cur;
    const size_t format_nchannels = 3;
    FILE *f = fopen(filename, "w");
    fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
    *pixels = (GLubyte*)realloc(*pixels, format_nchannels * sizeof(GLubyte) * width * height);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, *pixels);
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            cur = format_nchannels * ((height - i - 1) * width + j);
            fprintf(f, "%3d %3d %3d ", (*pixels)[cur], (*pixels)[cur + 1], (*pixels)[cur + 2]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

OpenGLRenderer::OpenGLRenderer()
{

}

OpenGLRenderer::~OpenGLRenderer()
{

}

std::string OpenGLRenderer::getRenderExtensionName()
{

	return XR_KHR_OPENGL_ENABLE_EXTENSION_NAME;
}

bool OpenGLRenderer::initPlatformResources(int width, int height)
{
    sizeX = width;
    sizeY = height;

    for(int i = 0; i < swapchains.size(); i++)
        glGenFramebuffers(swapchains[i].surfaceImages.size(), swapchains[i].framebuffers.data());

    // we also create one depth buffer that we need for OpenGL's depth testing.
    // TODO: One depth buffer per view because the size could theoretically be
    // different
    glGenTextures(1, &depthbuffer);
    glBindTexture(GL_TEXTURE_2D, depthbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                 sizeX, sizeY, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);

}

bool OpenGLRenderer::free()
{

}

bool OpenGLRenderer::beginEyeFrame(int eye, int textureIndex)
{
    unsigned int textureXR = swapchains[eye].surfaceImages[textureIndex].image;
    unsigned int fboXR = swapchains[eye].framebuffers[textureIndex];

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboXR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, textureXR, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           depthbuffer, 0);

    glViewport(0, 0, sizeX, sizeY);
}

bool OpenGLRenderer::endEyeRender(int eye, int textureIndex)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void *OpenGLRenderer::getGraphicsBinding(XrInstance &xrInstance, XrSystemId &xrSystem)
{
    XrGraphicsBindingOpenGLXlibKHR* xlibGraphicsBinding = new XrGraphicsBindingOpenGLXlibKHR{};
    PFN_xrGetOpenGLGraphicsRequirementsKHR getOpenglGraphicsRequirements_KHR;

    xrGetInstanceProcAddr(xrInstance, "xrGetOpenGLGraphicsRequirementsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&getOpenglGraphicsRequirements_KHR));

    XrGraphicsRequirementsOpenGLKHR glGraphicsRequirements = {};

    glGraphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;
    glGraphicsRequirements.next = nullptr;

    getOpenglGraphicsRequirements_KHR(xrInstance, xrSystem, &glGraphicsRequirements);
    xlibGraphicsBinding->type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR;

    Display* xDisplay = glXGetCurrentDisplay();
    if( !xDisplay )
    {
        std::cout << "GL context not initialized, glutInit should have been called before" << std::endl;
        return nullptr;
    }

    xlibGraphicsBinding->xDisplay = xDisplay;
    xlibGraphicsBinding->glxDrawable = glXGetCurrentDrawable();
    xlibGraphicsBinding->glxContext = glXGetCurrentContext();
    //->glxFBConfig = glXGetFBConfigs()

    return reinterpret_cast<void*>(xlibGraphicsBinding);

}

bool OpenGLRenderer::initSwapchains(XrSession &xrSession, std::vector<XrViewConfigurationView> &views)
{
    unsigned long view_count = views.size();
    swapchains = std::vector<Swapchain>(view_count);

    //images = std::vector<std::vector<_XR_SWAPCHAIN_IMAGE>>(view_count);
    //surface_data = std::vector<std::vector<swapchain_surfdata_t>>(view_count);
    unsigned int *swapchainLength = new unsigned int(view_count);
    XrResult res;
    for (int i = 0; i < view_count; i++) {
        XrViewConfigurationView &view = views[i];
        XrSwapchainCreateInfo swapchainCreateInfo;
        swapchainCreateInfo.type			= XR_TYPE_SWAPCHAIN_CREATE_INFO;
        swapchainCreateInfo.usageFlags		= XR_SWAPCHAIN_USAGE_SAMPLED_BIT |
                                                    XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.createFlags		= 0;
        swapchainCreateInfo.format			= GL_RGBA8_EXT;
        swapchainCreateInfo.sampleCount		= view.recommendedSwapchainSampleCount;
        swapchainCreateInfo.width			= view.recommendedImageRectWidth;
        swapchainCreateInfo.height			= view.recommendedImageRectHeight;
        swapchainCreateInfo.faceCount		= 1;
        swapchainCreateInfo.arraySize		= 1;
        swapchainCreateInfo.mipCount		= 1;
        swapchainCreateInfo.next			= nullptr;
        xrCreateSwapchain(xrSession, &swapchainCreateInfo, &swapchains[i].handle);

        uint32_t surface_count = 0;
        res = xrEnumerateSwapchainImages(swapchains[i].handle, 0, &surface_count, nullptr);

        swapchains[i].surfaceImages = std::vector<XrSwapchainImageOpenGLKHR>(surface_count, {XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR});
        res = xrEnumerateSwapchainImages(swapchains[i].handle, surface_count, &surface_count,
            (XrSwapchainImageBaseHeader*)swapchains[i].surfaceImages.data());


        res = xrCreateSwapchain(xrSession, &swapchainCreateInfo, &swapchains[i].handle);
        if (!XR_SUCCEEDED(res))
        {
            std::cout << "[ERROR] Swapchain creation failed!" << std::endl;
            return false;
        }

        res = xrEnumerateSwapchainImages(swapchains[i].handle, 0, &swapchainLength[i], NULL);
        if (!XR_SUCCEEDED(res))
        {
            std::cout << "[ERROR] Enumaration of swapchains images failed!" << std::endl;
            return false;
        }

    }

    // most likely all swapchains have the same length, but let's not fail
    // if they are not: create 2d array with the longest chain as second dim
    unsigned int maxSwapchainLength = 0;
    for (int i = 0; i < view_count; i++) {
        if (swapchainLength[i] > maxSwapchainLength) {
            maxSwapchainLength = swapchainLength[i];
        }

    }

    //images = std::vector<std::vector<XrSwapchainImageOpenGLKHR>>(view_count);
    for (unsigned int i = 0; i < view_count; i++)
        swapchains[i].surfaceImages= std::vector<XrSwapchainImageOpenGLKHR>(maxSwapchainLength);

    //framebuffers = std::vector<std::vector<GLuint>>(view_count);
    for (unsigned int i = 0; i < view_count; i++)
        swapchains[i].framebuffers = std::vector<GLuint>(maxSwapchainLength);

    for (unsigned int i = 0; i < view_count; i++) {
        for (unsigned int j = 0; j < swapchainLength[i]; j++) {
            swapchains[i].surfaceImages[j].type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
            swapchains[i].surfaceImages[j].next = nullptr;
        }

        res = xrEnumerateSwapchainImages(
            swapchains[i].handle,
            swapchainLength[i],
            &swapchainLength[i],
            (XrSwapchainImageBaseHeader *) (swapchains[i].surfaceImages.data())
        );

        if(!XR_SUCCEEDED(res))
        {
            std::cout << "[ERROR] Failed to enumerate swapchain images!" << std::endl;
            return false;
        }
    }

    return true;
}

void OpenGLRenderer::renderTest(int eye, int textureIndex)
{

}

XrSwapchain OpenGLRenderer::getSwapchian(int eye)
{
    return swapchains[eye].handle;
}
