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

//created when registering dynamic objects. sent to SceneExplorer
struct DynamicObjectManifestEntry
{
	friend class DynamicObject;
private:
	std::string Id = "";
	std::string Name = "";
	std::string MeshName = "";
	nlohmann::json Properties;
	bool IsController;
	std::string ControllerType = "";

	DynamicObjectManifestEntry(std::string id, std::string name, std::string mesh)
	{
		Id = id;
		Name = name;
		MeshName = mesh;
		IsController = false;
	}

	DynamicObjectManifestEntry(std::string id, std::string name, std::string mesh, nlohmann::json properties)
	{
		Id = id;
		Name = name;
		MeshName = mesh;
		Properties = properties;
		IsController = false;
	}

	DynamicObjectManifestEntry(std::string id, std::string name, std::string mesh, nlohmann::json properties, std::string controllerType)
	{
		Id = id;
		Name = name;
		MeshName = mesh;
		Properties = properties;
		if (controllerType.length() > 0)
		{
			IsController = true;
			ControllerType = controllerType;
		}
		else
		{
			IsController = false;
		}
	}
};

//used in the client to track which ids are used and which can be reused
struct DynamicObjectId
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

//dictionary of these in dynamic snapshot to record changes to controller inputs
struct COGNITIVEVRANALYTICS_API ControllerInputState
{
	friend class DynamicObject;
	friend class DynamicObjectSnapshot;
private:
	std::string AxisName;
	float AxisValue;
	bool IsJoystick;
	float X;
	float Y;

public:
	ControllerInputState(std::string AxisName, float AxisValue);
	ControllerInputState(std::string AxisName, float AxisValue, float X, float Y);
};

//the state of an object at a time
struct DynamicObjectSnapshot
{
	friend class DynamicObject;
private:
	std::vector<float> Position;
	std::vector<float> Rotation;
	bool useScale = true;
	std::vector<float> Scale;
	double Time = -1;
	std::string Id = "";
	nlohmann::json Properties = nlohmann::json();
	nlohmann::json Engagements = nlohmann::json();
	nlohmann::json Buttons = nlohmann::json();

	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::string objectId);
	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::string objectId, nlohmann::json properties);

	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string objectId);
	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string objectId, nlohmann::json properties);

	//for dynamic object controller type
	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::string objectId, std::vector<ControllerInputState> inputStates);
	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::string objectId, nlohmann::json properties, std::vector<ControllerInputState> inputStates);

	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string objectId, std::vector<ControllerInputState> inputStates);
	DynamicObjectSnapshot(std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string objectId, nlohmann::json properties, std::vector<ControllerInputState> inputStates);
};

//an interaction the player has with a dynamic object
struct DynamicObjectEngagementEvent
{
	friend class DynamicObject;
public:
	bool isActive = true;
private:
	double startTime = -1;
	double endTime = -1;
	std::string Name = "";
	std::string ParentObjectId = "";
	int EngagementNumber = 0;

	DynamicObjectEngagementEvent(std::string parentId, std::string engagementName, int engagementNumber);
};

class COGNITIVEVRANALYTICS_API DynamicObject
{
	friend class CognitiveVRAnalyticsCore;
protected:
	std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;

	int jsonpart = 1;

	int generatedIdOffset = 1000;
	int nextObjectId = 0;


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

	std::map<std::string, std::map<std::string, std::string>> controllerInputs;

	DynamicObject(std::shared_ptr<CognitiveVRAnalyticsCore> cog);

	void RegisterObjectCustomId_Internal(std::string name, std::string meshname, std::string customid, std::vector<float> position, std::vector<float> rotation, std::string controllerName, bool isRight);
	std::string RegisterObject_Internal(std::string name, std::string meshname, std::vector<float> position, std::vector<float> rotation, std::string controllerName, bool isRight);
	void RecordDynamic_Internal(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, bool useScale, nlohmann::json properties, std::vector<ControllerInputState> inputs);

	/** end all engagements on an object. to be used if the object is destroyed. requires a snapshot of the dynamic object to send engagement data!

		@param int objectid
	*/
	void EndActiveEngagements(std::string objectid);

	//object ids are refreshed between scenes. otherwise reused objects will not be written to a manifest for the new scene
	void RefreshObjectManifest();

	void EndSession();

