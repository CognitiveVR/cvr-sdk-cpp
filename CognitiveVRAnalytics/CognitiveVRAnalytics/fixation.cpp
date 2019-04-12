/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/
#include "stdafx.h"
#include "fixation.h"

namespace cognitive {
	Fixation::Fixation(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
	{
		cvr = cog;

		BatchedFixations = nlohmann::json::array();
	}

	void Fixation::RecordFixation(double Time, int DurationMs, float MaxRadius, std::string ObjectId, std::vector<float> &LocalPosition)
	{
		nlohmann::json se = nlohmann::json();
		se["time"] = Time;
		se["duration"] = DurationMs;
		se["maxradius"] = MaxRadius;
		se["objectid"] = ObjectId;
		se["p"] = { LocalPosition[0],LocalPosition[1] ,LocalPosition[2] };

		BatchedFixations.push_back(se);

		if (BatchedFixations.size() >= cvr->GetConfig()->FixationBatchSize)
		{
			SendData();
		}
	}

	void Fixation::RecordFixation(double Time, int DurationMs, float MaxRadius, std::vector<float> &WorldPosition)
	{
		nlohmann::json se = nlohmann::json();
		se["time"] = Time;
		se["duration"] = DurationMs;
		se["maxradius"] = MaxRadius;
		se["p"] = { WorldPosition[0],WorldPosition[1] ,WorldPosition[2] };

		BatchedFixations.push_back(se);

		if (BatchedFixations.size() >= cvr->GetConfig()->FixationBatchSize)
		{
			SendData();
		}
	}

	nlohmann::json Fixation::SendData()
	{
		if (!cvr->IsSessionActive())
		{
			cvr->GetLog()->Info("Fixation::SendData failed: no session active");
			BatchedFixations.clear();
			return nlohmann::json();
		}

		if (BatchedFixations.size() == 0)
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
		se["data"] = BatchedFixations;
		cvr->GetNetwork()->NetworkCall("fixations", se.dump());
		BatchedFixations.clear();
		return se;
	}
	void Fixation::EndSession()
	{
		BatchedFixations.clear();
	}
}