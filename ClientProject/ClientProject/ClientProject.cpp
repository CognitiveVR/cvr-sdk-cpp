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
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.IsSessionActive(), true);
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
	EXPECT_EQ(cog.IsSessionActive(), true);
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
	EXPECT_EQ(cog.IsSessionActive(), true);
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
	EXPECT_EQ(cog.IsSessionActive(), false);
}

TEST(Initialization, SessionStartEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	EXPECT_EQ(cog.IsSessionActive(), false);
	cog.StartSession();
	EXPECT_EQ(cog.IsSessionActive(), true);
	cog.EndSession();
	EXPECT_EQ(cog.IsSessionActive(), false);
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
	EXPECT_EQ(map.size(),3);
	EXPECT_EQ(cog.GetNewSessionProperties().size(), 0);
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

	auto c = cog.customevent->SendData();
	EXPECT_EQ(c.size(), 0);

	cog.StartSession();

	cog.customevent->Send("testing1", pos);

	c = cog.customevent->SendData();
	EXPECT_EQ(c["data"].size(), 2);
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
	cog.customevent->Send("testing1", pos);

	auto c = cog.customevent->SendData();
	EXPECT_EQ(c.size(), 0);

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
	cog.StartSession();
	cog.EndSession();
	auto c = cog.customevent->SendData();
	EXPECT_EQ(c.size(), 0);
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
	cog.StartSession();
	auto c = cog.customevent->SendData();
	EXPECT_EQ(c["data"].size(), 2);
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
	auto c = cog.customevent->SendData();
	EXPECT_EQ(c.size(), 0);
	cog.StartSession();
	cog.customevent->Send("testing1", pos, prop);
	c = cog.customevent->SendData();
	EXPECT_EQ(c["data"][1]["properties"]["location"], "vancouver"); //session start is data[0]
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
	cog.EndSession();
	auto c = cog.customevent->SendData();
	EXPECT_EQ(c.size(), 0);
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

	cog.StartSession(); //fourth transaction. should send all
	auto c = cog.customevent->SendData();
	EXPECT_EQ(c["data"].size(), 1);
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
	cog.customevent->Send("testing1", pos);
	auto c = cog.customevent->SendData();
	EXPECT_EQ(c.size(), 0);
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
	cog.customevent->Send("testing2", pos); //send here
	auto c = cog.customevent->SendData();
	EXPECT_EQ(c.size(), 0);	
	cog.customevent->Send("testing1", pos);
	cog.customevent->Send("testing1", pos);
	c = cog.customevent->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	cog.customevent->Send("testing1", pos);

	cog.customevent->Send("testing1", pos);
	cog.EndSession();
	c = cog.customevent->SendData();
	EXPECT_EQ(c.size(), 0);
}

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
	EXPECT_EQ(cog.GetSceneId(), "");

	auto c = cog.customevent->SendData();
	EXPECT_EQ(c["data"].size(), 4);
	cog.SendData();
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

	auto c = cog.customevent->SendData();
	EXPECT_EQ(c["data"].size(), 4);
	cog.SendData();
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
	EXPECT_EQ(cog.GetSceneId(), "");
	cog.SetScene("tutorial");
	EXPECT_EQ(cog.GetSceneId(), "DELETE_ME_1");

	std::vector<float> pos = { 1,0,0 };
	cog.customevent->Send("testing1", pos);
	pos[0] = 2;
	cog.customevent->Send("testing2", pos);
	pos[0] = 3;
	cog.customevent->Send("testing3", pos);
	auto c = cog.customevent->SendData();
	EXPECT_EQ(c["data"].size(), 4);
	cog.SendData();
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
	cog.SetScene("tutorial");

	std::vector<float> pos = { 1,0,0 };
	cog.customevent->Send("testing1", pos);
	cog.SetScene("menu");
	auto c = cog.customevent->SendData();
	EXPECT_EQ(c.size(), 0);
	cog.customevent->Send("testing2", pos);
	c = cog.customevent->SendData();
	EXPECT_EQ(c["data"].size(), 1);
	cog.SetScene("finalboss");
	cog.customevent->Send("testing3", pos);
	c = cog.customevent->SendData();
	EXPECT_EQ(c["data"].size(), 1);
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
	EXPECT_EQ(cog.GetSceneId(), "");
	cog.SetScene("non-existent");
	EXPECT_EQ(cog.GetSceneId(), "");

	std::vector<float> pos = { 1,0,0 };
	cog.customevent->Send("testing1", pos);
	auto c = cog.customevent->SendData();
	EXPECT_EQ(c["data"].size(), 2);

	cog.SendData();
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
	EXPECT_EQ(cog.GetSceneId(), "");
	cog.SetScene("non-existent");
	EXPECT_EQ(cog.GetSceneId(), "");

	std::vector<float> pos = { 1,0,0 };
	cog.customevent->Send("testing1", pos);
	auto c = cog.customevent->SendData();
	EXPECT_EQ(c["data"].size(), 2);

	cog.SendData();
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
	cog.SetScene("non-existent");
	std::vector<float> pos = { 1,0,0 };
	cog.customevent->Send("testing1", pos);
	auto c = cog.customevent->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	cog.SetScene("finalboss");
	cog.customevent->Send("testing1", pos);
	cog.customevent->Send("testing1", pos);

	c = cog.customevent->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	cog.SendData();
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
	EXPECT_EQ(questions.size(), 7);

	cog.exitpoll->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kHappySad, false));
	cog.exitpoll->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kMultiple, 0));
	cog.exitpoll->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kScale, 1));
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
	EXPECT_NE(questionString, "");

	cog.exitpoll->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kHappySad, false));
	cog.exitpoll->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kMultiple, 0));
	cog.exitpoll->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kScale, 1));
	auto a = cog.exitpoll->SendAllAnswers();
	EXPECT_EQ(a["answers"].size(), 3);
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

	auto g = cog.gaze->SendData();
	EXPECT_EQ(g.size(), 0);
	
	cog.StartSession();
	cog.EndSession();
	//EXPECT_EQ(cog.gaze->BatchedGaze.size(), 0); //no scene to send to. endsession clears everything
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
	auto g = cog.gaze->SendData();
	EXPECT_EQ(g.size(), 0);
	cog.gaze->RecordGaze(pos, rot);
	cog.StartSession();
	g = cog.gaze->SendData();
	EXPECT_EQ(g["data"].size(), 1);
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
	cog.StartSession();
	cog.SetScene("gazescene");
	auto g = cog.gaze->SendData();
	EXPECT_EQ(g["data"].size(), 10);
	cog.EndSession();
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

	cog.SetScene("gazescene");
	auto g = cog.gaze->SendData();
	EXPECT_EQ(g["data"].size(), 10);
	cog.EndSession();
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

	cog.SendData();
	auto g = cog.gaze->SendData();
	EXPECT_EQ(g["data"].size(), 0);
	cog.SetScene("gazescene");
	cog.gaze->RecordGaze(pos, rot);
	g = cog.gaze->SendData();
	EXPECT_EQ(g["data"].size(), 1);
	cog.EndSession();
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
	auto g = cog.gaze->SendData();
	EXPECT_EQ(g["data"].size(), 10);

	cog.dynamicobject->RegisterObjectCustomId("object1", "block", "1", pos, rot);

	for (float i = 0; i < 20; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot, point,"1");
	}

	g = cog.gaze->SendData();
	EXPECT_EQ(g["data"].size(), 20);

	cog.SendData();

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
	auto s = cog.sensor->SendData();
	EXPECT_EQ(s["data"].size(), 1);
	EXPECT_EQ(s["data"][0]["data"].size(), 10);
	cog.SendData();
	cog.EndSession();
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
	auto s = cog.sensor->SendData();
	EXPECT_EQ(s.size(), 0);
	cog.sensor->RecordSensor("test-sensor", 3);

	cog.StartSession();
	cog.sensor->RecordSensor("test-sensor", 1);
	cog.sensor->RecordSensor("test-sensor", 2);
	s = cog.sensor->SendData();
	EXPECT_EQ(s.size(), 0);
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
	auto s = cog.sensor->SendData();
	EXPECT_EQ(s.size(), 0);
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
	
	for (int i = 0; i < 6; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}
	auto s = cog.sensor->SendData();
	EXPECT_EQ(s["data"][0]["data"].size(), 1);
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
	for (int i = 0; i < 5; ++i)
	{
		cog.sensor->RecordSensor("test-sensor1", i);
	}
	auto s = cog.sensor->SendData();
	EXPECT_EQ(s["data"][0]["data"].size(), 5);
	EXPECT_EQ(s["data"].size(), 2);
	for (int i = 0; i < 5; ++i)
	{
		cog.sensor->RecordSensor("test-sensor2", i);
	}
	s = cog.sensor->SendData();
	EXPECT_EQ(s["data"][0]["data"].size(), 5);
	EXPECT_EQ(s["data"].size(), 1);
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
	auto s = cog.sensor->SendData();
	EXPECT_EQ(s["data"][0]["data"].size(), 5);
	cog.SetScene("non-existent-scene");
	s = cog.sensor->SendData();
	EXPECT_EQ(s["data"].size(), 0);
	for (int i = 0; i < 6; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}
	s = cog.sensor->SendData();
	EXPECT_EQ(s["data"][0]["data"].size(), 6);
	s = cog.sensor->SendData();
	EXPECT_EQ(s.size(), 0);
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
	auto s = cog.sensor->SendData();
	EXPECT_EQ(s.size(), 0);
	cog.StartSession();
	cog.SetScene("sensescene");
	s = cog.sensor->SendData();
	EXPECT_EQ(s.size(), 0);
	cog.EndSession();
}

