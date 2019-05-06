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
class COGNITIVEVRANALYTICS_API GazeTracker
{
	friend class CognitiveVRAnalyticsCore;

private:
	std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;
	int jsonPart = 1;

	double nextSendTimestamp = 0;
	double nextWarningTimestamp = 0;

	nlohmann::json BatchedGaze = nlohmann::json();

	GazeTracker(std::shared_ptr<CognitiveVRAnalyticsCore> cog);

	//clear gaze points
	void EndSession();

public:
	/** Record HMD position, rotation and gaze

		@param std::vector<float> hmdposition
		@param std::vector<float> hmdrotation
		@param std::vector<float> gazepoint - world position of gaze
	*/
	void RecordGaze(std::vector<float> &hmdposition, std::vector<float> &hmdrotation, std::vector<float> &gazepoint);

	/** Record HMD position, rotation, gaze and object hit

	@param std::vector<float> hmd position
	@param std::vector<float> hmd rotation
	@param std::vector<float> localgazepoint - position local to dynamic object looked at
	@param std::string objectid - object id of hit dynamic object
	*/
	void RecordGaze(std::vector<float> &hmdposition, std::vector<float> &hmdrotation, std::vector<float> &localgazepoint, std::string objectid);

	/** Record HMD position, rotation, gaze and object hit

	@param std::vector<float> hmd position
	@param std::vector<float> hmd rotation
	@param std::vector<float> localgazepoint - position local to dynamic object looked at
	@param std::string objectid - object id of hit dynamic object
	@param std::string mediaId - identifier for media as uploaded on the cognitive dashboard
	@param long mediaTime - time of video media in Milliseconds. 0 if image
	@param std::vector<float> uvs -u and v coordinate of the texture on the hit triange
	*/
	void RecordGaze(std::vector<float> &hmdposition, std::vector<float> &hmdrotation, std::vector<float> &localgazepoint, std::string objectid, std::string mediaId, long mediaTimeMs, std::vector<float> &uvs);

	/** Record HMD position and rotation. used when looking at sky or something without a distinct surface

	@param std::vector<float> hmd position
	@param std::vector<float> hmd rotation
	*/
	void RecordGaze(std::vector<float> &hmdposition, std::vector<float> &hmdrotation);

	//post any gaze points and any updated user/device properties
	nlohmann::json SendData();
};
}