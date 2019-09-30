/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

//Constructs urls and contents of web requests and parses responses

#pragma once

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"

#if defined(_MSC_VER)
//  Microsoft 
#ifdef COGNITIVEVRANALYTICS_EXPORTS  
#define COGNITIVEVRANALYTICS_API __declspec(dllexport)
#else  
#define COGNITIVEVRANALYTICS_API
#endif
#elif defined(__GNUC__)
//  GCC
#ifdef COGNITIVEVRANALYTICS_EXPORTS  
#define COGNITIVEVRANALYTICS_API __attribute__((visibility("default")))
#else  
#define COGNITIVEVRANALYTICS_API
#endif
#else
//  do nothing and hope for the best?
#define COGNITIVEVRANALYTICS_EXPORTS
#pragma warning Unknown dynamic link import/export semantics.
#endif
namespace cognitive
{
class CognitiveVRAnalyticsCore;
class GazeTracker;
class CustomEvent;
class Sensor;
class Fixation;
class DynamicObject;

class Network
{
	friend class GazeTracker;
	friend class CustomEvent;
	friend class Sensor;
	friend class Fixation;
	friend class DynamicObject;

    private:
		std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;
		std::vector<std::string> headers;

		//used for posting gaze/events/dynamics/sensors to dashboard and scene explorer
		void NetworkCall(std::string suburl, std::string content);

    public:
        Network(std::shared_ptr<CognitiveVRAnalyticsCore> cog);

		//used by exitpolls
		void NetworkExitpollGet(std::string hook);
		//used by exitpolls
		void NetworkExitpollPost(std::string questionsetname, std::string questionsetversion, std::string content);
};
}