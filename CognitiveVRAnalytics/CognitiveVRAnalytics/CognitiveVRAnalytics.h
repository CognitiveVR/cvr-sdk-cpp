#pragma once

#include "stdafx.h"
#include "network.h"
#include "transaction.h"
#include "gazetracker.h"
#include "sensor.h"
#include "cognitive_log.h"
#include "config.h"
#include <chrono>
using json = nlohmann::json;

#ifdef COGNITIVEANALYTICS_EXPORTS  
#define COGNITIVEANALYTICS_API __declspec(dllexport)   
#else  
#define COGNITIVEANALYTICS_API __declspec(dllimport)
#endif

typedef void(*WebResponse) (std::string content);
typedef void(*WebRequest) (std::string url, std::string content, WebResponse response);

class COGNITIVEANALYTICS_API CognitiveVRAnalyticsCore
{
	friend class CognitiveLog;
	friend class Network;
	friend class Transaction;
	friend class Tuning;
	friend class Sensor;
	friend class Config;
	friend class GazeTracker;

private:
	
	Network* network;
	Config* config;
	

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

	static CognitiveVRAnalyticsCore* Instance();
	CognitiveLog* log;
	Transaction* transaction;
	Sensor* sensor;
	Tuning* tuning;
	GazeTracker* gaze;

	CognitiveVRAnalyticsCore(WebRequest sendFunc);
	CognitiveVRAnalyticsCore(WebRequest sendFunc, std::string customerid, int gazecount, int eventcount, int sensorcount, int dynamiccount, std::map < std::string, std::string> sceneids);
	~CognitiveVRAnalyticsCore();

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