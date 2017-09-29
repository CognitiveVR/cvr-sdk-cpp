
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

	//using json = nlohmann::json;
typedef void(*WebResponse) (::std::string content);
typedef void(*WebRequest) (::std::string url, ::std::string content, WebResponse response);

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

private:
	
	::std::unique_ptr<Network> network = nullptr;
	::std::unique_ptr<Config> config = nullptr;
	

	bool bHasSessionStarted = false;
	bool bWasInitSuccessful = true; //set to false if there was an error in response
	double SessionTimestamp = -1;

	::std::string CurrentSceneKey = "";
	::std::string UserId = "";
	nlohmann::json UserProperties = nlohmann::json();
	::std::string DeviceId = "";
	nlohmann::json DeviceProperties = nlohmann::json();
	::std::string SessionId = "";

	WebRequest sendFunctionPointer = nullptr;

public:

	static ::std::shared_ptr<CognitiveVRAnalyticsCore> Instance();
	::std::unique_ptr<CognitiveLog> log = nullptr;
	::std::unique_ptr<Transaction> transaction = nullptr;
	::std::unique_ptr<Sensor> sensor = nullptr;
	::std::unique_ptr<Tuning> tuning = nullptr;
	::std::unique_ptr<GazeTracker> gaze = nullptr;
	::std::unique_ptr<DynamicObject> dynamicobject = nullptr;
	::std::unique_ptr<ExitPoll> exitpoll = nullptr;

	CognitiveVRAnalyticsCore(WebRequest sendFunc);
	CognitiveVRAnalyticsCore(WebRequest sendFunc, ::std::map<::std::string, ::std::string> sceneids);
	CognitiveVRAnalyticsCore(WebRequest sendFunc, ::std::string customerid, int gazecount, int eventcount, int sensorcount, int dynamiccount, ::std::map < ::std::string, ::std::string> sceneids);
	CognitiveVRAnalyticsCore(const CognitiveVRAnalyticsCore&);
	//CognitiveVRAnalyticsCore& operator=(CognitiveVRAnalyticsCore&&) = default;
	
	~CognitiveVRAnalyticsCore();

	void SetUser(::std::string user_id, nlohmann::json properties);
	void SetDevice(::std::string device_id, nlohmann::json properties);
	::std::string GetCustomerId();

	double GetSessionTimestamp();
	double GetTimestamp();
	::std::string GetSessionID();
	
	void SetHasStartedSession(bool started);
	bool HasStartedSession();
	void SetInitSuccessful(bool success);
	bool WasInitSuccessful();

	//returns true if successfully starting session. ie, not already started
	bool StartSession();
	void EndSession();

	void SendData();

	void SetScene(::std::string sceneName);
	::std::string GetSceneKey();
};
}