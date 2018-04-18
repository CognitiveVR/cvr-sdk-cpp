/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

//Record sensors and values of various types over periods of time

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

class COGNITIVEVRANALYTICS_API Sensor
{
    private:
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;

		::std::map<::std::string, nlohmann::json> allsensors;
		int jsonPart = 1;
    public:

		//TODO should be private. tests shouldn't directly test this value
		int sensorCount = 0;
		Sensor(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);

		/** Record data for a sensor

			@param std::string name
			@param float value
		*/
		void RecordSensor(::std::string name, float value);
		void SendData();

		//clear all saved sensor data
		void EndSession();
};
}