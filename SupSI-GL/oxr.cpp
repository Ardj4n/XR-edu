#include "oxr.h"

#ifdef _WINDOWS
#include "DirectXRenderer.h"

#else
#include "OpenGLRenderer.h"

#endif // _WINDOWS

OvXR::OvXR(const std::string & app_name)
    : appName{ app_name }
    , xrInstance{XR_NULL_HANDLE}
    , xrSession{XR_NULL_HANDLE}
	, sessionRunning{false}
{

#ifdef _WINDOWS
	platformRenderer = new DirectXRenderer();
#else
    platformRenderer = new OpenGLRenderer();
#endif // _WINDOWS
}

OvXR::~OvXR()
{
	free();
}

bool OvXR::init()
{
	XrResult res;

	std::vector<const char*> extensionsToEnable{};
	std::string ext = platformRenderer->getRenderExtensionName();
	extensionsToEnable.push_back(ext.c_str());
	//const char* extensionsToEnable[1] = { ext.c_str() };

	XrApplicationInfo application_info = {};
	strcpy(application_info.applicationName, appName.c_str());
	application_info.apiVersion = XR_CURRENT_API_VERSION;

	XrInstanceCreateInfo instance_create_info;
	instance_create_info.type = XR_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.next = nullptr;
	instance_create_info.createFlags = 0;
	instance_create_info.applicationInfo = application_info;

	instance_create_info.enabledApiLayerCount = 0;
	instance_create_info.enabledApiLayerNames = nullptr;
    instance_create_info.enabledExtensionCount = extensionsToEnable.size();
    instance_create_info.enabledExtensionNames = extensionsToEnable.data();

	// Try create instance and look for the correct return
	xrInstance = XR_NULL_HANDLE;
	res = xrCreateInstance(&instance_create_info, &xrInstance);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[ERROR] Instance creation failed!" << std::endl;
		return false;
	}

	//---------------------------------------
	//          SYSTEM[5]
	XrSystemGetInfo system_get_info;
	memset(&system_get_info, 0, sizeof(system_get_info));
	system_get_info.type = XR_TYPE_SYSTEM_GET_INFO;
	system_get_info.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

	xrGetSystem(xrInstance, &system_get_info, &xrSys);
	if (xrSys == XR_NULL_SYSTEM_ID)
	{
		std::cout << "[ERROR] xrGetSystem failed!" << std::endl;
		return false;
	}

	//-------------------------------------
	//          VIEW Configuration[8]

	XrViewConfigurationType stereoViewConfigType =
		XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

	uint32_t viewConfigurationCount;
	res = xrEnumerateViewConfigurations(xrInstance, xrSys, 0, &viewConfigurationCount, NULL);
	if (!XR_SUCCEEDED(res))
	{
		return false;
	}
	std::cout << "Runtime supports " << viewConfigurationCount << " view configurations" << std::endl;

    std::vector<XrViewConfigurationType> viewConfigurations = std::vector<XrViewConfigurationType>(viewConfigurationCount);
    xrEnumerateViewConfigurations(xrInstance, xrSys, viewConfigurationCount, &viewConfigurationCount, viewConfigurations.data());

	{
		XrViewConfigurationProperties stereoViewConfigProperties;
		for (unsigned int i = 0; i < viewConfigurationCount; ++i) {
			XrViewConfigurationProperties properties;
			properties.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;
			properties.next = NULL;

			res = xrGetViewConfigurationProperties(xrInstance, xrSys, viewConfigurations[i], &properties);
			if (!XR_SUCCEEDED(res))
			{
				return false;
			}

			if (viewConfigurations[i] == stereoViewConfigType &&
				/* just to verify */ properties.viewConfigurationType ==
				stereoViewConfigType) {
				std::cout << "Runtime supports stereo view configuration" << std::endl;
				stereoViewConfigProperties = properties;
			}
			else {
				std::cout << "Runtime supports a view configuration we are not interested in: " << properties.viewConfigurationType << std::endl;
			}
		}
		if (stereoViewConfigProperties.type !=
			XR_TYPE_VIEW_CONFIGURATION_PROPERTIES) {
			std::cout << "Couldn't get VR View Configuration from Runtime!" << std::endl;
			return 1;
		}

		std::cout << "VR View Configuration:\n" << std::endl;
		std::cout << "\tview configuratio type: " << stereoViewConfigProperties.viewConfigurationType << std::endl;
		std::cout << "\tFOV mutable           : " << (stereoViewConfigProperties.fovMutable ? "yes" : "no") << std::endl;
    }

	res = xrEnumerateViewConfigurationViews(xrInstance, xrSys, stereoViewConfigType, 0, &viewCount, nullptr);
	if (!XR_SUCCEEDED(res))
	{
		return false;
	}

	configurationViews = std::vector<XrViewConfigurationView>(viewCount);
	for (int i = 0; i < viewCount; i++) {
		configurationViews[i].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
		configurationViews[i].next = nullptr;
	}

	views = std::vector<XrView>(viewCount);
	for (int i = 0; i < viewCount; i++) {
		views[i].type = XR_TYPE_VIEW;
		views[i].next = nullptr;
	}
	res = xrEnumerateViewConfigurationViews(xrInstance, xrSys, stereoViewConfigType, viewCount,
		&viewCount, configurationViews.data());
	if (!XR_SUCCEEDED(res))
	{
		return false;
	}

	std::cout << "View count: " << viewCount << std::endl;
	for (int i = 0; i < viewCount; i++) {
		std::cout << "View " << i << std::endl;
		std::cout << "\tResolution       : Recommended: "
			<< configurationViews[i].recommendedImageRectWidth << "x"
			<< configurationViews[i].recommendedImageRectHeight
			<< ", Max: "
			<< configurationViews[i].maxImageRectWidth << "x"
			<< configurationViews[i].maxImageRectHeight << std::endl;
		std::cout << "\tSwapchain Samples: Recommended: "
			<< configurationViews[i].recommendedSwapchainSampleCount << ", Max: "
			<< configurationViews[i].maxSwapchainSampleCount << std::endl;
	}

	//--------------------------------------
	//          SESSION[9]

	void* graphics_binding = platformRenderer->getGraphicsBinding(xrInstance, xrSys);
	if (graphics_binding == nullptr)
	{
		std::cout << "[ERROR] Unable to get Platform Binding (OpenGL/DirectX)!" << std::endl;
		return false;
	}

	XrSessionCreateInfo session_create_info = {};
	session_create_info.type = XR_TYPE_SESSION_CREATE_INFO;
	session_create_info.systemId = xrSys;
	session_create_info.next = graphics_binding;
	res = xrCreateSession(xrInstance, &session_create_info, &xrSession);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[ERROR] Session creation failed!" << std::endl;
		return false;
	}

	std::cout << "[INFO] Session created successfuly" << std::endl;

    delete graphics_binding;

	//-------------------------------------
	//          BLEND MODES[10]
	unsigned int blend_count = 0;
	res = xrEnumerateEnvironmentBlendModes(xrInstance, xrSys, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &blend_count, nullptr);
	std::vector<XrEnvironmentBlendMode> blendModes(blend_count);
	res = xrEnumerateEnvironmentBlendModes(xrInstance, xrSys, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, blend_count, &blend_count, blendModes.data());

	//--------------------------------------
	//          SWAPCHAIN[10]

	{
		unsigned int swapchainFormatCount;
		res = xrEnumerateSwapchainFormats(xrSession, 0, &swapchainFormatCount, nullptr);
		if (!XR_SUCCEEDED(res))
		{
			return false;
		}

		std::cout << "Runtime supports " << swapchainFormatCount << " swapchain color formats" << std::endl;
		int64_t *swapchainFormats = new int64_t[swapchainFormatCount];
		res = xrEnumerateSwapchainFormats(xrSession, swapchainFormatCount, &swapchainFormatCount, swapchainFormats);
		if (!XR_SUCCEEDED(res))
		{
			return false;
		}

		// TODO: Determine which format we want to use instead of using the first one
		long swapchainFormatToUse = swapchainFormats[0];
		for (int i = 0; i < swapchainFormatCount; i++)
		{
			std::cout << "0x" << std::hex << swapchainFormats[i] << std::dec << std::endl;
		}
		std::cout << std::endl;

        delete [] swapchainFormats;
	}

	platformRenderer->initSwapchains(xrSession, configurationViews);
	platformRenderer->initPlatformResources(configurationViews[0].recommendedImageRectWidth
		, configurationViews[0].recommendedImageRectHeight);
	std::cout << "swapchain created" << std::endl;

	return true;
}

