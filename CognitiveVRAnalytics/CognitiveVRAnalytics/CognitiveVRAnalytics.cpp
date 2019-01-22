/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"
namespace cognitive {
static std::shared_ptr<CognitiveVRAnalyticsCore> instance;

std::shared_ptr<CognitiveVRAnalyticsCore> CognitiveVRAnalyticsCore::Instance()
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
	instance = std::shared_ptr<CognitiveVRAnalyticsCore>(this, D());

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
	if (!settings.CustomGateway.empty())
	{
		config->kNetworkHost = settings.CustomGateway;
	}
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
	config->AllSceneData = settings.AllSceneData;
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

void CognitiveVRAnalyticsCore::SetLobbyId(std::string lobbyId)
{
	LobbyId = lobbyId;
}
std::string CognitiveVRAnalyticsCore::GetLobbyId()
{
	return LobbyId;
}

bool CognitiveVRAnalyticsCore::StartSession()
{
	if (IsSessionActive())
	{
		return false;
	}

	if (GetUniqueID().empty())
	{
		log->Error("CognitiveVRAnalytics::StartSession failed - DeviceId or UserId must be set");
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

	std::vector<float> pos = { 0,0,0 };
	customevent->RecordEvent("Start Session", pos);

	return true;
}

std::string CognitiveVRAnalyticsCore::GetAPIKey()
{
	return config->APIKey;
}

void CognitiveVRAnalyticsCore::EndSession()
{
	if (!IsSessionActive()) { return; }

	log->Info("CognitiveVRAnalytics::EndSession");

	nlohmann::json props = nlohmann::json();

	std::vector<float> endPos = { 0,0,0 };

	double sessionLength = GetTimestamp() - GetSessionTimestamp();
	props["sessionlength"] = sessionLength;

	customevent->RecordEvent("End Session", endPos, props);

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

std::string CognitiveVRAnalyticsCore::GetSessionID()
{
	if (SessionId.empty())
	{
		SessionId = std::to_string((int)GetSessionTimestamp()) + "_" + GetUniqueID();
	}
	return SessionId;
}

std::string CognitiveVRAnalyticsCore::GetUniqueID()
{
	if (UniqueId.empty())
	{
		if (UserId.empty())
			UniqueId = DeviceId;
		else
			UniqueId = UserId;
		//should this fall back to 'anonymous'?
		//if (UniqueId.empty()) log->Error("CognitiveVRAnalyticsCore::GetUniqueID requires either DeviceId or UserId to be set!");
	}
	return UniqueId;
}

void CognitiveVRAnalyticsCore::SendData()
{
	//if (!IsSessionActive()) { log->Info("CognitiveVRAnalyticsCore::SendData failed: no session active"); return; }

	customevent->SendData();
	gaze->SendData();
	sensor->SendData();
	dynamicobject->SendData();
}

void CognitiveVRAnalyticsCore::SetUserName(std::string name)
{
	UserId = name;
	AllSessionProperties["userid"] = name;
	NewSessionProperties["userid"] = name;
}

void CognitiveVRAnalyticsCore::SetSessionProperty(std::string propertyType, int value)
{
	AllSessionProperties[propertyType] = value;
	NewSessionProperties[propertyType] = value;
}

void CognitiveVRAnalyticsCore::SetSessionProperty(std::string propertyType, std::string value)
{
	AllSessionProperties[propertyType] = value;
	NewSessionProperties[propertyType] = value;
}

void CognitiveVRAnalyticsCore::SetSessionProperty(std::string propertyType, float value)
{
	AllSessionProperties[propertyType] = value;
	NewSessionProperties[propertyType] = value;
}

nlohmann::json CognitiveVRAnalyticsCore::GetNewSessionProperties()
{
	nlohmann::json copiedmap = NewSessionProperties;
	NewSessionProperties = nlohmann::json();
	return copiedmap;
}

void CognitiveVRAnalyticsCore::SetDeviceName(std::string name)
{
	DeviceId = name;
	AllSessionProperties["deviceid"] = name;
	NewSessionProperties["deviceid"] = name;
}
void CognitiveVRAnalyticsCore::SetSessionName(std::string sessionName)
{
	AllSessionProperties["cvr.sessionname"] = sessionName;
	NewSessionProperties["cvr.sessionname"] = sessionName;
}

void CognitiveVRAnalyticsCore::SetScene(std::string sceneName)
{
	log->Info("CognitiveVRAnalytics::SetScene: " + sceneName);
	if (CurrentSceneId.size() > 0)
	{
		//send any remaining data to current scene, if there is a current scene
		SendData();
		//dynamicobject->RefreshObjectManifest();
	}
	//if no current scene, likely queuing up events/gaze/etc before setting the scene

	bool foundScene = false;
	for (auto const &ent : config->AllSceneData)
	{
		if (ent.SceneName == sceneName)
		{
			CurrentSceneId = ent.SceneId;
			CurrentSceneVersionNumber = ent.VersionNumber;
			CurrentSceneVersionId = ent.VersionId;

			foundScene = true;
			break;
		}
	}

	if (!foundScene)
	{
		log->Error("CognitiveVRAnalyticsCore::SetScene Config scene ids does not contain key for scene " + sceneName);
		CurrentSceneId = "";
		CurrentSceneVersionNumber = "";
		CurrentSceneVersionId = 0;
	}
	else
	{
		NewSessionProperties = AllSessionProperties;
		dynamicobject->RefreshObjectManifest();
	}
}

std::string CognitiveVRAnalyticsCore::GetSceneId()
{
	return CurrentSceneId;
}

std::string CognitiveVRAnalyticsCore::DevicePropertyToString(EDeviceProperty propertyType)
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