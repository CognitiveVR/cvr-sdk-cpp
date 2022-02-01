/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

#include "stdafx.h"
#include "gazetracker.h"

namespace cognitive {
// Sets default values for this component's properties
GazeTracker::GazeTracker(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
}

void GazeTracker::RecordGaze(std::vector<float> &Position, std::vector<float> &Rotation, std::vector<float> &Gaze)
{
	double time = cvr->GetTimestamp();
	if (time < nextSendTimestamp) { if (time > nextWarningTimestamp) { nextWarningTimestamp = time + 5; cvr->GetLog()->Warning("RecordGaze called more frequently than license allows. Some recorded data may be lost"); } return; }
	nextSendTimestamp = time + 0.1;

	nlohmann::json data = nlohmann::json();
	data["time"] = time;
	data["p"] = { Position[0],Position[1],Position[2] };
	data["g"] = { Gaze[0],Gaze[1],Gaze[2] };
	data["r"] = { Rotation[0],Rotation[1],Rotation[2],Rotation[3] };

	BatchedGaze.push_back(data);

	if (BatchedGaze.size() >= cvr->GetConfig()->GazeBatchSize)
	{
		SendData();
	}
}

void GazeTracker::RecordGaze(std::vector<float>& Position, std::vector<float>& Rotation, std::vector<float>& Gaze, double timestamp)
{
	double time = timestamp;
	//if (time < nextSendTimestamp) { if (time > nextWarningTimestamp) { nextWarningTimestamp = time + 5; cvr->GetLog()->Warning("RecordGaze called more frequently than license allows. Some recorded data may be lost"); } return; }
	//nextSendTimestamp = time + 0.1;

	nlohmann::json data = nlohmann::json();
	data["time"] = time;
	data["p"] = { Position[0],Position[1],Position[2] };
	data["g"] = { Gaze[0],Gaze[1],Gaze[2] };
	data["r"] = { Rotation[0],Rotation[1],Rotation[2],Rotation[3] };

	BatchedGaze.push_back(data);

	if (BatchedGaze.size() >= cvr->GetConfig()->GazeBatchSize)
	{
		SendData();
	}
}

void GazeTracker::RecordGaze(std::vector<float> &Position, std::vector<float> &Rotation, std::vector<float> &Gaze, std::string objectId)
{
	double time = cvr->GetTimestamp();
	if (time < nextSendTimestamp) { if (time > nextWarningTimestamp) { nextWarningTimestamp = time + 5; cvr->GetLog()->Warning("RecordGaze called more frequently than license allows. Some recorded data may be lost"); } return; }
	nextSendTimestamp = time + 0.1;

	nlohmann::json data = nlohmann::json();
	data["time"] = time;
	data["p"] = { Position[0],Position[1],Position[2] };
	data["g"] = { Gaze[0],Gaze[1],Gaze[2] };
	data["r"] = { Rotation[0],Rotation[1],Rotation[2],Rotation[3] };
	data["o"] = objectId;

	BatchedGaze.push_back(data);

	if (BatchedGaze.size() >= cvr->GetConfig()->GazeBatchSize)
	{
		SendData();
	}
}

void GazeTracker::RecordGaze(std::vector<float> &Position, std::vector<float> &Rotation, std::vector<float> &Gaze, std::string objectId, std::string mediaId, long mediaTime, std::vector<float> &uvs)
{
	double time = cvr->GetTimestamp();
	if (time < nextSendTimestamp) { if (time > nextWarningTimestamp) { nextWarningTimestamp = time + 5; cvr->GetLog()->Warning("RecordGaze called more frequently than license allows. Some recorded data may be lost"); } return; }
	nextSendTimestamp = time + 0.1;

	nlohmann::json data = nlohmann::json();
	data["time"] = time;
	data["p"] = { Position[0],Position[1],Position[2] };
	data["g"] = { Gaze[0],Gaze[1],Gaze[2] };
	data["r"] = { Rotation[0],Rotation[1],Rotation[2],Rotation[3] };
	data["o"] = objectId;
	data["mediaId"] = mediaId;
	data["mediatime"] = mediaTime;
	data["uvs"] = { uvs[0],uvs[1] };

	BatchedGaze.push_back(data);

	if (BatchedGaze.size() >= cvr->GetConfig()->GazeBatchSize)
	{
		SendData();
	}
}

void GazeTracker::RecordGaze(std::vector<float> &Position, std::vector<float> &Rotation)
{
	double time = cvr->GetTimestamp();
	if (time < nextSendTimestamp) { if (time > nextWarningTimestamp) { nextWarningTimestamp = time + 5; cvr->GetLog()->Warning("RecordGaze called more frequently than license allows. Some recorded data may be lost"); } return; }
	nextSendTimestamp = time + 0.1;

	nlohmann::json data = nlohmann::json();
	data["time"] = time;
	data["p"] = { Position[0],Position[1],Position[2] };
	data["r"] = { Rotation[0],Rotation[1],Rotation[2],Rotation[3] };

	BatchedGaze.push_back(data);

	if (BatchedGaze.size() >= cvr->GetConfig()->GazeBatchSize)
	{
		SendData();
	}
}

nlohmann::json GazeTracker::SendData()
{
	if (!cvr->IsSessionActive())
	{
		cvr->GetLog()->Info("GazeTracker::SendData failed: no session active");
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
	se["userid"] = cvr->GetUniqueID();
	if (!cvr->GetLobbyId().empty())
		se["lobbyId"] = cvr->GetLobbyId();
	se["timestamp"] = (int)cvr->GetSessionTimestamp();
	se["sessionid"] = cvr->GetSessionID();
	se["part"] = jsonPart;
	jsonPart++;
	se["hmdtype"] = cvr->GetConfig()->HMDType;
	se["interval"] = cvr->GetConfig()->GazeInterval;
	if (BatchedGaze.size() > 0)
		se["data"] = BatchedGaze;
	se["formatversion"] = "1.0";
	
	if (properties.size() > 0)
	{
		se["properties"] = properties;
	}

	cvr->GetNetwork()->NetworkCall("gaze", se.dump());
	BatchedGaze.clear();
	return se;
}

void GazeTracker::EndSession()
{
	BatchedGaze.clear();
}
}