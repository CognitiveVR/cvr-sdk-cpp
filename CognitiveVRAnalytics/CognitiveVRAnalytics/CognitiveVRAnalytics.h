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
	
	std::unique_ptr<Network> network;
	std::unique_ptr<Config> config;
	

	bool bHasSessionStarted;
	bool bWasInitSuccessful = true; //set to false if there was an error in response
	double SessionTimestamp;

	std::string CurrentSceneKey = "";
	std::string UserId;
	json UserProperties;
	std::string DeviceId;
	json DeviceProperties;
	std::string SessionId;

	WebRequest sendFunctionPointer;

public:

	static std::shared_ptr<CognitiveVRAnalyticsCore> Instance();
	std::unique_ptr<CognitiveLog> log;
	std::unique_ptr<Transaction> transaction;
	std::unique_ptr<Sensor> sensor;
	std::unique_ptr<Tuning> tuning;
	std::unique_ptr<GazeTracker> gaze;
	std::unique_ptr<DynamicObject> dynamicobject;
	std::unique_ptr<ExitPoll> exitpoll;

	CognitiveVRAnalyticsCore(WebRequest sendFunc);
	CognitiveVRAnalyticsCore(WebRequest sendFunc, std::string customerid, int gazecount, int eventcount, int sensorcount, int dynamiccount, std::map < std::string, std::string> sceneids);
	//CognitiveVRAnalyticsCore(const CognitiveVRAnalyticsCore&) = default;
	//CognitiveVRAnalyticsCore& operator=(CognitiveVRAnalyticsCore&&) = default;
	//~CognitiveVRAnalyticsCore();

	void SetUser(std::string user_id, json* properties);
	void SetDevice(std::string device_id, json* properties);
	std::string GetCustomerId();

	double GetSessionTimestamp();
	double GetTimestamp();
	std::string GetSessionID();
	
	void SetHasStartedSession(bool started);
	bool HasStartedSession();
	void SetInitSuccessful(bool success);
	bool WasInitSuccessful();

	bool StartSession();
	void EndSession();

	void SendData();

	void SetScene(std::string sceneName);
	std::string GetSceneKey();
};