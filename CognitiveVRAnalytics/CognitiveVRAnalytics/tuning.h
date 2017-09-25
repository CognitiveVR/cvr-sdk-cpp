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

enum EntityType
{
	kEntityTypeUser,
	kEntityTypeDevice
};

class TuningValue
{
private:
	std::string value = "";
	long ttl = -1;

public:
	TuningValue(std::string val, long time)
	{
		value = val;
		ttl = time;
	}

	long GetTtl()
	{
		return ttl;
	}

	std::string GetValue()
	{
		return value;
	}
};


class COGNITIVEVRANALYTICS_API Tuning
{

    private:
		std::shared_ptr<CognitiveVRAnalyticsCore> cvr = NULL;

        //std::map< std::string, std::map<std::string, TuningValue*> > users_value_cache;

		json user_value_cache = json();
        //std::map< std::string, std::map<std::string, TuningValue*> > devices_value_cache;

		json device_value_cache = json();
        //long getallval_cache_ttl;

        std::string GetEntityTypeString(EntityType entity_type);
        void CacheValues(std::string entity_id, json object, EntityType entity_type, bool getallc = false);

    public:
        Tuning(std::shared_ptr<CognitiveVRAnalyticsCore> cog);
        //~Tuning();

		//pass in string response from init. try to parse as json then put into cache using CacheValues
		void ReceiveValues(json jsonvalues);
		void ReceiveValues(std::string rawvalues);
		
		bool GetValue(std::string name, bool defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);
		int GetValue(std::string name, int defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);
		float GetValue(std::string name, float defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);
		std::string GetValue(std::string name, std::string defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);
		std::string GetValue(std::string name, char* defaultValue, EntityType entity_type = EntityType::kEntityTypeDevice);

		//in the unity sdk, this is limited to recording every 8 hours. ie once per session
		void RecordValueAccess(std::string name, std::string default_value, std::string user_id = "", std::string device_id = "");
};