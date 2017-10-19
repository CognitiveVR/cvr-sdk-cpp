/*
    Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

//Holds values used for experiments and updating the application without creating a new build

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

enum class EntityType
{
	kEntityTypeUser,
	kEntityTypeDevice
};

class COGNITIVEVRANALYTICS_API Tuning
{

    private:
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;

		nlohmann::json user_value_cache = nlohmann::json();
		nlohmann::json device_value_cache = nlohmann::json();

    public:
        Tuning(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);

		//pass in parsed json response from begin session response. put into cache
		void ReceiveValues(nlohmann::json jsonvalues);

		//pass in string response from begin session response. try to parse as json then put into cache
		void ReceiveValues(::std::string rawvalues);
		
		//get value of different types

		bool GetValue(::std::string name, bool defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);
		int GetValue(::std::string name, int defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);
		float GetValue(::std::string name, float defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);
		::std::string GetValue(::std::string name, ::std::string defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);
		::std::string GetValue(::std::string name, char* defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);

		//in the unity sdk, this is limited to recording every 8 hours. ie once per session
		void RecordValueAccess(::std::string name, ::std::string default_value, ::std::string user_id = "", ::std::string device_id = "");

		//clear tuning value caches
		void EndSession();
};
}