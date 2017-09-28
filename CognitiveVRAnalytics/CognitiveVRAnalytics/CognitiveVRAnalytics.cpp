
// CognitiveAnalytics.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"
namespace cognitive {
static ::std::shared_ptr<CognitiveVRAnalyticsCore> instance;

::std::shared_ptr<CognitiveVRAnalyticsCore> CognitiveVRAnalyticsCore::Instance()
{
	return instance;
}

CognitiveVRAnalyticsCore::CognitiveVRAnalyticsCore(WebRequest sendFunc)
{
	instance = ::std::shared_ptr<CognitiveVRAnalyticsCore>(this, [](auto p) {});

	sendFunctionPointer = sendFunc;
	bHasSessionStarted = false;

	log = make_unique_cognitive<CognitiveLog>(CognitiveLog(instance));
	log->Info("CognitiveVRAnalyticsCore()");
	
	config = make_unique_cognitive<Config>(Config(instance));
	network = make_unique_cognitive<Network>(Network(instance));

	tuning = make_unique_cognitive<Tuning>(Tuning(instance));
	transaction = make_unique_cognitive<Transaction>(Transaction(instance));
	gaze = make_unique_cognitive<GazeTracker>(GazeTracker(instance));
	sensor = make_unique_cognitive<Sensor>(Sensor(instance));
	dynamicobject = make_unique_cognitive<DynamicObject>(DynamicObject(instance));
	exitpoll = make_unique_cognitive<ExitPoll>(ExitPoll(instance));
}

CognitiveVRAnalyticsCore::CognitiveVRAnalyticsCore(WebRequest sendFunc, ::std::map<::std::string, ::std::string> sceneids)
{
	instance = ::std::shared_ptr<CognitiveVRAnalyticsCore>(this, [](auto p) {});

	sendFunctionPointer = sendFunc;
	bHasSessionStarted = false;

	log = make_unique_cognitive<CognitiveLog>(CognitiveLog(instance));
	log->Info("CognitiveVRAnalyticsCore()");

	config = make_unique_cognitive<Config>(Config(instance));
	config->sceneIds = sceneids;
	network = make_unique_cognitive<Network>(Network(instance));

	tuning = make_unique_cognitive<Tuning>(Tuning(instance));
	transaction = make_unique_cognitive<Transaction>(Transaction(instance));
	gaze = make_unique_cognitive<GazeTracker>(GazeTracker(instance));
	sensor = make_unique_cognitive<Sensor>(Sensor(instance));
	dynamicobject = make_unique_cognitive<DynamicObject>(DynamicObject(instance));
	exitpoll = make_unique_cognitive<ExitPoll>(ExitPoll(instance));
}

CognitiveVRAnalyticsCore::CognitiveVRAnalyticsCore(WebRequest sendFunc, ::std::string customerid, int gazecount, int eventcount, int sensorcount, int dynamiccount, ::std::map<::std::string, ::std::string> sceneids)
{
	instance = ::std::shared_ptr<CognitiveVRAnalyticsCore>(this, [](auto p) {});

	sendFunctionPointer = sendFunc;
	bHasSessionStarted = false;

	log = make_unique_cognitive<CognitiveLog>(CognitiveLog(instance));
	log->Info("CognitiveVRAnalyticsCore()");

	config = make_unique_cognitive<Config>(Config(instance));
	config->CustomerId = customerid;
	config->GazeBatchSize = gazecount;
	config->TransactionBatchSize = eventcount;
	config->SensorDataLimit = sensorcount;
	config->DynamicDataLimit = dynamiccount;
	config->sceneIds = sceneids;
	network = make_unique_cognitive<Network>(Network(instance));

	tuning = make_unique_cognitive<Tuning>(Tuning(instance));
	transaction = make_unique_cognitive<Transaction>(Transaction(instance));
	gaze = make_unique_cognitive<GazeTracker>(GazeTracker(instance));
	sensor = make_unique_cognitive<Sensor>(Sensor(instance));
	dynamicobject = make_unique_cognitive<DynamicObject>(DynamicObject(instance));
	exitpoll = make_unique_cognitive<ExitPoll>(ExitPoll(instance));
}

CognitiveVRAnalyticsCore::~CognitiveVRAnalyticsCore()
{
	log->Info("~CognitiveVRAnalyticsCore");
	//delete a bunch of stuff
	config.reset();
	log.reset();
	network.reset();
	transaction.reset();
	tuning.reset();
	sensor.reset();
	gaze.reset();
	dynamicobject.reset();
	exitpoll.reset();
	instance.reset();
}

void CognitiveVRAnalyticsCore::SetHasStartedSession(bool started)
{
	bHasSessionStarted = started;
}

bool CognitiveVRAnalyticsCore::HasStartedSession()
{
	return bHasSessionStarted;
}

void CognitiveVRAnalyticsCore::SetInitSuccessful(bool success)
{
	bWasInitSuccessful = success;
}

bool CognitiveVRAnalyticsCore::WasInitSuccessful()
{
	return bWasInitSuccessful;
}

bool CognitiveVRAnalyticsCore::StartSession()
{
	if (bHasSessionStarted)
	{
		return false;
	}

	log->Info("CognitiveVRAnalytics::StartSession");

	if (UserId.empty())
	{
		SetUser("anonymous", nlohmann::json());
	}

	if (DeviceId.empty())
	{
		SetDevice("unknown", nlohmann::json());
	}

	GetSessionTimestamp();
	GetSessionID();

	//std::string content = "[1504208516.4110603, 1504208516.4110603, \"38f44d5b70939aa215476c29d5735bf2c181019a\", \"46c0e715b45064cf607638bbca3dec69\", null, { \"cvr.app.name\":\"CognitiveVRUnity\",\"cvr.app.version\" : \"1.0\",\"cvr.unity.version\" : \"5.4.1p4\",\"cvr.device.model\" : \"System Product Name (System manufacturer)\",\"cvr.device.type\" : \"Desktop\",\"cvr.device.platform\" : \"WindowsEditor\",\"cvr.device.os\" : \"Windows 10  (10.0.0) 64bit\" }]";

	double ts = GetSessionTimestamp();

	nlohmann::json content = nlohmann::json::array();
	content.emplace_back(ts);
	content.emplace_back(ts);
	content.emplace_back(UserId);
	content.emplace_back(DeviceId);
	if (UserProperties.size() == 0)
	{
		content.emplace_back(nullptr);
	}
	else
	{
		content.emplace_back(UserProperties);
	}
	
	if (DeviceProperties.size() == 0)
	{
		content.emplace_back(nullptr);
	}
	else
	{
		content.emplace_back(DeviceProperties);
	}

	gaze->SetInterval(config->GazeInterval);
	gaze->SetHMDType(config->HMDType);

	network->DashboardCall("application_init", content.dump());

	//start session event happens from callback

	return true;
}

::std::string CognitiveVRAnalyticsCore::GetCustomerId()
{
	return config->CustomerId;
}

void CognitiveVRAnalyticsCore::EndSession()
{
	log->Info("CognitiveVRAnalytics::EndSession");

	nlohmann::json props = nlohmann::json();

	::std::vector<float> endPos = { 0,0,0 };

	double sessionLength = GetTimestamp() - GetSessionTimestamp();
	props["sessionlength"] = sessionLength;

	transaction->EndPosition("cvr.session", endPos, props);

	SendData();

	SessionTimestamp = -1;
	SessionId = "";

	bHasSessionStarted = false;
}

double CognitiveVRAnalyticsCore::GetSessionTimestamp()
{
	if (SessionTimestamp < 0)
	{
		SessionTimestamp = GetTimestamp();
	}
	return SessionTimestamp;
}

double CognitiveVRAnalyticsCore::GetTimestamp()
{
	//http://www.cplusplus.com/forum/general/43203/#msg234281
	//https://stackoverflow.com/questions/9089842/c-chrono-system-time-in-milliseconds-time-operations

	//std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();

	//TODO this should use gmt, not system clock
	auto now = std::chrono::system_clock::now();
	return now.time_since_epoch().count()* 0.0000001;

	//std::time_t t = std::time(nullptr);
	//auto gmt = std::gmtime(&t);
}

::std::string CognitiveVRAnalyticsCore::GetSessionID()
{
	if (SessionId.empty())
	{
		SessionId = ::std::to_string((int)GetSessionTimestamp()) + "_" + DeviceId;
	}
	return SessionId;
}

void CognitiveVRAnalyticsCore::SendData()
{
	transaction->SendData();
	gaze->SendData();
	sensor->SendData();
	dynamicobject->SendData();
}

void CognitiveVRAnalyticsCore::SetUser(::std::string user_id, nlohmann::json properties)
{
	log->Info("CognitiveVRAnalytics::set user");
	
	UserId = user_id;
	double ts = GetTimestamp();

	nlohmann::json args = nlohmann::json::array();

	args.emplace_back(ts);
	args.emplace_back(ts);
	args.emplace_back(UserId);
	args.emplace_back(DeviceId);

	if (properties.size() > 0)
	{
		UserProperties = properties;
		//add this to transactions batch
		args.emplace_back(properties);
		transaction->AddToBatch("datacollector_updateUserState", args);
	}
	else
	{
		UserProperties = nlohmann::json::array();
		args.emplace_back(nullptr);
		transaction->AddToBatch("datacollector_updateUserState", args);
	}
}

void CognitiveVRAnalyticsCore::SetDevice(::std::string device_id, nlohmann::json properties)
{
	log->Info("CognitiveVRAnalytics::set device");
	//requires timestamp, timestamp,userid, deviceid,deviceproperties

	DeviceId = device_id;
	double ts = GetTimestamp();

	nlohmann::json args = nlohmann::json::array();

	args.emplace_back(ts);
	args.emplace_back(ts);
	args.emplace_back(UserId);
	args.emplace_back(DeviceId);

	if (properties.size() > 0)
	{
		DeviceProperties = properties;
		//add this to transactions batch
		args.emplace_back(properties);
		transaction->AddToBatch("datacollector_updateDeviceState", args);
	}
	else
	{
		DeviceProperties = nlohmann::json::array();
		args.emplace_back(nullptr);
		transaction->AddToBatch("datacollector_updateDeviceState", args);
	}
}

void CognitiveVRAnalyticsCore::SetScene(::std::string sceneName)
{
	if (CurrentSceneKey.size() > 0)
	{
		//send any remaining data to current scene, if there is a current scene
		SendData();
	}
	//if no current scene, likely queuing up events/gaze/etc before setting the scene

	log->Info("CognitiveVRAnalytics::SetScene: " + sceneName);
	auto search = config->sceneIds.find(sceneName);
	if (search != config->sceneIds.end())
	{
		CurrentSceneKey = search->second;
	}
	else
	{
		log->Error("Config scene ids does not contain key for scene " + sceneName);
	}
}

::std::string CognitiveVRAnalyticsCore::GetSceneKey()
{
	return CurrentSceneKey;
}
}