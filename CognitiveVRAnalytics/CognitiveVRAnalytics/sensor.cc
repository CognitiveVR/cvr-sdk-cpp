
/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
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
		allsensors[Name] = json::array();
	}

	json data = json::array();
	data.emplace_back(cvr->GetTimestamp());
	data.emplace_back(value);

	allsensors[Name].emplace_back(data);

	sensorCount++;
	if (sensorCount >= cvr->config->SensorDataLimit)
	{
		SendData();
	}


	/*
	if (somedatapoints.Contains(Name))
	{
		somedatapoints[Name].Append(",[" + std::string::SanitizeFloat(Util::GetTimestamp()) + "," + std::string::SanitizeFloat(value) + "]");
	}
	else
	{
		somedatapoints.Emplace(Name, "[" + std::string::SanitizeFloat(Util::GetTimestamp()) + "," + std::string::SanitizeFloat(value) + "]");
	}*/
	

	/*TArray<FJsonValueNumber> sensorData;
	sensorData.Init(0, 2);
	sensorData[0] = Util::GetTimestamp();
	sensorData[1] = value;

	//[x.xxxx,y.yyyy]

	if (sensorDataJson.Contains(Name))
	{
		TArray<TArray<FJsonValueNumber>>* index = sensorDataJson.Find(Name);
		index->Add(sensorData);
	}
	else
	{
		sensorDataJson.Emplace(Name).Add(sensorData);
	}*/

	//just directly call network.sendstuff to url

	//json j;
	//s->batcher->AddJsonToBatch("sensor", j);
}

void Sensor::SendData()
{
	if (!cvr->WasInitSuccessful()) { cvr->log->Info("Sensor::SendData. init not successful"); return; }

	if (!cvr->HasStartedSession())
	{
		cvr->log->Warning("Sensor::SendData - Session not started!");
		return;
	}

	if (allsensors.size() == 0)
	{
		return;
	}

	//put together all json
	json data = json();

	data["name"] = cvr->UserId;
	data["sessionid"] = cvr->GetSessionID();
	data["timestamp"] = (int)cvr->GetTimestamp();
	data["part"] = jsonPart;
	jsonPart++;

	json sensors = json::array();
	
	for (auto const &ent1 : allsensors)
	{
		json obj = json();
		obj["name"] = ent1.first;
		obj["data"] = ent1.second;
		sensors.emplace_back(obj);
	}
	data["data"] = sensors;

	cvr->network->SceneExplorerCall("sensor", data.dump());

	sensorCount = 0;
	allsensors.clear();

	//send to sceneexplorer
	//cvr->network->Call("someurl2", "somecontent");
}

/*std::string Sensors::SensorDataToString()
{
	std::shared_ptr<json> wholeObj = MakeShareable(new json);

	TArray< std::shared_ptr<FJsonValue> > DataArray;

	wholeObj->SetStringField("name", s->GetDeviceID());
	wholeObj->SetNumberField("timestamp", (int)s->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", s->GetCognitiveSessionID());
	wholeObj->SetNumberField("part", jsonPart);
	jsonPart++;
	
	wholeObj->SetStringField("data", "SENSORDATAHERE");

	std::string OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);

	std::string allData;
	if (somedatapoints.Num() == 0)
	{
		return "";
	}
	for (const auto& Entry : somedatapoints)
	{
		allData = allData.Append("{\"name\":\""+Entry.Key + "\",\"data\":[" + Entry.Value + "]},");
	}
	allData.RemoveAt(allData.Len());

	std::string complete = "[" + allData + "]";
	const TCHAR* charcomplete = *complete;
	OutputString = OutputString.Replace(TEXT("\"SENSORDATAHERE\""), charcomplete);

	return OutputString;
}*/
}