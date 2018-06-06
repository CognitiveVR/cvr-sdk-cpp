// ClientProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"
#include "curl.h"

#include <iostream>
#include <cstdlib>

//for testing delay
#include <chrono>
#include <thread>

#if defined(_MSC_VER)
#include "gtest.h"
#else
#include "include/gtest/gtest.h"
#endif

std::string temp;

std::string TESTINGAPIKEY = "asdf1234hjkl5678";
std::string INVALIDAPIKEY = "INVALID_API_KEY";
long TestDelay = 1;

size_t handle(char* buf, size_t size, size_t nmemb, void* up)
{
	//std::cout << "handle handle handle";

	for (int c = 0; c < size*nmemb; c++)
	{
		temp.push_back(buf[c]);
	}
	return size * nmemb;
}

void INVALID_DoWebStuff(std::string url)
{

}

void NORESPONSE_DoWebStuff(std::string url, std::string content, std::vector<std::string> headers, cognitive::WebResponse response)
{

}

void DoWebStuff(std::string url, std::string content, std::vector<std::string> headers, cognitive::WebResponse response)
{
	CURL* curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	struct curl_slist *list = NULL;

	if (curl) {
		/* First set the URL that is about to receive our POST. This URL can
		just as well be a https:// URL if that is what should receive the
		data. */
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		//DEBUG verbose output
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Now specify the POST data */
		if (content.size() > 0)
		{
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str());
		}

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &handle);

		for (auto const &ent : headers)
		{
			list = curl_slist_append(list, ent.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
		//iterate through and set headers

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);

		//call response
		if (response != NULL)
			response(temp);

		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		else
		{

		}

		temp.clear();
		/* always cleanup */
		curl_easy_cleanup(curl);
		curl_slist_free_all(list); /* free the list again */
	}
}

//===========================TESTS
//----------------------INITIALIZATION

TEST(Initialization, MultipleStartSessions) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	
	bool first = cog.StartSession();
	EXPECT_EQ(first, true);
	bool second = cog.StartSession();
	EXPECT_EQ(second, false);
	bool third = cog.StartSession();
	EXPECT_EQ(third, false);
}

TEST(Initialization, MultipleStartEndSessions) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	bool first = cog.StartSession();
	EXPECT_EQ(first, true);
	cog.EndSession();
	bool second = cog.StartSession();
	EXPECT_EQ(second, true);
	cog.EndSession();
	bool third = cog.StartSession();
	EXPECT_EQ(third, true);
	cog.EndSession();
}

TEST(Initialization, SetLobbyId) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "tutorial";

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetLobbyId("my lobby id");

	cog.StartSession();
	cog.SendData();
}

TEST(Initialization, SetSessionName) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "tutorial";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetSessionName("my friendly session name");
	
	cog.StartSession();
	cog.SendData();
}

TEST(Initialization, SessionFullStartEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));
	
	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.StartSession();
	cog.customevent->Send("testing1", pos);
	cog.EndSession();
}

TEST(Initialization, SessionStart) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
}

TEST(Initialization, SessionStartNoWeb) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.loggingLevel == cognitive::LoggingLevel::kAll;
	//settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.WasInitSuccessful(), true);
}

TEST(Initialization, SessionStartNoWebResponse) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.loggingLevel == cognitive::LoggingLevel::kAll;
	settings.webRequest = &NORESPONSE_DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.WasInitSuccessful(), true);
}

TEST(Initialization, SessionStartInvalidCustomerId) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.APIKey = INVALIDAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	EXPECT_EQ(cog.GetAPIKey(), INVALIDAPIKEY);
	
	cog.StartSession();
	EXPECT_EQ(cog.GetAPIKey(), INVALIDAPIKEY);
	EXPECT_EQ(cog.WasInitSuccessful(), true);
}

TEST(Initialization, InstancePreConstructor) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	EXPECT_EQ(cognitive::CognitiveVRAnalyticsCore::Instance(), nullptr);
	
	cognitive::CoreSettings settings;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	EXPECT_NE(cognitive::CognitiveVRAnalyticsCore::Instance(), nullptr);
}

TEST(Initialization, Initialization) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.customevent->Send("testing1", pos);
	cog.StartSession();
}

TEST(Initialization, SessionEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.EndSession();
}

TEST(Initialization, SessionStartEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.EndSession();
}

//----------------------SET USER

TEST(UserSettings, UserPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.loggingLevel = cognitive::LoggingLevel::kAll;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetUserName("john");
	cog.SetSessionProperty("age", 21);
	cog.SetSessionProperty("location", "vancouver");
	
	auto props = cog.GetNewSessionProperties();
	EXPECT_EQ(cog.GetNewSessionProperties().size(), 0);

	EXPECT_EQ(props.size(), 3);

	cog.StartSession();
}