bool OvXR::beginSession()
{
	// --- Begin session
	XrSessionBeginInfo sessionBeginInfo;
	sessionBeginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
	sessionBeginInfo.next = NULL;
	sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	XrResult res = xrBeginSession(xrSession, &sessionBeginInfo);
	std::cout << "Session started!" << std::endl;

	sessionRunning = XR_SUCCEEDED(res);

	// projectionLayers struct reused for every frame
	projectionLayer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
	projectionLayer.next = nullptr;
	projectionLayer.layerFlags = 0;
	projectionLayer.space = xrSpace;
	projectionLayer.viewCount = viewCount;
	// views is const and can't be changed, has to be created new every time
	projectionLayer.views = nullptr;

	return sessionRunning;
}


bool OvXR::endSession()
{
	if (xrSession == XR_NULL_HANDLE && !sessionRunning)
		return false;

	if (!XR_SUCCEEDED(xrRequestExitSession(xrSession)))
		return false;

	xrEndSession(xrSession);
	sessionRunning = false;
}

void OvXR::beginFrame()
{
	XrEventDataBuffer runtimeEvent{};
	runtimeEvent.type = XR_TYPE_EVENT_DATA_BUFFER;
	runtimeEvent.next = nullptr;

	XrFrameState frameState;
	frameState.type = XR_TYPE_FRAME_STATE;
	frameState.next = nullptr;

	XrFrameWaitInfo frameWaitInfo;
	frameWaitInfo.type = XR_TYPE_FRAME_WAIT_INFO;
	frameWaitInfo.next = nullptr;

	XrResult res = xrWaitFrame(xrSession, &frameWaitInfo, &frameState);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[ERROR] wait frame failed!" << std::endl;
		return;
	}

	// --- Handle runtime Events
	// we do this right after xrWaitFrame() so we can go idle or
	// break out of the main render loop as early as possible into
	// the frame and don't have to uselessly render or submit one
	XrResult pollResult = xrPollEvent(xrInstance, &runtimeEvent);

	if (pollResult == XR_SUCCESS)
	{
		if (eventCallbacks[runtimeEvent.type])
			eventCallbacks[runtimeEvent.type](runtimeEvent);
	}
	else if (pollResult == XR_EVENT_UNAVAILABLE) {
		// this is the usual case
	}
	else {
		std::cout << "Failed to poll events!" << std::endl;
		return;
	}

	// --- Create projection matrices and view matrices for each eye
	XrViewLocateInfo viewLocateInfo;
	viewLocateInfo.next = nullptr;
	viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	viewLocateInfo.type = XR_TYPE_VIEW_LOCATE_INFO;
	viewLocateInfo.displayTime = frameState.predictedDisplayTime;
	viewLocateInfo.space = xrSpace;

	XrViewState viewState;
	viewState.type = XR_TYPE_VIEW_STATE;
	viewState.next = nullptr;
	viewState.viewStateFlags = 0;
	unsigned int viewCountOutput;

	res = xrLocateViews(xrSession, &viewLocateInfo, &viewState,
		viewCount, &viewCountOutput, views.data());
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[ERROR] locate views failed!" << std::endl;
		return;
	}

	XrFrameBeginInfo frameBeginInfo;
	frameBeginInfo.type = XR_TYPE_FRAME_BEGIN_INFO;
	frameBeginInfo.next = nullptr;

	res = xrBeginFrame(xrSession, &frameBeginInfo);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[ERROR] locate views failed!" << std::endl;
		return;
	}


	xrPredicedDisplayTime = frameState.predictedDisplayTime;
	projectionViews = std::vector<XrCompositionLayerProjectionView>(viewCount);
}

