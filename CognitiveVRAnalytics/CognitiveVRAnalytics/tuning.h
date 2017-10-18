
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

enum class EntityType
{
	kEntityTypeUser,
	kEntityTypeDevice
};

class TuningValue
{
private:
	::std::string value = "";
	long ttl = -1;

public:
	TuningValue(::std::string val, long time)
	{
		value = val;
		ttl = time;
	}

	long GetTtl()
	{
		return ttl;
	}

	::std::string GetValue()
	{
		return value;
	}
};


class COGNITIVEVRANALYTICS_API Tuning
{

    private:
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;

        //std::map< std::string, std::map<std::string, TuningValue*> > users_value_cache;

		nlohmann::json user_value_cache = nlohmann::json();
        //std::map< std::string, std::map<std::string, TuningValue*> > devices_value_cache;

		nlohmann::json device_value_cache = nlohmann::json();
        //long getallval_cache_ttl;

        ::std::string GetEntityTypeString(EntityType entity_type);
        void CacheValues(::std::string entity_id, nlohmann::json object, EntityType entity_type, bool getallc = false);

    public:
        Tuning(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);
        //~Tuning();

		//pass in string response from init. try to parse as json then put into cache using CacheValues
		void ReceiveValues(nlohmann::json jsonvalues);
		void ReceiveValues(::std::string rawvalues);
		
		bool GetValue(::std::string name, bool defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);
		int GetValue(::std::string name, int defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);
		float GetValue(::std::string name, float defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);
		::std::string GetValue(::std::string name, ::std::string defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);
		::std::string GetValue(::std::string name, char* defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);

		//in the unity sdk, this is limited to recording every 8 hours. ie once per session
		void RecordValueAccess(::std::string name, ::std::string default_value, ::std::string user_id = "", ::std::string device_id = "");

		void EndSession();
};
}