TEST(UserSettings, UserPostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.SetUserName("john");
	cog.SetSessionProperty("age", 21);
	cog.SetSessionProperty("location", "vancouver");
	
	cog.SendData();
	EXPECT_EQ(cog.GetNewSessionProperties().size(), 0);
}

TEST(UserSettings, UserNullPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.SendData();
}

TEST(UserSettings, UserNullPostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.SendData();
	cog.SetUserName("");
	cog.SendData();
}

//----------------------SET DEVICE

TEST(DeviceSettings, DevicePreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetDeviceName("7741345684915735");
	cog.SetSessionProperty("devicememory", 128);
	cog.SetSessionProperty("deviceos", "chrome os 16.9f");

	auto map = cog.GetNewSessionProperties();
	EXPECT_EQ(map.size(),0);

	cog.StartSession();
}

TEST(DeviceSettings, DevicePostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.SetDeviceName("7741345684915735");
	cog.SetSessionProperty("cpu", "i7-4770 CPU @ 3.40GHz");
	cog.SetSessionProperty("gpu", "GeForce GTX 970");
	cog.SetSessionProperty("memory", 128);
	cog.SetSessionProperty("os", "chrome os 16.9f");

	cog.SendData();
	auto map = cog.GetNewSessionProperties();
	EXPECT_EQ(map.size(), 0);
}

TEST(DeviceSettings, DevicePostSessionOverwrite) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.SetDeviceName("7741345684915735");
	cog.SetDeviceName("7741345684915736");
	cog.SetSessionProperty("cpu", "i5");
	cog.SetSessionProperty("gpu", "GeForce GTX 170");
	cog.SetSessionProperty("memory", 16);
	cog.SetSessionProperty("os", "chrome os 16.9f");

	cog.SetSessionProperty("cpu", "i7-4770 CPU @ 3.40GHz");
	cog.SetSessionProperty("gpu", "GeForce GTX 970");
	cog.SetSessionProperty("memory", 128);

	auto map = cog.GetNewSessionProperties();
	EXPECT_EQ(map.size(), 5);
}

TEST(DeviceSettings, DevicePostSessionOutOfOrder) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.SetSessionProperty("memory", 128);
	cog.SetDeviceName("7741345684915735");
	cog.SetSessionProperty("gpu", "GeForce GTX 970");
	cog.SetSessionProperty("os", "chrome os 16.9f");
	cog.SetSessionProperty("cpu", "i7-4770 CPU @ 3.40GHz");
	
	auto map = cog.GetNewSessionProperties();
	EXPECT_EQ(map.size(), 5);
}

TEST(DeviceSettings, DeviceNullPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetDeviceName("");
	cog.StartSession();
	cog.SendData();
}

TEST(DeviceSettings, DeviceNullPostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();
	cog.SetDeviceName("");
}

TEST(DeviceSettings, DeviceNullPreEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);


	cog.SetDeviceName("7741345684915735");
	cog.SetDeviceName("7741345684915736");
	cog.SetSessionProperty("memory", 128);
	cog.SetSessionProperty("os", "chrome os 16.9f");

	cog.EndSession();
}

//----------------------SET USER DEVICE
TEST(UserDeviceSettings, UserDevicePostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();

	cog.SetDeviceName("7741345684915735");
	cog.SetSessionProperty("memory", 128);
	cog.SetSessionProperty("os", "chrome os 16.9f");
	cog.SendData();
	auto dmap = cog.GetNewSessionProperties();
	EXPECT_EQ(dmap.size(), 0);

	cog.SetUserName("john");
	cog.SetSessionProperty("location", "vancouver");
	cog.SetSessionProperty("location", "seattle");
	cog.SendData();
	auto umap = cog.GetNewSessionProperties();
	EXPECT_EQ(umap.size(), 0);

	cog.EndSession();
}

TEST(UserDeviceSettings, UserDevicePreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetUserName("john");
	cog.SetSessionProperty("location", "vancouver");
	cog.SetSessionProperty("location", "seattle");

	cog.SetDeviceName("7741345684915735");
	cog.SetSessionProperty("memory", 128);
	cog.SetSessionProperty("os", "chrome os 16.9f");
	cog.SendData();

	cog.StartSession();
	cog.EndSession();
}

//----------------------CUSTOM EVENTS

TEST(CustomEvent, PreSessionNoEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.customevent->Send("testing1", pos);

	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 1);

	cog.StartSession();
}