	//TODO optional lightweight struct to use to register a dynamic instead of a bunch of parameters
public:
	/** put into dynamic manifest. reuses or creates new objectid. returns objectid. prefer using custom id when possible. also adds a snapshot with the property 'enabled'
	@param std::string name
	@param std::string meshname
	@param std::string customid
	@param std::vector<float> position
	@param std::vector<float> rotation
	*/
	void RegisterObjectCustomId(std::string name, std::string meshname, std::string customid, std::vector<float> position, std::vector<float> rotation);
	/** put into dynamic manifest. reuses or creates new objectid. returns objectid. prefer using custom id when possible. also adds a snapshot with the property 'enabled'
	@param std::string name
	@param std::string meshname
	@param std::string customid
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::vector<float> scale
	*/
	void RegisterObjectCustomId(std::string name, std::string meshname, std::string customid, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale);
	/** put into dynamic manifest. reuses or creates new objectid. returns objectid. prefer using custom id when possible. also adds a snapshot with the property 'enabled'
	@param std::string name
	@param std::string meshname
	@param std::string customid
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::string controllerType. either vivecontroller, oculustouchleft or oculustouchright
	@param bool isRight. if this dynamic represents the user's right hand
	*/
	void RegisterObjectCustomId(std::string name, std::string meshname, std::string customid, std::vector<float> position, std::vector<float> rotation, std::string controllertype, bool isRight);
	/** put into dynamic manifest. reuses or creates new objectid. returns objectid. prefer using custom id when possible. also adds a snapshot with the property 'enabled'
	@param std::string name
	@param std::string meshname
	@param std::string customid
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::vector<float> scale
	@param std::string controllerType. either vivecontroller, oculustouchleft or oculustouchright
	@param bool isRight. if this dynamic represents the user's right hand
	*/
	void RegisterObjectCustomId(std::string name, std::string meshname, std::string customid, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string controllertype, bool isRight);

	/** put into dynamic manifest. reuses or creates new objectid. returns objectid. prefer using custom id when possible. also adds a snapshot with the property 'enabled'
	@param std::string name
	@param std::string meshname
	@param std::vector<float> position
	@param std::vector<float> rotation
	*/
	std::string RegisterObject(std::string name, std::string meshname, std::vector<float> position, std::vector<float> rotation);
	/** put into dynamic manifest. reuses or creates new objectid. returns objectid. prefer using custom id when possible. also adds a snapshot with the property 'enabled'
	@param std::string name
	@param std::string meshname
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::vector<float> scale
	*/
	std::string RegisterObject(std::string name, std::string meshname, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale);
	/** put into dynamic manifest. reuses or creates new objectid. returns objectid. prefer using custom id when possible. also adds a snapshot with the property 'enabled'
	@param std::string name
	@param std::string meshname
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::string controllerType. either vivecontroller, oculustouchleft or oculustouchright
	@param bool isRight. if this dynamic represents the user's right hand
	*/
	std::string RegisterObject(std::string name, std::string meshname, std::vector<float> position, std::vector<float> rotation, std::string controllerType, bool isRight);
	/** put into dynamic manifest. reuses or creates new objectid. returns objectid. prefer using custom id when possible. also adds a snapshot with the property 'enabled'
	@param std::string name
	@param std::string meshname
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::vector<float> scale
	@param std::string controllerType. either vivecontroller, oculustouchleft or oculustouchright
	@param bool isRight. if this dynamic represents the user's right hand
	*/
	std::string RegisterObject(std::string name, std::string meshname, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string controllerType, bool isRight);


