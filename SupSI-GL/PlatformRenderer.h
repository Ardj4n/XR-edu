#pragma once


#include <vector>
#include <string>

#ifdef _WINDOWS
#else
#endif // _WINDOWS

#include <openxr/openxr.h>

/**
 * @brief Platform-specific rendering and resource handling interface.
 */
class PlatformRenderer
{
public:
	/**
	 * @brief Release graphics-API resources.
	 * @return TF
	 */
	virtual bool free() = 0;


	/**
	 * @return OpenXR platform-specific render extension name
	 */
	virtual std::string getRenderExtensionName() = 0;


	/**
	 * @brief Prepares graphics-API device for OpenXR.
	 * @param xrInstance OpenXR instance
	 * @param xrSystem OpenXR system
	 * @return OpenXR-ready device
	 */
	virtual void* getGraphicsBinding(XrInstance &xrInstance, XrSystemId &xrSystem) = 0;


	/**
	 * @brief initialize OpenXR swapchains.
	 * @param xrSession OpenXR session
	 * @param views OpenXR configuration views
	 * @return TF
	 */
	virtual bool initSwapchains(XrSession &xrSession, std::vector<XrViewConfigurationView> &views) = 0;
	

	/**
	 * @brief initialize platform-specific resources.
	 * @param width framebuffer width
	 * @param height framebuffer height
	 * @return TF
	 */
	virtual bool initPlatformResources(int width, int height) = 0;


	/**
	 * @brief prepares platform-specific render on swapchain.
	 * @param eye left or right eye
	 * @param textureIndex swapchain image index
	 * @return TF
	 */
	virtual bool beginEyeFrame(int eye, int textureIndex) = 0;


	/**
	 * @brief completes platform-specific render on swapchain.
	 * @param eye left or right eye
	 * @param textureIndex swapchain image index
	 * @return TF
	 */
	virtual bool endEyeRender(int eye, int textureIndex) = 0;


	/**
	 * @brief returns XrSwapchain.
	 * @param eye left or right eye
	 * @return XrSwapchain
	 */
	virtual XrSwapchain getSwapchian(int eye) = 0;
};
