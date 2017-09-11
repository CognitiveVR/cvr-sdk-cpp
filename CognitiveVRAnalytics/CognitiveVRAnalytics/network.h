/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

//#include <stdexcept>
#include "stdafx.h"
#include "CognitiveVRAnalytics.h"

class CognitiveVRAnalyticsCore;

//typedef void(*WebResponse) (std::string content);
//typedef void(*WebRequest) (std::string url, std::string content, WebResponse response);

class Network
{
    private:
		CognitiveVRAnalyticsCore* cvr;

    public:
        Network(CognitiveVRAnalyticsCore* cog);
        

        /** Make a call to the CognitiveVR API.

            @param std::string sub_path - The path for the API call.
            @param Json::Value content - JSON content sent to CognitiveVR.
			@param NetworkCallback callback - JSON content sent to CognitiveVR.
        */
		//void Call(std::string path, std::shared_ptr<json> content, NetworkCallback callback = NULL);
		void DashboardCall(std::string suburl, std::string content);

		///sub url is dynamic/sensor/gaze/event
		void SceneExplorerCall(std::string suburl, std::string content);

		//void Callback(std::string body);
};