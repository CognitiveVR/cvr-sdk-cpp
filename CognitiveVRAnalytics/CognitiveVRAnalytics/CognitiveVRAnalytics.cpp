
// CognitiveAnalytics.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"
namespace cognitive {
static ::std::shared_ptr<CognitiveVRAnalyticsCore> instance;

::std::shared_ptr<CognitiveVRAnalyticsCore> CognitiveVRAnalyticsCore::Instance()
{
	//TODO add an error if instance is nullptr. this may return null. developer should call this manually before referencing it!
	return instance;
}

struct D {
	void operator() (CognitiveVRAnalyticsCore* cvr) {
		//custom deleter for cognitiveanalyticscore. does nothing!
	}
};

CognitiveVRAnalyticsCore::CognitiveVRAnalyticsCore(CoreSettings settings)
{
	instance = ::std::shared_ptr<CognitiveVRAnalyticsCore>(this, D());

	//INIT EVERYTHING	
	sendFunctionPointer = settings.webRequest;
	bHasSessionStarted = false;

	log = make_unique_cognitive<CognitiveLog>(CognitiveLog(instance));
	log->SetLoggingLevel(settings.loggingLevel);
	log->Info("CognitiveVRAnalyticsCore()");

	config = make_unique_cognitive<Config>(Config(instance));
	network = make_unique_cognitive<Network>(Network(instance));

	tuning = make_unique_cognitive<Tuning>(Tuning(instance));
	transaction = make_unique_cognitive<Transaction>(Transaction(instance));
	gaze = make_unique_cognitive<GazeTracker>(GazeTracker(instance));
	sensor = make_unique_cognitive<Sensor>(Sensor(instance));
	dynamicobject = make_unique_cognitive<DynamicObject>(DynamicObject(instance));
	exitpoll = make_unique_cognitive<ExitPoll>(ExitPoll(instance));

	//SET VALUES FROM SETTINGS
	config->HMDType = settings.GetHMDType();
	config->CustomerId = settings.CustomerId;
	config->GazeBatchSize = settings.GazeBatchSize;
	config->TransactionBatchSize = settings.TransactionBatchSize;
	config->SensorDataLimit = settings.SensorDataLimit;
	config->DynamicDataLimit = settings.DynamicDataLimit;
	config->GazeInterval = settings.GazeInterval;
	config->kNetworkTimeout = settings.kNetworkTimeout;

	//set scenes
	config->sceneIds = settings.sceneIds;
	if (settings.DefaultSceneName.size() > 0)
		SetScene(settings.DefaultSceneName);
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
		SetUserId("anonymous");
	}

