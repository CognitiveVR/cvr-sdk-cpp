/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

#include "stdafx.h"
#include "dynamicobject.h"

namespace cognitive {
DynamicObjectSnapshot::DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::string objectId)
{
	//TODO conversion for xyz = -xzy or whatever
	Position = position;
	Rotation = rotation;
	Time = CognitiveVRAnalyticsCore::Instance()->GetTimestamp();
	Id = objectId;
}

DynamicObjectSnapshot::DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::string objectId, nlohmann::json properties)
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

DynamicObjectEngagementEvent::DynamicObjectEngagementEvent(std::string parentId, std::string engagementName, int engagementNumber)
{
	ParentObjectId = parentId;
	Name = engagementName;
	startTime = CognitiveVRAnalyticsCore::Instance()->GetTimestamp();
	EngagementNumber = engagementNumber;
}

DynamicObject::DynamicObject(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
	nextObjectId = generatedIdOffset;
}

void DynamicObject::RegisterObjectCustomId(std::string name, std::string meshname, std::string customid, std::vector<float> position, std::vector<float> rotation)
{
	for (auto& element : objectIds)
	{
		if (element.Id == customid)
		{
			cvr->GetLog()->Warning("DynamicObject::RegisterObjectCustomId object id " +customid + " already registered");
			break;
		}
	}

	DynamicObjectId registerId = DynamicObjectId(customid, meshname);
	objectIds.push_back(registerId);

	DynamicObjectManifestEntry dome = DynamicObjectManifestEntry(registerId.Id, name, meshname);

	manifestEntries.push_back(dome);
	fullManifest.push_back(dome);

	cognitive::nlohmann::json enable = cognitive::nlohmann::json();
	enable["enabled"] = true;
	cognitive::nlohmann::json props = cognitive::nlohmann::json::array();
	props.push_back(enable);

	RecordDynamic(customid, position, rotation, props);
}

std::string DynamicObject::RegisterObject(std::string name, std::string meshname, std::vector<float> position, std::vector<float> rotation)
{
	bool foundRecycledId = false;
	DynamicObjectId newObjectId = DynamicObjectId("0", meshname);
	
	for (auto& element : objectIds)
	{
		if (element.Used)
		{
			if (element.Id == std::to_string(nextObjectId)) { nextObjectId++; }
			continue;
		}
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
		newObjectId = DynamicObjectId(std::to_string(nextObjectId), meshname);
		objectIds.push_back(newObjectId);
		DynamicObjectManifestEntry dome = DynamicObjectManifestEntry(newObjectId.Id, name, meshname);

		manifestEntries.push_back(dome);
		fullManifest.push_back(dome);
	}

	cognitive::nlohmann::json enable = cognitive::nlohmann::json();
	enable["enabled"] = true;
	cognitive::nlohmann::json props = cognitive::nlohmann::json::array();
	props.push_back(enable);

	RecordDynamic(newObjectId.Id, position, rotation, props);

	return newObjectId.Id;
}

bool isInactive(DynamicObjectEngagementEvent* engagement)
{
	return engagement->isActive == false;
}

void DynamicObject::AddSnapshot(std::string objectId, std::vector<float> position, std::vector<float> rotation)
{
	RecordDynamic(objectId, position, rotation, cognitive::nlohmann::json());
}
void DynamicObject::AddSnapshot(std::string objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties)
{
	RecordDynamic(objectId, position, rotation, properties);
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation)
{
	RecordDynamic(objectId, position, rotation, cognitive::nlohmann::json());
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties)
{
	//if dynamic object id is not in manifest, display warning. likely object ids were cleared from scene change
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
		cvr->GetLog()->Warning("DynamicObject::Snapshot cannot find objectId " + objectId + " in full manifest. Did you Register this object?");
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
		for (auto& e : activeEngagements[objectId])
		{
			//if (e.isActive)
			//{
				nlohmann::json engagementEvent = nlohmann::json();
				engagementEvent["engagementparent"] = e->ParentObjectId;
				engagementEvent["engagement_count"] = e->EngagementNumber;
				engagementEvent["engagement_time"] = (e->endTime > 0 ? e->endTime - e->startTime : cvr->GetTimestamp() - e->startTime);
				engagementEvent["engagementtype"] = e->Name;
				snapshot.Engagements.push_back(engagementEvent);
			//}
		}

		//cvr->log->Info("all engagements pre " + std::to_string(allEngagements[objectId].size()));
		//cvr->log->Info("active engagements pre " + std::to_string(activeEngagements[objectId].size()));
		//remove inactive engagements https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
		allEngagements[objectId].erase(std::remove_if(allEngagements[objectId].begin(), allEngagements[objectId].end(), isInactive), allEngagements[objectId].end());
		activeEngagements[objectId].erase(std::remove_if(activeEngagements[objectId].begin(), activeEngagements[objectId].end(), isInactive), activeEngagements[objectId].end());
		//cvr->log->Info("all engagements post " + std::to_string(allEngagements[objectId].size()));
		//cvr->log->Info("active engagements post " + std::to_string(activeEngagements[objectId].size()));
	}

	snapshots.push_back(snapshot);

	if (snapshots.size() + manifestEntries.size() >= cvr->GetConfig()->DynamicDataLimit)
	{
		SendData();
	}
}

