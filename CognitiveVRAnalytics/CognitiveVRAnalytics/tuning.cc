/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "stdafx.h"
#include "tuning.h"

Tuning::Tuning(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
}

//response received values, split by user and device
void Tuning::ReceiveValues(json jsonvalues)
{
	//TODO error check - json count should be > 0
	if (jsonvalues.size() == 0)
	{
		cvr->log->Info("Tuning::ReceiveValues josn value count is 0 elements");
		return;
	}

	int errorcode = -1;
	errorcode = jsonvalues["error"].get<int>();
	if (errorcode != 0)
	{
		cvr->log->Info("Tuning::ReceiveValues error code " + errorcode);
		return;
	}

	user_value_cache = jsonvalues["data"]["usertuning"];
	device_value_cache = jsonvalues["data"]["devicetuning"];
}

void Tuning::ReceiveValues(std::string rawvalues)
{
	ReceiveValues(json::parse(rawvalues));
}

//TODO pass a reference of this json around. dereference and save copy
void Tuning::CacheValues(std::string entity_id, json values, EntityType entity_type, bool getallc)
{
	if (!cvr->HasStartedSession())
	{
		cvr->log->Warning("Tuning::CacheValues session has not started");
		return;
	}
}

//TODO make a template to return a generic type instead of json
json Tuning::GetValue(std::string name, std::string default_value)
{
	json j;

	if (cvr->UserId.size() > 0)
	{
		j[name] = user_value_cache[name];
	}
	else if (cvr->UserId.size() > 0)
	{
		j[name] = device_value_cache[name];
	}

	return j;
}


json Tuning::GetValue(std::string name, std::string default_value, std::string entity_id, EntityType entity_type)
{    
	json j;

	if (entity_type == EntityType::kEntityTypeUser)
	{
		j[name] = user_value_cache[name];
	}
	else if (entity_type == EntityType::kEntityTypeDevice)
	{
		j[name] = device_value_cache[name];
	}

	return j;
}

void Tuning::RecordValueAccess(std::string name, std::string default_value, std::string user_id, std::string device_id)
{

	double ts = cvr->GetSessionTimestamp();

	json content = json::array();
	content.emplace_back(ts);
	content.emplace_back(ts);
	content.emplace_back(cvr->UserId);
	content.emplace_back(cvr->DeviceId);
	content.emplace_back(name);
	content.emplace_back(default_value);

	cvr->transaction->AddToBatch("tuner_recordUsed", content);
}

std::string Tuning::GetEntityTypeString(EntityType entity_type)
{
    switch (entity_type) {
        case kEntityTypeUser:
            return "USER";
        case kEntityTypeDevice:
            return "DEVICE";
    }
    return "UNKNOWN";
}