TEST(Sensors, InitThenSenseThenSetScene) {
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
	cog.SetScene("sensescene");
	auto s = cog.sensor->SendData();
	EXPECT_EQ(s["data"][0]["data"].size(), 10);
	EXPECT_EQ(s["data"].size(), 1);
	cog.EndSession();
}

TEST(Sensors, InitThenSenseThenSendThenSetScene) {
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
	cog.SendData();
	cog.SetScene("sensescene");
	auto s = cog.sensor->SendData();
	EXPECT_EQ(s.size(), 0);
	cog.EndSession();
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

	auto s = cog.sensor->SendData();
	EXPECT_EQ(s["data"][0]["data"].size(), 10);
	EXPECT_EQ(s["data"][1]["data"].size(), 10);
	EXPECT_EQ(s["data"].size(), 2);

	cog.SetScene("sensescene");
	cog.EndSession();
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
	auto d = cog.dynamicobject->SendData();
	EXPECT_EQ(d.size(), 6);
	EXPECT_EQ(d["data"].size(), 1);
	EXPECT_EQ(d["manifest"].size(), 1);
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

	auto d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(), 2);
	EXPECT_EQ(d["manifest"].size(), 2);


	pos = { 0,0,5 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,1,6 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);

	pos = { 0,0,7 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,2,8 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);

	pos = { 0,0,9 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,3,10 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	
	d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(), 6);
	EXPECT_EQ(d["manifest"].size(), 0);
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

	cog.dynamicobject->AddSnapshot(object1id, pos, rot);

	cog.SetScene("two"); //refreshes object manifest

	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	auto d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(), 2);
	EXPECT_EQ(d["manifest"].size(), 2);
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

	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);

	auto d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(), 5);
	EXPECT_EQ(d["manifest"].size(), 2);
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
	
	auto d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(), 3);
	EXPECT_EQ(d["manifest"].size(), 1);
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
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object1id, pos, rot); //manifest + snapshots = limit

	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	auto d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(), 2);
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
	cog.dynamicobject->RegisterObjectCustomId("object2", "lamp", "2", pos, rot);
	cog.dynamicobject->RegisterObjectCustomId("object3", "lamp", "3", pos, rot); //limit. send
	cog.dynamicobject->RegisterObjectCustomId("object4", "lamp", "4", pos, rot);
	cog.dynamicobject->RegisterObjectCustomId("object5", "lamp", "5", pos, rot); //limit send
	auto d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(), 2);
	EXPECT_EQ(d["manifest"].size(), 2);
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
	cog.dynamicobject->RegisterObjectCustomId("object2", "lamp", "2", pos, rot);
	cog.dynamicobject->RegisterObjectCustomId("object3", "lamp", "3", pos, rot);
	cog.dynamicobject->RegisterObjectCustomId("object4", "lamp", "4", pos, rot);
	cog.dynamicobject->RegisterObjectCustomId("object5", "lamp", "5", pos, rot);
	
	auto d = cog.dynamicobject->SendData();
	EXPECT_EQ(d.size(), 0);

	cog.StartSession();
	cog.dynamicobject->RegisterObjectCustomId("object5", "lamp", "6", pos, rot); //auto snapshot and manifest triggers send
	d = cog.dynamicobject->SendData();
	EXPECT_EQ(d.size(), 0);
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

	cog.dynamicobject->RemoveObject(object1id, pos, rot);
	std::string object3id = cog.dynamicobject->RegisterObject("object3", "block", pos, rot);
	std::string object4id = cog.dynamicobject->RegisterObject("object4", "lamp", pos, rot);
	
	auto d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(), 5);
	EXPECT_EQ(d["manifest"].size(), 3);

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

	cog.dynamicobject->BeginEngagement("1", "grab");
	cog.dynamicobject->AddSnapshot("1", pos, rot); //snapshot will not add an entry to manifest automatically
	cog.dynamicobject->EndEngagement("1", "grab");

	auto d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(), 1);
	EXPECT_EQ(d["manifest"].size(), 0);

	cog.dynamicobject->RegisterObjectCustomId("object1", "lamp","1", pos, rot);
	d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(), 1);
	EXPECT_EQ(d["manifest"].size(), 1);

	cog.SendData();
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
	
	auto d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(),1);
	EXPECT_EQ(d["manifest"].size(), 0);
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
	std::string object2id = cog.dynamicobject->RegisterObject("object2", "lamp", pos, rot);
	cog.dynamicobject->BeginEngagement(object1id, "grab");
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->EndEngagement(object1id, "grab");

	auto d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(), 3);
	EXPECT_EQ(d["manifest"].size(), 2);

	cog.SetScene("two");

	cog.dynamicobject->BeginEngagement(object1id, "grab");
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	cog.dynamicobject->BeginEngagement(object2id, "grab");
	cog.dynamicobject->EndEngagement(object1id, "grab");
	cog.dynamicobject->EndEngagement(object2id, "grab");
	
	d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(), 2);
	EXPECT_EQ(d["manifest"].size(), 2);

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
	
	auto d = cog.dynamicobject->SendData();
	EXPECT_EQ(d["data"].size(), 3);
	EXPECT_EQ(d["manifest"].size(), 1);

	cog.SendData();
}


int main(int argc, char **argv)
{
	if (const char* env_p = std::getenv("COGNITIVEAPIKEY"))
	{
		//std::cout << "Custom APIKEY is: " << env_p << '\n';
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