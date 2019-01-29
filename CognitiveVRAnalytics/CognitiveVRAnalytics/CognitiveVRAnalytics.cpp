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

std::string CognitiveVRAnalyticsCore::GetCurrentSceneId()
{
	return CurrentSceneId;
}
int CognitiveVRAnalyticsCore::GetCurrentSceneVersionId()
{
	return CurrentSceneVersionId;
}
std::string CognitiveVRAnalyticsCore::GetCurrentSceneVersionNumber()
{
	return CurrentSceneVersionNumber;
}

//log
std::unique_ptr<CognitiveLog> const& CognitiveVRAnalyticsCore::GetLog() const
{
	return log;
}

//exitpoll
std::unique_ptr<ExitPoll> const& CognitiveVRAnalyticsCore::GetExitPoll() const
{
	return exitpoll;
}

//custom event
std::unique_ptr<CustomEvent> const& CognitiveVRAnalyticsCore::GetCustomEvent() const
{
	return customevent;
}

//gaze
std::unique_ptr<cognitive::GazeTracker> const& CognitiveVRAnalyticsCore::GetGazeTracker() const
{
	return gaze;
}

//sensor
std::unique_ptr<Sensor> const& CognitiveVRAnalyticsCore::GetSensor() const
{
	return sensor;
}

//dynamic
std::unique_ptr<DynamicObject> const& CognitiveVRAnalyticsCore::GetDynamicObject() const
{
	return dynamicobject;
}

//network
std::unique_ptr<Network> const& CognitiveVRAnalyticsCore::GetNetwork() const
{
	return network;
}

//config
std::unique_ptr<Config> const& CognitiveVRAnalyticsCore::GetConfig() const
{
	return config;
}

CognitiveVRAnalyticsCore::CognitiveVRAnalyticsCore(CoreSettings settings)
{
	instance = std::shared_ptr<CognitiveVRAnalyticsCore>(this, D());

	sendFunctionPointer = settings.webRequest;

	log = make_unique_cognitive<CognitiveLog>(CognitiveLog(instance));
	GetLog()->SetLoggingLevel(settings.loggingLevel);
	GetLog()->Info("CognitiveVRAnalyticsCore()");

	if (settings.webRequest == nullptr)
	{
		GetLog()->Error("CognitiveVRAnalyticsCore() webRequest is null!");
	}

	config = make_unique_cognitive<Config>(Config(instance));

	//SET VALUES FROM SETTINGS
	if (!settings.CustomGateway.empty())
	{
		GetConfig()->kNetworkHost = settings.CustomGateway;
	}
	GetConfig()->HMDType = settings.GetHMDType();
	GetConfig()->APIKey = settings.APIKey;
	GetConfig()->GazeBatchSize = settings.GazeBatchSize;
	GetConfig()->CustomEventBatchSize = settings.CustomEventBatchSize;
	GetConfig()->SensorDataLimit = settings.SensorDataLimit;
	GetConfig()->DynamicDataLimit = settings.DynamicDataLimit;
	GetConfig()->GazeInterval = settings.GazeInterval;

	network = make_unique_cognitive<Network>(Network(instance));

	customevent = make_unique_cognitive<CustomEvent>(CustomEvent(instance));
	gaze = make_unique_cognitive<GazeTracker>(GazeTracker(instance));
	sensor = make_unique_cognitive<Sensor>(Sensor(instance));
	dynamicobject = make_unique_cognitive<DynamicObject>(DynamicObject(instance));
	exitpoll = make_unique_cognitive<ExitPoll>(ExitPoll(instance));

	//set scenes
	GetConfig()->AllSceneData = settings.AllSceneData;
	if (settings.DefaultSceneName.size() > 0)
		SetScene(settings.DefaultSceneName);
}

