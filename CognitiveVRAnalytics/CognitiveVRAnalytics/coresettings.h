/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

//Temporary container to hold values for constructing Cognitive Analytics

#pragma once

#include "stdafx.h"
#include <map>

namespace cognitive
{
class CognitiveVRAnalyticsCore;

enum class ECognitiveHMDType
{
	kUnknown,
	kVive,
	kRift,
	kGear,
	kMobile
};

class CoreSettings
{
    public:
		CoreSettings()
		{

		}

		//pointer to a function that allows this SDK to make web requests and receive web responses
		WebRequest webRequest = nullptr;

		//which logs to display. default is all
		LoggingLevel loggingLevel = LoggingLevel::kAll;

		//which product to send data to. Ex companyname1234-productname-test
		::std::string APIKey = "asdf1234hjkl5678";

		::std::string CustomGateway;

		//how many sensor data points to batch before sending to SceneExplorer
		int SensorDataLimit = 64;
		//how many dynamic objects registered and snapshotted to batch before sending to SceneExplorer
		int DynamicDataLimit = 64;
		//how many custom events to batch before sending to SceneExplorer
		int CustomEventBatchSize = 64;
		//how many gaze data points to batch before sending to SceneExplorer
		int GazeBatchSize = 64;

		//the expected interval for gaze snapshots. used by SceneExplorer's timeline
		float GazeInterval = 0.1f;

		//the type of HMD the user has. used by SceneExplorer to display the correct player mesh
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

		//all 'scenes' the user might encounter. must contain at least one to use SceneExplorer
		std::vector<SceneData> AllSceneData;

		//::std::map < ::std::string, ::std::string> sceneIds;
		//the default scene to load after Cognitive Analytics constructor
		::std::string DefaultSceneName = "";
};
}