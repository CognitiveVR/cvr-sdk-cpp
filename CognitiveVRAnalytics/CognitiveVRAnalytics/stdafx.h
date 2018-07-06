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

	typedef void(*WebResponse) (std::string content);
	typedef void(*WebRequest) (std::string url, std::string content, std::vector<std::string> headers, WebResponse response);

	class SceneData
	{
	public:
		std::string SceneName;
		std::string SceneId;
		std::string VersionNumber;
		int VersionId;

		SceneData(std::string sceneName, std::string sceneId, std::string versionNumber, int versionId)
		{
			SceneName = sceneName;
			SceneId = sceneId;
			VersionNumber = versionNumber;
			VersionId = versionId;
		}
	};
}