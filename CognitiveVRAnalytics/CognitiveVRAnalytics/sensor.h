
/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"


#ifdef COGNITIVEVRANALYTICS_EXPORTS  
#define COGNITIVEVRANALYTICS_API __declspec(dllexport)   
#else  
#define COGNITIVEVRANALYTICS_API __declspec(dllimport)
#endif
namespace cognitive {
	//using json = nlohmann::json;
class CognitiveVRAnalyticsCore;

class COGNITIVEVRANALYTICS_API Sensor
{
    private:
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = NULL;

		//std::map<std::string, std::string> somedatapoints;
		
		//std::string SensorDataToString();
		//int jsonPart = 0;
		//int sensorDataCount = 0;
		//int SensorThreshold = 16;

		::std::map<::std::string, nlohmann::json> allsensors = ::std::map<::std::string, nlohmann::json>();
		int sensorCount = 0;
		int jsonPart = 1;

    public:
		Sensor(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);
		void RecordSensor(::std::string Name, float value);
		void SendData();
        
};
}