TEST(CustomEvent, PreSessionBatchClear) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.CustomEventBatchSize = 3;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.customevent->Send("testing1", pos);
	cog.customevent->Send("testing1", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 2);
	cog.customevent->Send("testing1", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);

	cog.StartSession();
}

TEST(CustomEvent, PreSessionEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.customevent->Send("testing1", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 1);
	cog.StartSession();
	cog.EndSession();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
}

TEST(CustomEvent, PreSessionSend) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.customevent->Send("testing1", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 1);
	cog.StartSession();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 2); //testing1 and sesionbegin
	cog.SendData();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
}

TEST(CustomEvent, PreSessionPropsSend) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };

	cognitive::nlohmann::json prop = cognitive::nlohmann::json();
	prop["age"] = 21;
	prop["location"] = "vancouver";

	cog.customevent->Send("testing1", pos, prop);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 1);
	cog.StartSession();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 2);
	cog.SendData();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
}

TEST(CustomEvent, SessionEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };

	cognitive::nlohmann::json props = cognitive::nlohmann::json();
	props["age"] = 21;
	props["location"] = "vancouver";

	cog.StartSession();
	cog.customevent->Send("testing1", pos, props);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 2);
	cog.EndSession();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
}

TEST(CustomEvent, SendLimitPreSessionThreshold) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.CustomEventBatchSize = 3; //on the third transaction it should send
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.customevent->Send("testing1", pos);
	cog.customevent->Send("testing2", pos);
	cog.customevent->Send("testing3", pos); //should try to send here. clear batched custom events
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);

	cog.StartSession(); //fourth transaction. should send all
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 1);
}

TEST(CustomEvent, SendLimitPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.CustomEventBatchSize = 3; //on the third transaction it should send
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.customevent->Send("testing1", pos);

	cog.StartSession();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 2);
	cog.customevent->Send("testing1", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
	cog.EndSession();
}

TEST(CustomEvent, SendLimitSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.CustomEventBatchSize = 3; //on the third transaction it should send
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.StartSession();
	cog.customevent->Send("testing1", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 2);
	cog.customevent->Send("testing2", pos); //send here
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
	
	cog.customevent->Send("testing1", pos);
	cog.customevent->Send("testing1", pos);
	cog.customevent->Send("testing1", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);

	cog.customevent->Send("testing1", pos);
	cog.EndSession();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
}

//----------------------TUNING
/*
TEST(Tuning, TuningGetValue) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();

	auto snow_attitude = cog.tuning->GetValue("snow_attitude", "mellow", cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, "ferocious");
	std::cout << snow_attitude << std::endl;

	auto blockPosition = cog.tuning->GetValue("vinegar_volume", 0, cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(blockPosition, 50);
	std::cout << blockPosition << std::endl;

	auto ExitPollActivated = cog.tuning->GetValue("ExitPollActivated", false, cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(ExitPollActivated, true);
	std::cout << ExitPollActivated << std::endl;

	auto pi = cog.tuning->GetValue("pi", (float)3.0, cognitive::EntityType::kEntityTypeDevice);
	EXPECT_FLOAT_EQ(pi, 3.1415927);
	std::cout << pi << std::endl;

	cog.EndSession();
}

TEST(Tuning, TuningGetValueNoSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	auto snow_attitude = cog.tuning->GetValue("snow_attitude", "mellow", cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, "mellow");
}

TEST(Tuning, TuningGetInvalidValue) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	auto snow_attitude = cog.tuning->GetValue("snow_raditude", "mellow", cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, "mellow");
	cog.EndSession();
}

TEST(Tuning, TuningGetInvalidCast) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	auto snow_attitude = cog.tuning->GetValue("snow_attitude", 500, cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, 500);
	cog.EndSession();
}

TEST(Tuning, TuningGetInvalidCastBool) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	auto snow_attitude = cog.tuning->GetValue("snow_attitude", false, cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, false);
	cog.EndSession();
}
*/
//----------------------SETTING SCENE KEYS FOR SCENE EXPLORER

TEST(Scenes, NoScenes) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();

	std::vector<float> pos = { 1,0,0 };
	cog.customevent->Send("testing1", pos);
	pos[0] = 2;
	cog.customevent->Send("testing2", pos);
	pos[0] = 3;
	cog.customevent->Send("testing3", pos);
	EXPECT_EQ(cog.CurrentSceneId, "");
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 4);
	cog.SendData();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
	cog.EndSession();
}

TEST(Scenes, InitScenes) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1",0));
	scenedatas.emplace_back(cognitive::SceneData("menu", "DELETE_ME_2", "1",0));
	scenedatas.emplace_back(cognitive::SceneData("finalboss", "DELETE_ME_3", "1",0));

	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();

	std::vector<float> pos = { 1,0,0 };
	cog.customevent->Send("testing1", pos);
	pos[0] = 2;
	cog.customevent->Send("testing2", pos);
	pos[0] = 3;
	cog.customevent->Send("testing3", pos);

	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 4);
	cog.SendData();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
	cog.EndSession();
}

