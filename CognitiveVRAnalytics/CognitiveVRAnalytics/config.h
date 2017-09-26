
/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include <string>
#include <map>

//Configuration defaults.
//std::string Config::kNetworkHost = "https://data.cognitivevr.io";
//std::string Config::kSsfApp = "isos-personalization";
//std::string Config::kSsfVersion = "4";
//std::string Config::kSsfOutput = "json";
//long Config::kTuningCacheTtl = 900000;
//long Config::kNetworkTimeout = 5;
namespace cognitive {
class CognitiveVRAnalyticsCore;

class Config {

	private:
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = NULL;

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
		::std::string CustomerId = "companyname1234-productname-test";

		int SensorDataLimit = 64;
		int DynamicDataLimit = 64;
		int TransactionBatchSize = 64;
		int GazeBatchSize = 64;

		float GazeInterval = 0.1f;
		//vive, rift, gear, mobile
		::std::string HMDType = "vive";

		::std::map < ::std::string, ::std::string> sceneIds = ::std::map<::std::string, ::std::string>();


		#pragma warning(pop)
};
}