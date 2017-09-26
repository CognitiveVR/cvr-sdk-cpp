// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"
using json = nlohmann::json;

class CognitiveVRAnalyticsCore;

enum CommonMeshName
{
	ViveController,
	OculusTouchLeft,
	OculusTouchRight,
	ViveTracker
};

//created when registering dynamic objects
class DynamicObjectManifestEntry
{
public:
	int Id = 0;
	std::string Name = "";
	std::string MeshName = "";
	//json Properties

	DynamicObjectManifestEntry(int id, std::string name, std::string mesh)
	{
		Id = id;
		Name = name;
		MeshName = mesh;
	}
};

class DynamicObjectId
{
public:
	int Id = 0;
	bool Used = true;
	std::string MeshName = "";

	DynamicObjectId(int id, std::string meshName)
	{
		Id = id;
		MeshName = meshName;
	}
};

struct DynamicObjectSnapshot
{
public:
	std::vector<float> Position;
	std::vector<float> Rotation;
	double Time = -1;
	int Id = 0;
	json Properties = json();
	json Engagements = json();

	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, int objectId);
	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, int objectId, json properties);
};

struct DynamicObjectEngagementEvent
{
public:
	bool isActive = true;

	double startTime = -1;
	std::string Name = "";
	int ObjectId = 0;
	int EngagementNumber = 0;

	DynamicObjectEngagementEvent(int id, std::string engagementName, int engagementNumber);
};

class COGNITIVEVRANALYTICS_API DynamicObject
{
private:
	std::shared_ptr<CognitiveVRAnalyticsCore> cvr = NULL;
	std::vector<DynamicObjectId> objectIds; //cumulative
	std::vector<DynamicObjectSnapshot> snapshots; //cleared on send
	std::vector<DynamicObjectManifestEntry> manifestEntries; //cleared on send
	
	
	std::map<int, std::vector<DynamicObjectEngagementEvent>> dirtyEngagements;
	std::map<int, std::vector<DynamicObjectEngagementEvent>> allEngagements;
	std::map<int, std::map < std::string, int >> engagementCounts;

	//creates a unique id for an object. stored in this class, but can be referenced by unique number
	DynamicObjectId GetUniqueId(std::string meshName);
	int jsonpart = 1;

	//object id, engagement name, count
	//std::map<int, std::map<std::string, int>> engagements;
	//int GetEngagementCount(int objectid, std::string name);

public:	

	//TODO put a bunch of the options above into the constructor
	DynamicObject(std::shared_ptr<CognitiveVRAnalyticsCore> cog);

	void SendData();

	
	
	

	//put into dynamic manifest. reuses or creates new objectid. returns objectid
	int RegisterObject(std::string name, std::string meshname);

	//append engagement from list if still active
	void Snapshot(int objectId, std::vector<float> position, std::vector<float> rotation);
	void Snapshot(int objectId, std::vector<float> position, std::vector<float> rotation, json properties);

	//add engagement to list
	void BeginEngagement(int objectId, std::string name);
	//calculate time and remove from list. create snapshot
	void EndEngagement(int objectId, std::string name);

	//deregister. recycles objectid. don't need to do this for objects that don't share ids
	void RemoveObject(int objectid);
};