#pragma once


#include <vector>

#ifdef _WINDOWS
#else
#endif // _WINDOWS

#include <openxr/openxr.h>


class PlatformRenderer
{
public:
	virtual bool init() = 0;
	virtual bool free() = 0;

	virtual std::string getRenderExtensionName() = 0;
	virtual void* getGraphicsBinding(XrInstance &xrInstance, XrSystemId &xrSystem) = 0;
	virtual bool initSwapchains(XrSession &xrSession, std::vector<XrViewConfigurationView> &views) = 0;
	virtual bool initPlatformResources(int width, int height) = 0;

	virtual bool beginEyeFrame(int eye, int textureIndex) = 0;
	virtual bool endEyeRender(int eye, int textureIndex) = 0;

	virtual XrSwapchain getSwapchian(int eye) = 0;
};