TEST(Scenes, InitSetScenes) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	scenedatas.emplace_back(cognitive::SceneData("menu", "DELETE_ME_2", "1", 0));
	scenedatas.emplace_back(cognitive::SceneData("finalboss", "DELETE_ME_3", "1", 0));

	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.CurrentSceneId, "");
	cog.SetScene("tutorial");
	EXPECT_EQ(cog.CurrentSceneId, "DELETE_ME_1");

	std::vector<float> pos = { 1,0,0 };
	cog.customevent->Send("testing1", pos);
	pos[0] = 2;
	cog.customevent->Send("testing2", pos);
	pos[0] = 3;
	cog.customevent->Send("testing3", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 4);
	cog.SendData();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
	cog.EndSession();
}

TEST(Scenes, InitSetSceneSwitch) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	scenedatas.emplace_back(cognitive::SceneData("menu", "DELETE_ME_2", "1", 0));
	scenedatas.emplace_back(cognitive::SceneData("finalboss", "DELETE_ME_3", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.CurrentSceneId,"");
	cog.SetScene("tutorial");
	EXPECT_EQ(cog.CurrentSceneId, "DELETE_ME_1");

	std::vector<float> pos = { 1,0,0 };
	cog.customevent->Send("testing1", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 2);
	cog.SetScene("menu");
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
	pos[0] = 2;
	cog.customevent->Send("testing2", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 1);
	cog.SetScene("finalboss");
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
	pos[0] = 3;
	cog.customevent->Send("testing3", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 1);

	cog.SendData();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
	cog.EndSession();
}

TEST(Scenes, InitSetInvalidScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	scenedatas.emplace_back(cognitive::SceneData("menu", "DELETE_ME_2", "1", 0));
	scenedatas.emplace_back(cognitive::SceneData("finalboss", "DELETE_ME_3", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.CurrentSceneId, "");
	cog.SetScene("non-existent");
	EXPECT_EQ(cog.CurrentSceneId, "");

	std::vector<float> pos = { 1,0,0 };
	cog.customevent->Send("testing1", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 2);

	cog.SendData();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
	cog.EndSession();
}

TEST(Scenes, InitSetInvalidNoScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.CurrentSceneId, "");
	cog.SetScene("non-existent");
	EXPECT_EQ(cog.CurrentSceneId, "");

	std::vector<float> pos = { 1,0,0 };
	cog.customevent->Send("testing1", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 2);

	cog.SendData();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
	cog.EndSession();
}

TEST(Scenes, SetSceneMiddle) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("finalboss", "DELETE_ME_3", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.CurrentSceneId, "");
	cog.SetScene("non-existent");
	EXPECT_EQ(cog.CurrentSceneId, "");

	std::vector<float> pos = { 1,0,0 };
	cog.customevent->Send("testing1", pos);
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 2); //startsession and testing1

	cog.SendData();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);
	cog.SetScene("finalboss");
	cog.SendData();
	EXPECT_EQ(cog.customevent->BatchedCustomEvents.size(), 0);

	cog.EndSession();
}
//----------------------EXITPOLL

TEST(ExitPoll, RequestSetNoInit) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.exitpoll->RequestQuestionSet("testing_new_sdk");

	EXPECT_EQ(cog.exitpoll->GetQuestionSetString(), "");

	cog.StartSession();
}

TEST(ExitPoll, BasicRequest) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.loggingLevel = cognitive::LoggingLevel::kAll;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	EXPECT_EQ(cog.GetAPIKey(), TESTINGAPIKEY);

	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("testing_new_sdk");
	EXPECT_NE(cog.exitpoll->GetQuestionSetString(), "");
	cog.EndSession();
}

TEST(ExitPoll, GetThenRequest) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);


	cog.StartSession();
	EXPECT_EQ(cog.exitpoll->GetQuestionSetString(), "");
	cog.exitpoll->RequestQuestionSet("testing_new_sdk");
	EXPECT_NE(cog.exitpoll->GetQuestionSetString(), "");
	cog.EndSession();
}

TEST(ExitPoll, RequestThenGet) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);


	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("testing_new_sdk");
	EXPECT_NE(cog.exitpoll->GetQuestionSetString(), "");
	cog.EndSession();
}

TEST(ExitPoll, InvalidRequestThenGet) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("question-does-not-exist");
	EXPECT_EQ(cog.exitpoll->GetQuestionSetString(), "");
	cog.EndSession();
}

