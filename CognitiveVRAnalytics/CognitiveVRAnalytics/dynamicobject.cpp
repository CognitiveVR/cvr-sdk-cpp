// Fill out your copyright notice in the Description page of Project Settings.

#include "stdafx.h"
#include "dynamicobject.h"

DynamicObjectSnapshot::DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, int objectId)
{
	Position = position;
	Rotation = rotation;
	Time = CognitiveVRAnalyticsCore::Instance()->GetTimestamp();
	Id = objectId;
}

DynamicObjectSnapshot::DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, int objectId, json properties)
{
	Position = position;
	Rotation = rotation;
	Time = CognitiveVRAnalyticsCore::Instance()->GetTimestamp();
	Id = objectId;
	if (properties.size() > 0)
	{
		Properties = properties;
	}
}

DynamicObjectEngagementEvent::DynamicObjectEngagementEvent(int id, std::string engagementName, int engagementNumber)
{
	ObjectId = id;
	Name = engagementName;
	startTime = CognitiveVRAnalyticsCore::Instance()->GetTimestamp();
	EngagementNumber = engagementNumber;
}

DynamicObject::DynamicObject(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
}

int DynamicObject::RegisterObject(std::string name, std::string meshname)
{
	DynamicObjectId registerId = GetUniqueId(meshname);
	objectIds.emplace_back(registerId);

	DynamicObjectManifestEntry dome = DynamicObjectManifestEntry(registerId.Id, name, meshname);

	manifestEntries.emplace_back(dome);

	if (snapshots.size() + manifestEntries.size() >= cvr->config->GazeBatchSize)
	{
		SendData();
	}

	return registerId.Id;
}

DynamicObjectId DynamicObject::GetUniqueId(std::string meshname)
{
	int nextObjectId = 0;
	for (auto& element : objectIds)
	{
		if (element.Id >= nextObjectId) { nextObjectId = element.Id + 1; }
		if (element.Used) { continue; }
		if (element.MeshName == meshname)
		{
			//found an unused objectid
			element.Used = true;
			return element;
		}
	}

	//create new objectid
	auto newObjectId = DynamicObjectId(nextObjectId, meshname);

	return newObjectId;
}

bool isInactive(DynamicObjectEngagementEvent engagement)
{
	return engagement.isActive == false;
}

void DynamicObject::Snapshot(std::vector<float> position, std::vector<float> rotation, int objectId)
{
	DynamicObjectSnapshot snapshot = DynamicObjectSnapshot(position, rotation, objectId);
	
	if (allEngagements[objectId].size() > 0)
	{
		int i = 0;
		
		//add engagements to snapshot
		for (auto& e : allEngagements[objectId])
		{
			if (e.isActive)
			{
				json engagementEvent = json();
				engagementEvent["engagementparent"] = objectId;
				engagementEvent["engagement_count"] = e.EngagementNumber;
				engagementEvent["engagement_time"] = cvr->GetTimestamp() - e.startTime;
				snapshot.Engagements.emplace_back(engagementEvent);
			}
		}

		//remove inactive engagements
		allEngagements[objectId].erase(std::remove_if(allEngagements[objectId].begin(), allEngagements[objectId].end(), isInactive), allEngagements[objectId].end());
		
		//TODO this could be improved. should loop through once https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
	}

	snapshots.emplace_back(snapshot);

	if (snapshots.size() + manifestEntries.size() >= cvr->config->GazeBatchSize)
	{
		SendData();
	}
}

void DynamicObject::Snapshot(std::vector<float> position, std::vector<float> rotation, int objectId, json properties)
{
	DynamicObjectSnapshot snapshot = DynamicObjectSnapshot(position, rotation, objectId, properties);

	if (allEngagements[objectId].size() > 0)
	{
		int i = 0;

		//add engagements to snapshot
		for (auto& e : allEngagements[objectId])
		{
			if (e.isActive)
			{
				json engagementEvent = json();
				engagementEvent["engagementparent"] = objectId;
				engagementEvent["engagement_count"] = e.EngagementNumber;
				engagementEvent["engagement_time"] = cvr->GetTimestamp() - e.startTime;
				snapshot.Engagements.emplace_back(engagementEvent);
			}
		}

		//remove inactive engagements
		allEngagements[objectId].erase(std::remove_if(allEngagements[objectId].begin(), allEngagements[objectId].end(), isInactive), allEngagements[objectId].end());

		//TODO this could be improved. should loop through once https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
	}

	snapshots.emplace_back(snapshot);

	if (snapshots.size() + manifestEntries.size() >= cvr->config->GazeBatchSize)
	{
		SendData();
	}
}

void DynamicObject::BeginEngagement(int objectId, std::string name)
{
	engagementCounts[objectId][name] += 1;

	DynamicObjectEngagementEvent engagement = DynamicObjectEngagementEvent(objectId, name, engagementCounts[objectId][name]);
	dirtyEngagements[objectId].emplace_back(engagement);
	allEngagements[objectId].emplace_back(engagement);	
}

void DynamicObject::EndEngagement(int objectId, std::string name)
{
	for (auto& e : dirtyEngagements[objectId])
	{
		if (e.Name == name)
		{
			e.isActive = false;
			break;
		}
	}
	//TODO otherwise create and end the engagement
}

void DynamicObject::SendData()
{
	if (!cvr->WasInitSuccessful()) { cvr->log->Info("DynamicObject::SendData. init not successful"); return; }

	if (!cvr->HasStartedSession())
	{
		cvr->log->Warning("DynamicObject::SendData - Session not started!");
		return;
	}

	if (manifestEntries.size() + snapshots.size() == 0)
	{
		return;
	}

	json sendJson = json();

	sendJson["userid"] = cvr->UserId;
	sendJson["timestamp"] = cvr->GetTimestamp();
	sendJson["sessionid"] = cvr->GetSessionID();
	sendJson["part"] = jsonpart;

	json manifest = json();

	for (auto& element : manifestEntries)
	{
		json entry = json();
		json entryValues = json();
		entryValues["name"] = element.Name;
		entryValues["mesh"] = element.MeshName;

		entry[element.Id] = entryValues;
		manifest.emplace_back(entry);
	}
	sendJson["manifest"] = manifest;

	json data = json::array();

	for (auto& element : snapshots)
	{
		json entry = json();
		entry["id"] = element.Id;
		entry["time"] = element.Time;
		entry["p"] = element.Position;
		entry["r"] = element.Rotation;
		data.emplace_back(data);
	}
	sendJson["data"] = data;

	//send to sceneexplorer
	cvr->network->SceneExplorerCall("dynamic", sendJson.dump());

	snapshots.clear();
	manifest.clear();
}

void DynamicObject::RemoveObject(int objectid)
{
	//TODO end any engagements if the object had any active
	//TODO one final snapshot

	for (auto& element : objectIds)
	{
		if (element.Id == objectid)
		{
			element.Used = false;
			return;
		}
	}
}