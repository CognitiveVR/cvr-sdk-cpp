/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

//Used internally. Save values from CoreSettings when Cognitive Analytics is constructed

#pragma once

#include <string>
#include <map>

namespace cognitive
{
class CognitiveVRAnalyticsCore;

class Config {

	private:
		std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;

    public:
		#pragma warning(push)
		#pragma warning(disable:4251) //Disable DLL warning that does not apply in this context.

		Config(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
		{
			cvr = cog;
		}

		std::string SdkVersion = "1.0.1";

		std::string kNetworkHost = "data.cognitive3d.com";

		//added to the header to authenticate requests
		std::string APIKey = "asdf1234jhkl5678";

		std::string networkVersion = "0";

		int SensorDataLimit = 64;
		int DynamicDataLimit = 64;
		int CustomEventBatchSize = 64;
		int GazeBatchSize = 64;

		//only holds the value between core construction and session begin. use cvr.gaze.PlayerSnapshotInterval
		float GazeInterval = 0.1f;
		std::string HMDType = "";

		std::vector<SceneData> AllSceneData;
		std::string DynamicObjectFileType = "obj";

		#pragma warning(pop)
};
}