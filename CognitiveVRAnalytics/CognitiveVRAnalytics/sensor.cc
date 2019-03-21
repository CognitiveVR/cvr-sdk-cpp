/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

#include "stdafx.h"
#include "sensor.h"
namespace cognitive {
Sensor::Sensor(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
}

void Sensor::RecordSensor(std::string Name, float value)
{
	//initialize json as array if needed
	auto search = allsensors.find(Name);
	if (search == allsensors.end())
	{
		allsensors[Name] = nlohmann::json::array();
	}

	nlohmann::json data = nlohmann::json::array();
	data.push_back(cvr->GetTimestamp());
	data.push_back(value);

	allsensors[Name].push_back(data);

	sensorCount++;
	if (sensorCount >= cvr->GetConfig()->SensorDataLimit)
	{
		SendData();
	}
}

nlohmann::json Sensor::SendData()
{
	if (!cvr->IsSessionActive())
	{
		cvr->GetLog()->Info("Sensor::SendData failed: no session active");
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

	data["name"] = cvr->GetUniqueID();
	if (!cvr->GetLobbyId().empty())
		data["lobbyId"] = cvr->GetLobbyId();
	data["sessionid"] = cvr->GetSessionID();
	data["timestamp"] = (int)cvr->GetSessionTimestamp();
	data["part"] = jsonPart;
	data["formatversion"] = "1.0";
	jsonPart++;

	nlohmann::json sensors = nlohmann::json::array();
	
	for (auto const &ent1 : allsensors)
	{
		nlohmann::json obj = nlohmann::json();
		obj["name"] = ent1.first;
		obj["data"] = ent1.second;
		sensors.push_back(obj);
	}
	data["data"] = sensors;

	cvr->GetNetwork()->NetworkCall("sensors", data.dump());
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