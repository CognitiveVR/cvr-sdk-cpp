/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/
#include "stdafx.h"
#include "customevent.h"
namespace cognitive {
CustomEvent::CustomEvent(::std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;

	BatchedCustomEvents = nlohmann::json::array();
}

void CustomEvent::Send(::std::string category, ::std::vector<float> &Position)
{
	Send(category, Position, nlohmann::json());
}

void CustomEvent::Send(::std::string category, ::std::vector<float> &Position, nlohmann::json properties)
{
	//if (!cvr->IsSessionActive()) { cvr->log->Info("CustomEvent::Send failed: no session active"); return; }

	double ts = cvr->GetTimestamp();

	//TODO conversion for xyz = -xzy or whatever

	nlohmann::json se = nlohmann::json();
	se["name"] = category;
	se["time"] = ts;
	se["point"] = { Position[0],Position[1] ,Position[2] };
	if (properties.size() > 0)
	{
		se["properties"] = properties;
	}

	BatchedCustomEvents.emplace_back(se);

	if (BatchedCustomEvents.size() >= cvr->config->TransactionBatchSize)
	{
		SendData();
	}
}

void CustomEvent::SendData()
{
	if (!cvr->IsSessionActive()) { cvr->log->Info("CustomEvent::SendData failed: no session active"); return; }

	if (BatchedCustomEvents.size() > 0)
	{
		//send to sceneexplorer
		nlohmann::json se = nlohmann::json();
		se["userid"] = cvr->UserId;
		se["timestamp"] = (int)cvr->GetTimestamp();
		se["sessionid"] = cvr->GetSessionID();
		se["part"] = jsonPart;
		jsonPart++;
		se["data"] = BatchedCustomEvents;
		cvr->network->NetworkCall("events", se.dump());
		BatchedCustomEvents.clear();
	}
}
void CustomEvent::EndSession()
{
	BatchedCustomEvents.clear();
}
}