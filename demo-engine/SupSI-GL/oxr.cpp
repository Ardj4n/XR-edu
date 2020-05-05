#include "oxr.h"

#ifdef _WINDOWS
#include "DirectXRenderer.h"
#else
#include "OpenGLRenderer.h"
#endif // _WINDOWS

OvXR::OvXR(const std::string & app_name)
	: appName{ app_name }
	, xrInstance{ XR_NULL_HANDLE }
	, xrSession{ XR_NULL_HANDLE }
	, sessionRunning{ false }
	, graphicsBinding { nullptr }
{
	// initialize the specif class / rendering layer based on the platform we are on
#ifdef _WINDOWS
	platformRenderer = new DirectXRenderer();
#else
	platformRenderer = new OpenGLRenderer();
#endif // _WINDOWS
}

OvXR::~OvXR()
{

}

bool OvXR::init()
{
	XrResult res;

	std::vector<const char*> extensionsToEnable;
	// retrive the platform render specific extension name and add it
	// to the list of extension that will use for creating the instance
	std::string ext = platformRenderer->getRenderExtensionName();
	extensionsToEnable.push_back(ext.c_str());

	// some informations that help runtimes recognize behavior inherent to classes of applications
	XrApplicationInfo applicationInfo;
	strcpy(applicationInfo.applicationName, appName.c_str()); // name of the application
	strcpy(applicationInfo.engineName, appName.c_str()); // name of the application
	applicationInfo.apiVersion = XR_CURRENT_API_VERSION; // version of this API against which the application will run

	XrInstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO; // the XrStructureType of this structure
	instanceCreateInfo.next = nullptr;
	instanceCreateInfo.createFlags = 0;
	instanceCreateInfo.applicationInfo = applicationInfo; // link to applicationInfo
	instanceCreateInfo.enabledApiLayerCount = 0; // the number of global API layers to enable
	instanceCreateInfo.enabledApiLayerNames = nullptr; //... no API layers to enable
	instanceCreateInfo.enabledExtensionCount = extensionsToEnable.size(); // numbero of extension to enable
	instanceCreateInfo.enabledExtensionNames = extensionsToEnable.data(); // link to extensionsToEnable

	// create the instance and check the result of the operation
	res = xrCreateInstance(&instanceCreateInfo, &xrInstance);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[OvXR | ERROR] Instance creation failed!" << std::endl;
		return false;
	}

	std::cout << "[OvXR | INFO] XrInstance created" << std::endl;


	//---------------------------------------
	//          SYSTEM[5]
	// structure that specifies attributes about a system as desired by an application
	XrSystemGetInfo systemGetInfo;
	memset(&systemGetInfo, 0, sizeof(systemGetInfo));
	systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
	// the following attribute specifics the form factor
	// XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY --> The tracked display is attached to
	// the user�s head (VR headset use case)
	systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

	// retrive the system ID and check the result of the operation
	xrGetSystem(xrInstance, &systemGetInfo, &xrSys);
	if (xrSys == XR_NULL_SYSTEM_ID)
	{
		std::cout << "[OvXR | ERROR] xrGetSystem failed!" << std::endl;
		return false;
	}

	std::cout << "[OvXR | INFO] System ready" << std::endl;

	//-------------------------------------
	//          VIEW Configuration[8]
	// setting of view configuration
	// XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO --> Two views representing the form
	// factor�s two primary displays, which map to a left-eye and right-eye view.
	// This configuration requires two views in XrViewConfigurationProperties and two
	// views in each XrCompositionLayerProjection layer.
	// View index 0 must represent the left eye and view index 1 must represent the right eye.
	XrViewConfigurationType stereoViewConfigType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

	uint32_t viewConfigurationCount;
	// enumerateing the view configuration types supported by the XrSystemId
	// viewConfigurationsTypeCapacityInput = 0 indicate a request to retrieve the required capacity
	res = xrEnumerateViewConfigurations(xrInstance, xrSys, 0, &viewConfigurationCount, NULL);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[OvXR | ERROR] xrEnumerateViewConfigurations failed!" << std::endl;
		return false;
	}
	std::cout << "[OvXR | INFO] Runtime supports " << viewConfigurationCount << " view configurations" << std::endl;

	std::vector<XrViewConfigurationType> viewConfigurations = std::vector<XrViewConfigurationType>(viewConfigurationCount);
	xrEnumerateViewConfigurations(xrInstance, xrSys, viewConfigurationCount, &viewConfigurationCount, viewConfigurations.data());

	// check if runtime support stereo view configuration type to continue
	{
		bool stereoMode = false;
		XrViewConfigurationProperties stereoViewConfigProperties;
		for (unsigned int i = 0; i < viewConfigurationCount; ++i) {
			XrViewConfigurationProperties properties;
			properties.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;
			properties.next = NULL;
			res = xrGetViewConfigurationProperties(xrInstance, xrSys, viewConfigurations[i], &properties);
			if (!XR_SUCCEEDED(res))
			{
				std::cout << "[OvXR | ERROR] xrGetViewConfigurationProperties failed!" << std::endl;
				return false;
			}
			if (viewConfigurations[i] == stereoViewConfigType && properties.viewConfigurationType == stereoViewConfigType) {
				std::cout << "[OvXR | INFO] Runtime supports stereo view configuration" << std::endl;
				stereoMode = true;
				stereoViewConfigProperties = properties;
			}
		}

		if (!stereoMode) {
			std::cout << "[OvXR | Error] Runtime does not support stereo view configuration" << std::endl;
			return false;
		}

		// print stereo view configuration properties
		std::cout << "[OvXR | INFO] VR View Configuration:" << std::endl;
		std::cout << "\tview configuratio type: " << stereoViewConfigProperties.viewConfigurationType << std::endl; // stereo is 2
		// fov mutable indicates if the view field of view can be modified by the application
		std::cout << "\tFOV mutable           : " << (stereoViewConfigProperties.fovMutable ? "yes" : "no") << std::endl;
	}

	// retrieve the view capacity querying more details about the view configuration type
	res = xrEnumerateViewConfigurationViews(xrInstance, xrSys, stereoViewConfigType, 0, &viewCount, nullptr);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[OvXR | ERROR] xrEnumerateViewConfigurationViews failed!" << std::endl;
		return false;
	}
	// prepare the collection of view of our configuration views
	configurationViews = std::vector<XrViewConfigurationView>(viewCount);
	for (int i = 0; i < viewCount; i++) {
		configurationViews[i].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
		configurationViews[i].next = nullptr;
	}
	// allocate a view collection according to viewCount
	views = std::vector<XrView>(viewCount);
	for (int i = 0; i < viewCount; i++) {
		views[i].type = XR_TYPE_VIEW;
		views[i].next = nullptr;
	}

	// get properties related to rendering of an individual view within a view configuration...
	res = xrEnumerateViewConfigurationViews(xrInstance, xrSys, stereoViewConfigType, viewCount,
		&viewCount, configurationViews.data());
	if (!XR_SUCCEEDED(res))
	{
		return false;
	}

	// ...and let's print them
	std::cout << "[OvXR | INFO] View count: " << viewCount << std::endl;
	for (int i = 0; i < viewCount; i++) {
		std::cout << "\tView " << i << std::endl;
		std::cout << "\t\tResolution       : Recommended: "
			<< configurationViews[i].recommendedImageRectWidth << "x"
			<< configurationViews[i].recommendedImageRectHeight
			<< ", Max: "
			<< configurationViews[i].maxImageRectWidth << "x"
			<< configurationViews[i].maxImageRectHeight << std::endl;
		std::cout << "\t\tSwapchain Samples: Recommended: "
			<< configurationViews[i].recommendedSwapchainSampleCount << ", Max: "
			<< configurationViews[i].maxSwapchainSampleCount << std::endl;
	}

	//--------------------------------------
	//          SESSION[9]
	// perform the graphics API binding (OpenGL/DirectX) and check the result of the operation
	graphicsBinding = platformRenderer->getGraphicsBinding(xrInstance, xrSys);
	if (graphicsBinding == nullptr)
	{
		std::cout << "[OvXR | ERROR] Unable to get Platform Binding (OpenGL/DirectX)!" << std::endl;
		return false;
	}

	// prepare the session create info structure
	XrSessionCreateInfo sessionCreateInfo;
	sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO;
	sessionCreateInfo.systemId = xrSys; // link system
	sessionCreateInfo.next = graphicsBinding; // link to graphics API extension
    sessionCreateInfo.createFlags = 0;
	// create the session and check the result of the operation
	res = xrCreateSession(xrInstance, &sessionCreateInfo, &xrSession);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[OvXR | ERROR] Session creation failed!" << std::endl;
		return false;
	}
	std::cout << "[OvXR | INFO] Session created successfuly" << std::endl;
	

	//-------------------------------------
	//          BLEND MODES[10]
	// settings of the environment blend mode
	unsigned int blendCount = 0;
	// retrive the enviroment blend modes count and...
	res = xrEnumerateEnvironmentBlendModes(xrInstance, xrSys, stereoViewConfigType, 0, &blendCount, nullptr);
	std::vector<XrEnvironmentBlendMode> blendModes(blendCount);
	// ...enumerate them
	res = xrEnumerateEnvironmentBlendModes(xrInstance, xrSys, stereoViewConfigType, blendCount, &blendCount, blendModes.data());

	//Check if system supports opaque mode, if not return.
	//VR applications will generally choose the XR_ENVIRONMENT_BLEND_MODE_OPAQUE blend mode
	//The composition layers will be displayed with no view of the physical world behind them
	bool opaqueMode = false;
	for (XrEnvironmentBlendMode& bm : blendModes) {
		if (bm == XR_ENVIRONMENT_BLEND_MODE_OPAQUE)
			opaqueMode = true;
	}

	if (opaqueMode)
		std::cout << "[OvXR | INFO] XR_ENVIRONMENT_BLEND_MODE_OPAQUE supported" << std::endl;
	else {
		std::cout << "[OvXR | ERROR] XR_ENVIRONMENT_BLEND_MODE_OPAQUE not supported" << std::endl;
		return false;
	}

	//--------------------------------------
	//          SWAPCHAIN[10]
	{
		// retrive the number of Swapchain image format support by the runtime
		unsigned int swapchainFormatCount;
		res = xrEnumerateSwapchainFormats(xrSession, 0, &swapchainFormatCount, nullptr);
		if (!XR_SUCCEEDED(res))
		{
			std::cout << "[OvXR | ERROR] xrEnumerateSwapchainFormats failed!" << std::endl;
			return false;
		}
		std::cout << "[OvXR | INFO] Runtime supports " << swapchainFormatCount << " swapchain color formats:" << std::endl;
		int64_t *swapchainFormats = new int64_t[swapchainFormatCount];
		// retrive the Swapchain image format support by the runtime...
		res = xrEnumerateSwapchainFormats(xrSession, swapchainFormatCount, &swapchainFormatCount, swapchainFormats);
		if (!XR_SUCCEEDED(res))
		{
			std::cout << "[OvXR | ERROR] xrEnumerateSwapchainFormats failed!" << std::endl;
			return false;
		}

		//... and print them
		for (int i = 0; i < swapchainFormatCount; i++)
		{
			std::cout << "0x" << std::hex << swapchainFormats[i] << std::dec << ", ";
		}
		std::cout << std::endl;

		delete[] swapchainFormats;
	}
	// call to platform specific renderer for initialize swapchains
	if (!platformRenderer->initSwapchains(xrSession, configurationViews))
	{
		std::cout << "[OvXR | ERROR] Swapchains cration failed" << std::endl;
		return false;
	}

	// call to platform specific renderer for initialize resources
	if(!platformRenderer->initPlatformResources(
		configurationViews[0].recommendedImageRectWidth,
		configurationViews[0].recommendedImageRectHeight))
	{
		std::cout << "[OvXR | ERROR] Platform resources initialization failed" << std::endl;
		return false;
	}
	std::cout << "[OvXR | INFO] Swapchains created" << std::endl;

	return true;
}