	/** record the position, rotation and other properties of an object
	@param std::string objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation);
	/** record the position, rotation and other properties of an object
	@param std::string objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param cognitive::nlohmann::json properties
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties);
	/** record the position, rotation and other properties of an object
	@param std::string objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::vector<float> scale
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale);
	/** record the position, rotation and other properties of an object
	@param std::string objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::vector<float> scale
	@param cognitive::nlohmann::json properties
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, nlohmann::json properties);

	/** record the position, rotation and other properties of an object
	@param std::string objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::string inputName - the name of the controller input. valid names are vive_trigger, vive_grip, vive_menubtn,vive_homebtn, rift_abtn, rift_bbtn, rift_xbtn, rift_ybtn, rift_thumbrest, rift_start, rift_trigger, rift_grip
	@param std::string inputValue - the value of the input. between 0 and 100
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::string inputName, float inputValue);
	/** record the position, rotation and other properties of an object
	@param std::string objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param cognitive::nlohmann::json properties
	@param std::string inputName - the name of the controller input. valid names are vive_trigger, vive_grip, vive_menubtn,vive_homebtn, rift_abtn, rift_bbtn, rift_xbtn, rift_ybtn, rift_thumbrest, rift_start, rift_trigger, rift_grip
	@param std::string inputValue - the value of the input. between 0 and 100
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties, std::string inputName, float inputValue);
	/** record the position, rotation and other properties of an object
	@param std::string objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::vector<float> scale
	@param std::string inputName - the name of the controller input. valid names are vive_trigger, vive_grip, vive_menubtn,vive_homebtn, rift_abtn, rift_bbtn, rift_xbtn, rift_ybtn, rift_thumbrest, rift_start, rift_trigger, rift_grip
	@param std::string inputValue - the value of the input. between 0 and 100
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string inputName, float inputValue);
	/** record the position, rotation and other properties of an object
	@param std::string objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::vector<float> scale
	@param cognitive::nlohmann::json properties
	@param std::string inputName - the name of the controller input. valid names are vive_trigger, vive_grip, vive_menubtn,vive_homebtn, rift_abtn, rift_bbtn, rift_xbtn, rift_ybtn, rift_thumbrest, rift_start, rift_trigger, rift_grip
	@param std::string inputValue - the value of the input. between 0 and 100
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, nlohmann::json properties, std::string inputName, float inputValue);

	/** record the position, rotation and other properties of an object
	@param std::string objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::string inputName - the name of the controller input. valid names are vive_touchpad and rift_joystick
	@param std::string inputValue - the amount this input has been pressed. 0 for nothing. 0.5 for touch. 1 for press
	@param std::string x - the x position of the input. between -1 and 1
	@param std::string y - the y position of the input. between -1 and 1
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::string inputName, float inputValue, float x, float y);
	/** record the position, rotation and other properties of an object
	@param std::string objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param cognitive::nlohmann::json properties
	@param std::string inputName - the name of the controller input. valid names are vive_touchpad and rift_joystick
	@param std::string inputValue - the amount this input has been pressed. 0 for nothing. 0.5 for touch. 1 for press
	@param std::string x - the x position of the input. between -1 and 1
	@param std::string y - the y position of the input. between -1 and 1
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties, std::string inputName, float inputValue, float x, float y);
	/** record the position, rotation and other properties of an object
	@param std::string objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::vector<float> scale
	@param std::string inputName - the name of the controller input. valid names are vive_touchpad and rift_joystick
	@param std::string inputValue - the amount this input has been pressed. 0 for nothing. 0.5 for touch. 1 for press
	@param std::string x - the x position of the input. between -1 and 1
	@param std::string y - the y position of the input. between -1 and 1
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::string inputName, float inputValue, float x, float y);
	/** record the position, rotation and other properties of an object
	@param std::string objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::vector<float> scale
	@param cognitive::nlohmann::json properties
	@param std::string inputName - the name of the controller input. valid names are vive_touchpad and rift_joystick
	@param std::string inputValue - the amount this input has been pressed. 0 for nothing. 0.5 for touch. 1 for press
	@param std::string x - the x position of the input. between -1 and 1
	@param std::string y - the y position of the input. between -1 and 1
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, nlohmann::json properties, std::string inputName, float inputValue, float x, float y);

	/** record the position, rotation and other properties of an object
	@param int objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::vector<cognitive::ControllerInputState> inputState. records any number of input states (ie button presses, thumbstick positions) in a frame
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<ControllerInputState> inputState);
	/** record the position, rotation and other properties of an object
	@param int objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param cognitive::nlohmann::json properties
	@param std::vector<cognitive::ControllerInputState> inputState. records any number of input states (ie button presses, thumbstick positions) in a frame
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, nlohmann::json properties, std::vector<ControllerInputState> inputState);
	/** record the position, rotation and other properties of an object
	@param int objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::vector<float> scale
	@param std::vector<cognitive::ControllerInputState> inputState. records any number of input states (ie button presses, thumbstick positions) in a frame
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, std::vector<ControllerInputState> inputState);
	/** record the position, rotation and other properties of an object
	@param int objectId
	@param std::vector<float> position
	@param std::vector<float> rotation
	@param std::vector<float> scale
	@param cognitive::nlohmann::json properties
	@param std::vector<cognitive::ControllerInputState> inputState. records any number of input states (ie button presses, thumbstick positions) in a frame
	*/
	void RecordDynamic(std::string objectId, std::vector<float> position, std::vector<float> rotation, std::vector<float> scale, nlohmann::json properties, std::vector<ControllerInputState> inputState);


	/** add engagement to dynamic object. requires a snapshot of the dynamic object to send engagement data!
	@param std::string objectid of the object being engaged with
	@param std::string name of the engagement ex "grab"
	*/
	void BeginEngagement(std::string objectId, std::string name);

	/** add engagement to dynamic object. requires a snapshot of the dynamic object to send engagement data!
	@param std::string objectid of the object being engaged with
	@param std::string name of the engagement ex "grab"
	@param std::string parentId unique identifier for the engagement
	*/
	void BeginEngagement(std::string objectId, std::string name, std::string parentId);

	/** end engagement on dynamic object. immediately begins and ends if engagement does not already exist. requires a snapshot of the dynamic object to send engagement data!
	@param std::string objectid of the object the engagement is being ended with
	@param std::string name of the engagement ex "grab"
	*/
	void EndEngagement(std::string objectId, std::string name);

	/** end engagement on dynamic object. immediately begins and ends if engagement does not already exist. requires a snapshot of the dynamic object to send engagement data!
	@param std::string objectid of the object the engagement is being ended with
	@param std::string name of the engagement ex "grab"
	@param std::string parentId unique identifier for the engagement
	*/
	void EndEngagement(std::string objectId, std::string name, std::string parentId);

	/**deregister dynamic object and recycles objectid. don't need to do this for objects that were registered without a custom id. also sends a snapshot

	@param int objectid
	@param std::vector<float> position
	@param std::vector<float> rotation
	*/
	void RemoveObject(std::string objectid, std::vector<float> position, std::vector<float> rotation);

	nlohmann::json SendData();
};

}