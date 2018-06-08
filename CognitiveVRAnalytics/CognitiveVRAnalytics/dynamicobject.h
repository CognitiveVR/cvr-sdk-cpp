/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

//Register and record objects that created, moved or changed during the experience. Also record being and end engagements

#pragma once

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"

namespace cognitive
{
class CognitiveVRAnalyticsCore;

enum class CommonMeshName
{
	kViveController,
	kOculusTouchLeft,
	kOculusTouchRight,
	kViveTracker
};

//created when registering dynamic objects. sent to SceneExplorer
class DynamicObjectManifestEntry
{
	friend class DynamicObject;
private:
	std::string Id = "";
	std::string Name = "";
	std::string MeshName = "";
	//json Properties

	DynamicObjectManifestEntry(std::string id, std::string name, std::string mesh)
	{
		Id = id;
		Name = name;
		MeshName = mesh;
	}
};

//used in the client to track which ids are used and which can be reused
class DynamicObjectId
{
	friend class DynamicObject;
private:
	std::string Id = "";
	bool Used = true;
	std::string MeshName = "";

	DynamicObjectId(std::string id, std::string meshName)
	{
		Id = id;
		MeshName = meshName;
	}
};

//the state of an object at a time
struct DynamicObjectSnapshot
{
	friend class DynamicObject;
private:
	std::vector<float> Position;
	std::vector<float> Rotation;
	double Time = -1;
	std::string Id = "";
	nlohmann::json Properties = nlohmann::json();
	nlohmann::json Engagements = nlohmann::json();

	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::string objectId);
	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::string objectId, nlohmann::json properties);
};

//an interaction the player has with a dynamic object
class DynamicObjectEngagementEvent
{
	friend class DynamicObject;
public:
	bool isActive = true;
private:
	double startTime = -1;
	double endTime = -1;
	std::string Name = "";
	std::string ObjectId = "";
	int EngagementNumber = 0;

	DynamicObjectEngagementEvent(std::string id, std::string engagementName, int engagementNumber);
};

class COGNITIVEVRANALYTICS_API DynamicObject
{
	friend class CognitiveVRAnalyticsCore;
private:
	std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;

	int jsonpart = 1;

	int generatedIdOffset = 1000;

	//public for testing, but shouldn't be used outside this class

	//tracks all used ids in client
	std::vector<DynamicObjectId> objectIds;
	//oustanding snapshots of dynamic objects. cleared on send
	std::vector<DynamicObjectSnapshot> snapshots;
	//all objects that have existed in the scene. resent on scene change
	std::vector<DynamicObjectManifestEntry> fullManifest;
	//all objects that have not yet been sent to SceneExplorer. cleared on send
	std::vector<DynamicObjectManifestEntry> manifestEntries;

	//engagements that are currently active
	std::map<std::string, std::vector<DynamicObjectEngagementEvent*>> activeEngagements;
	//all engagements that need to be written to snapshots. active or inactive. inactive engagements are removed after being sent
	std::map<std::string, std::vector<DynamicObjectEngagementEvent*>> allEngagements;
	//count of engagements on dynamic objects of type
	std::map<std::string, std::map < std::string, int >> engagementCounts;


	DynamicObject(std::shared_ptr<CognitiveVRAnalyticsCore> cog);

	/** end all engagements on an object. to be used if the object is destroyed. requires a snapshot of the dynamic object to send engagement data!

		@param int objectid
	*/
	void EndActiveEngagements(std::string objectid);

	//object ids are refreshed between scenes. otherwise reused objects will not be written to a manifest for the new scene
	void RefreshObjectManifest();

	void EndSession();

public:
	/** put into dynamic manifest with an id. returns objectid. also adds a snapshot with the property 'enabled'

	@param std::string name
	@param std::string meshname
	@param int customid
	*/
	void RegisterObjectCustomId(std::string name, std::string meshname, std::string customid, std::vector<float> position, std::vector<float> rotation);

	//
	/** put into dynamic manifest. reuses or creates new objectid. returns objectid. prefer using custom id when possible. also adds a snapshot with the property 'enabled'

	@param std::string name
	@param std::string meshname
	*/
	std::string RegisterObject(std::string name, std::string meshname, std::vector<float> position, std::vector<float> rotation);

	/** record the position, rotation and other properties of an object

	@param int objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param nlohmann::json properties - Optional
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation);
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties);

	[[deprecated("Use RecordDynamic instead")]]
	void AddSnapshot(std::string objectId, std::vector<float> position, std::vector<float> rotation);
	[[deprecated("Use RecordDynamic instead")]]
	void AddSnapshot(std::string objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties);

	/** add engagement to dynamic object. requires a snapshot of the dynamic object to send engagement data!

	@param int objectid
	@param std::string name
	*/
	void BeginEngagement(std::string objectId, std::string name);

	/** end engagement on dynamic object. immediately begins and ends if engagement does not already exist. requires a snapshot of the dynamic object to send engagement data!

	@param int objectid
	@param std::string name
	*/
	void EndEngagement(std::string objectId, std::string name);

	/**deregister dynamic object and recycles objectid. don't need to do this for objects that were registered without a custom id. also sends a snapshot

	@param int objectid
	@param std::vector<float> position
	@param std::vector<float> rotation
	*/
	void RemoveObject(std::string objectid, std::vector<float> position, std::vector<float> rotation);

	nlohmann::json SendData();
};
}