bool OvXR::beginSession()
{
	// begin the session
	XrSessionBeginInfo sessionBeginInfo;
	sessionBeginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
	sessionBeginInfo.next = NULL;
	sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	XrResult res = xrBeginSession(xrSession, &sessionBeginInfo);
	std::cout << "[OvXR | INFO] Session started" << std::endl;

	// update field
	sessionRunning = XR_SUCCEEDED(res);
	// setting up the projection composition
	// projectionLayers struct reused for every frame
	projectionLayer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
	projectionLayer.next = nullptr;
	projectionLayer.layerFlags = 0;
	// the XrSpace in which the pose of each XrCompositionLayerProjectionView is evaluated over time by the compositor
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

	// session can be ended
	xrEndSession(xrSession);
	sessionRunning = false;

	std::cout << "[OvXR | INFO] Session ended" << std::endl;

	return true;
}

bool OvXR::beginFrame()
{
	// perform an event polling
	// initialize an event buffer to hold the event output
	XrEventDataBuffer runtimeEvent;
	runtimeEvent.type = XR_TYPE_EVENT_DATA_BUFFER;
	runtimeEvent.next = nullptr;

	XrFrameState frameState;
	frameState.type = XR_TYPE_FRAME_STATE;
	frameState.next = nullptr;

	XrFrameWaitInfo frameWaitInfo;
	frameWaitInfo.type = XR_TYPE_FRAME_WAIT_INFO;
	frameWaitInfo.next = nullptr;

	// Wait for a new frame
	XrResult res = xrWaitFrame(xrSession, &frameWaitInfo, &frameState);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[OvXR | ERROR] xrWaitFrame failed!" << std::endl;
		return false;
	}

	// Handle runtime Events
	// we do this right after xrWaitFrame() so we can go idle or
	// break out of the main render loop as early as possible into
	// the frame and don't have to uselessly render or submit one
	XrResult pollResult = xrPollEvent(xrInstance, &runtimeEvent);

	if (pollResult == XR_SUCCESS)
	{
		if (eventCallbacks[runtimeEvent.type]) // if event is registered...
			eventCallbacks[runtimeEvent.type](runtimeEvent); // ...invoke it
	}
	else if (pollResult == XR_EVENT_UNAVAILABLE) {
		// this is the usual case
	}
	else {
		std::cout << "[OvXR | ERROR] xrPollEvent failed!" << std::endl;
		return false;
	}

	// create projection matrices and view matrices for each eye
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
	// retrieve the viewer pose and projection parameters needed to render each view
	// for use in a composition projection layer
	res = xrLocateViews(xrSession, &viewLocateInfo, &viewState,
		viewCount, &viewCountOutput, views.data());
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[OvXR | ERROR] xrLocateViews failed!" << std::endl;
		return false;
	}

	XrFrameBeginInfo frameBeginInfo;
	frameBeginInfo.type = XR_TYPE_FRAME_BEGIN_INFO;
	frameBeginInfo.next = nullptr;
	// begin the frame
	res = xrBeginFrame(xrSession, &frameBeginInfo);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[OvXR | ERROR] xrBeginFrame failed!" << std::endl;
		return false;
	}
	// setting up component for the (immediately after frame is built) submission
	xrPredicedDisplayTime = frameState.predictedDisplayTime;
	projectionViews = std::vector<XrCompositionLayerProjectionView>(viewCount);
	return true;
}