TEST(ExitPoll, RequestThenGetAnswersJson) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("testing_new_sdk");
	cognitive::nlohmann::json questions = cog.exitpoll->GetQuestionSet();

	cog.exitpoll->AddAnswer(cognitive::FExitPollAnswer(cognitive::EQuestionType::kHappySad, false));
	cog.exitpoll->AddAnswer(cognitive::FExitPollAnswer(cognitive::EQuestionType::kMultiple, 0));
	cog.exitpoll->AddAnswer(cognitive::FExitPollAnswer(cognitive::EQuestionType::kScale, 1));
	cog.exitpoll->SendAllAnswers();
	cog.EndSession();
}

TEST(ExitPoll, RequestThenGetAnswersString) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("testing_new_sdk");
	std::string questionString = cog.exitpoll->GetQuestionSetString();

	cog.exitpoll->AddAnswer(cognitive::FExitPollAnswer(cognitive::EQuestionType::kHappySad, false));
	cog.exitpoll->AddAnswer(cognitive::FExitPollAnswer(cognitive::EQuestionType::kMultiple, 0));
	cog.exitpoll->AddAnswer(cognitive::FExitPollAnswer(cognitive::EQuestionType::kScale, 1));
	cog.exitpoll->SendAllAnswers();
	cog.EndSession();
}

//----------------------GAZE

TEST(Gaze, GazeThenInit) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	for (float i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot);
	}

	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 10);
	
	cog.StartSession();
	cog.EndSession();
	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 0); //no scene to send to. endsession clears everything
}

TEST(Gaze, PreSessionGazeBatchClear) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.GazeBatchSize = 3;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	
	cog.gaze->RecordGaze(pos, rot);
	cog.gaze->RecordGaze(pos, rot);
	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 2);
	cog.gaze->RecordGaze(pos, rot);
	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 0);

	cog.StartSession();
}

TEST(Gaze, GazeThenInitSetScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("gazescene", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);


	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	for (float i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot);
	}
	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 10);

	cog.StartSession();
	cog.SetScene("gazescene");
	cog.EndSession();
	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 0);
}

TEST(Gaze, InitThenGazeThenSetScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("gazescene", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	for (float i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot);
	}
	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 10);

	cog.SetScene("gazescene");
	cog.EndSession();
	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 0);
}

TEST(Gaze, InitThenGazeThenSendThenSetScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("gazescene", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	for (float i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot);
	}
	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 10);

	cog.SendData();
	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 0);
	cog.SetScene("gazescene");
	cog.SendData();
	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 0);
	cog.EndSession();
	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 0);
}

TEST(Gaze, GazeOnDynamic) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("gazescene", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>point = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	cog.StartSession();
	cog.SetScene("gazescene");

	for (float i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot, point);
	}

	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 10);

	cog.dynamicobject->RegisterObjectCustomId("object1", "block", "1", pos, rot);

	for (float i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot, point,"1");
	}

	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 20);

	cog.SendData();
	EXPECT_EQ(cog.gaze->BatchedGaze.size(), 0);

	cog.EndSession();
}

//----------------------SENSORS

TEST(Sensors, SenseThenInit) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor",i);
	}

	cog.StartSession();
	EXPECT_EQ(cog.sensor->sensorCount, 10);
	cog.SendData();
	EXPECT_EQ(cog.sensor->sensorCount, 0);
	cog.EndSession();
	EXPECT_EQ(cog.sensor->sensorCount, 0);
}

TEST(Sensors, PreSessionSenseBatchClear) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.SensorDataLimit = 3;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.sensor->RecordSensor("test-sensor", 1);
	cog.sensor->RecordSensor("test-sensor", 2);
	EXPECT_EQ(cog.sensor->sensorCount, 2);
	cog.sensor->RecordSensor("test-sensor", 3);
	EXPECT_EQ(cog.sensor->sensorCount, 0);

	cog.StartSession();
}

TEST(Sensors, EndSessionThenSense) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.EndSession();

	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}

	EXPECT_EQ(cog.sensor->sensorCount, 10);
}

TEST(Sensors, SensorLimitSingle) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.SensorDataLimit = 10;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("sensescene", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "sensescene";

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();

	for (int i = 0; i < 5; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}
	EXPECT_EQ(cog.sensor->sensorCount, 5);
	for (int i = 0; i < 6; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}
	EXPECT_EQ(cog.sensor->sensorCount, 1);
}

