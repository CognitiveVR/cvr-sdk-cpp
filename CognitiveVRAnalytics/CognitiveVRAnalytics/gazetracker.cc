
// Fill out your copyright notice in the Description page of Project Settings.

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
	if (!cvr->WasInitSuccessful()) { return; }

	//TODO put some kind of lookup/conversion table into config for xyz = -xzy or whatever

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

	gazeCount++;
	if (gazeCount >= cvr->config->GazeBatchSize)
	{
		SendData();
	}
}

void GazeTracker::RecordGaze(::std::vector<float> &Position, ::std::vector<float> &Rotation)
{
	if (!cvr->WasInitSuccessful()) { return; }

	//TODO put some kind of lookup/conversion table into config for xyz = -xzy or whatever

	nlohmann::json data = nlohmann::json();
	data["time"] = cvr->GetTimestamp();
	data["p"] = { Position[0],Position[1],Position[2] };
	data["r"] = { Rotation[0],Rotation[1],Rotation[2],Rotation[3] };

	BatchedGazeSE.emplace_back(data);

	gazeCount++;
	if (gazeCount >= cvr->config->GazeBatchSize)
	{
		SendData();
	}
}

void GazeTracker::SendData()
{
	if (!cvr->WasInitSuccessful()) { cvr->log->Info("GazeTracker::SendData. init not successful"); return; }

	if (!cvr->HasStartedSession())
	{
		cvr->log->Warning("GazeTracker::SendData - Session not started!");
		return;
	}

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
	cvr->network->SceneExplorerCall("gaze", se.dump());

	BatchedGazeSE.clear();

	gazeCount = 0;
}
}