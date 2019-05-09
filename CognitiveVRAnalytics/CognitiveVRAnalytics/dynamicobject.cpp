/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

#include "stdafx.h"
#include "dynamicobject.h"

namespace cognitive {

ControllerInputState::ControllerInputState(std::string axisName, float axisValue, float x, float y)
{
	AxisName = axisName;
	AxisValue = axisValue;
	IsJoystick = true;
	X = x;
	Y = y;
}

ControllerInputState::ControllerInputState(std::string axisName, float axisValue)
{
	AxisName = axisName;
	AxisValue = axisValue;
	IsJoystick = false;
}

DynamicObjectSnapshot::DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::string objectId)
{
	Position = position;
	Rotation = rotation;
	useScale = false;
	Time = CognitiveVRAnalyticsCore::Instance()->GetTimestamp();
	Id = objectId;
}

DynamicObjectSnapshot::DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::string objectId, nlohmann::json properties)
{
	Position = position;
	Rotation = rotation;
	useScale = false;
	Time = CognitiveVRAnalyticsCore::Instance()->GetTimestamp();
	Id = objectId;
	if (properties.size() > 0)
	{
		Properties = properties;
	}
}

DynamicObjectSnapshot::DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string objectId)
{
	Position = position;
	Rotation = rotation;
	Scale = scale;
	Time = CognitiveVRAnalyticsCore::Instance()->GetTimestamp();
	Id = objectId;
}

DynamicObjectSnapshot::DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string objectId, nlohmann::json properties)
{
	Position = position;
	Rotation = rotation;
	Scale = scale;
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
	std::vector<float> scale = { 1, 1, 1 };
	RegisterObjectCustomId_Internal(name, meshname, customid, position, rotation, "",false);
	cognitive::nlohmann::json enable = cognitive::nlohmann::json();
	enable["enabled"] = true;
	cognitive::nlohmann::json props = cognitive::nlohmann::json::array();
	props.push_back(enable);
	RecordDynamic_Internal(customid, position, rotation, scale, true, props, std::vector<cognitive::ControllerInputState>());
}

void DynamicObject::RegisterObjectCustomId(std::string name, std::string meshname, std::string customid, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale)
{
	RegisterObjectCustomId_Internal(name, meshname, customid, position, rotation,"", false);

	cognitive::nlohmann::json enable = cognitive::nlohmann::json();
	enable["enabled"] = true;
	cognitive::nlohmann::json props = cognitive::nlohmann::json::array();
	props.push_back(enable);
	RecordDynamic_Internal(customid, position, rotation, scale, true, props, std::vector<cognitive::ControllerInputState>());
}

void DynamicObject::RegisterObjectCustomId(std::string name, std::string meshname, std::string customid, std::vector<float> position, std::vector<float> rotation, std::string controllername, bool isRight)
{
	std::vector<float> scale = { 1, 1, 1 };
	RegisterObjectCustomId_Internal(name, meshname, customid, position, rotation, controllername,isRight);
	cognitive::nlohmann::json enable = cognitive::nlohmann::json();
	enable["enabled"] = true;
	cognitive::nlohmann::json props = cognitive::nlohmann::json::array();
	props.push_back(enable);
	RecordDynamic_Internal(customid, position, rotation, scale, true, props, std::vector<cognitive::ControllerInputState>());
}

void DynamicObject::RegisterObjectCustomId(std::string name, std::string meshname, std::string customid, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string controllername, bool isRight)
{
	RegisterObjectCustomId_Internal(name, meshname, customid, position, rotation, controllername,isRight);

	cognitive::nlohmann::json enable = cognitive::nlohmann::json();
	enable["enabled"] = true;
	cognitive::nlohmann::json props = cognitive::nlohmann::json::array();
	props.push_back(enable);
	RecordDynamic_Internal(customid, position, rotation, scale, true, props, std::vector<cognitive::ControllerInputState>());
}

