/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

#include "stdafx.h"
#include "gazetracker.h"

namespace cognitive {
// Sets default values for this component's properties
GazeTracker::GazeTracker(::std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
}

void GazeTracker::SetInterval(float interval)
{
	PlayerSnapshotInterval = interval;
}

void GazeTracker::SetHMDType(::std::string hmdtype)
{
	HMDType = hmdtype;
}

void GazeTracker::RecordGaze(::std::vector<float> &Position, ::std::vector<float> &Rotation, ::std::vector<float> &Gaze)
{
	//TODO conversion for xyz = -xzy or whatever

	nlohmann::json data = nlohmann::json();
	data["time"] = cvr->GetTimestamp();
	data["p"] = { Position[0],Position[1],Position[2] };
	data["g"] = { Gaze[0],Gaze[1],Gaze[2] };
	data["r"] = { Rotation[0],Rotation[1],Rotation[2],Rotation[3] };

	BatchedGaze.emplace_back(data);

	if (BatchedGaze.size() >= cvr->config->GazeBatchSize)
	{
		SendData();
	}
}

void GazeTracker::RecordGaze(::std::vector<float> &Position, ::std::vector<float> &Rotation, ::std::vector<float> &Gaze, std::string objectId)
{
	//TODO conversion for xyz = -xzy or whatever

	nlohmann::json data = nlohmann::json();
	data["time"] = cvr->GetTimestamp();
	data["p"] = { Position[0],Position[1],Position[2] };
	data["g"] = { Gaze[0],Gaze[1],Gaze[2] };
	data["r"] = { Rotation[0],Rotation[1],Rotation[2],Rotation[3] };
	data["o"] = objectId;

	BatchedGaze.emplace_back(data);

	if (BatchedGaze.size() >= cvr->config->GazeBatchSize)
	{
		SendData();
	}
}

void GazeTracker::RecordGaze(::std::vector<float> &Position, ::std::vector<float> &Rotation)
{
	//TODO conversion for xyz = -xzy or whatever

	nlohmann::json data = nlohmann::json();
	data["time"] = cvr->GetTimestamp();
	data["p"] = { Position[0],Position[1],Position[2] };
	data["r"] = { Rotation[0],Rotation[1],Rotation[2],Rotation[3] };

	BatchedGaze.emplace_back(data);

	if (BatchedGaze.size() >= cvr->config->GazeBatchSize)
	{
		SendData();
	}
}

nlohmann::json GazeTracker::SendData()
{
	if (!cvr->IsSessionActive())
	{
		cvr->log->Info("GazeTracker::SendData failed: no session active");
		BatchedGaze.clear();
		return nlohmann::json();
	}

	auto properties = cvr->GetNewSessionProperties();

	if (BatchedGaze.size() == 0 && properties.size() == 0)
	{
		return nlohmann::json();
	}

	//send to sceneexplorer
	nlohmann::json se = nlohmann::json();
	se["userid"] = cvr->UserId;
	if (!cvr->GetLobbyId().empty())
		se["lobbyId"] = cvr->GetLobbyId();
	se["timestamp"] = (int)cvr->GetTimestamp();
	se["sessionid"] = cvr->GetSessionID();
	se["part"] = jsonPart;
	jsonPart++;
	se["hmdtype"] = HMDType;
	se["interval"] = PlayerSnapshotInterval;
	se["data"] = BatchedGaze;

	
	if (properties.size() > 0)
	{
		se["properties"] = properties;
	}

	cvr->network->NetworkCall("gaze", se.dump());
	BatchedGaze.clear();
	return se;
}

void GazeTracker::EndSession()
{
	BatchedGaze.clear();
}
}