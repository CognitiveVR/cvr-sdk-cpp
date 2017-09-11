/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"
using json = nlohmann::json;

#ifdef COGNITIVEVRANALYTICS_EXPORTS  
#define COGNITIVEVRANALYTICS_API __declspec(dllexport)   
#else  
#define COGNITIVEVRANALYTICS_API __declspec(dllimport)
#endif

class CognitiveVRAnalyticsCore;

class COGNITIVEVRANALYTICS_API Sensor
{
    private:
		CognitiveVRAnalyticsCore* cvr;

		//std::map<std::string, std::string> somedatapoints;
		
		//std::string SensorDataToString();
		//int jsonPart = 0;
		//int sensorDataCount = 0;
		//int SensorThreshold = 16;

		std::map<std::string, json> allsensors;
		int sensorCount = 0;
		int jsonPart = 1;

    public:
		Sensor(CognitiveVRAnalyticsCore* cog);
		void RecordSensor(std::string Name, float value);
		void SendData();
        
};