bool OvXR::endFrame()
{
	// set the array of type XrCompositionLayerProjectionView containing each projection layer view
	projectionLayer.views = projectionViews.data();
	std::vector<XrCompositionLayerBaseHeader*> projectionlayers;
	projectionlayers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&projectionLayer));

	XrFrameEndInfo frameEndInfo;
	frameEndInfo.type					= XR_TYPE_FRAME_END_INFO;
	frameEndInfo.displayTime			= xrPredicedDisplayTime;
	frameEndInfo.layerCount				= (uint32_t)projectionlayers.size(); // the number of composition layers in this frame
	frameEndInfo.layers					= projectionlayers.data();
	frameEndInfo.environmentBlendMode	= XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	frameEndInfo.next = nullptr;
	// submit frame
	XrResult res = xrEndFrame(xrSession, &frameEndInfo);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[OvXR | ERROR] xrEndFrame failed!" << std::endl;
		return false;
	}
	return true;
}

bool OvXR::lockSwapchain(OvEye eye)
{
	// acquire the swapchain, delegate this to platform specific render
	XrSwapchain swapchain = platformRenderer->getSwapchain(eye);
	if (swapchain == XR_NULL_HANDLE) 
	{
		std::cout << "[OvXR | ERROR] PlafformRenderer returned an invalid XrSwapchain!" << std::endl;
		return false;
	}

	// acquire the swapchain image
	XrSwapchainImageAcquireInfo swapchainImageAcquireInfo;
	swapchainImageAcquireInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
	swapchainImageAcquireInfo.next = nullptr;
	// swapchain is the swapchain from which to acquire an image
	// textureIndex is a pointer to the image index that was acquired
	XrResult res = xrAcquireSwapchainImage(swapchain, &swapchainImageAcquireInfo, &textureIndex);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[OvXR | ERROR] xrAcquireSwapchainImage failed!" << std::endl;
		return false;
	}
	// Before an application can begin writing to a swapchain image, it must first wait on the
	// image to avoid writing to it before the compositor has finished reading from it.
	XrSwapchainImageWaitInfo swapchainImageWaitInfo;
	swapchainImageWaitInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
	swapchainImageWaitInfo.next = nullptr;
	// how many nanoseconds the call should block waiting for the image to become available for writing
	swapchainImageWaitInfo.timeout = XR_INFINITE_DURATION; 
	// wait for the image
	res = xrWaitSwapchainImage(swapchain, &swapchainImageWaitInfo);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[OvXR | ERROR] Failed to wait for swapchain image!" << std::endl;
		return false;
	}

	// setting up the projection composition layer
	projectionViews[eye].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
	projectionViews[eye].next = nullptr;
	projectionViews[eye].pose = views[eye].pose; // location and orientation of this projection element
	projectionViews[eye].fov = views[eye].fov; // fov for this projection element
	projectionViews[eye].subImage.swapchain = swapchain; // the image layer to use
	projectionViews[eye].subImage.imageArrayIndex = 0;
	projectionViews[eye].subImage.imageRect.offset.x = 0;
	projectionViews[eye].subImage.imageRect.offset.y = 0;
	projectionViews[eye].subImage.imageRect.extent.width = configurationViews[eye].recommendedImageRectWidth;
	projectionViews[eye].subImage.imageRect.extent.height = configurationViews[eye].recommendedImageRectHeight;

	// prepares platform-specific render
	if (!platformRenderer->beginEyeFrame(eye, textureIndex))
	{
		std::cout << "[OvXR | ERROR] Plafform-specific render setup failed!" << std::endl;
		return false;
	}
	return true;
}