TEST(Sensors, SensorLimitMany) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.SensorDataLimit = 15;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("sensescene", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "sensescene";

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();

	for (int i = 0; i < 5; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}
	EXPECT_EQ(cog.sensor->sensorCount, 5);
	for (int i = 0; i < 5; ++i)
	{
		cog.sensor->RecordSensor("test-sensor1", i);
	}
	EXPECT_EQ(cog.sensor->sensorCount, 10);
	for (int i = 0; i < 5; ++i)
	{
		cog.sensor->RecordSensor("test-sensor2", i);
	}
	EXPECT_EQ(cog.sensor->sensorCount, 0);
}

TEST(Sensors, SensorSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("sensescene", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "sensescene";

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();

	for (int i = 0; i < 5; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}
	EXPECT_EQ(cog.sensor->sensorCount, 5);
	cog.SetScene("non-existent-scene");
	EXPECT_EQ(cog.sensor->sensorCount, 0);
	for (int i = 0; i < 6; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}
	EXPECT_EQ(cog.sensor->sensorCount, 6);
	cog.SendData();
	EXPECT_EQ(cog.sensor->sensorCount, 0);
}

TEST(Sensors, SenseThenInitSetScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("sensescene", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}
	EXPECT_EQ(cog.sensor->sensorCount, 10);
	cog.StartSession();
	cog.SetScene("sensescene");
	EXPECT_EQ(cog.sensor->sensorCount, 10);
	cog.SendData();
	EXPECT_EQ(cog.sensor->sensorCount, 0);
	cog.EndSession();
}

TEST(Sensors, InitThenGazeThenSetScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("sensescene", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}
	EXPECT_EQ(cog.sensor->sensorCount, 10);
	cog.SetScene("sensescene");
	cog.EndSession();
	EXPECT_EQ(cog.sensor->sensorCount, 0);
}

TEST(Sensors, InitThenGazeThenSendThenSetScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("sensescene", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}
	EXPECT_EQ(cog.sensor->sensorCount, 10);
	cog.SendData();
	cog.SetScene("sensescene");
	EXPECT_EQ(cog.sensor->sensorCount, 0);
	cog.EndSession();
	EXPECT_EQ(cog.sensor->sensorCount, 0);
}

TEST(Sensors, ManySensors) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("sensescene", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor1", i);
	}
	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor2", i);
	}

	EXPECT_EQ(cog.sensor->sensorCount, 20);
	cog.SetScene("sensescene");
	cog.EndSession();
	EXPECT_EQ(cog.sensor->sensorCount, 0);
}

//----------------------DYNAMICS

TEST(Dynamics, InitRegisterSend) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	std::string object1id = cog.dynamicobject->RegisterObject("object1", "lamp", pos, rot);
	return;
	std::string object2id = cog.dynamicobject->RegisterObject("object2", "lamp", pos, rot);
	EXPECT_EQ(cog.dynamicobject->fullManifest.size(), 2);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 2);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 2);

	
	pos = { 0,0,5 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,1,6 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 4);

	pos = { 0,0,7 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,2,8 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 6);

	pos = { 0,0,9 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,3,10 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 8);

	cog.SendData();
	EXPECT_EQ(cog.dynamicobject->fullManifest.size(), 2);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 2);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 8);
	cog.EndSession();
}

TEST(Dynamics, InitRegisterSceneSend) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("dynamicscene", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	std::string object1id = cog.dynamicobject->RegisterObject("object1", "lamp", pos, rot);
	std::string object2id = cog.dynamicobject->RegisterObject("object2", "lamp", pos, rot);

	EXPECT_EQ(cog.dynamicobject->fullManifest.size(), 2);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 2);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 2);



	pos = { 0,0,5 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,1,6 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 4);

	pos = { 0,0,7 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,2,8 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 6);

	pos = { 0,0,9 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,3,10 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 8);
	
	cog.SetScene("dynamicscene");
	cog.SendData();
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 0);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 0);
	EXPECT_EQ(cog.dynamicobject->fullManifest.size(), 2);
	cog.EndSession();
}

TEST(Dynamics, ResetObjectIdsSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("one", "DELETE_ME_1", "1", 0));
	scenedatas.emplace_back(cognitive::SceneData("two", "DELETE_ME_2", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "one";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	std::string object1id = cog.dynamicobject->RegisterObject("object1", "lamp", pos, rot);
	std::string object2id = cog.dynamicobject->RegisterObject("object2", "lamp", pos, rot);

	auto props = cognitive::nlohmann::json();
	props["enabled"] = false;
	cog.dynamicobject->AddSnapshot(object1id, pos, rot, props);

	EXPECT_EQ(2, cog.dynamicobject->manifestEntries.size());

	cog.dynamicobject->AddSnapshot(object1id, pos, rot);

	cog.SetScene("two"); //refreshes object manifest
	EXPECT_EQ(2, cog.dynamicobject->manifestEntries.size());

	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);

	EXPECT_EQ(2, cog.dynamicobject->manifestEntries.size());
	cog.SendData();
	EXPECT_EQ(0, cog.dynamicobject->manifestEntries.size());

	cog.SendData();
	cog.EndSession();
}

