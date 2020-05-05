#include "PlatformRenderer.h"

#include <map>
#include <string>


class MockRenderer
	: public virtual PlatformRenderer
{
public:
	MockRenderer(PlatformRenderer * ext) 
		: concrete{ext}
	{
	}


	bool free() {
		funcs[__func__]++;
		return concrete->free();
	}


	std::string getRenderExtensionName() {
		funcs[__func__]++;
		return concrete->getRenderExtensionName();
	}

	void* getGraphicsBinding(XrInstance &xrInstance, XrSystemId &xrSystem) {
		funcs[__func__]++;
		return concrete->getGraphicsBinding(xrInstance, xrSystem);
	}

	bool initSwapchains(XrSession &xrSession, std::vector<XrViewConfigurationView> &views) {
		funcs[__func__]++;
		return concrete->initSwapchains(xrSession, views);
	}

	bool initPlatformResources(int width, int height) {
		funcs[__func__]++;
		return concrete->initPlatformResources(width, height);
	}

	bool beginEyeFrame(int eye, int textureIndex) {
		funcs[__func__]++;
		return concrete->beginEyeFrame(eye, textureIndex);
	}

	bool endEyeFrame(int eye, int textureIndex) {
		funcs[__func__]++;
		return concrete->endEyeFrame(eye, textureIndex);
	}

	XrSwapchain getSwapchain(int eye) {
		funcs[__func__]++;
		return concrete->getSwapchain(eye);
	}

	std::map<std::string, int> & getMap() {
		return funcs;
	}

private:
	std::map<std::string, int> funcs;

	PlatformRenderer* concrete;
};