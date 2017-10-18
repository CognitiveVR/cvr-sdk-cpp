
/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
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
namespace cognitive {
	//using json = nlohmann::json;
class CognitiveVRAnalyticsCore;

class COGNITIVEVRANALYTICS_API Sensor
{
    private:
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;

		//std::map<std::string, std::string> somedatapoints;
		
		//std::string SensorDataToString();
		//int jsonPart = 0;
		//int sensorDataCount = 0;
		//int SensorThreshold = 16;

		::std::map<::std::string, nlohmann::json> allsensors = ::std::map<::std::string, nlohmann::json>();
		int jsonPart = 1;

    public:
		int sensorCount = 0;

		Sensor(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);
		void RecordSensor(::std::string Name, float value);
		void SendData();
		void EndSession();
};
}