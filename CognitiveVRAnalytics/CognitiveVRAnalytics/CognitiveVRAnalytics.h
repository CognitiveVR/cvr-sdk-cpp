/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

//Contains functions to construct Cognitive Analytics, set user and device properties, start and end sessions, set scenes, and get timestamps

#pragma once

#include "stdafx.h"
#include "network.h"
#include "customevent.h"
#include "gazetracker.h"
#include "sensor.h"
#include "exitpoll.h"
#include "dynamicobject.h"
#include "cognitive_log.h"
#include "config.h"
#include "coresettings.h"
#include <chrono>


#if defined(_MSC_VER)
	//  Microsoft 
	#ifdef COGNITIVEVRANALYTICS_EXPORTS  
		#define COGNITIVEVRANALYTICS_API __declspec(dllexport)
	#else  
		#define COGNITIVEVRANALYTICS_API __declspec(dllimport)
	#endif
#elif defined(__GNUC__)
	//  GCC
	#ifdef COGNITIVEVRANALYTICS_EXPORTS  
		#define COGNITIVEVRANALYTICS_API __attribute__((visibility("default")))
	#else  
		#define COGNITIVEVRANALYTICS_API
	#endif
#else
	//  do nothing and hope for the best?
	#define COGNITIVEVRANALYTICS_EXPORTS
	#pragma warning Unknown dynamic link import/export semantics.
#endif



namespace cognitive {

//std::make_unique is added in c++14. this is a rough implementation for c++11
template<typename T, typename... Args>
std::unique_ptr<T> make_unique_cognitive(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

enum class EDeviceProperty
{
	kAppName, //(string) your app name
	kAppVersion, //(string) 1.0
	kAppEngine, //(string) custom cpp engine
	kAppEngineVersion, //(string) v1.1.25a
	
	kDeviceType, //(string) mobile, desktop
	kDeviceModel, //(string) iphone 7, pixel
	kDeviceMemory, //(int) 8
	kDeviceOS, //(string) ios 11.0, 6.0.1
	
	kDeviceCPUCores, //(int) 4
	kDeviceCPU, //(string) i7-4770 CPU @ 3.40GHz
	kDeviceCPUVendor, //(string) intel
	
	kDeviceGPU, //(string) GeForce GTX 970
	kDeviceGPUDriver, //(string) 382.05
	kDeviceGPUVendor, //(string) nvidia, amd
	kDeviceGPUMemory, //(int) 6103
	
	kVRModel, //(string) oculus rift dk2
	kVRVendor //(string) oculus
};

class DynamicObject;
class ExitPoll;
class CognitiveLog;
class GazeTracker;
class Network;
class CustomEvent;
class Sensor;
class CoreSettings;
//class SceneData;

class COGNITIVEVRANALYTICS_API CognitiveVRAnalyticsCore
{
	friend class CognitiveLog;
	friend class Network;
	friend class CustomEvent;
	friend class Sensor;
	friend class Config;
	friend class DynamicObject;
	friend class GazeTracker;
	friend class ExitPoll;
	friend class CoreSettings;

private:
	
	::std::unique_ptr<Network> network = nullptr;
	::std::unique_ptr<Config> config = nullptr;

	double SessionTimestamp = -1;
	::std::string SessionId = "";

	WebRequest sendFunctionPointer = nullptr;
	::std::string DevicePropertyToString(EDeviceProperty propertyType);

	bool isSessionActive = false;

	::std::string UserId = "";
	::std::string DeviceId = "";

public:

	//unique id of scene that that receives recorded data
	::std::string CurrentSceneId = "";
	::std::string CurrentSceneVersionNumber = "";

	//this may return null. constructor should call this manually before referencing instance!
	static ::std::shared_ptr<CognitiveVRAnalyticsCore> Instance();
	::std::unique_ptr<CognitiveLog> log = nullptr;
	::std::unique_ptr<CustomEvent> customevent = nullptr;
	::std::unique_ptr<Sensor> sensor = nullptr;
	::std::unique_ptr<GazeTracker> gaze = nullptr;
	::std::unique_ptr<DynamicObject> dynamicobject = nullptr;
	::std::unique_ptr<ExitPoll> exitpoll = nullptr;

	CognitiveVRAnalyticsCore(CoreSettings settings);
	CognitiveVRAnalyticsCore(const CognitiveVRAnalyticsCore&);
	
	~CognitiveVRAnalyticsCore();

	std::map<std::string, std::string> NewUserProperties;
	std::map<std::string, std::string> NewDeviceProperties;

	void SetUserName(std::string name);

	/** set single user property
	@param std::string propertyType
	@param int value
	*/
	void SetUserProperty(::std::string propertyType, int value);
	/** set single user property
		@param std::string propertyType
		@param float value
	*/
	void SetUserProperty(::std::string propertyType, float value);
	/** set single user property
		@param std::string propertyType
		@param std::string value
	*/
	void SetUserProperty(::std::string propertyType, ::std::string value);
	
	//returns new device properties map. clears new properties
	std::map<std::string,std::string> GetUserProperties();


	void SetDeviceName(std::string name);
	/** set single device property
		@param EDeviceProperty propertyType
		@param int value
	*/
	void SetDeviceProperty(EDeviceProperty propertyType, int value);
	/** set single device property
		@param EDeviceProperty propertyType
		@param std::string value
	*/
	void SetDeviceProperty(EDeviceProperty propertyType, ::std::string value);

	//returns new device properties map. clears new properties
	std::map<std::string, std::string> GetDeviceProperties();



	//the identifying key for the product. used to send to sceneexplorer
	::std::string GetAPIKey();
	//used to identify the session start
	double GetSessionTimestamp();
	//returns the current timestamp
	double GetTimestamp();
	//the session timestamp and userid. creates a unique session in SceneExplorer
	::std::string GetSessionID();
	
	bool IsSessionActive();

	//TODO this should only return true if IsSessionActive and network can access sceneexplorer
	//OBSOLETE this returns true if session has begun
	bool WasInitSuccessful();

	/** start a session. returns true if successfully starting session. ie, not already started
	*/
	bool StartSession();

	/** end the session. sends all final data to SceneExplorer
	*/
	void EndSession();

	//must have started session before sending
	//send all outstanding data to scene. if scene doesn't exist, clear data - it's not relevant to other scenes anyway
	void SendData();

	/** set which scene on SceneExplorer should recieve the recorded data
		if the current scene is not null, this will also send data before changing the active scene
		@param std::string scenename
	*/
	void SetScene(::std::string scenename);
	::std::string GetSceneId();
};
}