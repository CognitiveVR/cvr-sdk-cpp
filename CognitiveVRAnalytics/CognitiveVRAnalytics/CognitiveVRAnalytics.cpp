/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"
namespace cognitive {
static ::std::shared_ptr<CognitiveVRAnalyticsCore> instance;

::std::shared_ptr<CognitiveVRAnalyticsCore> CognitiveVRAnalyticsCore::Instance()
{
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

	sendFunctionPointer = settings.webRequest;

	log = make_unique_cognitive<CognitiveLog>(CognitiveLog(instance));
	log->SetLoggingLevel(settings.loggingLevel);
	log->Info("CognitiveVRAnalyticsCore()");

	if (settings.webRequest == nullptr)
	{
		log->Error("CognitiveVRAnalyticsCore() webRequest is null!");
	}

	config = make_unique_cognitive<Config>(Config(instance));
	//SET VALUES FROM SETTINGS
	config->HMDType = settings.GetHMDType();
	config->APIKey = settings.APIKey;
	config->GazeBatchSize = settings.GazeBatchSize;
	config->CustomEventBatchSize = settings.CustomEventBatchSize;
	config->SensorDataLimit = settings.SensorDataLimit;
	config->DynamicDataLimit = settings.DynamicDataLimit;
	config->GazeInterval = settings.GazeInterval;

	network = make_unique_cognitive<Network>(Network(instance));

	customevent = make_unique_cognitive<CustomEvent>(CustomEvent(instance));
	gaze = make_unique_cognitive<GazeTracker>(GazeTracker(instance));
	sensor = make_unique_cognitive<Sensor>(Sensor(instance));
	dynamicobject = make_unique_cognitive<DynamicObject>(DynamicObject(instance));
	exitpoll = make_unique_cognitive<ExitPoll>(ExitPoll(instance));

	//set scenes
	config->SceneData = settings.SceneData;
	if (settings.DefaultSceneName.size() > 0)
		SetScene(settings.DefaultSceneName);
}

CognitiveVRAnalyticsCore::~CognitiveVRAnalyticsCore()
{
	log->Info("~CognitiveVRAnalyticsCore");
	config.reset();
	log.reset();
	network.reset();
	customevent.reset();
	sensor.reset();
	gaze.reset();
	dynamicobject.reset();
	exitpoll.reset();
	instance.reset();
}

bool CognitiveVRAnalyticsCore::IsSessionActive()
{
	return isSessionActive;
}

bool CognitiveVRAnalyticsCore::WasInitSuccessful()
{
	return isSessionActive;
}

bool CognitiveVRAnalyticsCore::StartSession()
{
	if (IsSessionActive())
	{
		return false;
	}

	log->Info("CognitiveVRAnalytics::StartSession");

	//TODO maybe set device and user ids here if not previously set?

	GetSessionTimestamp();
	GetSessionID();

	double ts = GetSessionTimestamp();

	gaze->SetInterval(config->GazeInterval);
	gaze->SetHMDType(config->HMDType);

	isSessionActive = true;

	::std::vector<float> pos = { 0,0,0 };
	customevent->Send("Start Session", pos);

	return true;
}

::std::string CognitiveVRAnalyticsCore::GetAPIKey()
{
	return config->APIKey;
}

void CognitiveVRAnalyticsCore::EndSession()
{
	if (!IsSessionActive()) { return; }

	log->Info("CognitiveVRAnalytics::EndSession");

	nlohmann::json props = nlohmann::json();

	::std::vector<float> endPos = { 0,0,0 };

	double sessionLength = GetTimestamp() - GetSessionTimestamp();
	props["sessionlength"] = sessionLength;

	customevent->Send("End Session", endPos, props);

	SendData();

	SessionTimestamp = -1;
	SessionId = "";

	isSessionActive = false;

	gaze->EndSession();
	customevent->EndSession();
	dynamicobject->EndSession();
	sensor->EndSession();
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
	//timezone? daylight savings time?
	//system clock duration between now and epoch in milliseconds
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return (double)milliseconds*0.001;
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
	if (!IsSessionActive()) { log->Info("CognitiveVRAnalyticsCore::SendData failed: no session active"); return; }

	customevent->SendData();
	gaze->SendData();
	sensor->SendData();
	dynamicobject->SendData();
}

void CognitiveVRAnalyticsCore::SetUserName(std::string name)
{
	UserId = name;
	NewUserProperties["name"] = name;
}

void CognitiveVRAnalyticsCore::SetUserProperty(::std::string propertyType, int value)
{
	NewUserProperties[propertyType] = value;
}

void CognitiveVRAnalyticsCore::SetUserProperty(::std::string propertyType, ::std::string value)
{
	NewUserProperties[propertyType] = value;
}

void CognitiveVRAnalyticsCore::SetUserProperty(::std::string propertyType, float value)
{
	NewUserProperties[propertyType] = value;
}

/*void CognitiveVRAnalyticsCore::SetUserProperties(nlohmann::json properties)
{
	for (nlohmann::json::iterator it = properties.begin(); it != properties.end(); ++it) {

		if (it.value().is_string())
		{
			std::string tmp = properties[it.key()];
			SetUserProperty(it.key(), tmp);
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
}*/

std::map<std::string, std::string> CognitiveVRAnalyticsCore::GetUserProperties()
{
	std::map<std::string, std::string> copiedmap = NewUserProperties;
	NewUserProperties = std::map<std::string, std::string>();
	return copiedmap;
}
/*
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
		args.emplace_back(UserProperties);
		//TODO user properties to gaze recording
	}
	else
	{
		args.emplace_back(nullptr);
		//TODO user properties to gaze recording
	}
}

void CognitiveVRAnalyticsCore::SetDeviceId(::std::string device_id)
{
	DeviceId = device_id;
}*/

void CognitiveVRAnalyticsCore::SetDeviceName(std::string name)
{
	DeviceId = name;
	NewDeviceProperties["name"] = name;
}

void CognitiveVRAnalyticsCore::SetDeviceProperty(EDeviceProperty propertyType, int value)
{
	NewDeviceProperties[DevicePropertyToString(propertyType)] = value;
}

void CognitiveVRAnalyticsCore::SetDeviceProperty(EDeviceProperty propertyType, ::std::string value)
{
	NewDeviceProperties[DevicePropertyToString(propertyType)] = value;
}

std::map<std::string, std::string> CognitiveVRAnalyticsCore::GetDeviceProperties()
{
	std::map<std::string, std::string> copiedmap = NewDeviceProperties;
	NewDeviceProperties = std::map<std::string, std::string>();
	return copiedmap;
}
/*
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
		args.emplace_back(DeviceProperties);
		//TODO device properties to gaze recording
	}
	else
	{
		args.emplace_back(nullptr);
		//TODO device properties to gaze recording
	}
}*/

void CognitiveVRAnalyticsCore::SetScene(::std::string sceneName)
{
	log->Info("CognitiveVRAnalytics::SetScene: " + sceneName);
	if (CurrentSceneId.size() > 0)
	{
		//send any remaining data to current scene, if there is a current scene
		SendData();
		dynamicobject->RefreshObjectManifest();
	}
	//if no current scene, likely queuing up events/gaze/etc before setting the scene

	//config->SceneData.


	//std::get(config->SceneData)

	//config->SceneData.

	bool foundScene = false;
	for (auto const &ent : config->SceneData)
	{
		if (ent.SceneName == sceneName)
		{
			CurrentSceneId = ent.SceneId;
			CurrentSceneVersionNumber = ent.VersionNumber;

			foundScene = true;
			break;
		}
	}

	if (!foundScene)
	{
		log->Error("CognitiveVRAnalyticsCore::SetScene Config scene ids does not contain key for scene " + sceneName);
		CurrentSceneId = "";
		CurrentSceneVersionNumber = "";
	}
}

::std::string CognitiveVRAnalyticsCore::GetSceneId()
{
	return CurrentSceneId;
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