bool OvXR::unlockSwapchain(OvEye eye)
{
	// completes platform-specific render...
	if(!platformRenderer->endEyeFrame(eye, textureIndex))
	{
		std::cout << "[OvXR | ERROR] Plafform-specific render finalization failed!" << std::endl;
		return false;
	}

	XrSwapchain swapchain = platformRenderer->getSwapchain(eye);

	XrSwapchainImageReleaseInfo swapchainImageReleaseInfo;
	swapchainImageReleaseInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
	swapchainImageReleaseInfo.next = nullptr;
	// ... and finally release the swapchain image
	XrResult res = xrReleaseSwapchainImage(swapchain, &swapchainImageReleaseInfo);
	if (!XR_SUCCEEDED(res))
	{
		std::cout << "[OvXR | ERROR] Failed to release swapchain image!" << std::endl;
		return false;
	}

	return true;
}

bool OvXR::free()
{
	if (sessionRunning)
		return false;

	//free platfomr resources
	platformRenderer->free();

	// destroy space
	if (xrSpace != XR_NULL_HANDLE)
		xrDestroySpace(xrSpace);
	// destroy session
	if (xrSession != XR_NULL_HANDLE)
		xrDestroySession(xrSession);
	// destroy instance
	if (xrInstance != XR_NULL_HANDLE)
		xrDestroyInstance(xrInstance);

	if (graphicsBinding)
		delete graphicsBinding;

	return true;
}


