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

void GazeTracker::RecordGaze(::std::vector<float> &Position, ::std::vector<float> &Rotation, ::std::vector<float> &Gaze, int objectId)
{
	if (!cvr->WasInitSuccessful() && !cvr->IsPendingInit()) { return; }

	//TODO conversion for xyz = -xzy or whatever

	nlohmann::json data = nlohmann::json();
	data["time"] = cvr->GetTimestamp();
	data["p"] = { Position[0],Position[1],Position[2] };
	data["g"] = { Gaze[0],Gaze[1],Gaze[2] };
	data["r"] = { Rotation[0],Rotation[1],Rotation[2],Rotation[3] };

	if (objectId >= 0)
	{
		data["o"] = objectId;
	}

	BatchedGazeSE.emplace_back(data);

	if (BatchedGazeSE.size() >= cvr->config->GazeBatchSize)
	{
		SendData();
	}
}

void GazeTracker::RecordGaze(::std::vector<float> &Position, ::std::vector<float> &Rotation)
{
	if (!cvr->WasInitSuccessful() && !cvr->IsPendingInit()) { return; }

	//TODO conversion for xyz = -xzy or whatever

	nlohmann::json data = nlohmann::json();
	data["time"] = cvr->GetTimestamp();
	data["p"] = { Position[0],Position[1],Position[2] };
	data["r"] = { Rotation[0],Rotation[1],Rotation[2],Rotation[3] };

	BatchedGazeSE.emplace_back(data);

	if (BatchedGazeSE.size() >= cvr->config->GazeBatchSize)
	{
		SendData();
	}
}

void GazeTracker::SendData()
{
	if (cvr->IsPendingInit()) { cvr->log->Info("GazeTracker::SendData failed: init pending"); return; }
	if (!cvr->WasInitSuccessful()) { cvr->log->Info("GazeTracker::SendData. init not successful"); return; }

	if (BatchedGazeSE.size() == 0)
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
	se["data"] = BatchedGazeSE;
	if (cvr->network->SceneExplorerCall("gaze", se.dump()))
	{
		BatchedGazeSE.clear();
	}
}

void GazeTracker::EndSession()
{
	BatchedGazeSE.clear();
}
}