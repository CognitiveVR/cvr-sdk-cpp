/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

//Contains functions to construct Cognitive Analytics, set user and device properties, start and end sessions, set scenes, and get timestamps

#pragma once

#include "stdafx.h"
#include "network.h"
#include "transaction.h"
#include "gazetracker.h"
#include "sensor.h"
#include "tuning.h"
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

typedef void(*WebResponse) (::std::string content);
typedef void(*WebRequest) (::std::string url, ::std::string content, WebResponse response);

class DynamicObject;
class ExitPoll;
class CognitiveLog;
class GazeTracker;
class Network;
class Tuning;
class Transaction;
class Sensor;
class CoreSettings;

class COGNITIVEVRANALYTICS_API CognitiveVRAnalyticsCore
{
	friend class CognitiveLog;
	friend class Network;
	friend class Transaction;
	friend class Tuning;
	friend class Sensor;
	friend class Config;
	friend class DynamicObject;
	friend class GazeTracker;
	friend class ExitPoll;
	friend class CoreSettings;

private:
	
	::std::unique_ptr<Network> network = nullptr;
	::std::unique_ptr<Config> config = nullptr;
	
	//application_init returns expected values from dashboard
	bool bWasInitSuccessful = false;
	//after constructor and before session init has responded. allows batching data before session begins
	bool bPendingInit = true;

	double SessionTimestamp = -1;
	::std::string SessionId = "";

	WebRequest sendFunctionPointer = nullptr;
	::std::string DevicePropertyToString(EDeviceProperty propertyType);

public:

	//unique id of scene that that receives recorded data
	::std::string CurrentSceneId = "";

	::std::string UserId = "";
	nlohmann::json UserProperties = nlohmann::json();
	::std::string DeviceId = "";
	nlohmann::json DeviceProperties = nlohmann::json();

	//this may return null. constructor should call this manually before referencing instance!
	static ::std::shared_ptr<CognitiveVRAnalyticsCore> Instance();
	::std::unique_ptr<CognitiveLog> log = nullptr;
	::std::unique_ptr<Transaction> transaction = nullptr;
	::std::unique_ptr<Sensor> sensor = nullptr;
	::std::unique_ptr<Tuning> tuning = nullptr;
	::std::unique_ptr<GazeTracker> gaze = nullptr;
	::std::unique_ptr<DynamicObject> dynamicobject = nullptr;
	::std::unique_ptr<ExitPoll> exitpoll = nullptr;

	CognitiveVRAnalyticsCore(CoreSettings settings);
	CognitiveVRAnalyticsCore(const CognitiveVRAnalyticsCore&);
	
	~CognitiveVRAnalyticsCore();

	/** set a unique user id
		@param std::string user_id
	*/
	void SetUserId(::std::string user_id);
	//* set many user properties
	void SetUserProperties(nlohmann::json properties);
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
	/** confirm user properties. send to dashboard
	*/
	void UpdateUserState();

	//* set unique device id
	void SetDeviceId(::std::string device_id);
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
	//* confirm device properties. send to dashboard
	void UpdateDeviceState();

	//which organization and product this project belongs to. used to send to dashboard
	::std::string GetCustomerId();
	//used to identify which session this sends to for SceneExplorer
	double GetSessionTimestamp();
	double GetTimestamp();
	//the session timestamp and user. creates a unique session in SceneExplorer
	::std::string GetSessionID();
	
	void SetPendingInit(bool isPending);
	bool IsPendingInit();
	void SetInitSuccessful(bool success);
	bool WasInitSuccessful();

	/** start a session. returns true if successfully starting session. ie, not already started
	*/
	bool StartSession();

	/** end the session. sends all final data to dashboard and SceneExplorer
	*/
	void EndSession();

	void SendData();

	/** set which scene on SceneExplorer should recieve the recorded data
		@param std::string scenename
	*/
	void SetScene(::std::string scenename);
	::std::string GetSceneId();
};
}