CognitiveVRAnalyticsCore::~CognitiveVRAnalyticsCore()
{
	GetLog()->Info("~CognitiveVRAnalyticsCore");
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
		GetLog()->Error("CognitiveVRAnalytics::StartSession failed - DeviceId or UserId must be set");
		return false;
	}

	GetLog()->Info("CognitiveVRAnalytics::StartSession");

	GetSessionTimestamp();
	GetSessionID();

	GetGazeTracker()->SetInterval(config->GazeInterval);
	GetGazeTracker()->SetHMDType(config->HMDType);

	isSessionActive = true;

	std::vector<float> pos = { 0,0,0 };
	GetCustomEvent()->RecordEvent("Start Session", pos);

	return true;
}

std::string CognitiveVRAnalyticsCore::GetAPIKey()
{
	return config->APIKey;
}

void CognitiveVRAnalyticsCore::EndSession()
{
	if (!IsSessionActive()) { return; }

	GetLog()->Info("CognitiveVRAnalytics::EndSession");

	nlohmann::json props = nlohmann::json();

	std::vector<float> endPos = { 0,0,0 };

	double sessionLength = GetTimestamp() - GetSessionTimestamp();
	props["sessionlength"] = sessionLength;

	GetCustomEvent()->RecordEvent("End Session", endPos, props);

	SendData();

	SessionTimestamp = -1;
	SessionId = "";

	isSessionActive = false;

	GetGazeTracker()->EndSession();
	GetCustomEvent()->EndSession();
	GetDynamicObject()->EndSession();
	GetSensor()->EndSession();
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
	if (!IsSessionActive()) { GetLog()->Info("CognitiveVRAnalyticsCore::SendData failed: no session active"); return; }

	GetCustomEvent()->SendData();
	GetGazeTracker()->SendData();
	GetSensor()->SendData();
	GetDynamicObject()->SendData();
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
	AllSessionProperties["c3d.deviceid"] = name;
	NewSessionProperties["c3d.deviceid"] = name;
}
void CognitiveVRAnalyticsCore::SetSessionName(std::string sessionName)
{
	AllSessionProperties["c3d.sessionname"] = sessionName;
	NewSessionProperties["c3d.sessionname"] = sessionName;
}

void CognitiveVRAnalyticsCore::SetScene(std::string sceneName)
{
	GetLog()->Info("CognitiveVRAnalytics::SetScene: " + sceneName);
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
		GetLog()->Error("CognitiveVRAnalyticsCore::SetScene Config scene ids does not contain key for scene " + sceneName);
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
	if (propertyType == EDeviceProperty::kAppName) { return"c3d.app.name"; }
	if (propertyType == EDeviceProperty::kAppVersion) { return"c3d.app.version"; }
	if (propertyType == EDeviceProperty::kAppEngine) { return"c3d.app.engine"; }
	if (propertyType == EDeviceProperty::kAppEngineVersion) { return"c3d.app.engine.version"; }

	if (propertyType == EDeviceProperty::kDeviceType) { return"c3d.device.type"; }
	if (propertyType == EDeviceProperty::kDeviceModel) { return"c3d.device.model"; }
	if (propertyType == EDeviceProperty::kDeviceMemory) { return"c3d.device.memory"; }
	if (propertyType == EDeviceProperty::kDeviceOS) { return"c3d.device.os"; }
	
	if (propertyType == EDeviceProperty::kDeviceCPU) { return"c3d.device.cpu"; }
	if (propertyType == EDeviceProperty::kDeviceCPUCores) { return"c3d.device.cpu.cores"; }
	if (propertyType == EDeviceProperty::kDeviceCPUVendor) { return"c3d.device.cpu.vendor"; }

	if (propertyType == EDeviceProperty::kDeviceGPU) { return"c3d.device.gpu"; }
	if (propertyType == EDeviceProperty::kDeviceGPUDriver) { return"c3d.device.gpu.driver"; }
	if (propertyType == EDeviceProperty::kDeviceGPUVendor) { return"c3d.device.gpu.vendor"; }
	if (propertyType == EDeviceProperty::kDeviceGPUMemory) { return"c3d.device.gpu.memory"; }

	if (propertyType == EDeviceProperty::kVRModel) { return"c3d.vr.model"; }
	if (propertyType == EDeviceProperty::kVRVendor) { return"c3d.vr.vendor"; }

	return "unknown.property";
}
}