void DynamicObject::RegisterObjectCustomId_Internal(std::string name, std::string meshname, std::string customid, std::vector<float> position, std::vector<float> rotation, std::string controllerName, bool isRight)
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

	if (controllerName.length() > 0) //register controller with properties
	{
		auto controllerProps = nlohmann::json::array();
		auto j = nlohmann::json();
		j["controller"] = isRight ? "right" : "left";
		controllerProps.push_back(j);
		DynamicObjectManifestEntry dome = DynamicObjectManifestEntry(registerId.Id, name, meshname, controllerProps, controllerName);
		manifestEntries.push_back(dome);
		fullManifest.push_back(dome);
	}
	else //register normal dynamic object
	{
		DynamicObjectManifestEntry dome = DynamicObjectManifestEntry(registerId.Id, name, meshname);
		manifestEntries.push_back(dome);
		fullManifest.push_back(dome);
	}
}

std::string DynamicObject::RegisterObject(std::string name, std::string meshname, std::vector<float> position, std::vector<float> rotation)
{
	std::vector<float> scale = { 1,1,1 };
	std::string newObjectId = RegisterObject_Internal(name, meshname, position, rotation,"",false);

	cognitive::nlohmann::json enable = cognitive::nlohmann::json();
	enable["enabled"] = true;
	cognitive::nlohmann::json props = cognitive::nlohmann::json::array();
	props.push_back(enable);

	RecordDynamic_Internal(newObjectId, position, rotation, scale, true, props, std::vector<cognitive::ControllerInputState>());
	return newObjectId;
}

std::string DynamicObject::RegisterObject(std::string name, std::string meshname, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale)
{
	std::string newObjectId = RegisterObject_Internal(name, meshname, position, rotation,"", false);

	cognitive::nlohmann::json enable = cognitive::nlohmann::json();
	enable["enabled"] = true;
	cognitive::nlohmann::json props = cognitive::nlohmann::json::array();
	props.push_back(enable);

	RecordDynamic_Internal(newObjectId, position, rotation, scale, true, props, std::vector<cognitive::ControllerInputState>());
	return newObjectId;
}

std::string DynamicObject::RegisterObject(std::string name, std::string meshname, std::vector<float> position, std::vector<float> rotation, std::string controllername, bool isRight)
{
	std::vector<float> scale = { 1,1,1 };
	std::string newObjectId = RegisterObject_Internal(name, meshname, position, rotation, controllername, isRight);

	cognitive::nlohmann::json enable = cognitive::nlohmann::json();
	enable["enabled"] = true;
	cognitive::nlohmann::json props = cognitive::nlohmann::json::array();
	props.push_back(enable);

	RecordDynamic_Internal(newObjectId, position, rotation, scale, true, props, std::vector<cognitive::ControllerInputState>());
	return newObjectId;
}

std::string DynamicObject::RegisterObject(std::string name, std::string meshname, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string controllername, bool isRight)
{
	std::string newObjectId = RegisterObject_Internal(name, meshname, position, rotation, controllername, isRight);

	cognitive::nlohmann::json enable = cognitive::nlohmann::json();
	enable["enabled"] = true;
	cognitive::nlohmann::json props = cognitive::nlohmann::json::array();
	props.push_back(enable);

	RecordDynamic_Internal(newObjectId, position, rotation, scale, true, props, std::vector<cognitive::ControllerInputState>());
	return newObjectId;
}

std::string DynamicObject::RegisterObject_Internal(std::string name, std::string meshname, std::vector<float> position, std::vector<float> rotation, std::string controllerName, bool isRight)
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
		auto controllerProps = nlohmann::json::array();
		auto j = nlohmann::json();
		j["controller"] = isRight ? "right" : "left";
		controllerProps.push_back(j);
		DynamicObjectManifestEntry dome = DynamicObjectManifestEntry(newObjectId.Id, name, meshname, controllerProps, controllerName);

		manifestEntries.push_back(dome);
		fullManifest.push_back(dome);
	}

	return newObjectId.Id;
}

