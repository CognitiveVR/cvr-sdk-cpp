
/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

//#include <stdexcept>
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
namespace cognitive {
class CognitiveVRAnalyticsCore;

//typedef void(*WebResponse) (std::string content);
//typedef void(*WebRequest) (std::string url, std::string content, WebResponse response);

class Network
{
    private:
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;

    public:
        Network(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);
        

        /** Make a call to the CognitiveVR API.

            @param std::string sub_path - The path for the API call.
            @param Json::Value content - JSON content sent to CognitiveVR.
			@param NetworkCallback callback - JSON content sent to CognitiveVR.
        */
		
		void DashboardCall(::std::string suburl, ::std::string content);

		//TODO use an enum to specify the type of call + bind to correct callback function instead of string content
		void APICall(::std::string suburl, ::std::string callType, ::std::string content = "");

		//sub url is dynamic/sensor/gaze/event. for sending stuff to sceneexplorer
		//returns true if successfully called. false if scene not set or webrequest not set
		bool SceneExplorerCall(::std::string suburl, ::std::string content);
};
}