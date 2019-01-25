/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/
#include "stdafx.h"
#include "customevent.h"
namespace cognitive {
CustomEvent::CustomEvent(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;

	BatchedCustomEvents = nlohmann::json::array();
}

void CustomEvent::Send(std::string category, std::vector<float> &Position)
{
	RecordEvent(category, Position, nlohmann::json(), "");
}

void CustomEvent::Send(std::string category, std::vector<float> &Position, nlohmann::json properties)
{
	RecordEvent(category, Position, properties, "");
}

void CustomEvent::RecordEvent(std::string category, std::vector<float> &Position)
{
	RecordEvent(category, Position, nlohmann::json(),"");
}

void CustomEvent::RecordEvent(std::string category, std::vector<float> &Position, nlohmann::json properties)
{
	RecordEvent(category, Position, properties, "");
}

void CustomEvent::RecordEvent(std::string category, std::vector<float> &Position, std::string dynamicObjectId)
{
	RecordEvent(category, Position, nlohmann::json(), dynamicObjectId);
}

void CustomEvent::RecordEvent(std::string category, std::vector<float> &Position, nlohmann::json properties, std::string dynamicObjectId)
{
	double ts = cvr->GetTimestamp();

	//TODO conversion for xyz = -xzy or whatever

	nlohmann::json se = nlohmann::json();
	se["name"] = category;
	se["time"] = ts;
	if (!dynamicObjectId.empty())
		se["dynamicId"] = dynamicObjectId;
	se["point"] = { Position[0],Position[1] ,Position[2] };
	if (properties.size() > 0)
	{
		se["properties"] = properties;
	}

	BatchedCustomEvents.push_back(se);

	if (BatchedCustomEvents.size() >= cvr->GetConfig()->CustomEventBatchSize)
	{
		SendData();
	}
}

nlohmann::json CustomEvent::SendData()
{
	if (!cvr->IsSessionActive())
	{
		cvr->GetLog()->Info("CustomEvent::SendData failed: no session active");
		BatchedCustomEvents.clear();
		return nlohmann::json();
	}

	if (BatchedCustomEvents.size() == 0)
	{
		return nlohmann::json();
	}

	nlohmann::json se = nlohmann::json();
	se["userid"] = cvr->GetUniqueID();
	if (!cvr->GetLobbyId().empty())
		se["lobbyId"] = cvr->GetLobbyId();
	se["timestamp"] = (int)cvr->GetTimestamp();
	se["sessionid"] = cvr->GetSessionID();
	se["part"] = jsonPart;
	se["formatversion"] = "1.0";
	jsonPart++;
	se["data"] = BatchedCustomEvents;
	cvr->GetNetwork()->NetworkCall("events", se.dump());
	BatchedCustomEvents.clear();
	return se;
}
void CustomEvent::EndSession()
{
	BatchedCustomEvents.clear();
}
}