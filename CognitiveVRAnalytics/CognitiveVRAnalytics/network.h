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

class Network
{
    private:
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;
		::std::string query = "";

    public:
        Network(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);
		
		//send batched transactions to dashboard
		void DashboardCall(::std::string suburl, ::std::string content);

		//exitpoll calls to get question sets and send answers
		void APICall(::std::string suburl, ::std::string callType, ::std::string content = "");

		//sub url is dynamic/sensor/gaze/event. for sending stuff to sceneexplorer
		//returns true if successfully called. false if scene not set or webrequest not set
		bool SceneExplorerCall(::std::string suburl, ::std::string content);
};
}