bool OvXR::setReferenceSpace(XrPosef pose, XrReferenceSpaceType spaceType)
{
	XrReferenceSpaceCreateInfo localSpaceCreateInfo;
	localSpaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
	localSpaceCreateInfo.next = NULL;
	localSpaceCreateInfo.referenceSpaceType = spaceType;
	localSpaceCreateInfo.poseInReferenceSpace = pose;
	// Sets reference space of application
	XrResult result = xrCreateReferenceSpace(xrSession, &localSpaceCreateInfo, &xrSpace);
	return XR_SUCCEEDED(result);
}

std::string OvXR::getRuntimeName()
{
	XrResult res;
	XrInstanceProperties instanceProperties;
	instanceProperties.type = XR_TYPE_INSTANCE_PROPERTIES;
	instanceProperties.next = NULL;
	res = xrGetInstanceProperties(xrInstance, &instanceProperties);
	std::stringstream ss;
	ss << instanceProperties.runtimeName << " v"
		<< XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << "."
		<< XR_VERSION_MINOR(instanceProperties.runtimeVersion) << "."
		<< XR_VERSION_PATCH(instanceProperties.runtimeVersion);
	// return runtime name
	return ss.str();
}

void OvXR::setCallback(XrStructureType event, std::function<void(XrEventDataBuffer)> callback)
{
	// set event caalback
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
	// return manufactuter name
	return systemProperties.systemName;
}

