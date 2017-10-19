/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/
#include "stdafx.h"
#include "tuning.h"
namespace cognitive {
Tuning::Tuning(::std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
}

//response received values, split by user and device
void Tuning::ReceiveValues(nlohmann::json jsonvalues)
{
	if (jsonvalues.size() == 0)
	{
		cvr->log->Info("Tuning::ReceiveValues json value count is 0 elements");
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

void Tuning::ReceiveValues(::std::string rawvalues)
{
	ReceiveValues(nlohmann::json::parse(rawvalues));
}

bool Tuning::GetValue(::std::string name, bool defaultValue, EntityType entity_type)
{
	if (!cvr->WasInitSuccessful()) { cvr->log->Info("Tuning::GetValue failed: init not successful"); return defaultValue; }

	cvr->log->Info("tuning variable bool");
	bool result = defaultValue;
	if (entity_type == EntityType::kEntityTypeUser)
	{
		if (user_value_cache.find(name) != user_value_cache.end())
		{
			if (user_value_cache[name].is_boolean())
			{
				result = user_value_cache[name].get<bool>();
			}
			else
			{
				if (user_value_cache[name] == "true")
				{
					result = true;
					user_value_cache[name] = result;
				}
				else if (user_value_cache[name] == "false")
				{
					result = false;
					user_value_cache[name] = result;
				}
				else
				{
					cvr->log->Warning("Tuning Variable does cannot cast to bool: " + name);
				}
			}
		}
		else
		{
			cvr->log->Warning("Tuning Variable does not exist: " + name);
		}
	}
	else
	{
		if (device_value_cache.find(name) != device_value_cache.end())
		{
			if (device_value_cache[name].is_boolean())
			{
				result = device_value_cache[name].get<bool>();
			}
			else
			{
				if (device_value_cache[name] == "true")
				{
					result = true;
					device_value_cache[name] = result;
				}
				else if (device_value_cache[name] == "false")
				{
					result = false;
					device_value_cache[name] = result;
				}
				else
				{
					cvr->log->Warning("Tuning Variable does cannot cast to bool: " + name);
				}
			}
		}
		else
		{
			cvr->log->Warning("Tuning Variable does not exist: " + name);
		}
	}
	return result;
}

int Tuning::GetValue(::std::string name, int defaultValue, EntityType entity_type)
{
	if (!cvr->WasInitSuccessful()) { cvr->log->Info("Tuning::GetValue failed: init not successful"); return defaultValue; }

	cvr->log->Info("tuning variable int");
	int result = defaultValue;
	if (entity_type == EntityType::kEntityTypeUser)
	{
		if (user_value_cache.find(name) != user_value_cache.end())
		{
			if (user_value_cache[name].is_number_integer())
			{
				result = user_value_cache[name].get<int>();
			}
			else
			{
				try
				{
					result = ::std::stoi(user_value_cache[name].get<std::string>());
					user_value_cache[name] = result;
				}
				catch (const ::std::invalid_argument& e)
				{
					cvr->log->Warning("Tuning Variable does cannot cast to int: " + name);
				}
			}
		}
		else
		{
			cvr->log->Warning("Tuning Variable does not exist: " + name);
		}
	}
	else
	{
		if (device_value_cache.find(name) != device_value_cache.end())
		{
			if (device_value_cache[name].is_number_integer())
			{
				result = device_value_cache[name].get<int>();
			}
			else
			{
				try
				{
					result = ::std::stoi(device_value_cache[name].get<::std::string>());
					device_value_cache[name] = result;
				}
				catch (const ::std::invalid_argument& e)
				{
					cvr->log->Warning("Tuning Variable does cannot cast to int: " + name);
				}
			}
		}
		else
		{
			cvr->log->Warning("Tuning Variable does not exist: " + name);
		}
	}
	return result;
}

float Tuning::GetValue(::std::string name, float defaultValue, EntityType entity_type)
{
	if (!cvr->WasInitSuccessful()) { cvr->log->Info("Tuning::GetValue failed: init not successful"); return defaultValue; }

	cvr->log->Info("tuning variable float");
	float result = defaultValue;
	if (entity_type == EntityType::kEntityTypeUser)
	{
		if (user_value_cache.find(name) != user_value_cache.end())
		{
			if (user_value_cache[name].is_number_float())
			{
				result = user_value_cache[name].get<float>();
			}
			else
			{
				try
				{
					result = ::std::stof(user_value_cache[name].get<::std::string>());
					user_value_cache[name] = result;
				}
				catch (const ::std::invalid_argument& e)
				{
					cvr->log->Warning("Tuning Variable does cannot cast to float: " + name);
				}
			}
		}
		else
		{
			cvr->log->Warning("Tuning Variable does not exist: " + name);
		}
	}
	else
	{
		if (device_value_cache.find(name) != device_value_cache.end())
		{
			if (device_value_cache[name].is_number_float())
			{
				result = device_value_cache[name].get<float>();
			}
			else
			{
				try
				{
					result = ::std::stof(device_value_cache[name].get<::std::string>());
					device_value_cache[name] = result;
				}
				catch (const ::std::invalid_argument& e)
				{
					cvr->log->Warning("Tuning Variable does cannot cast to float: " + name);
				}
			}
		}
		else
		{
			cvr->log->Warning("Tuning Variable does not exist: " + name);
		}
	}
	return result;
}

::std::string Tuning::GetValue(::std::string name, ::std::string defaultValue, EntityType entity_type)
{
	if (!cvr->WasInitSuccessful()) { cvr->log->Info("Tuning::GetValue failed: init not successful"); return defaultValue; }

	cvr->log->Info("tuning variable string");
	::std::string result = defaultValue;
	if (entity_type == EntityType::kEntityTypeUser)
	{
		if (user_value_cache.find(name) != user_value_cache.end())
		{
			result = user_value_cache[name].get<::std::string>();
		}
		else
		{
			cvr->log->Warning("Tuning Variable does not exist: " + name);
		}
	}
	else
	{
		if (device_value_cache.find(name) != device_value_cache.end())
		{
			result = device_value_cache[name].get<::std::string>();
		}
		else
		{
			cvr->log->Warning("Tuning Variable does not exist: " + name);
		}
	}
	return result;
}

::std::string Tuning::GetValue(::std::string name, char* defaultValue, EntityType entity_type)
{
	if (!cvr->WasInitSuccessful()) { cvr->log->Info("Tuning::GetValue failed: init not successful"); return defaultValue; }

	cvr->log->Info("tuning variable char*");
	::std::string result = defaultValue;
	if (entity_type == EntityType::kEntityTypeUser)
	{
		if (user_value_cache.find(name) != user_value_cache.end())
		{
			result = user_value_cache[name].get<::std::string>();
		}
		else
		{
			cvr->log->Warning("Tuning Variable does not exist: " + name);
		}
	}
	else
	{
		if (device_value_cache.find(name) != device_value_cache.end())
		{
			result = device_value_cache[name].get<::std::string>();
		}
		else
		{
			cvr->log->Warning("Tuning Variable does not exist: " + name);
		}
	}
	return result;
}

void Tuning::RecordValueAccess(::std::string name, ::std::string default_value, ::std::string user_id, ::std::string device_id)
{
	double ts = cvr->GetSessionTimestamp();

	nlohmann::json content = nlohmann::json::array();
	content.emplace_back(ts);
	content.emplace_back(ts);
	content.emplace_back(cvr->UserId);
	content.emplace_back(cvr->DeviceId);
	content.emplace_back(name);
	content.emplace_back(default_value);

	cvr->transaction->AddToBatch("tuner_recordUsed", content);
}

void Tuning::EndSession()
{
	user_value_cache.clear();
	device_value_cache.clear();
}
}