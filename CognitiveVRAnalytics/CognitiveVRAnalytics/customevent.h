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
class COGNITIVEVRANALYTICS_API CustomEvent
{
	friend class CognitiveVRAnalyticsCore;

    private:
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;
		int jsonPart = 1;

		CustomEvent(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);

		nlohmann::json BatchedCustomEvents = nlohmann::json();
		//send and clear saved events
		void EndSession();

    public:


		/** Record a new custom event

		@param std::string category
		@param std::vector<float> position of event
		@param nlohmann::json properties - Optional
		*/
		void Send(::std::string category, ::std::vector<float> &Position);
		void Send(::std::string category, ::std::vector<float> &Position, nlohmann::json properties);

		nlohmann::json SendData();
};
}