TEST(Dynamics, CustomIds) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("one", "DELETE_ME_1", "1", 0));
	scenedatas.emplace_back(cognitive::SceneData("two", "DELETE_ME_2", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "one";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	std::string object1id = "1";
	std::string object2id = "2";
	cog.dynamicobject->RegisterObjectCustomId("object1", "lamp", object1id, pos, rot);
	cog.dynamicobject->RegisterObjectCustomId("object2", "lamp", object2id, pos, rot);



	EXPECT_EQ(2, cog.dynamicobject->manifestEntries.size());
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 5);

	cog.SendData();
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 0);
	cog.EndSession();
}

TEST(Dynamics, CustomIdMultiples) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("one", "DELETE_ME_1", "1", 0));
	scenedatas.emplace_back(cognitive::SceneData("two", "DELETE_ME_2", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "one";
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.APIKey = TESTINGAPIKEY;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	std::string object1id = "1";
	std::string object2id = "2";
	std::string object3id = "3";

	cog.dynamicobject->RegisterObjectCustomId("object1", "lamp", "1", pos, rot);
	cog.dynamicobject->RegisterObjectCustomId("object2", "lamp", "1", pos, rot);
	cog.dynamicobject->RegisterObjectCustomId("object3", "lamp", "1", pos, rot);
	
	EXPECT_EQ(cog.dynamicobject->objectIds.size(), 3);

	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 3);
	EXPECT_EQ(cog.dynamicobject->fullManifest.size(), 3);
}

TEST(Dynamics, LimitSnapshots) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("one", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "one";
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.DynamicDataLimit = 5;
	settings.APIKey = TESTINGAPIKEY;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	std::string object1id = "1";
	cog.dynamicobject->RegisterObjectCustomId("object1", "lamp", object1id, pos, rot);

	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 1);

	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 3);
	cog.dynamicobject->AddSnapshot(object1id, pos, rot); //manifest + snapshots = limit
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 0);
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 2);
}

TEST(Dynamics, LimitRegister) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("one", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "one";
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.DynamicDataLimit = 5;
	settings.APIKey = TESTINGAPIKEY;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	cog.dynamicobject->RegisterObjectCustomId("object1", "lamp", "1", pos, rot);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 1);
	cog.dynamicobject->RegisterObjectCustomId("object2", "lamp", "2", pos, rot);
	cog.dynamicobject->RegisterObjectCustomId("object3", "lamp", "3", pos, rot); //limit. send
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 0);
	cog.dynamicobject->RegisterObjectCustomId("object4", "lamp", "4", pos, rot);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 1);
	cog.dynamicobject->RegisterObjectCustomId("object5", "lamp", "5", pos, rot); //limit send
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 2);
}

TEST(Dynamics, LimitPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("one", "DELETE_ME_1", "1",0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "one";
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.DynamicDataLimit = 5;
	settings.APIKey = TESTINGAPIKEY;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);


	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	cog.dynamicobject->RegisterObjectCustomId("object1", "lamp", "1", pos, rot);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 1);
	cog.dynamicobject->RegisterObjectCustomId("object2", "lamp", "2", pos, rot);
	cog.dynamicobject->RegisterObjectCustomId("object3", "lamp", "3", pos, rot);
	cog.dynamicobject->RegisterObjectCustomId("object4", "lamp", "4", pos, rot);
	cog.dynamicobject->RegisterObjectCustomId("object5", "lamp", "5", pos, rot);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 5);
	cog.StartSession();
	cog.dynamicobject->RegisterObjectCustomId("object5", "lamp", "6", pos, rot);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 0);
}

TEST(Dynamics, ReuseObjectId) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("one", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "one";
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.APIKey = TESTINGAPIKEY;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);


	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };
	cog.StartSession();

	std::string object1id = cog.dynamicobject->RegisterObject("object1", "lamp", pos, rot);
	std::string object2id = cog.dynamicobject->RegisterObject("object2", "lamp", pos, rot);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 2);

	cog.dynamicobject->RemoveObject(object1id, pos, rot);
	std::string object3id = cog.dynamicobject->RegisterObject("object3", "block", pos, rot);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 3);
	std::string object4id = cog.dynamicobject->RegisterObject("object4", "lamp", pos, rot);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 3);
	cog.SendData();
}

