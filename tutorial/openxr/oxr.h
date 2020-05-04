/**
 * @file		ovr.h
 * @brief	Self-contained helper class for interfacing OpenGL and OpenVR. Shortened version of Overvision's OvXR module.
 *
 * @author	Achille Peternier (C) SUPSI [achille.peternier@supsi.ch]
 */
#pragma once


//////////////
// #INCLUDE //
//////////////

// GLEW:
#include <GL/glew.h>

// FreeGLUT:
#include <GL/freeglut.h>

// GLM:
#include <glm/glm.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/type_ptr.hpp>


// C/C++:
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <functional>

#include "PlatformRenderer.h"
#include <openxr/openxr.h>


////////////////
// CLASS OvXR //
////////////////

/**
 * @brief OpenGL-OpenXR interface.
 */
class OvXR
{
public: 
	
    /**
     * Eye enums
     */
    enum OvEye : int
    {
        EYE_LEFT = 0,
        EYE_RIGHT = 1,

        // Terminator:
        EYE_LAST
    };


    /**
	* Constructor OvXR class
	* @param app_name: name of the application
	*/
	OvXR(const std::string & app_name = "OpenXR App");


	/**
	 * Destructor OvXR class
	 */
	~OvXR();


	/**
	 * @brief Init VR components.
	 * @return TF
	 */
	bool init();


	/**
	 * @brief Sets reference space of application
	 * @param pose: reference pose
	 * @param spaceType: reference space type (eg local)
	 * @return result of xrCreateReferenceSpace == XR_SUCCESS
	 */
	bool setReferenceSpace(XrPosef pose, XrReferenceSpaceType spaceType);


	/**
	 * @brief begin OpenXR session.
	 * @return TF
	 */
	bool beginSession();
   

	/**
	 * @brief Called prior to the start of frame rendering.
	 * updates view locations and polls events
	 */
	void beginFrame();


	/**
	 * @brief Acquire the image from the swapchain before graphics API strats rendering.
	 * @param eye left or right eye
	 */
	void lockSwapchain(OvEye eye);


	/**
	 * @brief Releases the swapchain image after that the application is done rendering.
	 * @param eye left or right eye
	 */
	void unlockSwapchain(OvEye eye);


	/**
	 * @brief Performs frame submission to the HMD
	 */
	void endFrame();


	/**
	 * @brief Ends OpenXR render session.
	 * @return TF
	 */
	bool endSession();


    /**
     * Release VR components.
     * @return TF
     */
	bool free();


    /**
      * @return Runtime's name (WMR/Monado)
      */
	std::string getRuntimeName();


    /**
     * @brief Set handle of event
     * @param event: event type
     * @param callback: handle of event
     */
	void setCallback(XrStructureType event, std::function<void(XrEventDataBuffer)> callback);


    /**
     * @brief Get manufacturer name.
     * @return manufacturer system name
     */
	std::string getManufacturerName();


    /**
     * Get HMD proper horizontal resolution in pixels.
     * @return HMD horizontal resolution in pixels
     */
	unsigned int getHmdIdealHorizRes();


    /**
     * Get HMD proper vertical resolution in pixels.
     * @return HMD vertical resolution in pixels
     */
	unsigned int getHmdIdealVertRes();

    /**
     * Get the projection matrix for the given eye and plane params.
     * @param eye left or right eye (use enum)
     * @param nearPlane user camera near plane distance
     * @param farPlane user camera far plane distance
     * @return projection matrix ready for OpenGL
     */
	glm::mat4 getProjMatrix(OvEye eye, float nearPlane, float farPlane);


    /**
     * Get the eye modelview matrix for the given eye.
     * @param eye left or right eye (use enum)
	 * @param baseMat enables support for moving user position with keyboard and mouse
     * @return eye-to-head modelview matrix ready for OpenGL
     */
	glm::mat4 getEyeModelviewMatrix(OvEye eye, const glm::mat4 &baseMat = glm::mat4{ 1.f });


    /**
     * @return return OpenXR Instance
     */
	XrInstance getInstane();


    /**
     * @return return OpenXR Session
     */
	XrSession getSession();


    /**
     * @return return OpenXR System referred by ID
     */
	XrSystemId getSystem();

private:
	// Session running flag
	bool sessionRunning;
	// Application name
    std::string appName;
	// Platform-specific rendering implementation reference and graphics binding structure used during session creation
	PlatformRenderer *platformRenderer;
	void* graphicsBinding;
   
	// OpenXR components
    XrInstance xrInstance;
    XrSystemId xrSys;
    XrSession xrSession;

	// Reference Space
    XrSpace xrSpace;

	// Views containing view pose view pose and projection state
    std::vector<XrView>views;
    unsigned int viewCount;
	
	// Properties related to rendering of an individual view within a view configuration 
    std::vector<XrViewConfigurationView> configurationViews;
    
	// Frame submission pack
	std::vector<XrCompositionLayerProjectionView> projectionViews;
	// Planar projected images rendered from the eye point of each eye using a standard perspective projection.
	XrCompositionLayerProjection projectionLayer;
    XrTime xrPredicedDisplayTime;
	unsigned int textureIndex;
	
	// Collection of XrEvents callbacks
    std::map<XrStructureType, std::function<void(XrEventDataBuffer)>> eventCallbacks;
};