void DynamicObject::BeginEngagement(std::string objectId, std::string name)
{
	BeginEngagement(objectId, name, "0");
}

void DynamicObject::BeginEngagement(std::string objectId, std::string name, std::string parentId)
{
	cvr->GetLog()->Info("DynamicObject::BeginEngagement engagement " + name + " on object " + objectId);

	engagementCounts[objectId][name] += 1;

	DynamicObjectEngagementEvent* engagement = new DynamicObjectEngagementEvent(parentId, name, engagementCounts[objectId][name]);
	activeEngagements[objectId].push_back(engagement);
	allEngagements[objectId].push_back(engagement);
}

void DynamicObject::EndEngagement(std::string objectId, std::string name)
{
	EndEngagement(objectId, name, "0");
}

void DynamicObject::EndEngagement(std::string objectId, std::string name, std::string parentId)
{
	for (auto& e : allEngagements[objectId])
	{
		if (e->Name == name && e->ParentObjectId == parentId)
		{
			e->isActive = false;
			e->endTime = cvr->GetTimestamp();
			return;
		}
	}

	//otherwise create and end the engagement

	cvr->GetLog()->Info("DynamicObject::EndEngagement engagement " + name + " not found on object"+ objectId +". Begin+End");
	BeginEngagement(objectId, name, parentId);

	auto rit = allEngagements[objectId].rbegin();
	for (; rit != allEngagements[objectId].rend(); ++rit)
	{
		if ((*rit)->Name == name && (*rit)->ParentObjectId == parentId)
		{
			(*rit)->isActive = false;
			(*rit)->endTime = cvr->GetTimestamp();
			cvr->GetLog()->Info("end engagement just started");
			return;
		}
	}
	cvr->GetLog()->Info("FAIL ------------------- cannot find just started engagement to end");
}

nlohmann::json DynamicObject::SendData()
{
	if (!cvr->IsSessionActive()) { cvr->GetLog()->Info("DynamicObject::SendData failed: no session active"); return nlohmann::json(); }

	if (manifestEntries.size() + snapshots.size() == 0)
	{
		return nlohmann::json();
	}

	nlohmann::json sendJson = nlohmann::json();

	sendJson["userid"] = cvr->GetUniqueID();
	if (!cvr->GetLobbyId().empty())
		sendJson["lobbyId"] = cvr->GetLobbyId();
	sendJson["timestamp"] = cvr->GetTimestamp();
	sendJson["sessionid"] = cvr->GetSessionID();
	sendJson["part"] = jsonpart;
	jsonpart++;
	sendJson["formatversion"] = "1.0";

	if (manifestEntries.size() > 0)
	{
		nlohmann::json manifest = nlohmann::json();

		for (auto& element : manifestEntries)
		{
			nlohmann::json entry = nlohmann::json();
			nlohmann::json entryValues = nlohmann::json();
			entryValues["name"] = element.Name;
			entryValues["mesh"] = element.MeshName;

			manifest[element.Id] = entryValues;
		}
		sendJson["manifest"] = manifest;
	}

	nlohmann::json data = nlohmann::json::array();

	for (auto& element : snapshots)
	{
		nlohmann::json entry = nlohmann::json();
		entry["id"] = element.Id;
		entry["time"] = element.Time;
		entry["p"] = element.Position;
		entry["r"] = element.Rotation;
		if (element.Properties.size() > 0)
			entry["properties"] = element.Properties;
		if (element.Engagements.size() > 0)
			entry["engagements"] = element.Engagements;
		data.push_back(entry);
	}
	if (data.size() > 0)
		sendJson["data"] = data;

	//send to sceneexplorer
	cvr->GetNetwork()->NetworkCall("dynamics", sendJson.dump());
	manifestEntries.clear();
	snapshots.clear();
	return sendJson;
}

void DynamicObject::EndActiveEngagements(std::string objectid)
{
	for (auto& element : activeEngagements[objectid])
	{
		if (element->isActive)
		{
			EndEngagement(objectid, element->Name);
		}
	}
}

void DynamicObject::RemoveObject(std::string objectid, std::vector<float> position, std::vector<float> rotation)
{
	//end any engagements if the object had any active
	EndActiveEngagements(objectid);

	//one final snapshot to send all the ended engagements
	cognitive::nlohmann::json enable = cognitive::nlohmann::json();
	enable["enabled"] = false;
	cognitive::nlohmann::json props = cognitive::nlohmann::json::array();
	props.push_back(enable);
	RecordDynamic(objectid, position, rotation,props);

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

//re-add all manifest entries when a scene changes. otherwise there could be snapshots for dynamic objects without any identification in the new scene
void DynamicObject::RefreshObjectManifest()
{
	for (auto& element : fullManifest)
	{
		manifestEntries.push_back(element);
	}
}

void DynamicObject::EndSession()
{
	fullManifest.clear();
	manifestEntries.clear();

	objectIds.clear();
	snapshots.clear();

	engagementCounts.clear();
	activeEngagements.clear();
	allEngagements.clear();
}
}