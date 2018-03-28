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
class CognitiveVRAnalyticsCore;

class COGNITIVEVRANALYTICS_API CustomEvent
{
    private:
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;

		int jsonPart = 1;

    public:
		CustomEvent(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);

		//the array of objects {"name":"whatever","time":100,"point":[0,1,2]} to send to dashboard
		nlohmann::json BatchedCustomEvents = nlohmann::json();

        /** Begin a new transaction.

			@param std::string category
			@param std::vector<float> position - Optional
            @param nlohmann::json properties - Optional
			@param std::string transaction_id - Optional
        */
		void Send(::std::string category, ::std::vector<float> &Position);
		void Send(::std::string category, ::std::vector<float> &Position, nlohmann::json properties);


		void SendData();
		
		//clear saved transactions
		void EndSession();
};
}