	if (DeviceId.empty())
	{
		SetDeviceId("unknown");
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

void CognitiveVRAnalyticsCore::SetUserId(::std::string user_id)
{
	UserId = user_id;
}

void CognitiveVRAnalyticsCore::SetUserProperty(::std::string propertyType, int value)
{
	UserProperties[propertyType] = value;
}

void CognitiveVRAnalyticsCore::SetUserProperty(::std::string propertyType, ::std::string value)
{
	UserProperties[propertyType] = value;
}

void CognitiveVRAnalyticsCore::SetUserProperty(::std::string propertyType, float value)
{
	UserProperties[propertyType] = value;
}

void CognitiveVRAnalyticsCore::SetUserProperties(nlohmann::json properties)
{
	for (nlohmann::json::iterator it = properties.begin(); it != properties.end(); ++it) {

		if (it.value().is_string())
		{
			SetUserProperty(it.key(), it.value().dump());
		}
		else if (it.value().is_number_integer())
		{
			SetUserProperty(it.key(), (int)it.value());
		}
		else if (it.value().is_number_float())
		{
			SetUserProperty(it.key(), (float)it.value());
		}
	}
}

void CognitiveVRAnalyticsCore::UpdateUserState()
{
	double ts = GetTimestamp();

	nlohmann::json args = nlohmann::json::array();

	args.emplace_back(ts);
	args.emplace_back(ts);
	args.emplace_back(UserId);
	args.emplace_back(DeviceId);

	if (UserProperties.size() > 0)
	{
		//UserProperties = properties;
		//add this to transactions batch
		args.emplace_back(UserProperties);
		transaction->AddToBatch("datacollector_updateUserState", args);
	}
	else
	{
		//UserProperties = nlohmann::json::array();
		args.emplace_back(nullptr);
		transaction->AddToBatch("datacollector_updateUserState", args);
	}
}

void CognitiveVRAnalyticsCore::SetDeviceId(::std::string device_id)
{
	DeviceId = device_id;
}

void CognitiveVRAnalyticsCore::SetDeviceProperty(EDeviceProperty propertyType, int value)
{
	DeviceProperties[DevicePropertyToString(propertyType)] = value;
}

void CognitiveVRAnalyticsCore::SetDeviceProperty(EDeviceProperty propertyType, ::std::string value)
{
	DeviceProperties[DevicePropertyToString(propertyType)] = value;
}

void CognitiveVRAnalyticsCore::UpdateDeviceState()
{
	double ts = GetTimestamp();

	nlohmann::json args = nlohmann::json::array();

	args.emplace_back(ts);
	args.emplace_back(ts);
	args.emplace_back(UserId);
	args.emplace_back(DeviceId);

	if (DeviceProperties.size() > 0)
	{
		//DeviceProperties = properties;
		//add this to transactions batch
		args.emplace_back(DeviceProperties);
		transaction->AddToBatch("datacollector_updateDeviceState", args);
	}
	else
	{
		//DeviceProperties = nlohmann::json::array();
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
		log->Error("CognitiveVRAnalyticsCore::SetScene Config scene ids does not contain key for scene " + sceneName);
	}
}

::std::string CognitiveVRAnalyticsCore::GetSceneKey()
{
	return CurrentSceneKey;
}

::std::string CognitiveVRAnalyticsCore::DevicePropertyToString(EDeviceProperty propertyType)
{
	if (propertyType == EDeviceProperty::kAppName) { return"cvr.app.name"; }
	if (propertyType == EDeviceProperty::kAppVersion) { return"cvr.app.version"; }
	if (propertyType == EDeviceProperty::kAppEngine) { return"cvr.app.engine"; }
	if (propertyType == EDeviceProperty::kAppEngineVersion) { return"cvr.app.engine.version"; }

	if (propertyType == EDeviceProperty::kDeviceType) { return"cvr.device.type"; }
	if (propertyType == EDeviceProperty::kDeviceModel) { return"cvr.device.model"; }
	if (propertyType == EDeviceProperty::kDeviceMemory) { return"cvr.device.memory"; }
	if (propertyType == EDeviceProperty::kDeviceOS) { return"cvr.device.os"; }
	
	if (propertyType == EDeviceProperty::kDeviceCPU) { return"cvr.device.cpu"; }
	if (propertyType == EDeviceProperty::kDeviceCPUCores) { return"cvr.device.cpu.cores"; }
	if (propertyType == EDeviceProperty::kDeviceCPUVendor) { return"cvr.device.cpu.vendor"; }

	if (propertyType == EDeviceProperty::kDeviceGPU) { return"cvr.device.gpu"; }
	if (propertyType == EDeviceProperty::kDeviceGPUDriver) { return"cvr.device.gpu.driver"; }
	if (propertyType == EDeviceProperty::kDeviceGPUVendor) { return"cvr.device.gpu.vendor"; }
	if (propertyType == EDeviceProperty::kDeviceGPUMemory) { return"cvr.device.gpu.memory"; }

	if (propertyType == EDeviceProperty::kVRModel) { return"cvr.vr.model"; }
	if (propertyType == EDeviceProperty::kVRVendor) { return"cvr.vr.vendor"; }

	return "unknown.property";
}
}