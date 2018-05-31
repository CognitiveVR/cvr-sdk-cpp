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

void GazeTracker::SendData()
{
	if (!cvr->IsSessionActive())
	{
		cvr->log->Info("GazeTracker::SendData failed: no session active");
		BatchedGaze.clear();
		return;
	}

	auto dproperties = cvr->GetDeviceProperties();
	auto uproperties = cvr->GetUserProperties();

	if (BatchedGaze.size() == 0 && dproperties.size() == 0 && uproperties.size() == 0)
	{
		return;
	}

	//send to sceneexplorer
	nlohmann::json se = nlohmann::json();
	se["userid"] = cvr->UserId;
	se["timestamp"] = (int)cvr->GetTimestamp();
	se["sessionid"] = cvr->GetSessionID();
	se["part"] = jsonPart;
	jsonPart++;
	se["hmdtype"] = HMDType;
	se["interval"] = PlayerSnapshotInterval;
	se["data"] = BatchedGaze;

	
	if (dproperties.size() > 0)
	{
		se["device"] = dproperties;
	}
	
	if (uproperties.size() > 0)
	{
		se["user"] = uproperties;
	}
	cvr->network->NetworkCall("gaze", se.dump());
	BatchedGaze.clear();
}

void GazeTracker::EndSession()
{
	BatchedGaze.clear();
}
}