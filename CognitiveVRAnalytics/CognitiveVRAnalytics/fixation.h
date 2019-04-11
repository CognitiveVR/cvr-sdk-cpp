/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

//Record beginning and ending of events
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
	class COGNITIVEVRANALYTICS_API Fixation
	{
		friend class CognitiveVRAnalyticsCore;

	private:
		std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;
		int jsonPart = 1;

		Fixation(std::shared_ptr<CognitiveVRAnalyticsCore> cog);

		nlohmann::json BatchedFixations = nlohmann::json();
		//send and clear saved events
		void EndSession();

	public:


		/** Record a new Fixation

		@param double Time - start time of fixation
		@param int DurationMs - length of fixation in number of milliseconds
		@param float MaxRadius - maximum radius of the fixation based on distance and eye fixation angle
		@param std::string ObjectId - if fixation is on a dynamic object
		@param std::vector<float> Position - local position of fixation relative to dynamic object
		*/

		void RecordFixation(double Time, int DurationMs, float MaxRadius, std::string ObjectId, std::vector<float> &LocalPosition);

		/** Record a new Fixation

		@param double Time - start time of fixation
		@param int DurationMs - length of fixation in number of milliseconds
		@param float MaxRadius - maximum radius of the fixation based on distance and eye fixation angle
		@param std::vector<float> Position - world position of fixation
		*/

		void RecordFixation(double Time, int DurationMs, float MaxRadius, std::vector<float> &WorldPosition);

		nlohmann::json SendData();
	};
}