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
using json = nlohmann::json;

#ifdef COGNITIVEVRANALYTICS_EXPORTS  
#define COGNITIVEVRANALYTICS_API __declspec(dllexport)
#else  
#define COGNITIVEVRANALYTICS_API __declspec(dllimport)
#endif

typedef void(*WebResponse) (std::string content);
typedef void(*WebRequest) (std::string url, std::string content, WebResponse response);

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
	
	std::unique_ptr<Network> network = NULL;
	std::unique_ptr<Config> config = NULL;
	

	bool bHasSessionStarted = false;
	bool bWasInitSuccessful = true; //set to false if there was an error in response
	double SessionTimestamp = -1;

	std::string CurrentSceneKey = "";
	std::string UserId = "";
	json UserProperties = json();
	std::string DeviceId = "";
	json DeviceProperties = json();
	std::string SessionId = "";

	WebRequest sendFunctionPointer = NULL;

public:

	static std::shared_ptr<CognitiveVRAnalyticsCore> Instance();
	std::unique_ptr<CognitiveLog> log = NULL;
	std::unique_ptr<Transaction> transaction = NULL;
	std::unique_ptr<Sensor> sensor = NULL;
	std::unique_ptr<Tuning> tuning = NULL;
	std::unique_ptr<GazeTracker> gaze = NULL;
	std::unique_ptr<DynamicObject> dynamicobject = NULL;
	std::unique_ptr<ExitPoll> exitpoll = NULL;

	CognitiveVRAnalyticsCore(WebRequest sendFunc);
	CognitiveVRAnalyticsCore(WebRequest sendFunc, std::map<std::string, std::string> sceneids);
	CognitiveVRAnalyticsCore(WebRequest sendFunc, std::string customerid, int gazecount, int eventcount, int sensorcount, int dynamiccount, std::map < std::string, std::string> sceneids);
	CognitiveVRAnalyticsCore(const CognitiveVRAnalyticsCore&);
	//CognitiveVRAnalyticsCore& operator=(CognitiveVRAnalyticsCore&&) = default;
	
	~CognitiveVRAnalyticsCore();

	void SetUser(std::string user_id, json properties);
	void SetDevice(std::string device_id, json properties);
	std::string GetCustomerId();

	double GetSessionTimestamp();
	double GetTimestamp();
	std::string GetSessionID();
	
	void SetHasStartedSession(bool started);
	bool HasStartedSession();
	void SetInitSuccessful(bool success);
	bool WasInitSuccessful();

	//returns true if successfully starting session. ie, not already started
	bool StartSession();
	void EndSession();

	void SendData();

	void SetScene(std::string sceneName);
	std::string GetSceneKey();
};