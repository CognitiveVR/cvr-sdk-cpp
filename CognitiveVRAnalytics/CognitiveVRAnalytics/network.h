
/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

//#include <stdexcept>
#include "stdafx.h"
#include "CognitiveVRAnalytics.h"

#ifdef COGNITIVEVRANALYTICS_EXPORTS  
#define COGNITIVEVRANALYTICS_API __declspec(dllexport)   
#else  
#define COGNITIVEVRANALYTICS_API __declspec(dllimport)
#endif
namespace cognitive {
class CognitiveVRAnalyticsCore;

//typedef void(*WebResponse) (std::string content);
//typedef void(*WebRequest) (std::string url, std::string content, WebResponse response);

class Network
{
    private:
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = NULL;

    public:
        Network(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);
        

        /** Make a call to the CognitiveVR API.

            @param std::string sub_path - The path for the API call.
            @param Json::Value content - JSON content sent to CognitiveVR.
			@param NetworkCallback callback - JSON content sent to CognitiveVR.
        */
		
		void DashboardCall(::std::string suburl, ::std::string content);

		//TODO use an enum to specify the type of call + bind to correct callback function
		void APICall(::std::string suburl, ::std::string callType, ::std::string content = "");

		//sub url is dynamic/sensor/gaze/event. for sending stuff to sceneexplorer
		void SceneExplorerCall(::std::string suburl, ::std::string content);
};
}