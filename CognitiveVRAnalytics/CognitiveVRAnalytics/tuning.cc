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


	//CacheValues(jsonvalues["data"]["userid"], jsonvalues["data"]["usertuning"], EntityType::kEntityTypeUser, true);
	//CacheValues(jsonvalues["data"]["deviceid"], jsonvalues["data"]["devicetuning"], EntityType::kEntityTypeDevice, true);
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

	

	//TODO split apart json values
	/*

    long cache_time = Util::GetTimestamp() + Config::kTuningCacheTtl;

    if (getallc) {
        Tuning::getallval_cache_ttl = cache_time;
    }

	CognitiveLog::Info("CACHING TUNING VALUES: " + entity_id + ":" + Tuning::GetEntityTypeString(entity_type));
		
	// Iterate over Json Values
	for (auto currJsonValue = values.Values.CreateConstIterator(); currJsonValue; ++currJsonValue)
	{
		// Get the key name
		const std::string Name = (*currJsonValue).Key;
		std::string skey = TCHAR_TO_UTF8(*Name);

		// Get the value as a FJsonValue object
		std::shared_ptr< FJsonValue > Value = (*currJsonValue).Value;

		// Do your stuff with crazy casting and other questionable rituals

		std::string ValueString = Value.Get()->AsString();

		TuningValue* tval = new TuningValue(TCHAR_TO_UTF8(*ValueString), cache_time);
		if (entity_type == kEntityTypeUser) {
			users_value_cache[entity_id][skey] = tval;
		}
		else {
			devices_value_cache[entity_id][skey] = tval;
		}
	}
	*/
}

/*void Tuning::GetAllValues(std::string entity_id, EntityType entity_type)
{
	std::shared_ptr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	if (s == nullptr || !s->bHasSessionStarted)
	{
		CognitiveLog::Warning("Tuning::GetAllValues provider is null!");
		return;
	}

	/*
    long curtime = Util::GetTimestamp();

    if (curtime <= Tuning::getallval_cache_ttl) {

        bool entvalid = false;
        CognitiveVRResponse resp(true);
		json root;
        if (entity_type == kEntityTypeUser) {
            if (users_value_cache.count(entity_id)) {
                entvalid = true;
                std::map<std::string, TuningValue*> entcache = users_value_cache.find(entity_id)->second;
                for(std::map<std::string, TuningValue*>::iterator it = entcache.begin(); it != entcache.end(); it++) {
                    root[it->first] = it->second->GetValue();
                }
            }
        } else {
            if (devices_value_cache.count(entity_id)) {
                entvalid = true;
                std::map<std::string, TuningValue*> entcache = devices_value_cache.find(entity_id)->second;
                for(std::map<std::string, TuningValue*>::iterator it = entcache.begin(); it != entcache.end(); it++) {
                    root[it->first] = it->second->GetValue();
                }
            }
        }

        if (entvalid) {
            CognitiveLog::Info("Get All Value cache has NOT expired and entity already exists.");
            resp.SetContent(root);

            CognitiveLog::Info("GET: " + Tuning::GetEntityTypeString(entity_type) + " " + resp.GetContent().toStyledString());
            return resp;
        }
    }
	*//*

	TArray< std::shared_ptr<FJsonValue> > ObjArray;
	std::shared_ptr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));
	std::string fs = std::string::SanitizeFloat(Util::GetTimestamp());
	std::string empty;
	std::string entityType = Tuning::GetEntityTypeString(entity_type);

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, entity_id);
	Util::AppendToJsonArray(jsonArray, entityType);

	CognitiveLog::Info("Get All Value cache has expired or the entity does not exist.");
		
	//CognitiveVRResponse resp = s->GetNetwork()->Call("tuner_getAllValues", jsonArray);
	//resp.SetContent(resp.GetContent().GetObjectField("value").Get);// ["value"]);
    //CacheValues(entity_id, resp.GetContent(), entity_type, true);
}*/

