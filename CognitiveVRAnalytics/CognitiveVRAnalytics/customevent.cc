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

void CustomEvent::RecordEvent(std::string category, std::vector<float> &Position)
{
	RecordEvent(category, Position, nlohmann::json(),"",cvr->GetTimestamp());
}

void CustomEvent::RecordEvent(std::string category, std::vector<float>& Position, nlohmann::json properties, double timestamp)
{
	RecordEvent(category, Position, properties, "", timestamp);
}

void CustomEvent::RecordEvent(std::string category, std::vector<float> &Position, nlohmann::json properties)
{
	RecordEvent(category, Position, properties, "", cvr->GetTimestamp());
}

void CustomEvent::RecordEvent(std::string category, std::vector<float> &Position, std::string dynamicObjectId)
{
	RecordEvent(category, Position, nlohmann::json(), dynamicObjectId, cvr->GetTimestamp());
}

void CustomEvent::RecordEvent(std::string category, std::vector<float> &Position, nlohmann::json properties, std::string dynamicObjectId)
{
	RecordEvent(category, Position, properties, dynamicObjectId, cvr->GetTimestamp());
}

void CustomEvent::RecordEvent(std::string category, std::vector<float>& Position, nlohmann::json properties, std::string dynamicObjectId, double timestamp)
{
	double ts = timestamp;

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
	se["timestamp"] = (int)cvr->GetSessionTimestamp();
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