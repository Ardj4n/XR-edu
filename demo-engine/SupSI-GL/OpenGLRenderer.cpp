#include "OpenGLRenderer.h"

#include <iostream>

//Constructor
OpenGLRenderer::OpenGLRenderer()
{
    depthbuffer = 0;
}

//Destructor
OpenGLRenderer::~OpenGLRenderer()
{
    free();
}

//extension that must be enabled before calling xrCreateInstance
std::string OpenGLRenderer::getRenderExtensionName()
{

	return XR_KHR_OPENGL_ENABLE_EXTENSION_NAME;
}

/* When creating an OpenGL-backed XrSession on any Linux/Unix platform that utilizes X11 and GLX, via the Xlib library,
 * the application will provide a pointer to an XrGraphicsBindingOpenGLXlibKHR in the next chain of the XrSessionCreateInfo.
 *
 * The required window system configuration define to expose this structure type is XR_USE_PLATFORM_XLIB.
 * which defined before including openxr/openxr_platform.h
*/
void *OpenGLRenderer::getGraphicsBinding(XrInstance &xrInstance, XrSystemId &xrSystem)

{
    XrGraphicsBindingOpenGLXlibKHR* xLibGraphicsBinding = new XrGraphicsBindingOpenGLXlibKHR{};
    PFN_xrGetOpenGLGraphicsRequirementsKHR getOpenglGraphicsRequirements_KHR;

    xrGetInstanceProcAddr(xrInstance, "xrGetOpenGLGraphicsRequirementsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&getOpenglGraphicsRequirements_KHR));

    XrGraphicsRequirementsOpenGLKHR glGraphicsRequirements = {};
    glGraphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;
    glGraphicsRequirements.next = nullptr;

    getOpenglGraphicsRequirements_KHR(xrInstance, xrSystem, &glGraphicsRequirements);

    std::cout << "Minimum OpenGL version supported by the runtime: " << XR_VERSION_MAJOR(glGraphicsRequirements.minApiVersionSupported)
              << "." << XR_VERSION_MINOR(glGraphicsRequirements.minApiVersionSupported)
              << "." << XR_VERSION_PATCH(glGraphicsRequirements.minApiVersionSupported) << std::endl;
    std::cout << "Maximum OpenGL version supported by the runtime: " << XR_VERSION_MAJOR(glGraphicsRequirements.maxApiVersionSupported)
              << "." << XR_VERSION_MINOR(glGraphicsRequirements.maxApiVersionSupported)
              << "." << XR_VERSION_PATCH(glGraphicsRequirements.maxApiVersionSupported) << std::endl;

    // OpenGL context must be already initialized
    // returns X11 display
    Display* xDisplay = glXGetCurrentDisplay();
    if( !xDisplay )
    {
        std::cout << "GL context not initialized, glutInit should have been called before" << std::endl;
        return nullptr;
    }

    // fill structure with related X11 display
    xLibGraphicsBinding->type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR;
    xLibGraphicsBinding->xDisplay = xDisplay;
    xLibGraphicsBinding->glxDrawable = glXGetCurrentDrawable();
    xLibGraphicsBinding->glxContext = glXGetCurrentContext();
    //->glxFBConfig = glXGetFBConfigs()

    return reinterpret_cast<void*>(xLibGraphicsBinding);

}

bool OpenGLRenderer::initSwapchains(XrSession &xrSession, std::vector<XrViewConfigurationView> &views)
{
    unsigned long view_count = views.size();
    swapchains = std::vector<Swapchain>(view_count);

    unsigned int *swapchainLength = new unsigned int(view_count);
    XrResult res;
    for (int i = 0; i < view_count; i++) {
        XrViewConfigurationView &view = views[i];
        XrSwapchainCreateInfo swapchainCreateInfo;
        // Swapchain creation. IMPORTANT! format must be GL_RGBA8_EXT
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

        //create swapchain
        xrCreateSwapchain(xrSession, &swapchainCreateInfo, &swapchains[i].handle);
        if (!XR_SUCCEEDED(res))
        {
            std::cout << "[ERROR] Swapchain creation failed!" << std::endl;
            return false;
        }

        // Find out how many textures needs to be generate for each swapchain
        // setting image capacity input to 0 and image to null
        // indicates a request to retrieve the required capacity.
        res = xrEnumerateSwapchainImages(swapchains[i].handle, 0, &swapchainLength[i], nullptr);

        // xrEnumerateSwapchainImages fills an array of graphics API-specific XrSwapchainImage (XrSwapchainImageOpenGLKHR)
        // Each image created needs a fbo be initialized and atteched to
        swapchains[i].surfaceImages = std::vector<XrSwapchainImageOpenGLKHR>(swapchainLength[i], {XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR});
        res = xrEnumerateSwapchainImages(swapchains[i].handle, swapchainLength[i], &swapchainLength[i],
            (XrSwapchainImageBaseHeader*)swapchains[i].surfaceImages.data());
        if(!XR_SUCCEEDED(res))
        {
            std::cout << "[ERROR] Failed to enumerate swapchain images!" << std::endl;
            return false;
        }
    }
    delete swapchainLength;

    return true;
}

bool OpenGLRenderer::initPlatformResources(int width, int height)
{
    //set swapchain image size
    sizeX = width;
    sizeY = height;

    //create a framebuffer for each OpenXR generated texture resulting in a matrix (#eye x #swapchainSize)
    for(int i = 0; i < swapchains.size(); i++) {
        swapchains[i].framebuffers = std::vector<GLuint>(swapchains[i].surfaceImages.size());
        glGenFramebuffers(swapchains[i].surfaceImages.size(), swapchains[i].framebuffers.data());
    }

    // create one depth buffer needed for OpenGL's depth testing.
    // currently only one buffer is used but each fbo should have its own
    glGenTextures(1, &depthbuffer);
    glBindTexture(GL_TEXTURE_2D, depthbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                 sizeX, sizeY, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);

}

XrSwapchain OpenGLRenderer::getSwapchain(int eye)
{
    return swapchains[eye].handle;
}

bool OpenGLRenderer::beginEyeFrame(int eye, int textureIndex)
{
    //get fbo and texture ids
    unsigned int textureXR = swapchains[eye].surfaceImages[textureIndex].image;
    unsigned int fboXR = swapchains[eye].framebuffers[textureIndex];

    //bind framebuffer, texture and attach depthbuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboXR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, textureXR, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           depthbuffer, 0);

    //reset GL view port
    glViewport(0, 0, sizeX, sizeY);
}

bool OpenGLRenderer::endEyeFrame(int eye, int textureIndex)
{
    //bind default framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

bool OpenGLRenderer::free()
{
    if(depthbuffer)
        glDeleteFramebuffers(1, &depthbuffer);

    if(swapchains.size() > 0) {
        for (int i = 0; i < swapchains.size(); i++) {
            // destroy each swapchain created by OpenXR
            xrDestroySwapchain(swapchains[i].handle);

            // destroy each framebuffer
            glDeleteFramebuffers(swapchains[i].framebuffers.size(), swapchains[i].framebuffers.data());

            // destroy each texture
            for (int j = 0; j < swapchains[i].surfaceImages.size(); j++) {
                glDeleteTextures(1, &swapchains[i].surfaceImages[j].image);
            }
        }
    }
}
