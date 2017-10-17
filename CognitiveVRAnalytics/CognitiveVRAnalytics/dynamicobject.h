
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"

//central class for registering / snapshotting all dynamic objects

namespace cognitive {
	//using json = nlohmann::json;
class CognitiveVRAnalyticsCore;

enum class CommonMeshName
{
	kViveController,
	kOculusTouchLeft,
	kOculusTouchRight,
	kViveTracker
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
	nlohmann::json Properties = nlohmann::json();
	nlohmann::json Engagements = nlohmann::json();

	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, int objectId);
	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, int objectId, nlohmann::json properties);
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
	std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;
	std::vector<DynamicObjectId> objectIds; //cumulative
	std::vector<DynamicObjectSnapshot> snapshots; //cleared on send
	std::vector<DynamicObjectManifestEntry> manifestEntries; //cleared on send
	
	
	std::map<int, std::vector<DynamicObjectEngagementEvent>> dirtyEngagements;
	std::map<int, std::vector<DynamicObjectEngagementEvent>> allEngagements;
	std::map<int, std::map < std::string, int >> engagementCounts;

	//creates a unique id for an object. stored in this class, but can be referenced by unique number
	DynamicObjectId GetUniqueId(std::string meshName);
	int jsonpart = 1;

	int generatedIdOffset = 1000;

	//object id, engagement name, count
	//std::map<int, std::map<std::string, int>> engagements;
	//int GetEngagementCount(int objectid, std::string name);

public:	


	//bool UseCustomMeshName = false;
	//::std::string CustomMeshName = "";

	//CommonMeshName CommonMeshName = CommonMeshName::kViveController;

	//int CustomUniqueId = -1;

	//TODO put a bunch of the options above into the constructor
	DynamicObject(std::shared_ptr<CognitiveVRAnalyticsCore> cog);

	void SendData();

	//put into dynamic manifest with an id. returns objectid
	int RegisterObjectCustomId(std::string name, std::string meshname, int customid);

	//put into dynamic manifest. reuses or creates new objectid. returns objectid
	int RegisterObject(std::string name, std::string meshname);

	//append engagement from list if still active
	void Snapshot(int objectId, std::vector<float> position, std::vector<float> rotation);
	void Snapshot(int objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties);

	//add engagement to list
	void BeginEngagement(int objectId, std::string name);
	//calculate time and remove from list. create snapshot
	void EndEngagement(int objectId, std::string name);

	//deregister. recycles objectid. don't need to do this for objects that don't share ids
	//automatically ends all active engagements on this object
	void RemoveObject(int objectid);

	//TODO end all engagements on an object. to be used if the object is destroyed

	//object ids must be cleared between scenes. otherwise reused objects will not be written to a manifest for the new scene
	void ClearObjectIds();
};
}