bool isInactive(DynamicObjectEngagementEvent* engagement)
{
	return engagement->isActive == false;
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation)
{
	//should record a dynamic object snapshot and set ignore scale to true
	std::vector<float> scale = { 1,1,1 };
	RecordDynamic_Internal(objectId, position, rotation, scale, false, cognitive::nlohmann::json(), std::vector<cognitive::ControllerInputState>());
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties)
{
	std::vector<float> scale = { 1,1,1 };
	RecordDynamic_Internal(objectId, position, rotation, scale, false, properties, std::vector<cognitive::ControllerInputState>());
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale)
{
	RecordDynamic_Internal(objectId, position, rotation, scale, true, cognitive::nlohmann::json(),std::vector<cognitive::ControllerInputState>());
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, nlohmann::json properties)
{
	RecordDynamic_Internal(objectId, position, rotation, scale, true, properties, std::vector<cognitive::ControllerInputState>());
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::string axis, float value)
{
	std::vector<cognitive::ControllerInputState> inputs = std::vector<cognitive::ControllerInputState>();
	cognitive::ControllerInputState input = cognitive::ControllerInputState(axis, value);
	inputs.push_back(input);
	std::vector<float> scale = { 1,1,1 };
	RecordDynamic_Internal(objectId, position, rotation, scale, false, cognitive::nlohmann::json(), inputs);
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties, std::string axis, float value)
{
	std::vector<cognitive::ControllerInputState> inputs = std::vector<cognitive::ControllerInputState>();
	cognitive::ControllerInputState input = cognitive::ControllerInputState(axis, value);
	inputs.push_back(input);
	std::vector<float> scale = { 1,1,1 };
	RecordDynamic_Internal(objectId, position, rotation, scale, false, properties, inputs);
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string axis, float value)
{
	std::vector<cognitive::ControllerInputState> inputs = std::vector<cognitive::ControllerInputState>();
	cognitive::ControllerInputState input = cognitive::ControllerInputState(axis, value);
	inputs.push_back(input);
	RecordDynamic_Internal(objectId, position, rotation, scale, true, cognitive::nlohmann::json(), inputs);
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, nlohmann::json properties, std::string axis, float value)
{
	std::vector<cognitive::ControllerInputState> inputs = std::vector<cognitive::ControllerInputState>();
	cognitive::ControllerInputState input = cognitive::ControllerInputState(axis, value);
	inputs.push_back(input);
	RecordDynamic_Internal(objectId, position, rotation, scale, true, properties, inputs);
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::string axis, float value, float x, float y)
{
	std::vector<cognitive::ControllerInputState> inputs = std::vector<cognitive::ControllerInputState>();
	cognitive::ControllerInputState input = cognitive::ControllerInputState(axis, value, x, y);
	inputs.push_back(input);
	std::vector<float> scale = { 1,1,1 };
	RecordDynamic_Internal(objectId, position, rotation, scale, false, cognitive::nlohmann::json(), inputs);
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties, std::string axis, float value, float x, float y)
{
	std::vector<cognitive::ControllerInputState> inputs = std::vector<cognitive::ControllerInputState>();
	cognitive::ControllerInputState input = cognitive::ControllerInputState(axis, value, x, y);
	inputs.push_back(input);
	std::vector<float> scale = { 1,1,1 };
	RecordDynamic_Internal(objectId, position, rotation, scale, false, properties, inputs);
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string axis, float value, float x, float y)
{
	std::vector<cognitive::ControllerInputState> inputs = std::vector<cognitive::ControllerInputState>();
	cognitive::ControllerInputState input = cognitive::ControllerInputState(axis, value, x, y);
	inputs.push_back(input);
	RecordDynamic_Internal(objectId, position, rotation, scale, true, cognitive::nlohmann::json(), inputs);
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, nlohmann::json properties, std::string axis, float value, float x, float y)
{
	std::vector<cognitive::ControllerInputState> inputs = std::vector<cognitive::ControllerInputState>();
	cognitive::ControllerInputState input = cognitive::ControllerInputState(axis, value, x, y);
	inputs.push_back(input);
	RecordDynamic_Internal(objectId, position, rotation, scale, true, properties, inputs);
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<ControllerInputState> inputState)
{
	std::vector<float> scale = { 1,1,1 };
	RecordDynamic_Internal(objectId, position, rotation, scale, false, cognitive::nlohmann::json(), inputState);
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties, std::vector<ControllerInputState> inputState)
{
	std::vector<float> scale = { 1,1,1 };
	RecordDynamic_Internal(objectId, position, rotation, scale, false, properties, inputState);
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::vector<ControllerInputState> inputState)
{
	RecordDynamic_Internal(objectId, position, rotation, scale, true, cognitive::nlohmann::json(), inputState);
}