unsigned int OvXR::getHmdIdealHorizRes()
{
	if (configurationViews.size() == 0) {
		return 0;
	}

	// return ideal horizontal resolution
	return configurationViews[0].recommendedImageRectWidth;
}

unsigned int OvXR::getHmdIdealVertRes()
{
	if (configurationViews.size() == 0) {
		return 0;
	}

	// return ideal vertical resolution
	return configurationViews[0].recommendedImageRectHeight;
}

glm::mat4 OvXR::getProjMatrix(OvEye eye, float nearPlane, float farPlane)
{
	// return the projection matrix for the given eye and plane params
	return glm::perspective(views[eye].fov.angleUp - views[eye].fov.angleDown,
		(float)getHmdIdealHorizRes() / (float)getHmdIdealVertRes(),
		nearPlane, farPlane);
}

glm::mat4 OvXR::getEyeModelviewMatrix(OvEye eye, const glm::mat4 &baseMat)
{
	// rotation
	glm::mat4 rot = glm::mat4{
	glm::quat{ views[eye].pose.orientation.w,
		 views[eye].pose.orientation.x,
		 views[eye].pose.orientation.y,
		 views[eye].pose.orientation.z
		}
	};
	// translation
	glm::mat4 trans = glm::translate(glm::mat4{ 1.f },
		glm::vec3{
		views[eye].pose.position.x,
		views[eye].pose.position.y,
		views[eye].pose.position.z,
		}
	);

	glm::mat4 viewMatrix = trans * rot;
	glm::mat4 inverseViewMatrix = glm::inverse(viewMatrix * baseMat);
	// Get the eye-to-head modelview matrix for the given eye
	return inverseViewMatrix;
}

XrInstance OvXR::getInstane()
{
	// return instance
	return xrInstance;
}

XrSession OvXR::getSession()
{
	// return session
	return xrSession;
}

XrSystemId OvXR::getSystem()
{
	// return system ID
	return xrSys;
}

void OvXR::setPlatformRenderer(PlatformRenderer * ext)
{
	if (platformRenderer != nullptr)
		delete platformRenderer;

	platformRenderer = ext;
}
