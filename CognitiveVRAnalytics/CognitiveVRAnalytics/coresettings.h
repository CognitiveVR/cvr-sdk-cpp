
/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include "stdafx.h"
#include <map>

//container class to ensure all settings are correct when CognitiveVRAnalyticsCore is constructed
namespace cognitive {
class CognitiveVRAnalyticsCore;

enum class ECognitiveHMDType
{
	kUnknown,
	kVive,
	kRift,
	kGear,
	kMobile
};

typedef void(*WebResponse) (::std::string content);
typedef void(*WebRequest) (::std::string url, ::std::string content, WebResponse response);

class CoreSettings {

	private:
		nlohmann::json UserProperties = nlohmann::json();
		nlohmann::json DeviceProperties = nlohmann::json();

    public:
		CoreSettings()
		{

		}

		LoggingLevel loggingLevel;

        //Network call timeout, measured in seconds. Ex. 5
		long kNetworkTimeout = 5;

		//which product to send data to. Ex companyname1234-productname-test
		::std::string CustomerId = "companyname1234-productname-test";

		int SensorDataLimit = 64;
		int DynamicDataLimit = 64;
		int TransactionBatchSize = 64;
		int GazeBatchSize = 64;

		float GazeInterval = 0.1f;

		ECognitiveHMDType HMDType = ECognitiveHMDType::kUnknown;
		::std::string GetHMDType()
		{
			if (HMDType == ECognitiveHMDType::kUnknown) { return "unknown"; }
			if (HMDType == ECognitiveHMDType::kGear) { return "gear"; }
			if (HMDType == ECognitiveHMDType::kMobile) { return "mobile"; }
			if (HMDType == ECognitiveHMDType::kVive) { return "vive"; }
			if (HMDType == ECognitiveHMDType::kRift) { return "rift"; }
			return "unknown";
		}

		::std::map < ::std::string, ::std::string> sceneIds;// = ::std::map<::std::string, ::std::string>();
		::std::string DefaultSceneName = "";

		WebRequest webRequest;

		//TODO if user and/or device has anything set, set this after constructor
		//::std::string UserId = "";
		//should be able to set user properties here

		//::std::string DeviceId = "";
		//should be able to set device properties here
};
}