void DynamicObject::RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, nlohmann::json properties, std::vector<ControllerInputState> inputState)
{
	RecordDynamic_Internal(objectId, position, rotation, scale, true, properties, inputState);
}

void DynamicObject::RecordDynamic_Internal(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, bool useScale, nlohmann::json properties, std::vector<ControllerInputState> inputs)
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

	DynamicObjectSnapshot snapshot = DynamicObjectSnapshot(position, rotation, scale, objectId);
	if (!useScale)
		snapshot.useScale = false;

	if (properties.size() > 0)
	{
		if (!properties.is_array())
		{
			cvr->GetLog()->Warning("Record Dynamic properties json must be formatted as array!");
		}
		else
		{
			snapshot.Properties = properties;
		}
	}

	if (allEngagements[objectId].size() > 0)
	{
		int i = 0;

		//add engagements to snapshot
		for (auto& e : activeEngagements[objectId])
		{
			nlohmann::json engagementEvent = nlohmann::json();
			engagementEvent["engagementparent"] = e->ParentObjectId;
			engagementEvent["engagement_count"] = e->EngagementNumber;
			engagementEvent["engagement_time"] = (e->endTime > 0 ? e->endTime - e->startTime : cvr->GetTimestamp() - e->startTime);
			engagementEvent["engagementtype"] = e->Name;
			snapshot.Engagements.push_back(engagementEvent);
		}
		//remove inactive engagements https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
		allEngagements[objectId].erase(std::remove_if(allEngagements[objectId].begin(), allEngagements[objectId].end(), isInactive), allEngagements[objectId].end());
		activeEngagements[objectId].erase(std::remove_if(activeEngagements[objectId].begin(), activeEngagements[objectId].end(), isInactive), activeEngagements[objectId].end());
	}

	if (inputs.size() > 0)
	{
		for (int i = 0; i < inputs.size(); i++)
		{
			//add button inputs to snapshot
			snapshot.Buttons[inputs[i].AxisName] = nlohmann::json();
			snapshot.Buttons[inputs[i].AxisName]["buttonPercent"] = inputs[i].AxisValue;
			if (inputs[i].IsJoystick)
			{
				snapshot.Buttons[inputs[i].AxisName]["x"] = inputs[i].X;
				snapshot.Buttons[inputs[i].AxisName]["y"] = inputs[i].Y;
			}
		}
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
	sendJson["timestamp"] = (int)cvr->GetSessionTimestamp();
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
			entryValues["fileType"] = cvr->GetConfig()->DynamicObjectFileType;
			if (element.Properties.size() > 0)
			{
				nlohmann::json proparray = nlohmann::json::array();
				for (auto& entryprop : element.Properties)
				{
					proparray.push_back(entryprop);
				}
				entryValues["properties"] = proparray;
			}
			if (element.IsController)
				entryValues["controllerType"] = element.ControllerType;

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
		if (element.useScale)
			entry["s"] = element.Scale;
		if (element.Properties.size() > 0)
			entry["properties"] = element.Properties;
		if (element.Engagements.size() > 0)
			entry["engagements"] = element.Engagements;
		if (element.Buttons.size() > 0)
			entry["buttons"] = element.Buttons;
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
	RecordDynamic(objectid, position, rotation, props);

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