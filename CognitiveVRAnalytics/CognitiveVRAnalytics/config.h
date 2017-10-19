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
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;

    public:
		#pragma warning(push)
		#pragma warning(disable:4251) //Disable DLL warning that does not apply in this context.

		Config(::std::shared_ptr<CognitiveVRAnalyticsCore> cog)
		{
			cvr = cog;
		}

		::std::string SdkVersion = "0.1";

        //CognitiveVR API host. -- Only enterprise customers need this. Ex: https://data.cognitivevr.io
		::std::string kNetworkHost = "https://data.cognitivevr.io";
		::std::string kNetworkHostAPI = "https://api.cognitivevr.io";
		::std::string kSceneExplorerAPI = "https://sceneexplorer.com/api/";

        //SSF app. Ex: isos-personalization
		::std::string kSsfApp = "isos-personalization";

        //SSF Version. Ex: 4
		::std::string kSsfVersion = "4";

        //SSF Output. Ex: json
		::std::string kSsfOutput = "json";

        //Tuning variable cache time to live, measured in milliseconds. Ex. 900 000 - 15 Minutes
        long kTuningCacheTtl = 900000;

        //Network call timeout, measured in seconds. Ex. 5
		long kNetworkTimeout = 5;

		//which product to send data to. Ex companyname1234-productname-test
		::std::string CustomerId = "altimagegames59340-unitywanderdemo-test";

		int SensorDataLimit = 64;
		int DynamicDataLimit = 64;
		int TransactionBatchSize = 64;
		int GazeBatchSize = 64;

		//only holds the value between core construction and session begin. use cvr.gaze.PlayerSnapshotInterval
		float GazeInterval = 0.1f;
		::std::string HMDType = "";

		::std::map < ::std::string, ::std::string> sceneIds;

		#pragma warning(pop)
};
}