
// Fill out your copyright notice in the Description page of Project Settings.

#include "stdafx.h"
#include "dynamicobject.h"

namespace cognitive {
DynamicObjectSnapshot::DynamicObjectSnapshot(::std::vector<float> position, ::std::vector<float> rotation, int objectId)
{
	//TODO conversion for xyz = -xzy or whatever
	Position = position;
	Rotation = rotation;
	Time = CognitiveVRAnalyticsCore::Instance()->GetTimestamp();
	Id = objectId;
}

DynamicObjectSnapshot::DynamicObjectSnapshot(::std::vector<float> position, ::std::vector<float> rotation, int objectId, nlohmann::json properties)
{
	//TODO conversion for xyz = -xzy or whatever
	Position = position;
	Rotation = rotation;
	Time = CognitiveVRAnalyticsCore::Instance()->GetTimestamp();
	Id = objectId;
	if (properties.size() > 0)
	{
		Properties = properties;
	}
}

DynamicObjectEngagementEvent::DynamicObjectEngagementEvent(int id, ::std::string engagementName, int engagementNumber)
{
	ObjectId = id;
	Name = engagementName;
	startTime = CognitiveVRAnalyticsCore::Instance()->GetTimestamp();
	EngagementNumber = engagementNumber;
}

DynamicObject::DynamicObject(::std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
}

int DynamicObject::RegisterObjectCustomId(::std::string name, ::std::string meshname, int customid)
{
	DynamicObjectId registerId = DynamicObjectId(customid, meshname);
	objectIds.emplace_back(registerId);

	for (auto& element : objectIds)
	{
		if (element.Id == customid)
		{
			cvr->log->Warning("DynamicObject::RegisterObjectCustomId object id " + std::to_string(customid) + "already registered");
		}
	}

	DynamicObjectManifestEntry dome = DynamicObjectManifestEntry(registerId.Id, name, meshname);

	manifestEntries.emplace_back(dome);
	fullManifest.emplace_back(dome);

	if (snapshots.size() + manifestEntries.size() >= cvr->config->DynamicDataLimit)
	{
		SendData();
	}

	return registerId.Id;
}

int DynamicObject::RegisterObject(::std::string name, ::std::string meshname)
{
	bool foundRecycledId = false;
	DynamicObjectId newObjectId = DynamicObjectId(0, meshname);

	static int nextObjectId = generatedIdOffset;
	for (auto& element : objectIds)
	{
		if (element.Id == nextObjectId) { nextObjectId++; continue; }
		if (element.Used) { continue; }
		if (element.MeshName == meshname)
		{
			//found an unused objectid
			element.Used = true;
			newObjectId = element;
			foundRecycledId = true;
			break;
		}
	}

	if (!foundRecycledId)
	{
		newObjectId = DynamicObjectId(nextObjectId, meshname);
		objectIds.emplace_back(newObjectId);
		DynamicObjectManifestEntry dome = DynamicObjectManifestEntry(newObjectId.Id, name, meshname);

		manifestEntries.emplace_back(dome);
		fullManifest.emplace_back(dome);
	}

	if (snapshots.size() + manifestEntries.size() >= cvr->config->DynamicDataLimit)
	{
		SendData();
	}

	return newObjectId.Id;
}

bool isInactive(DynamicObjectEngagementEvent engagement)
{
	return engagement.isActive == false;
}

void DynamicObject::AddSnapshot(int objectId, ::std::vector<float> position, ::std::vector<float> rotation)
{
	AddSnapshot(objectId, position, rotation, cognitive::nlohmann::json());
}

void DynamicObject::AddSnapshot(int objectId, ::std::vector<float> position, ::std::vector<float> rotation, nlohmann::json properties)
{
	//if dynamic object id is not in manifest, add to manifest. likely object ids were cleared from scene change
	bool foundId = false;
	for (auto& element : objectIds)
	{
		if (objectId == element.Id)
		{
			foundId = true;
			break;
		}
	}
	if (!foundId)
	{
		cvr->log->Warning("DynamicObject::Snapshot cannot find objectId " + std::to_string(objectId) + " in full manifest. Did you Register this object?");
	}

	DynamicObjectSnapshot snapshot = DynamicObjectSnapshot(position, rotation, objectId);
	if (properties.size() > 0)
	{
		snapshot.Properties = properties;
	}

	if (allEngagements[objectId].size() > 0)
	{
		int i = 0;

		//add engagements to snapshot
		for (auto& e : allEngagements[objectId])
		{
			if (e.isActive)
			{
				nlohmann::json engagementEvent = nlohmann::json();
				engagementEvent["engagementparent"] = objectId;
				engagementEvent["engagement_count"] = e.EngagementNumber;
				engagementEvent["engagement_time"] = cvr->GetTimestamp() - e.startTime;
				snapshot.Engagements.emplace_back(engagementEvent);
			}
		}

		//remove inactive engagements https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
		allEngagements[objectId].erase(::std::remove_if(allEngagements[objectId].begin(), allEngagements[objectId].end(), isInactive), allEngagements[objectId].end());
	}

	snapshots.emplace_back(snapshot);

	if (snapshots.size() + manifestEntries.size() >= cvr->config->DynamicDataLimit)
	{
		SendData();
	}
}

void DynamicObject::BeginEngagement(int objectId, ::std::string name)
{
	engagementCounts[objectId][name] += 1;

	DynamicObjectEngagementEvent engagement = DynamicObjectEngagementEvent(objectId, name, engagementCounts[objectId][name]);
	dirtyEngagements[objectId].emplace_back(engagement);
	allEngagements[objectId].emplace_back(engagement);	
}

void DynamicObject::EndEngagement(int objectId, ::std::string name)
{
	for (auto& e : dirtyEngagements[objectId])
	{
		if (e.Name == name)
		{
			e.isActive = false;
			return;
		}
	}
	//otherwise create and end the engagement

	BeginEngagement(objectId, name);

	auto rit = dirtyEngagements[objectId].rbegin();
	for (; rit != dirtyEngagements[objectId].rend(); ++rit)
	{
		if (rit->Name == name)
		{
			rit->isActive = false;
			return;
		}
	}
}

void DynamicObject::SendData()
{
	if (cvr->IsPendingInit()) { cvr->log->Info("DynamicObject::SendData failed: init pending"); return; }
	if (!cvr->WasInitSuccessful()) { cvr->log->Info("DynamicObject::SendData failed: init not successful"); return; }

	if (manifestEntries.size() + snapshots.size() == 0)
	{
		return;
	}

	nlohmann::json sendJson = nlohmann::json();

	sendJson["userid"] = cvr->UserId;
	sendJson["timestamp"] = cvr->GetTimestamp();
	sendJson["sessionid"] = cvr->GetSessionID();
	sendJson["part"] = jsonpart;

	nlohmann::json manifest = nlohmann::json();

	for (auto& element : manifestEntries)
	{
		nlohmann::json entry = nlohmann::json();
		nlohmann::json entryValues = nlohmann::json();
		entryValues["name"] = element.Name;
		entryValues["mesh"] = element.MeshName;

		manifest[::std::to_string(element.Id)] = entryValues;
	}
	sendJson["manifest"] = manifest;

	nlohmann::json data = nlohmann::json::array();

	for (auto& element : snapshots)
	{
		nlohmann::json entry = nlohmann::json();
		entry["id"] = element.Id;
		entry["time"] = element.Time;
		entry["p"] = element.Position;
		entry["r"] = element.Rotation;
		data.emplace_back(entry);
	}
	sendJson["data"] = data;

	//send to sceneexplorer
	if (cvr->network->SceneExplorerCall("dynamic", sendJson.dump()))
	{
		manifestEntries.clear();
		snapshots.clear();
	}
}

void DynamicObject::EndActiveEngagements(int objectid)
{
	for (auto& element : dirtyEngagements[objectid])
	{
		if (element.isActive)
		{
			EndEngagement(objectid, element.Name);
		}
	}
}

void DynamicObject::RemoveObject(int objectid, std::vector<float> position, std::vector<float> rotation)
{
	//end any engagements if the object had any active
	EndActiveEngagements(objectid);

	//one final snapshot to send all the ended engagements
	AddSnapshot(objectid, position, rotation);

	//set the object as not used
	for (auto& element : objectIds)
	{
		if (element.Id == objectid)
		{
			element.Used = false;
			return;
		}
	}
}

//re add all manifest entries when a scene changes. otherwise there could be snapshots for dynamic objects without any identification in the new scene
void DynamicObject::RefreshObjectManifest()
{
	for (auto& element : fullManifest)
	{
		manifestEntries.emplace_back(element);
	}
}

void DynamicObject::EndSession()
{
	fullManifest.clear();
	manifestEntries.clear();

	objectIds.clear();
	snapshots.clear();

	engagementCounts.clear();
	dirtyEngagements.clear();
	allEngagements.clear();
}
}