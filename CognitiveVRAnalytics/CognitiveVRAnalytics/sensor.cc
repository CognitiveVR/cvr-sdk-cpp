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
	//if (!cvr->WasInitSuccessful()) { cvr->log->Info("Sensor::RecordSensor failed: init not successful"); return; }
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

void Sensor::SendData()
{
	if (cvr->IsPendingInit()) { cvr->log->Info("Sensor::SendData failed: init pending"); return; }
	if (!cvr->WasInitSuccessful()) { cvr->log->Info("Sensor::SendData. init not successful"); return; }

	if (allsensors.size() == 0)
	{
		return;
	}

	//put together all json
	nlohmann::json data = nlohmann::json();

	data["name"] = cvr->UserId;
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

	if (cvr->network->SceneExplorerCall("sensor", data.dump()))
	{
		sensorCount = 0;
		allsensors.clear();
	}
}

void Sensor::EndSession()
{
	allsensors.clear();
	sensorCount = 0;
}
}