/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

#include "stdafx.h"
#include "sensor.h"
namespace cognitive {
Sensor::Sensor(::std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
}

void Sensor::RecordSensor(::std::string Name, float value)
{
	//initialize json as array if needed
	auto search = allsensors.find(Name);
	if (search == allsensors.end())
	{
		allsensors[Name] = nlohmann::json::array();
	}

	nlohmann::json data = nlohmann::json::array();
	data.emplace_back(cvr->GetTimestamp());
	data.emplace_back(value);

	allsensors[Name].emplace_back(data);

	sensorCount++;
	if (sensorCount >= cvr->config->SensorDataLimit)
	{
		SendData();
	}
}

nlohmann::json Sensor::SendData()
{
	if (!cvr->IsSessionActive())
	{
		cvr->log->Info("Sensor::SendData failed: no session active");
		sensorCount = 0;
		allsensors.clear();
		return nlohmann::json();
	}

	if (allsensors.size() == 0)
	{
		return nlohmann::json();
	}

	//put together all json
	nlohmann::json data = nlohmann::json();

	data["name"] = cvr->UserId;
	if (!cvr->GetLobbyId().empty())
		data["lobbyId"] = cvr->GetLobbyId();
	data["sessionid"] = cvr->GetSessionID();
	data["timestamp"] = (int)cvr->GetTimestamp();
	data["part"] = jsonPart;
	jsonPart++;

	nlohmann::json sensors = nlohmann::json::array();
	
	for (auto const &ent1 : allsensors)
	{
		nlohmann::json obj = nlohmann::json();
		obj["name"] = ent1.first;
		obj["data"] = ent1.second;
		sensors.emplace_back(obj);
	}
	data["data"] = sensors;

	cvr->network->NetworkCall("sensors", data.dump());
	sensorCount = 0;
	allsensors.clear();

	return data;
}

void Sensor::EndSession()
{
	allsensors.clear();
	sensorCount = 0;
}
}