void OvXR::endFrame()
{
	projectionLayer.views = projectionViews.data();
	const XrCompositionLayerBaseHeader* const projectionlayers[1] = {
		(const XrCompositionLayerBaseHeader* const)& projectionLayer };

	XrFrameEndInfo frameEndInfo;
	frameEndInfo.type = XR_TYPE_FRAME_END_INFO;
	frameEndInfo.displayTime = xrPredicedDisplayTime;
	frameEndInfo.layerCount = 1;
	frameEndInfo.layers = projectionlayers;
	frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	frameEndInfo.next = nullptr;

	XrResult res = xrEndFrame(xrSession, &frameEndInfo);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[ERROR] failed to end frame!" << std::endl;
		return;
	}
}

void OvXR::lockSwapchain(OvEye eye)
{
	XrSwapchain swapchain = platformRenderer->getSwapchian(eye);

	XrSwapchainImageAcquireInfo swapchainImageAcquireInfo;
	swapchainImageAcquireInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
	swapchainImageAcquireInfo.next = nullptr;

	XrResult res = xrAcquireSwapchainImage(swapchain, &swapchainImageAcquireInfo, &textureIndex);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[ERROR] xrAcquireSwapchainImage failed!" << std::endl;
		return;
	}

	XrSwapchainImageWaitInfo swapchainImageWaitInfo;
	swapchainImageWaitInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
	swapchainImageWaitInfo.next = nullptr;
	swapchainImageWaitInfo.timeout = 1000;
	res = xrWaitSwapchainImage(swapchain, &swapchainImageWaitInfo);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[ERROR] failed to wait for swapchain image!" << std::endl;
		return;
	}

	projectionViews[eye].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
	projectionViews[eye].next = nullptr;
	projectionViews[eye].pose = views[eye].pose;
	projectionViews[eye].fov = views[eye].fov;
	projectionViews[eye].subImage.swapchain = swapchain;
	projectionViews[eye].subImage.imageArrayIndex = 0;
	projectionViews[eye].subImage.imageRect.offset.x = 0;
	projectionViews[eye].subImage.imageRect.offset.y = 0;
	projectionViews[eye].subImage.imageRect.extent.width = configurationViews[eye].recommendedImageRectWidth;
	projectionViews[eye].subImage.imageRect.extent.height = configurationViews[eye].recommendedImageRectHeight;


	platformRenderer->beginEyeFrame(eye, textureIndex);
}