//TODO make a template to return a generic type instead of json
json Tuning::GetValue(std::string name, std::string default_value)
{
	/*
	std::string user = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->GetUserID();
	std::string device = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->GetDeviceID();
	std::string selectedID;
	EntityType entityType;


	if (!user.IsEmpty())
	{
		entityType = EntityType::kEntityTypeUser;
		selectedID = user;
	}
	else if (!device.IsEmpty())
	{
		entityType = EntityType::kEntityTypeDevice;
		selectedID = device;
	}
	else
	{
		CognitiveLog::Warning("Tuning::GetValue no entity found!");
		CognitiveVRResponse failresp(false);
		return failresp;
	}

	return GetValue(name, default_value, TCHAR_TO_UTF8(*selectedID), entityType);
	*/
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
	/*std::shared_ptr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	if (!Provider.IsValid() || !bHasSessionStarted)
	{
		CognitiveLog::Warning("Tuning::GetValue provider is null!");
		CognitiveVRResponse failresp(false);
		return failresp;
	}

    //long curtime = Util::GetTimestamp();
    TuningValue* v = NULL;
    
	if (entity_type == kEntityTypeUser) {
        if (users_value_cache.count(entity_id)) {
            std::map<std::string, TuningValue*> entcache = users_value_cache.find(entity_id)->second;
            if (entcache.count(name)) {
                v = entcache.find(name)->second;
            }
        }
    } else {
        if (devices_value_cache.count(entity_id)) {
            std::map<std::string, TuningValue*> entcache = devices_value_cache.find(entity_id)->second;
            if (entcache.count(name)) {
                v = entcache.find(name)->second;
            }
        }
    }

	//this seems to request the tuning variable after they 'expire'
    /*long ttl = 0L;

    if (v != NULL) {
        ttl = v->GetTtl();
    }
	
    if (curtime >= ttl) {
        CognitiveLog::Info("Value cache has expired.");

		json json;

        std::string ts = Util::GetTimestampStr();
        json.append(ts);
        json.append(ts);
        json.append(name);
        json.append(entity_id);
        std::string entity_type_string = Tuning::GetEntityTypeString(entity_type);
        json.append(entity_type_string);

        CognitiveVRResponse resp(false);
        try {
            resp = s->GetNetwork()->Call("tuner_getValue", json);
        } catch(cognitivevrapi::cognitivevr_exception e) {
            CognitiveLog::Error("Get Tuning Value Fail(Returning Default): " + e.GetResponse().GetErrorMessage());

            CognitiveVRResponse dresp(true);
			json content;
            content[name] = default_value;
            dresp.SetContent(content);
            return dresp;
        }

        resp.SetContent(resp.GetContent()["value"]);
        if (resp.GetContent() == json::null) {
            cognitivevrapi::ThrowDummyResponseException("Tuning variable '" + name + "' for " + entity_type_string + " \"" + entity_id + "\" does not exist.");
            return resp;
        }

        CacheValues(entity_id, resp.GetContent(), entity_type);

        return resp;
    }
	*//*
	
	if (v != NULL)
	{
		CognitiveVRResponse resp(true);
		json root;
		root.SetStringField(name.c_str(), v->GetValue().c_str());
		resp.SetContent(root);
		return resp;
	}

	CognitiveLog::Info("Tuning::GetValue could not find " + name);

	CognitiveVRResponse resp(false);
	return resp;*/
    
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
	/*TArray< std::shared_ptr<FJsonValue> > ObjArray;
	std::shared_ptr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	std::string fs = std::string::SanitizeFloat(Util::GetTimestamp());

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	s->AppendUD(jsonArray);
	Util::AppendToJsonArray(jsonArray, name);
	Util::AppendToJsonArray(jsonArray, default_value);

    //s->batcher->PushTask(callback, "tuner_recordUsed", jsonArray);
	std::shared_ptr<json> jsonObject = MakeShareable(new json());
	jsonObject.Get()->SetStringField("method", "tuner_recordUsed");
	jsonObject.Get()->SetField("args", jsonArray);

	s->batcher->AddJsonToBatch(jsonObject);*/

	double ts = cvr->GetSessionTimestamp();

	json content = json::array();
	content.emplace_back(ts);
	content.emplace_back(ts);
	content.emplace_back(cvr->UserId);
	content.emplace_back(cvr->DeviceId);
	content.emplace_back(name);
	content.emplace_back(default_value);

	cvr->transaction->AddToBatch("tuner_recordUsed", &content);
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