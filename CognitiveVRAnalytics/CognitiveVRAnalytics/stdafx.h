// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
//#include <windows.h>


// reference additional headers your program requires here
#include <string>
#include <memory>
#include <array>
#include "json.hpp"

namespace cognitive
{
	enum class LoggingLevel
	{
		kAll = 0,
		kErrorsOnly = -1,
		kNone = -2
	};
}