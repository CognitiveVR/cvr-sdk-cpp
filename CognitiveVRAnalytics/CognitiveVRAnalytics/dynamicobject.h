
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

	//creates a unique id for an object. stored in this class, but can be referenced by unique number
	DynamicObjectId GetUniqueId(std::string meshName);
	int jsonpart = 1;

	int generatedIdOffset = 1000;

public:

	//public for testing, but shouldn't be used outside this class
	std::vector<DynamicObjectId> objectIds; //cumulative. only used by sdk to assign unique ids
	std::vector<DynamicObjectSnapshot> snapshots; //cleared on send
	std::vector<DynamicObjectManifestEntry> fullManifest; //refreshed on scene change
	std::vector<DynamicObjectManifestEntry> manifestEntries; //cleared on send

	std::map<int, std::vector<DynamicObjectEngagementEvent>> dirtyEngagements; //engagements that are active
	std::map<int, std::vector<DynamicObjectEngagementEvent>> allEngagements; //all engagements that need to be written to snapshots. inactive engagements are removed
	std::map<int, std::map < std::string, int >> engagementCounts;




	DynamicObject(std::shared_ptr<CognitiveVRAnalyticsCore> cog);

	void SendData();

	//put into dynamic manifest with an id. returns objectid
	int RegisterObjectCustomId(std::string name, std::string meshname, int customid);

	//put into dynamic manifest. reuses or creates new objectid. returns objectid
	int RegisterObject(std::string name, std::string meshname);

	//append engagement from list if still active
	void AddSnapshot(int objectId, std::vector<float> position, std::vector<float> rotation);
	void AddSnapshot(int objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties);

	//add engagement to list
	void BeginEngagement(int objectId, std::string name);
	//calculate time and remove from list. create snapshot
	void EndEngagement(int objectId, std::string name);

	//deregister. recycles objectid. don't need to do this for objects that don't share ids
	//also ends all active engagements on this object
	void RemoveObject(int objectid, std::vector<float> position, std::vector<float> rotation);

	//end all engagements on an object. to be used if the object is destroyed
	void EndActiveEngagements(int objectId);

	//object ids must be refreshed between scenes. otherwise reused objects will not be written to a manifest for the new scene
	void RefreshObjectManifest();
};
}