TEST(Dynamics, EngagementBeforeRegister) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("one", "DELETE_ME_1", "1", 0));
	scenedatas.emplace_back(cognitive::SceneData("two", "DELETE_ME_2", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "one";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(),0);

	cog.dynamicobject->BeginEngagement("1", "grab");
	cog.dynamicobject->AddSnapshot("1", pos, rot);
	cog.dynamicobject->EndEngagement("1", "grab");
	EXPECT_EQ(cog.dynamicobject->activeEngagements.size(), 1);

	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 0);
	cog.dynamicobject->RegisterObjectCustomId("object1", "lamp","1", pos, rot);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 1);
	cog.SendData();
	EXPECT_EQ(cog.dynamicobject->activeEngagements.size(), 1);
	EXPECT_EQ(cog.dynamicobject->allEngagements.size(), 1);
}

TEST(Dynamics, EngagementNeverRegister) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("one", "DELETE_ME_1", "1", 0));
	scenedatas.emplace_back(cognitive::SceneData("two", "DELETE_ME_2", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "one";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();



	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	cog.dynamicobject->BeginEngagement("1", "grab");
	cog.dynamicobject->AddSnapshot("1", pos, rot);
	cog.dynamicobject->EndEngagement("1", "grab");
	EXPECT_EQ(cog.dynamicobject->activeEngagements.size(), 1);

	//cog.dynamicobject->RegisterObjectCustomId("object1", "lamp", 1);
	cog.SendData();
	EXPECT_EQ(cog.dynamicobject->activeEngagements.size(), 1);
	EXPECT_EQ(cog.dynamicobject->allEngagements.size(), 1);
}

TEST(Dynamics, EngagementsScenes) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("one", "DELETE_ME_1", "1", 0));
	scenedatas.emplace_back(cognitive::SceneData("two", "DELETE_ME_2", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "one";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	std::string object1id = cog.dynamicobject->RegisterObject("object1", "lamp", pos, rot);
	EXPECT_EQ(cog.dynamicobject->allEngagements.size(), 1);
	std::string object2id = cog.dynamicobject->RegisterObject("object2", "lamp", pos, rot);
	EXPECT_EQ(cog.dynamicobject->allEngagements.size(), 2);

	EXPECT_EQ(2, cog.dynamicobject->manifestEntries.size());

	cog.dynamicobject->BeginEngagement(object1id, "grab");
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->EndEngagement(object1id, "grab");
	EXPECT_EQ(cog.dynamicobject->activeEngagements.size(),1);
	EXPECT_EQ(cog.dynamicobject->allEngagements.size(),2); //engagements[object2] is a null vector, so count = 2

	cog.SetScene("two"); //refreshes object manifest
	EXPECT_EQ(2, cog.dynamicobject->manifestEntries.size());

	cog.dynamicobject->BeginEngagement(object1id, "grab");
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	cog.dynamicobject->BeginEngagement(object2id, "grab");
	cog.dynamicobject->EndEngagement(object1id, "grab");
	cog.dynamicobject->EndEngagement(object2id, "grab");
	EXPECT_EQ(cog.dynamicobject->activeEngagements.size(), 2);
	EXPECT_EQ(cog.dynamicobject->allEngagements.size(), 2);

	EXPECT_EQ(2, cog.dynamicobject->manifestEntries.size());
	cog.SendData();
	EXPECT_EQ(0, cog.dynamicobject->manifestEntries.size());

	cog.SendData();
	cog.EndSession();
}

TEST(Dynamics, EngagementRemove) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.APIKey = TESTINGAPIKEY;

	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("one", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "one";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	
	cog.dynamicobject->RegisterObjectCustomId("object1", "lamp", "1", pos, rot);
	
	cog.dynamicobject->BeginEngagement("1", "grab");
	cog.dynamicobject->AddSnapshot("1", pos, rot);
	cog.dynamicobject->RemoveObject("1", pos, rot);
	EXPECT_EQ(cog.dynamicobject->activeEngagements.size(), 1);


	cog.SendData();
	EXPECT_EQ(cog.dynamicobject->activeEngagements.size(), 1);
	EXPECT_EQ(cog.dynamicobject->allEngagements.size(), 1);
}


int main(int argc, char **argv)
{
	if (const char* env_p = std::getenv("COGNITIVEAPIKEY"))
	{
		std::cout << "Custom APIKEY is: " << env_p << '\n';
		TESTINGAPIKEY = env_p;
	}

	::testing::InitGoogleTest(&argc, argv);

#if defined(_MSC_VER)
	RUN_ALL_TESTS();
	system("pause");
	return 0;
#else
	return RUN_ALL_TESTS();
#endif
	
	

	//should use only this in a build
	//return RUN_ALL_TESTS();
}