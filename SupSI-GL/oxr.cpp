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
{

#ifdef _WINDOWS
	platformRenderer = new DirectXRenderer();
#else
    platformRenderer = new OpenGLRenderer();
#endif // _WINDOWS
}

