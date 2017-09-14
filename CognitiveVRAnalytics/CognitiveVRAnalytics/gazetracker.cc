// Fill out your copyright notice in the Description page of Project Settings.

#include "stdafx.h"
#include "gazetracker.h"


// Sets default values for this component's properties
GazeTracker::GazeTracker(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
	SetInterval(cvr->config->GazeInterval);
	SetHMDType(cvr->config->HMDType);
}

void GazeTracker::SetInterval(float interval)
{
	PlayerSnapshotInterval = interval;
}

void GazeTracker::SetHMDType(std::string hmdtype)
{
	HMDType = hmdtype;
}

void GazeTracker::RecordGaze(std::vector<float> &Position, std::vector<float> &Rotation, std::vector<float> &Gaze, int objectId)
{
	if (!cvr->WasInitSuccessful()) { return; }

	//TODO put some kind of lookup/conversion table into config for xyz = -xzy or whatever

	json data = json();
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

	/*std::shared_ptr<json>snapObj = MakeShareable(new json);

	snapObj->SetNumberField("time", time);

	//positions
	TArray<std::shared_ptr<FJsonValue>> posArray;
	std::shared_ptr<FJsonValueNumber> JsonValue;
	JsonValue = MakeShareable(new FJsonValueNumber(-(int)position.X)); //right
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int)position.Z)); //up
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int)position.Y));  //forward
	posArray.Add(JsonValue);

	snapObj->SetArrayField("p", posArray);

	if (objectId >= 0)
	{
		snapObj->SetNumberField("o", objectId);
	}

	TArray<std::shared_ptr<FJsonValue>> gazeArray;
	JsonValue = MakeShareable(new FJsonValueNumber(-(int)gaze.X));
	gazeArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int)gaze.Z));
	gazeArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int)gaze.Y));
	gazeArray.Add(JsonValue);

	snapObj->SetArrayField("g", gazeArray);

	//rotation
	TArray<std::shared_ptr<FJsonValue>> rotArray;

	FQuat quat;
	FRotator adjustedRot = rotation;
	adjustedRot.Yaw -= 90;
	quat = adjustedRot.Quaternion();

	JsonValue = MakeShareable(new FJsonValueNumber(quat.Y));
	rotArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(quat.Z));
	rotArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(quat.X));
	rotArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(quat.W));
	rotArray.Add(JsonValue);

	snapObj->SetArrayField("r", rotArray);

	snapshots.Add(snapObj);
	if (snapshots.Num() > GazeBatchSize)
	{
		SendGazeEventDataToSceneExplorer();
		//s->FlushEvents();
		snapshots.Empty();
		events.Empty();
	}*/
}

void GazeTracker::RecordGaze(std::vector<float> &Position, std::vector<float> &Rotation)
{
	if (!cvr->WasInitSuccessful()) { return; }

	//TODO put some kind of lookup/conversion table into config for xyz = -xzy or whatever

	json data = json();
	data["time"] = cvr->GetTimestamp();
	data["p"] = { Position[0],Position[1],Position[2] };
	data["r"] = { Rotation[0],Rotation[1],Rotation[2],Rotation[3] };

	BatchedGazeSE.emplace_back(data);

	gazeCount++;
	if (gazeCount >= cvr->config->GazeBatchSize)
	{
		SendData();
	}

	/*std::shared_ptr<json>snapObj = MakeShareable(new json);

	snapObj->SetNumberField("time", time);

	//positions
	TArray<std::shared_ptr<FJsonValue>> posArray;
	std::shared_ptr<FJsonValueNumber> JsonValue;
	JsonValue = MakeShareable(new FJsonValueNumber(-(int)position.X)); //right
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int)position.Z)); //up
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int)position.Y));  //forward
	posArray.Add(JsonValue);

	snapObj->SetArrayField("p", posArray);

	//rotation
	TArray<std::shared_ptr<FJsonValue>> rotArray;

	FQuat quat;
	FRotator adjustedRot = rotation;
	adjustedRot.Yaw -= 90;
	quat = adjustedRot.Quaternion();

	JsonValue = MakeShareable(new FJsonValueNumber(quat.Y));
	rotArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(quat.Z));
	rotArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(quat.X));
	rotArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(quat.W));
	rotArray.Add(JsonValue);

	snapObj->SetArrayField("r", rotArray);

	snapshots.Add(snapObj);
	if (snapshots.Num() > GazeBatchSize)
	{
		SendGazeEventDataToSceneExplorer();
		//s->FlushEvents();
		snapshots.Empty();
		events.Empty();
	}*/
}

void GazeTracker::SendData()
{
	if (!cvr->WasInitSuccessful()) { return; }

	if (!cvr->HasStartedSession())
	{
		cvr->log->Warning("GazeTracker::SendData - Session not started!");
		return;
	}

	//send to sceneexplorer
	json se = json();
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
