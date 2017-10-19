/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

//Record hmd position, direction of gaze, point of gaze and (it applicable) the object that was gazed at

#pragma once

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"


#if defined(_MSC_VER)
//  Microsoft 
#ifdef COGNITIVEVRANALYTICS_EXPORTS  
#define COGNITIVEVRANALYTICS_API __declspec(dllexport)
#else  
#define COGNITIVEVRANALYTICS_API __declspec(dllimport)
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

class COGNITIVEVRANALYTICS_API GazeTracker
{
private:
	::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;
	int jsonPart = 1;
	
	//set from config
	float PlayerSnapshotInterval = 0.1f;
	//set from config
	::std::string HMDType = "";

public:

	nlohmann::json BatchedGazeSE = nlohmann::json();

	GazeTracker(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);
	void SetInterval(float interval);
	void SetHMDType(::std::string hmdtype);

	/** Record HMD position, rotation, gaze and object hit

		@param std::vector<float> position
		@param std::vector<float> rotation
		@param std::vector<float> gazepoint - Optional
		@param std::vector<float> objectid - Opitonal
	*/
	void RecordGaze(::std::vector<float> &position, ::std::vector<float> &rotation, ::std::vector<float> &gazepoint, int objectid = -1);
	void RecordGaze(::std::vector<float> &position, ::std::vector<float> &rotation);

	void SendData();
	//clear gaze points
	void EndSession();
};
}