void OvXR::unlockSwapchain(OvEye eye)
{
	platformRenderer->endEyeRender(eye, textureIndex);
	XrSwapchain swapchain = platformRenderer->getSwapchian(eye);

	XrSwapchainImageReleaseInfo swapchainImageReleaseInfo;
	swapchainImageReleaseInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
	swapchainImageReleaseInfo.next = nullptr;
	XrResult res = xrReleaseSwapchainImage(swapchain, &swapchainImageReleaseInfo);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[ERROR] failed to release swapchain image!" << std::endl;
		return;
	}
}

bool OvXR::free()
{
    if(!sessionRunning)
        return false;

	endSession();
	//free platfomr resources
	platformRenderer->free();

	if (xrSpace != XR_NULL_HANDLE)
		xrDestroySpace(xrSpace);
	if (xrSession != XR_NULL_HANDLE)
		xrDestroySession(xrSession);
	if (xrInstance != XR_NULL_HANDLE)
		xrDestroyInstance(xrInstance);

	return true;
}


bool OvXR::setReferenceSpace(XrPosef pose, XrReferenceSpaceType spaceType)
{
	XrReferenceSpaceCreateInfo localSpaceCreateInfo;
	localSpaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
	localSpaceCreateInfo.next = NULL;
	localSpaceCreateInfo.referenceSpaceType = spaceType;
	localSpaceCreateInfo.poseInReferenceSpace = pose;

	XrResult result = xrCreateReferenceSpace(xrSession, &localSpaceCreateInfo, &xrSpace);

	return XR_SUCCEEDED(result);
}

std::string OvXR::getRuntimeName()
{
	XrResult result;

	XrInstanceProperties instanceProperties;
	instanceProperties.type = XR_TYPE_INSTANCE_PROPERTIES;
	instanceProperties.next = NULL;

	result = xrGetInstanceProperties(xrInstance, &instanceProperties);
	//if (!xr_result(NULL, result, "Failed to get instance info"))
		//return 1;


	std::stringstream ss;

	ss << instanceProperties.runtimeName << " v"
		<< XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << "."
		<< XR_VERSION_MINOR(instanceProperties.runtimeVersion) << "."
		<< XR_VERSION_PATCH(instanceProperties.runtimeVersion);

	return ss.str();
}

void OvXR::setCallback(XrStructureType event, std::function<void(XrEventDataBuffer)> callback)
{
	eventCallbacks[event] = callback;
}

std::string OvXR::getManufacturerName()
{
	XrSystemProperties systemProperties;
	systemProperties.type = XR_TYPE_SYSTEM_PROPERTIES;
	systemProperties.next = NULL;
	systemProperties.graphicsProperties = { 0 };
	systemProperties.trackingProperties = { 0 };

	xrGetSystemProperties(xrInstance, xrSys, &systemProperties);

	return systemProperties.systemName;
}

unsigned int OvXR::getHmdIdealHorizRes()
{
	return configurationViews[0].recommendedImageRectWidth;
}

unsigned int OvXR::getHmdIdealVertRes()
{
	return configurationViews[0].recommendedImageRectHeight;
}

glm::mat4 OvXR::getProjMatrix(OvEye eye, float nearPlane, float farPlane)
{
	return glm::perspective(views[eye].fov.angleUp,
		(float)getHmdIdealHorizRes() / (float)getHmdIdealVertRes(),
		nearPlane, farPlane);
}

glm::mat4 OvXR::getEyeModelviewMatrix(OvEye eye, const glm::mat4 &baseMat)
{
	glm::mat4 rot = glm::mat4{
	glm::quat{ views[eye].pose.orientation.w,
		 views[eye].pose.orientation.x,
		 views[eye].pose.orientation.y,
		 views[eye].pose.orientation.z
		}
	};

	glm::mat4 trans = glm::translate(glm::mat4{ 1.f },
		glm::vec3{
		views[eye].pose.position.x,
		views[eye].pose.position.y,
		views[eye].pose.position.z,
		}
	);

	glm::mat4 viewMatrix = trans * rot;
	glm::mat4 inverseViewMatrix = glm::inverse(viewMatrix * baseMat);

	return inverseViewMatrix;
}

XrInstance OvXR::getInstane()
{
	return xrInstance;
}

XrSession OvXR::getSession()
{
	return xrSession;
}

XrSystemId OvXR::getSystem()
{
	return xrSys;
}
