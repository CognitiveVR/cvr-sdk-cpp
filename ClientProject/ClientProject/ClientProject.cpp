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
#include <mutex>

//requires a valid API key from travis command line. if testing and expecting not to have this key, can disable all tests that are expected to fail
#define EXITPOLLVALID

#if defined(_MSC_VER)
#include "gtest.h"
#else
#include "include/gtest/gtest.h"
#endif

//std::string temp;

std::string TESTINGAPIKEY = "asdf1234hjkl5678";
std::string INVALIDAPIKEY = "INVALID_API_KEY";
long TestDelay = 1;

//size_t handle(char* buf, size_t size, size_t nmemb, void* up)
//{
//	for (int c = 0; c < size*nmemb; c++)
//	{
//		temp.push_back(buf[c]);
//	}
//	return size * nmemb;
//}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
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
		std::string responseBody;
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);

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
			response(responseBody);

		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		else
		{

		}

		/* always cleanup */
		curl_easy_cleanup(curl);
		curl_slist_free_all(list); /* free the list again */
	}
}

void myThread(std::string url, std::string content, std::vector<std::string> headers,cognitive::WebResponse response)
{
	CURL* curl;
	CURLcode res;
	curl = curl_easy_init();

	struct curl_slist *list = NULL;

	std::string readBuffer;

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

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

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
			response(readBuffer);

		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		else
		{
			fprintf(stderr, "curl_easy_perform() passed: %s\n", curl_easy_strerror(res));
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
		curl_slist_free_all(list); /* free the list again */
	}
}

static std::vector<std::thread*> activeThreads;

void DoAsyncWebStuff(std::string url, std::string content, std::vector<std::string> headers, cognitive::WebResponse response)
{
	//TODO don't allow unlimited threads
	curl_global_init(CURL_GLOBAL_ALL);
	std::thread* t1 = new std::thread(myThread, url,content,headers,response);

	activeThreads.push_back(t1);
}


//===========================TESTS
TEST(Curl, AsyncThread) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoAsyncWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "tutorial";


	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.GetSensor()->RecordSensor("sensor", 1);
	cog.GetCustomEvent()->RecordEvent("event", pos);
	cog.GetDynamicObject()->RegisterObject("object", "mesh", pos, rot);
	cog.GetFixation()->RecordFixation(1000, 100, 0.5f, pos);

	cog.StartSession();

	//send any data
	auto d = cog.GetGazeTracker()->SendData();
	EXPECT_NE(d.size(), 0);

	//join all outstanding threads
	for (int i = 0; i < activeThreads.size(); i++)
	{
		activeThreads[i]->join();
		delete(activeThreads[i]);
	}
	activeThreads.clear();
}

TEST(Curl, MultipleAsyncThread) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoAsyncWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "tutorial";


	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.GetSensor()->RecordSensor("sensor", 1);
	cog.GetCustomEvent()->RecordEvent("event", pos);
	cog.GetDynamicObject()->RegisterObject("object", "mesh", pos, rot);
	cog.GetFixation()->RecordFixation(1000, 100, 0.5f, pos);

	cog.StartSession();

	//send data 1
	auto d = cog.GetGazeTracker()->SendData();
	EXPECT_NE(d.size(), 0);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.GetSensor()->RecordSensor("sensor", 1);
	cog.GetCustomEvent()->RecordEvent("event", pos);
	cog.GetDynamicObject()->RegisterObject("object", "mesh", pos, rot);
	cog.GetFixation()->RecordFixation(1000, 100, 0.5f, pos);

	//send data 2
	d = cog.GetGazeTracker()->SendData();
	EXPECT_NE(d.size(), 0);

	//join all outstanding threads
	for (int i = 0; i < activeThreads.size(); i++)
	{
		activeThreads[i]->join();
		delete(activeThreads[i]);
	}
	activeThreads.clear();
}

TEST(Curl, GazeAsyncThread) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoAsyncWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "tutorial";


	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();


	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);

	//send data 1
	auto d = cog.GetGazeTracker()->SendData();
	EXPECT_NE(d.size(), 0);

	cog.SetSessionName("client project test");
	cog.SetSessionProperty("cpu", "good");
	cog.SetSessionProperty("gpu", "good");
	cog.SetSessionProperty("harddrive", "large");

	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);


	//send data 2
	d = cog.GetGazeTracker()->SendData();
	EXPECT_NE(d.size(), 0);

	//join all outstanding threads
	for (int i = 0; i < activeThreads.size(); i++)
	{
		activeThreads[i]->join();
		delete(activeThreads[i]);
	}
	activeThreads.clear();
}

//----------------------INITIALIZATION

TEST(Initialization, StartSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	EXPECT_EQ(cog.IsSessionActive(), false);
	bool first = cog.StartSession();
	EXPECT_EQ(first, true);
}

TEST(Initialization, StartSessionNoUser) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	//cog.SetUserName("travis");

	EXPECT_EQ(cog.IsSessionActive(), false);
	bool first = cog.StartSession();
	EXPECT_EQ(first, false);
}

TEST(Initialization, StartSessionNoApiKey) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	EXPECT_EQ(cog.IsSessionActive(), false);
	bool first = cog.StartSession();
	EXPECT_EQ(first, true);
}

TEST(Initialization, StartSessionMultiple) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

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
	cog.SetUserName("travis");

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

TEST(Initialization, DefaultScene) {
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
	cog.SetUserName("travis");
	EXPECT_EQ(cog.GetSceneId(), "DELETE_ME_1");
	cog.StartSession();
}

TEST(Initialization, SetScene){
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");
	EXPECT_EQ(cog.GetSceneId(), "");
	cog.StartSession();
	EXPECT_EQ(cog.GetSceneId(), "");
	cog.SetScene("tutorial");
	EXPECT_EQ(cog.GetSceneId(), "DELETE_ME_1");
}

TEST(Initialization, SetSceneById) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");
	EXPECT_EQ(cog.GetSceneId(), "");
	cog.StartSession();
	EXPECT_EQ(cog.GetSceneId(), "");
	cog.SetSceneById("asdf1234");
	EXPECT_EQ(cog.GetSceneId(), "asdf1234");
	EXPECT_EQ(cog.GetCurrentSceneVersionNumber(), "");
}

TEST(Initialization, SetSceneByIdVersion) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");
	EXPECT_EQ(cog.GetSceneId(), "");
	cog.StartSession();
	EXPECT_EQ(cog.GetSceneId(), "");
	cog.SetSceneById("asdf1234","4");
	EXPECT_EQ(cog.GetSceneId(), "asdf1234");
	EXPECT_EQ(cog.GetCurrentSceneVersionNumber(), "4");
}

TEST(Initialization, SetLobbyId){
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");
	cog.SetLobbyId("my lobby id");

	cog.StartSession();
	EXPECT_EQ(cog.GetLobbyId(), "my lobby id");
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
	cog.SetUserName("travis");
	cog.StartSession();
	cog.SetSessionName("my friendly session name");

	auto c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c["properties"]["c3d.sessionname"], "my friendly session name");
}

TEST(Initialization, SessionFullStartEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));
	
	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	cog.StartSession();
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	cog.EndSession();
	EXPECT_EQ(cog.IsSessionActive(), false);
}

TEST(Initialization, SessionStartNoWeb) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	EXPECT_EQ(cog.IsSessionActive(), false);
	cog.StartSession();
	EXPECT_EQ(cog.IsSessionActive(), true);
}

TEST(Initialization, SessionStartNoWebResponse) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.webRequest = &NORESPONSE_DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	EXPECT_EQ(cog.IsSessionActive(), false);
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
	cog.SetUserName("travis");

	EXPECT_EQ(cog.GetAPIKey(), INVALIDAPIKEY);
	EXPECT_EQ(cog.IsSessionActive(), false);
	
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
	cog.SetUserName("travis");

	EXPECT_NE(cognitive::CognitiveVRAnalyticsCore::Instance(), nullptr);
}

TEST(Initialization, SessionEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");
	EXPECT_EQ(cog.IsSessionActive(), false);

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
	cog.SetUserName("travis");

	EXPECT_EQ(cog.IsSessionActive(), false);
	cog.StartSession();
	EXPECT_EQ(cog.IsSessionActive(), true);
	cog.EndSession();
	EXPECT_EQ(cog.IsSessionActive(), false);
}

//----------------------GENERAL

TEST(CustomEvent, UniquePtr) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::unique_ptr<cognitive::CognitiveVRAnalyticsCore> cog(new cognitive::CognitiveVRAnalyticsCore(settings));
	cog->SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	cog->GetCustomEvent()->RecordEvent("testing1", pos);

	cog->StartSession();
	auto c = cog->GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"].size(), 2);	
}

TEST(General, CoreSendDataPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.GetSensor()->RecordSensor("sensor", 1);
	cog.GetCustomEvent()->RecordEvent("event", pos);
	cog.GetDynamicObject()->RegisterObject("object", "mesh", pos, rot);
	cog.GetFixation()->RecordFixation(1000, 100, 0.5f, pos);

	cog.SendData();

	auto d = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(d.size(), 0);
	d = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(d.size(), 0);
	d = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(d.size(), 0);
	d = cog.GetSensor()->SendData();
	EXPECT_EQ(d.size(), 0);
	d = cog.GetFixation()->SendData();
	EXPECT_EQ(d.size(), 0);
}

TEST(General, CoreSendData) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.GetSensor()->RecordSensor("sensor", 1);
	cog.GetCustomEvent()->RecordEvent("event", pos);
	cog.GetDynamicObject()->RegisterObject("object", "mesh", pos, rot);
	cog.GetFixation()->RecordFixation(1000, 100, 0.5f, pos);

	cog.StartSession();

	//send any data
	auto d = cog.GetGazeTracker()->SendData();
	EXPECT_NE(d.size(), 0);
	d = cog.GetCustomEvent()->SendData();
	EXPECT_NE(d.size(), 0);
	d = cog.GetDynamicObject()->SendData();
	EXPECT_NE(d.size(), 0);
	d = cog.GetSensor()->SendData();
	EXPECT_NE(d.size(), 0);
	d = cog.GetFixation()->SendData();
	EXPECT_NE(d.size(), 0);

	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.GetSensor()->RecordSensor("sensor", 1);
	cog.GetCustomEvent()->RecordEvent("event", pos);
	cog.GetDynamicObject()->RegisterObject("object", "mesh", pos, rot);
	cog.GetFixation()->RecordFixation(1000, 100, 0.5f, pos);

	//sends all data
	cog.SendData();

	//nothing to send
	d = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(d.size(), 0);
	d = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(d.size(), 0);
	d = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(d.size(), 0);
	d = cog.GetSensor()->SendData();
	EXPECT_EQ(d.size(), 0);
	d = cog.GetFixation()->SendData();
	EXPECT_EQ(d.size(), 0);
}

TEST(General, CoreSendDataEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.GetSensor()->RecordSensor("sensor", 1);
	cog.GetCustomEvent()->RecordEvent("event", pos);
	cog.GetDynamicObject()->RegisterObject("object", "mesh", pos, rot);
	cog.GetFixation()->RecordFixation(1000, 100, 0.5f, pos);

	cog.StartSession();
	cog.EndSession(); //end calls senddata

	//nothing to send
	auto d = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(d.size(), 0);
	d = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(d.size(), 0);
	d = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(d.size(), 0);
	d = cog.GetSensor()->SendData();
	EXPECT_EQ(d.size(), 0);
	d = cog.GetFixation()->SendData();
	EXPECT_EQ(d.size(), 0);
}

TEST(General, JsonPartSendData) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();

	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.GetSensor()->RecordSensor("sensor", 1);
	cog.GetCustomEvent()->RecordEvent("event", pos);
	cog.GetDynamicObject()->RegisterObject("object", "mesh", pos, rot);
	cog.GetFixation()->RecordFixation(1000, 100, 0.5f, pos);

	cog.SendData();

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.GetSensor()->RecordSensor("sensor", 1);
	cog.GetCustomEvent()->RecordEvent("event", pos);
	cog.GetDynamicObject()->RegisterObject("object", "mesh", pos, rot);
	cog.GetFixation()->RecordFixation(1000, 100, 0.5f, pos);


	//json part increments
	auto d = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(d["part"],2);
	d = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(d["part"], 2);
	d = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(d["part"], 2);
	d = cog.GetSensor()->SendData();
	EXPECT_EQ(d["part"], 2);
	d = cog.GetFixation()->SendData();
	EXPECT_EQ(d["part"], 2);
}

TEST(General, Lobby) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.SetLobbyId("7891234");

	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.GetSensor()->RecordSensor("sensor", 1);
	cog.GetCustomEvent()->RecordEvent("event", pos);
	cog.GetDynamicObject()->RegisterObject("object", "mesh", pos, rot);
	cog.GetFixation()->RecordFixation(1000, 100, 0.5f, pos);

	//send any data
	auto d = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(d["lobbyId"],"7891234");
	d = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(d["lobbyId"], "7891234");
	d = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(d["lobbyId"], "7891234");
	d = cog.GetSensor()->SendData();
	EXPECT_EQ(d["lobbyId"], "7891234");
	d = cog.GetFixation()->SendData();
	EXPECT_EQ(d["lobbyId"], "7891234");
}

//----------------------PROPERTIES

TEST(SessionProperties, PreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.loggingLevel = cognitive::LoggingLevel::kAll;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetUserName("john");
	cog.SetDeviceName("12345678");
	cog.SetSessionProperty("age", 21);
	cog.SetSessionProperty("location", "vancouver");
	
	auto props = cog.GetNewSessionProperties();
	EXPECT_EQ(cog.GetNewSessionProperties().size(), 0);
	EXPECT_EQ(props.size(), 4);

	cog.StartSession();
}

TEST(SessionProperties, DuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetUserName("john");
	cog.StartSession();
	cog.SetSessionProperty("age", 21);
	cog.SetSessionProperty("location", "vancouver");
	
	cog.SendData();
	EXPECT_EQ(cog.GetNewSessionProperties().size(), 0);
}

TEST(SessionProperties, DuringSessionGaze) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetUserName("john");
	cog.StartSession();
	cog.SetSessionProperty("age", 21);
	cog.SetSessionProperty("location", "vancouver");

	cog.GetGazeTracker()->SendData();
	EXPECT_EQ(cog.GetNewSessionProperties().size(), 0);
}

TEST(SessionProperties, DuringSessionValues) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetUserName("john");
	cog.StartSession();
	cog.SetSessionProperty("age", 21);
	cog.SetSessionProperty("location", "seattle");
	cog.SetSessionProperty("location", "vancouver");

	auto p = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(p["properties"]["age"], 21);
	EXPECT_EQ(p["properties"]["c3d.username"], "john");
	EXPECT_EQ(p["properties"]["location"], "vancouver");
}

TEST(SessionProperties, PostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetUserName("john");
	cog.StartSession();
	cog.SetSessionProperty("age", 21);
	cog.SetSessionProperty("location", "seattle");
	cog.SetSessionProperty("location", "vancouver");
	cog.EndSession(); //send data should send gaze with all session properties

	auto p = cog.GetNewSessionProperties();
	EXPECT_EQ(p.size(), 0);
}

TEST(SessionProperties, SceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetUserName("john");
	cog.StartSession();
	cog.SetSessionProperty("age", 21);
	cog.SetSessionProperty("location", "vancouver");
	auto p = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(p["properties"]["age"], 21);
	EXPECT_EQ(p["properties"]["c3d.username"], "john");
	EXPECT_EQ(p["properties"]["location"], "vancouver");

	cog.SetScene("tutorial");
	p = cog.GetGazeTracker()->SendData(); //not a 'new' scene, but needs to know properties not yet reported to it
	EXPECT_EQ(p["properties"]["age"], 21);
	EXPECT_EQ(p["properties"]["c3d.username"], "john");
	EXPECT_EQ(p["properties"]["location"], "vancouver");
}

TEST(SessionProperties, InvalidSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetUserName("john");
	cog.StartSession();
	cog.SetSessionProperty("age", 21);
	cog.SetSessionProperty("location", "vancouver");
	auto p = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(p["properties"]["age"], 21);
	EXPECT_EQ(p["properties"]["c3d.username"], "john");
	EXPECT_EQ(p["properties"]["location"], "vancouver");

	cog.SetScene("invalid");
	p = cog.GetGazeTracker()->SendData(); //not a 'new' scene, but needs to know properties not yet reported to it
	EXPECT_EQ(p.size(), 0);
}

TEST(SessionProperties, SceneChangeMultiple) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetUserName("john");
	cog.StartSession();
	cog.SetSessionProperty("age", 21);
	cog.SetSessionProperty("location", "vancouver");

	cog.SetScene("tutorial");
	auto p = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(p["properties"]["age"], 21);
	EXPECT_EQ(p["properties"]["c3d.username"], "john");
	EXPECT_EQ(p["properties"]["location"], "vancouver");

	cog.SetScene("tutorial");
	p = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(p["properties"]["age"], 21);
	EXPECT_EQ(p["properties"]["c3d.username"], "john");
	EXPECT_EQ(p["properties"]["location"], "vancouver");

	cog.SetScene("invalid");
	p = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(p.size(), 0);

	cog.SetScene("tutorial");
	p = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(p["properties"]["age"], 21);
	EXPECT_EQ(p["properties"]["c3d.username"], "john");
	EXPECT_EQ(p["properties"]["location"], "vancouver");
}

//----------------------CUSTOM EVENTS

TEST(CustomEvent, PreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	cog.GetCustomEvent()->RecordEvent("testing1", pos);

	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(CustomEvent, DuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	cog.GetCustomEvent()->RecordEvent("testing1", pos);

	cog.StartSession();
	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
}

TEST(CustomEvent, PostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	
	cog.StartSession();
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	cog.EndSession();

	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(CustomEvent, PostSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };

	cog.StartSession();
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"].size(), 2);

	cog.SetScene("tutorial");
	c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(CustomEvent, PostSceneChangeMultiple) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };

	cog.StartSession();
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"].size(), 2);

	cog.SetScene("tutorial");
	cog.GetCustomEvent()->RecordEvent("testing1", pos);

	cog.SetScene("tutorial");
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(CustomEvent, PostInvalidSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };

	cog.StartSession();
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"].size(), 2);

	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	cog.SetScene("invalid");
	c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(CustomEvent, Values) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	cognitive::nlohmann::json prop = cognitive::nlohmann::json();
	prop["number"] = 1;
	prop["text"] = "rewrite";
	prop["text"] = "string";

	std::string dynamicId = "asdf1234";

	cog.StartSession();
	double timestamp = cog.GetSessionTimestamp();
	cog.GetCustomEvent()->RecordEvent("testing1", pos, prop);
	cog.GetCustomEvent()->RecordEvent("testing2", pos, prop,dynamicId);
	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["userid"], "travis");
	EXPECT_EQ(c["part"], 1);
	EXPECT_EQ(c["formatversion"], "1.0");
	EXPECT_EQ(c["timestamp"], (int)timestamp);
	EXPECT_EQ(c["data"].size(), 3);
	EXPECT_EQ(c["data"][0]["name"], "c3d.sessionStart");
	EXPECT_EQ(c["data"][1]["name"], "testing1");
	EXPECT_EQ(c["data"][1]["properties"].size(), 2);
	EXPECT_EQ(c["data"][1]["properties"]["number"], 1);
	EXPECT_EQ(c["data"][1]["properties"]["text"], "string");
	EXPECT_EQ(c["data"][2]["name"], "testing2");
	EXPECT_EQ(c["data"][2]["dynamicId"], "asdf1234");
	EXPECT_EQ(c["data"][2]["properties"].size(), 2);
	EXPECT_EQ(c["data"][2]["properties"]["number"], 1);
	EXPECT_EQ(c["data"][2]["properties"]["text"], "string");
}

TEST(CustomEvent, LimitPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.CustomEventBatchSize = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };

	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	cog.GetCustomEvent()->RecordEvent("testing1", pos); //clear

	cog.StartSession();
	
	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(CustomEvent, LimitDuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.CustomEventBatchSize = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();

	std::vector<float> pos = { 0,0,0 };

	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	cog.GetCustomEvent()->RecordEvent("testing1", pos);//clear
	cog.GetCustomEvent()->RecordEvent("testing1", pos);

	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(CustomEvent, LimitPostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.CustomEventBatchSize = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	cog.StartSession(); //record
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	cog.EndSession(); //send

	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	cog.StartSession();

	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"].size(), 3);
}

TEST(CustomEvent, WithDynamic) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0 };
	std::string dynamicId = cog.GetDynamicObject()->RegisterObject("lamp01", "lamp", pos, rot);

	cog.GetCustomEvent()->RecordEvent("testing1", pos, dynamicId);

	cog.StartSession();
	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"][0]["name"], "testing1");
	EXPECT_EQ(c["data"][0]["dynamicId"], "1000"); //generated
	EXPECT_EQ(c["data"][1]["name"], "c3d.sessionStart");
}

TEST(CustomEvent, NoDynamic) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::string dynamicId;

	cog.GetCustomEvent()->RecordEvent("testing1", pos, dynamicId);

	cog.StartSession();
	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"][0]["name"], "testing1");
	EXPECT_EQ(c["data"][0]["dynamicId"], nullptr);
	EXPECT_EQ(c["data"][1]["name"], "c3d.sessionStart");
}

//-----------------------------------SCENES

TEST(Scenes, NoScenes) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();

	std::vector<float> pos = { 1,0,0 };
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	pos[0] = 2;
	cog.GetCustomEvent()->RecordEvent("testing2", pos);
	pos[0] = 3;
	cog.GetCustomEvent()->RecordEvent("testing3", pos);
	EXPECT_EQ(cog.GetSceneId(), "");

	auto c = cog.GetCustomEvent()->SendData();
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
	cog.SetUserName("travis");

	cog.StartSession();

	std::vector<float> pos = { 1,0,0 };
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	pos[0] = 2;
	cog.GetCustomEvent()->RecordEvent("testing2", pos);
	pos[0] = 3;
	cog.GetCustomEvent()->RecordEvent("testing3", pos);

	auto c = cog.GetCustomEvent()->SendData();
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
	cog.SetUserName("travis");

	cog.StartSession();
	EXPECT_EQ(cog.GetSceneId(), "");
	cog.SetScene("tutorial");
	EXPECT_EQ(cog.GetSceneId(), "DELETE_ME_1");

	std::vector<float> pos = { 1,0,0 };
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	pos[0] = 2;
	cog.GetCustomEvent()->RecordEvent("testing2", pos);
	pos[0] = 3;
	cog.GetCustomEvent()->RecordEvent("testing3", pos);
	auto c = cog.GetCustomEvent()->SendData();
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
	cog.SetUserName("travis");

	cog.StartSession();
	cog.SetScene("tutorial");

	std::vector<float> pos = { 1,0,0 };
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	cog.SetScene("menu");
	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c.size(), 0);
	cog.GetCustomEvent()->RecordEvent("testing2", pos);
	c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
	cog.SetScene("finalboss");
	cog.GetCustomEvent()->RecordEvent("testing3", pos);
	c = cog.GetCustomEvent()->SendData();
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
	cog.SetUserName("travis");

	cog.StartSession();
	EXPECT_EQ(cog.GetSceneId(), "");
	cog.SetScene("non-existent");
	EXPECT_EQ(cog.GetSceneId(), "");

	std::vector<float> pos = { 1,0,0 };
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	auto c = cog.GetCustomEvent()->SendData();
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
	cog.SetUserName("travis");

	cog.StartSession();
	EXPECT_EQ(cog.GetSceneId(), "");
	cog.SetScene("non-existent");
	EXPECT_EQ(cog.GetSceneId(), "");

	std::vector<float> pos = { 1,0,0 };
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	auto c = cog.GetCustomEvent()->SendData();
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
	cog.SetUserName("travis");

	cog.StartSession();
	cog.SetScene("non-existent");
	std::vector<float> pos = { 1,0,0 };
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	cog.SetScene("finalboss");
	cog.GetCustomEvent()->RecordEvent("testing1", pos);
	cog.GetCustomEvent()->RecordEvent("testing1", pos);

	c = cog.GetCustomEvent()->SendData();
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
	cog.SetUserName("travis");

	cog.GetExitPoll()->RequestQuestionSet("testing_new_sdk");

	EXPECT_EQ(cog.GetExitPoll()->GetQuestionSetString(), "");

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
	cog.SetUserName("travis");
	
	cog.StartSession();
	cog.GetExitPoll()->RequestQuestionSet("testing_new_sdk");
	auto q = cog.GetExitPoll()->GetQuestionSet();

#ifdef EXITPOLLVALID

	EXPECT_NE(cog.GetExitPoll()->GetQuestionSetString(), "");
	EXPECT_EQ(q["id"], "testing:1");
	EXPECT_EQ(q["version"], 1);
	EXPECT_EQ(q["title"], "Testing new dash Exitpoll");
	EXPECT_EQ(q["status"], "active");
	EXPECT_EQ(q["questions"].size(), 6);
	
	//true false
	EXPECT_EQ(q["questions"][0]["title"], "TF");
	EXPECT_EQ(q["questions"][0]["type"], "BOOLEAN");
	
	//happy sad
	EXPECT_EQ(q["questions"][1]["title"], "Happy");
	EXPECT_EQ(q["questions"][1]["type"], "HAPPYSAD");
	
	//thumbs up down
	EXPECT_EQ(q["questions"][2]["title"], "Thumbs");
	EXPECT_EQ(q["questions"][2]["type"], "THUMBS");
	
	//multiple choice
	EXPECT_EQ(q["questions"][3]["title"], "MC");
	EXPECT_EQ(q["questions"][3]["type"], "MULTIPLE");
	EXPECT_EQ(q["questions"][3]["answers"].size(), 4);
	EXPECT_EQ(q["questions"][3]["answers"][0]["answer"], "answer one");
	EXPECT_EQ(q["questions"][3]["answers"][1]["answer"], "answer two");
	EXPECT_EQ(q["questions"][3]["answers"][2]["answer"], "answer three");
	EXPECT_EQ(q["questions"][3]["answers"][3]["answer"], "answer four");
	
	//multiple choice
	EXPECT_EQ(q["questions"][4]["title"], "scale");
	EXPECT_EQ(q["questions"][4]["type"], "SCALE");
	EXPECT_EQ(q["questions"][4]["minLabel"], "lower label");
	EXPECT_EQ(q["questions"][4]["maxLabel"], "upper label");
	EXPECT_EQ(q["questions"][4]["range"]["start"], 0);
	EXPECT_EQ(q["questions"][4]["range"]["end"], 10);
	
	//multiple choice
	EXPECT_EQ(q["questions"][5]["title"], "voice q");
	EXPECT_EQ(q["questions"][5]["type"], "VOICE");
	EXPECT_EQ(q["questions"][5]["maxResponseLength"], 20);
#endif // EXITPOLLVALID
}

TEST(ExitPoll, GetThenRequest) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

#ifdef EXITPOLLVALID
	cog.StartSession();
	EXPECT_EQ(cog.GetExitPoll()->GetQuestionSetString(), "");
	cog.GetExitPoll()->RequestQuestionSet("testing_new_sdk");
	EXPECT_NE(cog.GetExitPoll()->GetQuestionSetString(), "");
	cog.EndSession();
#endif // EXITPOLLVALID
}

TEST(ExitPoll, InvalidRequestThenGet) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();
	cog.GetExitPoll()->RequestQuestionSet("question-does-not-exist");
	EXPECT_EQ(cog.GetExitPoll()->GetQuestionSetString(), "");
	cog.EndSession();
}

TEST(ExitPoll, RequestThenGetAnswersJson) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();
	cog.GetExitPoll()->RequestQuestionSet("testing_new_sdk");
	cognitive::nlohmann::json questions = cog.GetExitPoll()->GetQuestionSet();

#ifdef EXITPOLLVALID
	EXPECT_EQ(questions.size(), 7);
#endif

	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kHappySad, false));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kMultiple, 0));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kScale, 1));
	cog.GetExitPoll()->SendAllAnswers();
	auto a = cog.GetExitPoll()->SendAllAnswers();
	EXPECT_EQ(a.size(), 0);
	cog.EndSession();
}

TEST(ExitPoll, RequestThenGetAnswersString) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();
	cog.GetExitPoll()->RequestQuestionSet("testing_new_sdk");
	std::string questionString = cog.GetExitPoll()->GetQuestionSetString();

#ifdef EXITPOLLVALID
	EXPECT_NE(questionString, "");
#endif

	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kHappySad, false));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kMultiple, 0));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kScale, 1));
	auto a = cog.GetExitPoll()->SendAllAnswers();
	EXPECT_EQ(a["answers"].size(), 3);
	cog.EndSession();
}

TEST(ExitPoll, AnswerValues) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "6", 2));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "tutorial";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();
	double timestamp = cog.GetSessionTimestamp();
	cog.GetExitPoll()->RequestQuestionSet("testing_new_sdk");

	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kBoolean, true));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kHappySad, false));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kThumbs, true));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kMultiple, 2));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kScale, 9));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kVoice, "ASDF=="));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kBoolean, -1)); //skip
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kBoolean, -32768)); //skip
	auto a = cog.GetExitPoll()->SendAllAnswers();

#ifdef EXITPOLLVALID
	EXPECT_EQ(a["userId"], "travis");

	EXPECT_EQ(a["questionSetId"], "testing:1");
	EXPECT_EQ(a["hook"], "testing_new_sdk");
	
	EXPECT_EQ(a["sceneId"], "DELETE_ME_1");
	EXPECT_EQ(a["sessionId"], cog.GetSessionID());
	EXPECT_EQ(a["versionNumber"], "6");
	EXPECT_EQ(a["versionId"], 0);
	
	EXPECT_EQ(a["answers"].size(), 8);
	
	EXPECT_EQ(a["answers"][0]["type"], "BOOLEAN");
	EXPECT_EQ(a["answers"][0]["value"], 1);
	
	EXPECT_EQ(a["answers"][1]["type"], "HAPPYSAD");
	EXPECT_EQ(a["answers"][1]["value"], 0);
	
	EXPECT_EQ(a["answers"][2]["type"], "THUMBS");
	EXPECT_EQ(a["answers"][2]["value"], 1);
	
	EXPECT_EQ(a["answers"][3]["type"], "MULTIPLE");
	EXPECT_EQ(a["answers"][3]["value"], 2);
	
	EXPECT_EQ(a["answers"][4]["type"], "SCALE");
	EXPECT_EQ(a["answers"][4]["value"], 9);
	
	EXPECT_EQ(a["answers"][5]["type"], "VOICE");
	EXPECT_EQ(a["answers"][5]["value"], "ASDF==");
	
	EXPECT_EQ(a["answers"][6]["type"], "BOOLEAN");
	EXPECT_EQ(a["answers"][6]["value"], -1);
	
	EXPECT_EQ(a["answers"][7]["type"], "BOOLEAN");
	EXPECT_EQ(a["answers"][7]["value"], -32768);
#endif
}


TEST(ExitPoll, AsyncBasicRequest) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoAsyncWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.loggingLevel = cognitive::LoggingLevel::kAll;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();
	cog.GetExitPoll()->RequestQuestionSet("testing_new_sdk");

	int loopLimit = 5; //5 seconds
	while (!cog.GetExitPoll()->HasQuestionSet() && loopLimit > 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		loopLimit--;
	}

	auto q = cog.GetExitPoll()->GetQuestionSet();

#ifdef EXITPOLLVALID

	EXPECT_NE(cog.GetExitPoll()->GetQuestionSetString(), "");
	EXPECT_EQ(q["id"], "testing:1");
	EXPECT_EQ(q["version"], 1);
	EXPECT_EQ(q["title"], "Testing new dash Exitpoll");
	EXPECT_EQ(q["status"], "active");
	EXPECT_EQ(q["questions"].size(), 6);

	//true false
	EXPECT_EQ(q["questions"][0]["title"], "TF");
	EXPECT_EQ(q["questions"][0]["type"], "BOOLEAN");

	//happy sad
	EXPECT_EQ(q["questions"][1]["title"], "Happy");
	EXPECT_EQ(q["questions"][1]["type"], "HAPPYSAD");

	//thumbs up down
	EXPECT_EQ(q["questions"][2]["title"], "Thumbs");
	EXPECT_EQ(q["questions"][2]["type"], "THUMBS");

	//multiple choice
	EXPECT_EQ(q["questions"][3]["title"], "MC");
	EXPECT_EQ(q["questions"][3]["type"], "MULTIPLE");
	EXPECT_EQ(q["questions"][3]["answers"].size(), 4);
	EXPECT_EQ(q["questions"][3]["answers"][0]["answer"], "answer one");
	EXPECT_EQ(q["questions"][3]["answers"][1]["answer"], "answer two");
	EXPECT_EQ(q["questions"][3]["answers"][2]["answer"], "answer three");
	EXPECT_EQ(q["questions"][3]["answers"][3]["answer"], "answer four");

	//multiple choice
	EXPECT_EQ(q["questions"][4]["title"], "scale");
	EXPECT_EQ(q["questions"][4]["type"], "SCALE");
	EXPECT_EQ(q["questions"][4]["minLabel"], "lower label");
	EXPECT_EQ(q["questions"][4]["maxLabel"], "upper label");
	EXPECT_EQ(q["questions"][4]["range"]["start"], 0);
	EXPECT_EQ(q["questions"][4]["range"]["end"], 10);

	//multiple choice
	EXPECT_EQ(q["questions"][5]["title"], "voice q");
	EXPECT_EQ(q["questions"][5]["type"], "VOICE");
	EXPECT_EQ(q["questions"][5]["maxResponseLength"], 20);
#endif

	//send some answers
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kBoolean, 0));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kHappySad, 0));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kThumbs, 0));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kMultiple, 0));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kScale, 0));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kVoice, "ASDF=="));
	cog.GetExitPoll()->SendAllAnswers();

	//join all outstanding threads
	for (int i = 0; i < activeThreads.size(); i++)
	{
		activeThreads[i]->join();
		delete(activeThreads[i]);
	}
	activeThreads.clear();
}

TEST(ExitPoll, CustomEventAnswerValues) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "6", 2));
	settings.AllSceneData = scenedatas;
	settings.DefaultSceneName = "tutorial";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();
	double timestamp = cog.GetSessionTimestamp();
	cog.GetExitPoll()->RequestQuestionSet("testing_new_sdk");

	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kBoolean, true));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kHappySad, false));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kThumbs, true));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kMultiple, 2));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kScale, 9));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kVoice, "ASDF=="));
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kBoolean, -1)); //skip
	cog.GetExitPoll()->AddAnswer(cognitive::ExitPollAnswer(cognitive::EQuestionType::kBoolean, -32768)); //skip
	auto a = cog.GetExitPoll()->SendAllAnswers();

	auto c = cog.GetCustomEvent()->SendData();
	EXPECT_EQ(c["userid"], "travis");
	EXPECT_EQ(c["part"], 1);
	EXPECT_EQ(c["formatversion"], "1.0");
	EXPECT_EQ(c["timestamp"], (int)timestamp);
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["data"][0]["name"], "c3d.sessionStart");
	EXPECT_EQ(c["data"][1]["name"], "c3d.exitpoll");
	EXPECT_EQ(c["data"][1]["properties"].size(), 16);
	EXPECT_EQ(c["data"][1]["properties"]["userId"], "travis");
	EXPECT_EQ(c["data"][1]["properties"]["questionSetId"], "testing:1");
	EXPECT_EQ(c["data"][1]["properties"]["hook"], "testing_new_sdk");
	EXPECT_NE(c["data"][1]["properties"]["sessionId"], ""); //expecting timestamp_travis
	EXPECT_EQ(c["data"][1]["properties"]["sceneId"], "tutorial");
	EXPECT_EQ(c["data"][1]["properties"]["versionNumber"], "6");
	EXPECT_EQ(c["data"][1]["properties"]["versionId"], 2);

	EXPECT_GT(c["data"][1]["properties"]["duration"], 0.0f);
	EXPECT_LT(c["data"][1]["properties"]["duration"], 5.0f); //total exitpoll duration
	
	EXPECT_EQ(c["data"][1]["properties"]["Answer0"], 1);
	EXPECT_EQ(c["data"][1]["properties"]["Answer1"], 0);
	EXPECT_EQ(c["data"][1]["properties"]["Answer2"], 1);
	EXPECT_EQ(c["data"][1]["properties"]["Answer3"], 2); //multiple choice index
	EXPECT_EQ(c["data"][1]["properties"]["Answer4"], 9); //scale value
	EXPECT_EQ(c["data"][1]["properties"]["Answer5"], 0); //voice. 0 unless skipped
	EXPECT_EQ(c["data"][1]["properties"]["Answer6"], -1); //skip
	EXPECT_EQ(c["data"][1]["properties"]["Answer7"], -32768); //skip
}


//----------------------GAZE

TEST(Gaze, PreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetGazeTracker()->RecordGaze(pos,rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot, pos);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos,rot,pos,"1");

	auto c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c.size(), 0);

	cog.StartSession();
}

TEST(Gaze, DuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot, pos);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot, pos, "1");

	cog.StartSession();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot, pos);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot, pos, "1");
	auto c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c["data"].size(), 6);
}

TEST(Gaze, PostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot, pos);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot, pos, "1");

	cog.StartSession();
	
	cog.EndSession();

	auto c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(Gaze, PostSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot, pos);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot, pos, "1");
	auto c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c["data"].size(), 3);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot, pos, "1");
	cog.SetScene("tutorial");
	c = cog.GetGazeTracker()->SendData();

	EXPECT_EQ(c["data"].size(), 1);
}

TEST(Gaze, PostSceneChangeMultiple) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	auto c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c["data"].size(), 1);

	cog.SetScene("tutorial");
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);

	cog.SetScene("tutorial");
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(Gaze, PostInvalidSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	auto c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c["data"].size(), 1);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.SetScene("invalid");
	c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(Gaze, Values) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.HMDType = cognitive::ECognitiveHMDType::kVive;
	settings.GazeInterval = 0.134f;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,2,3 };
	std::vector<float> rot = { 4,5,6,7 };

	cog.StartSession();
	double timestamp = cog.GetSessionTimestamp();
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot, pos);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot, pos, "1");

	auto c = cog.GetGazeTracker()->SendData();

	EXPECT_EQ(c["userid"], "travis");
	EXPECT_EQ(c["part"], 1);
	EXPECT_EQ(c["hmdtype"], "vive");
	EXPECT_EQ(c["interval"],0.134f);
	EXPECT_EQ(c["formatversion"], "1.0");
	EXPECT_EQ(c["timestamp"], (int)timestamp);
	
	//sky
	EXPECT_EQ(c["data"][0]["p"][0], 1);
	EXPECT_EQ(c["data"][0]["p"][1], 2);
	EXPECT_EQ(c["data"][0]["p"][2], 3);
	EXPECT_EQ(c["data"][0]["r"][0], 4);
	EXPECT_EQ(c["data"][0]["r"][1], 5);
	EXPECT_EQ(c["data"][0]["r"][2], 6);
	EXPECT_EQ(c["data"][0]["r"][3], 7);
	
	//world
	EXPECT_EQ(c["data"][1]["p"][0], 1);
	EXPECT_EQ(c["data"][1]["p"][1], 2);
	EXPECT_EQ(c["data"][1]["p"][2], 3);
	EXPECT_EQ(c["data"][1]["r"][0], 4);
	EXPECT_EQ(c["data"][1]["r"][1], 5);
	EXPECT_EQ(c["data"][1]["r"][2], 6);
	EXPECT_EQ(c["data"][1]["r"][3], 7);
	EXPECT_EQ(c["data"][1]["g"][0], 1);
	EXPECT_EQ(c["data"][1]["g"][1], 2);
	EXPECT_EQ(c["data"][1]["g"][2], 3);

	//dynamic
	EXPECT_EQ(c["data"][2]["p"][0], 1);
	EXPECT_EQ(c["data"][2]["p"][1], 2);
	EXPECT_EQ(c["data"][2]["p"][2], 3);
	EXPECT_EQ(c["data"][2]["r"][0], 4);
	EXPECT_EQ(c["data"][2]["r"][1], 5);
	EXPECT_EQ(c["data"][2]["r"][2], 6);
	EXPECT_EQ(c["data"][2]["r"][3], 7);
	EXPECT_EQ(c["data"][2]["g"][0], 1);
	EXPECT_EQ(c["data"][2]["g"][1], 2);
	EXPECT_EQ(c["data"][2]["g"][2], 3);
	EXPECT_EQ(c["data"][2]["o"], "1");
	
}

TEST(Gaze, LimitPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.GazeBatchSize = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot); //clear

	cog.StartSession();

	auto c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c["data"].size(), 0);
}

TEST(Gaze, LimitDuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.GazeBatchSize = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);//clear
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);

	auto c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(Gaze, LimitPostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.GazeBatchSize = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.StartSession();
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.EndSession(); //send

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.StartSession();

	auto c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
}

TEST(Gaze, Media) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	bool first = cog.StartSession();

	std::vector<float> hmdpos = { 0,0,0 }; //xyz location in world space of the hmd
	std::vector<float> localgazepos = { 0,0,0 }; //xyz location of the raycast hit point in local space of the hit object
	std::vector<float> hmdrot = { 0,0,0,1 }; //xyzw world rotation in quaternions of the hmd
	std::string hitObjectId = "DynamicObjectUniqueId"; //the unique identifier of the dynamic object instance
	std::string mediaId = "MediaUniqueId"; //this comes from the dashboard
	long mediaTime = 1000; //the current frame number of a video media, or 0 for images
	std::vector<float> uvs = { 0.3f,0.9f }; //the U and V texture coordinates
	cog.GetGazeTracker()->RecordGaze(hmdpos, hmdrot, localgazepos, hitObjectId, mediaId, mediaTime, uvs);

	auto c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
	EXPECT_EQ(c["data"][0]["o"], "DynamicObjectUniqueId");
	EXPECT_EQ(c["data"][0]["mediaId"], "MediaUniqueId");
	EXPECT_EQ(c["data"][0]["mediatime"], 1000);
	EXPECT_EQ(c["data"][0]["uvs"].size(), 2);
	EXPECT_EQ(c["data"][0]["uvs"][0], 0.3f);
	EXPECT_EQ(c["data"][0]["uvs"][1], 0.9f);
}

TEST(Gaze, LimitRecordRate) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.GazeBatchSize = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.StartSession();
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	auto c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c["data"].size(), 1);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cog.GetGazeTracker()->RecordGaze(pos, rot);
	c = cog.GetGazeTracker()->SendData();
	EXPECT_EQ(c["data"].size(), 3);
}


//----------------------SENSORS

TEST(Sensors, PreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.GetSensor()->RecordSensor("testing1", 1);

	auto c = cog.GetSensor()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(Sensors, DuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.GetSensor()->RecordSensor("testing1", 1);

	cog.StartSession();
	auto c = cog.GetSensor()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(Sensors, PostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();
	cog.GetSensor()->RecordSensor("testing1", 1);
	cog.EndSession();

	auto c = cog.GetSensor()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(Sensors, PostSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();
	cog.GetSensor()->RecordSensor("testing1", 1);
	auto c = cog.GetSensor()->SendData();
	EXPECT_EQ(c["data"].size(), 1);

	cog.SetScene("tutorial");
	c = cog.GetSensor()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(Sensors, PostSceneChangeMultiple) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();
	cog.GetSensor()->RecordSensor("testing1", 1);
	auto c = cog.GetSensor()->SendData();
	EXPECT_EQ(c["data"].size(), 1);

	cog.SetScene("tutorial");
	cog.GetSensor()->RecordSensor("testing2", 1);

	cog.SetScene("tutorial");
	cog.GetSensor()->RecordSensor("testing3", 1);
	c = cog.GetSensor()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(Sensors, PostInvalidSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();
	cog.GetSensor()->RecordSensor("testing1", 1);
	auto c = cog.GetSensor()->SendData();
	EXPECT_EQ(c["data"].size(), 1);

	cog.GetSensor()->RecordSensor("testing1", 1);
	cog.SetScene("invalid");
	c = cog.GetSensor()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(Sensors, Values) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();
	double timestamp = cog.GetSessionTimestamp();
	
	cog.GetSensor()->RecordSensor("testing1", 1);
	cog.GetSensor()->RecordSensor("testing1", 1);
	cog.GetSensor()->RecordSensor("testing1", 1);

	cog.GetSensor()->RecordSensor("testing2", 2);
	cog.GetSensor()->RecordSensor("testing2", 2);

	cog.GetSensor()->RecordSensor("testing3", 3);
	cog.GetSensor()->RecordSensor("testing3", 3);
	cog.GetSensor()->RecordSensor("testing3", 3);
	cog.GetSensor()->RecordSensor("testing3", 3);

	auto c = cog.GetSensor()->SendData();
	EXPECT_EQ(c["name"], "travis");
	EXPECT_EQ(c["part"], 1);
	EXPECT_EQ(c["formatversion"], "1.0");
	EXPECT_EQ(c["timestamp"], (int)timestamp);
	EXPECT_EQ(c["data"].size(), 3);
	EXPECT_EQ(c["data"][0]["name"], "testing1");
	EXPECT_EQ(c["data"][0]["data"].size(), 3);
	EXPECT_EQ(c["data"][0]["data"][0][1], 1);

	EXPECT_EQ(c["data"][1]["name"], "testing2");
	EXPECT_EQ(c["data"][1]["data"].size(), 2);
	EXPECT_EQ(c["data"][1]["data"][0][1], 2);

	EXPECT_EQ(c["data"][2]["name"], "testing3");
	EXPECT_EQ(c["data"][2]["data"].size(), 4);
	EXPECT_EQ(c["data"][2]["data"][0][1], 3);
}

TEST(Sensors, LimitPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.SensorDataLimit = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.GetSensor()->RecordSensor("testing1", 1);
	cog.GetSensor()->RecordSensor("testing1", 1);
	cog.GetSensor()->RecordSensor("testing1", 1);
	cog.GetSensor()->RecordSensor("testing1", 1);//clear

	cog.StartSession();

	auto c = cog.GetSensor()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(Sensors, LimitDuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.SensorDataLimit = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();

	cog.GetSensor()->RecordSensor("testing1", 1);
	cog.GetSensor()->RecordSensor("testing1", 1);
	cog.GetSensor()->RecordSensor("testing1", 1);
	cog.GetSensor()->RecordSensor("testing1", 1);//clear
	cog.GetSensor()->RecordSensor("testing1", 1);

	auto c = cog.GetSensor()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(Sensors, LimitPostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.SensorDataLimit = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();
	cog.GetSensor()->RecordSensor("testing1", 1);
	cog.EndSession(); //send

	cog.GetSensor()->RecordSensor("testing1", 1);
	cog.GetSensor()->RecordSensor("testing1", 1);
	cog.StartSession();

	auto c = cog.GetSensor()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}



//----------------------DYNAMICS

TEST(Dynamics, PreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh","0", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1",pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);

	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c.size(), 0);

	cog.StartSession();
}

TEST(Dynamics, DuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name1", "mesh1", "1", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name2", "mesh2", "2", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("3", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("3", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["data"].size(), 8);
	EXPECT_EQ(c["manifest"].size(), 4);
}

TEST(Dynamics, DuringSessionOverwriteManifest) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");
	cog.StartSession();

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RegisterObjectCustomId("name1", "mesh1", "1000", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);

	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["data"].size(), 5);
	EXPECT_EQ(c["manifest"].size(), 2);
}

TEST(Dynamics, MultipleManifests) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");
	cog.StartSession();

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name1", "mesh1", "1000", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot); //id 1001
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot); //id 1002
	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 3);
	EXPECT_EQ(c["manifest"].size(), 3);

	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["manifest"], nullptr);
}

TEST(Dynamics, PostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);

	cog.StartSession();

	cog.EndSession();

	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(Dynamics, RemovePreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name1", "mesh1", "1", pos, rot);
	cog.GetDynamicObject()->RemoveObject("1", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c.size(), 0);
	cog.StartSession();
	c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["manifest"].size(), 1);
}

TEST(Dynamics, RemoveDuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name1", "mesh1", "1", pos, rot);
	cog.GetDynamicObject()->RemoveObject("1", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["manifest"].size(), 1);
}

TEST(Dynamics, RemoveDuringSessionPropertyValues) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name1", "mesh1", "1", pos, rot);
	cog.GetDynamicObject()->RemoveObject("1", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["manifest"].size(), 1);

	cognitive::nlohmann::json truetarget = cognitive::nlohmann::json();
	truetarget["enabled"] = true;
	cognitive::nlohmann::json falsetarget = cognitive::nlohmann::json();
	falsetarget["enabled"] = false;

	EXPECT_EQ(c["data"][0]["properties"][0], truetarget);
	EXPECT_EQ(c["data"][1]["properties"][0], falsetarget);
}

TEST(Dynamics, RemoveDuringSessionUnregistered) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RemoveObject("1", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["data"].size(), 1);
	EXPECT_EQ(c["manifest"].size(), 0);
}

TEST(Dynamics, RemovePostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();

	cog.EndSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name1", "mesh1", "1", pos, rot);
	cog.GetDynamicObject()->RemoveObject("1", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c.size(), 0);
}

TEST(Dynamics, PostSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 4);
	EXPECT_EQ(c["manifest"].size(), 2);

	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.SetScene("tutorial"); //no previous scene. this is just setting it late
	c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["manifest"].size(), 2);
}

TEST(Dynamics, PostSceneChangeMultiple) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 4);
	EXPECT_EQ(c["manifest"].size(), 2);

	cog.SetScene("tutorial");
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);

	cog.SetScene("tutorial");
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
	EXPECT_EQ(c["manifest"].size(), 3);
}

TEST(Dynamics, PostInvalidSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 4);
	EXPECT_EQ(c["manifest"].size(), 2);

	cog.SetScene("invalid");
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
	EXPECT_EQ(c["manifest"].size(), 0);
}

TEST(Dynamics, Values) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,2,3 };
	std::vector<float> rot = { 4,5,6,7 };

	cog.StartSession();
	double timestamp = cog.GetSessionTimestamp();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name2", "mesh2", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("2", pos, rot);

	cognitive::nlohmann::json color;
	color["color"] = "yellow";
	cognitive::nlohmann::json size;
	size["size"] = 5;
	cognitive::nlohmann::json props = cognitive::nlohmann::json::array({color,size});

	cog.GetDynamicObject()->RecordDynamic("0", pos, rot,props);
	auto c = cog.GetDynamicObject()->SendData();

	cognitive::nlohmann::json enabledtarget;
	enabledtarget["enabled"] = true;
	cognitive::nlohmann::json colortarget;
	colortarget["color"] = "yellow";
	cognitive::nlohmann::json sizetarget;
	sizetarget["size"] = 5;

	EXPECT_EQ(c["userid"], "travis");
	EXPECT_EQ(c["part"], 1);
	EXPECT_EQ(c["formatversion"], "1.0");
	EXPECT_EQ(c["timestamp"], (int)timestamp);

	//manifest
	EXPECT_EQ(c["manifest"].size(), 2);
	EXPECT_EQ(c["manifest"]["0"]["name"], "name");
	EXPECT_EQ(c["manifest"]["0"]["mesh"], "mesh");

	EXPECT_EQ(c["manifest"]["1000"]["name"], "name2");
	EXPECT_EQ(c["manifest"]["1000"]["mesh"], "mesh2");

	EXPECT_EQ(c["data"].size(), 4);
		
	//dynamic 1
	EXPECT_EQ(c["data"][0]["id"], "0");
	EXPECT_EQ(c["data"][0]["p"][0], 1);
	EXPECT_EQ(c["data"][0]["p"][1], 2);
	EXPECT_EQ(c["data"][0]["p"][2], 3);
	EXPECT_EQ(c["data"][0]["properties"][0], enabledtarget);
	
	//dynamic 2
	EXPECT_EQ(c["data"][1]["id"], "1000");
	EXPECT_EQ(c["data"][1]["p"][0], 1);
	EXPECT_EQ(c["data"][1]["p"][1], 2);
	EXPECT_EQ(c["data"][1]["p"][2], 3);
	EXPECT_EQ(c["data"][1]["properties"][0], enabledtarget);

	//dynamic 3
	EXPECT_EQ(c["data"][2]["id"], "2");
	EXPECT_EQ(c["data"][2]["p"][0], 1);
	EXPECT_EQ(c["data"][2]["p"][1], 2);
	EXPECT_EQ(c["data"][2]["p"][2], 3);
	//no enabled property - never registered

	//dynamic 4
	EXPECT_EQ(c["data"][3]["id"], "0");
	EXPECT_EQ(c["data"][3]["p"][0], 1);
	EXPECT_EQ(c["data"][3]["p"][1], 2);
	EXPECT_EQ(c["data"][3]["p"][2], 3);
	EXPECT_EQ(c["data"][3]["properties"][0], colortarget);
	EXPECT_EQ(c["data"][3]["properties"][1], sizetarget);
}

TEST(Dynamics, RegisterScaleValues) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,2,3 };
	std::vector<float> scale2 = { 2,2,2 };
	std::vector<float> scale3 = { 1,2,3 };
	std::vector<float> rot = { 4,5,6,7 };

	cog.StartSession();
	double timestamp = cog.GetSessionTimestamp();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "1", pos, rot, scale2);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot); //1000
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot, scale3); //1001

	cognitive::nlohmann::json enabledtarget;
	enabledtarget["enabled"] = true;

	//check here
	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 4);

	//dynamic 1
	EXPECT_EQ(c["data"][0]["id"], "0");
	EXPECT_EQ(c["data"][0]["s"].size(), 3);
	EXPECT_EQ(c["data"][0]["s"][0], 1);
	EXPECT_EQ(c["data"][0]["s"][1], 1);
	EXPECT_EQ(c["data"][0]["s"][2], 1);
	EXPECT_EQ(c["data"][0]["properties"][0], enabledtarget);

	//dynamic 2
	EXPECT_EQ(c["data"][1]["id"], "1");
	EXPECT_EQ(c["data"][1]["s"][0], 2);
	EXPECT_EQ(c["data"][1]["s"][1], 2);
	EXPECT_EQ(c["data"][1]["s"][2], 2);
	EXPECT_EQ(c["data"][1]["properties"][0], enabledtarget);

	//dynamic 3
	EXPECT_EQ(c["data"][2]["id"], "1000");
	EXPECT_EQ(c["data"][2]["s"][0], 1);
	EXPECT_EQ(c["data"][2]["s"][1], 1);
	EXPECT_EQ(c["data"][2]["s"][2], 1);
	EXPECT_EQ(c["data"][2]["properties"][0], enabledtarget);

	//dynamic 4
	EXPECT_EQ(c["data"][3]["id"], "1001");
	EXPECT_EQ(c["data"][3]["s"][0], 1);
	EXPECT_EQ(c["data"][3]["s"][1], 2);
	EXPECT_EQ(c["data"][3]["s"][2], 3);
	EXPECT_EQ(c["data"][3]["properties"][0], enabledtarget);
}

TEST(Dynamics, RecordScaleValues) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,2,3 };
	std::vector<float> scale1 = { 1,1,1 };
	std::vector<float> scale2 = { 2,2,2 };
	std::vector<float> scale3 = { 1,2,3 };
	std::vector<float> rot = { 4,5,6,7 };

	cog.StartSession();
	double timestamp = cog.GetSessionTimestamp();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);

	cognitive::nlohmann::json color;
	color["color"] = "yellow";
	cognitive::nlohmann::json props = cognitive::nlohmann::json::array({ color });

	cognitive::nlohmann::json colortarget;
	colortarget["color"] = "yellow";

	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot,scale3);
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot, scale2, props); //scale and json
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot, props); //no scale. only json

	//check here
	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 6);

	//dynamic 1 (from registering object)
	EXPECT_EQ(c["data"][0]["id"], "0");
	EXPECT_EQ(c["data"][0]["s"].size(), 3);
	EXPECT_EQ(c["data"][0]["s"][0], 1);
	EXPECT_EQ(c["data"][0]["s"][1], 1);
	EXPECT_EQ(c["data"][0]["s"][2], 1);

	//dynamic 2
	EXPECT_EQ(c["data"][1]["id"], "0");
	EXPECT_EQ(c["data"][1]["s"].size(), 0);

	//dynamic 3
	EXPECT_EQ(c["data"][2]["id"], "0");
	EXPECT_EQ(c["data"][2]["s"][0], 1);
	EXPECT_EQ(c["data"][2]["s"][1], 2);
	EXPECT_EQ(c["data"][2]["s"][2], 3);

	//dynamic 4
	EXPECT_EQ(c["data"][3]["id"], "0");
	EXPECT_EQ(c["data"][3]["s"].size(), 0);

	//dynamic 5 (scale and json)
	EXPECT_EQ(c["data"][4]["id"], "0");
	EXPECT_EQ(c["data"][4]["s"][0], 2);
	EXPECT_EQ(c["data"][4]["s"][1], 2);
	EXPECT_EQ(c["data"][4]["s"][2], 2);
	EXPECT_EQ(c["data"][4]["properties"][0], colortarget);

	//dynamic 6 (json)
	EXPECT_EQ(c["data"][5]["id"], "0");
	EXPECT_EQ(c["data"][5]["s"].size(), 0);
	EXPECT_EQ(c["data"][5]["properties"][0], colortarget);
}

TEST(Dynamics, DynamicMeshFileType) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,2,3 };
	std::vector<float> rot = { 4,5,6,7 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);

	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["userid"], "travis");
	EXPECT_EQ(c["part"], 1);
	EXPECT_EQ(c["formatversion"], "1.0");

	//manifest
	EXPECT_EQ(c["manifest"].size(), 1);
	EXPECT_EQ(c["manifest"]["0"]["name"], "name");
	EXPECT_EQ(c["manifest"]["0"]["mesh"], "mesh");
	EXPECT_EQ(c["manifest"]["0"]["fileType"], "obj");
	cog.EndSession();

	//part 2

	settings.DynamicObjectFileType = "customfiletype";
	auto cog2 = cognitive::CognitiveVRAnalyticsCore(settings);
	cog2.SetUserName("travis");
	cog2.StartSession();
	cog2.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);

	c = cog2.GetDynamicObject()->SendData();
	EXPECT_EQ(c["userid"], "travis");
	EXPECT_EQ(c["part"], 1);
	EXPECT_EQ(c["formatversion"], "1.0");

	//manifest
	EXPECT_EQ(c["manifest"].size(), 1);
	EXPECT_EQ(c["manifest"]["0"]["name"], "name");
	EXPECT_EQ(c["manifest"]["0"]["mesh"], "mesh");
	EXPECT_EQ(c["manifest"]["0"]["fileType"], "customfiletype");
}

TEST(Dynamics, PropertyJsonType) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,2,3 };
	std::vector<float> rot = { 4,5,6,7 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObject("name2", "mesh2", pos, rot);

	cognitive::nlohmann::json color;
	color["color"] = "yellow";

	cognitive::nlohmann::json size;
	size["size"] = 5;

	cog.GetDynamicObject()->RecordDynamic("1000", pos, rot, color);

	//send json object to properties - ignore
	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"][1]["id"], "1000");
	EXPECT_EQ(c["data"][1]["properties"], nullptr);

	cognitive::nlohmann::json props = cognitive::nlohmann::json::array({ color,size });
	cog.GetDynamicObject()->RecordDynamic("1000", pos, rot, props);
	c = cog.GetDynamicObject()->SendData();

	cognitive::nlohmann::json colortarget;
	colortarget["color"] = "yellow";
	cognitive::nlohmann::json sizetarget;
	sizetarget["size"] = 5;

	EXPECT_EQ(c["data"][0]["id"], "1000");
	EXPECT_EQ(c["data"][0]["properties"][0], colortarget);
	EXPECT_EQ(c["data"][0]["properties"][1], sizetarget);
}

TEST(Dynamics, LimitPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.DynamicDataLimit = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);

	cog.StartSession();

	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 4);
	EXPECT_EQ(c["manifest"].size(), 2);
}

TEST(Dynamics, LimitDuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.DynamicDataLimit = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);

	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["manifest"].size(), 0);
}

TEST(Dynamics, LimitPostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.DynamicDataLimit = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.EndSession(); //send

	cog.GetDynamicObject()->RegisterObjectCustomId("name5", "mesh5", "5", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("6", pos, rot);
	cog.StartSession();

	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 3);
	EXPECT_EQ(c["manifest"].size(), 2);
}

TEST(Dynamics, IdReuse) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.StartSession();

	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1000", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1000", pos, rot);

	cog.GetDynamicObject()->RemoveObject("1000", pos, rot);

	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1000", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1000", pos, rot);

	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["data"].size(), 7);
	EXPECT_EQ(c["manifest"].size(), 1);
}

//----------------------------------ENGAGEMENTS

TEST(Engagements, PreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("0", "grab");

	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c.size(), 0);

	cog.StartSession();
}

TEST(Engagements, DuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("0", "grab");
	cog.GetDynamicObject()->BeginEngagement("0", "grab2");

	cog.StartSession();
	cog.GetDynamicObject()->EndEngagement("0", "grab3"); //adds engagement + sets as inactive
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["data"][1]["engagements"].size(), 3); //ended grab3
	EXPECT_EQ(c["data"][2]["engagements"].size(), 2);
	EXPECT_EQ(c["data"][3]["engagements"].size(), 2);
	EXPECT_EQ(c["data"][4]["engagements"].size(), 2);
	EXPECT_EQ(c["data"][5]["engagements"].size(), 2);
}

TEST(Engagements, DuringSessionParentId) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");
	cog.StartSession();

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("0", "grab","left");
	cog.GetDynamicObject()->BeginEngagement("0", "grab", "right");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"][1]["engagements"][0]["engagementparent"], "left");
	EXPECT_EQ(c["data"][1]["engagements"][1]["engagementparent"], "right");

	cog.GetDynamicObject()->EndEngagement("0", "grab", "left");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"][0]["engagements"].size(), 2);
	EXPECT_EQ(c["data"][0]["engagements"][0]["engagementparent"], "left");
	EXPECT_EQ(c["data"][0]["engagements"][1]["engagementparent"], "right");
	EXPECT_EQ(c["data"][1]["engagements"].size(), 1);
	EXPECT_EQ(c["data"][1]["engagements"][0]["engagementparent"], "right");

	cog.GetDynamicObject()->EndEngagement("0", "grab", "left");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	c = cog.GetDynamicObject()->SendData();
	
	EXPECT_EQ(c["data"][0]["engagements"].size(), 2);
	EXPECT_EQ(c["data"][0]["engagements"][0]["engagementparent"], "right");
	EXPECT_EQ(c["data"][0]["engagements"][1]["engagementparent"], "left");
}

TEST(Engagements, DuringSessionOverwrite) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("0", "grab"); //engagement count 1
	cog.GetDynamicObject()->BeginEngagement("0", "grab"); //engagement count 2

	cog.StartSession();
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["data"][1]["engagements"].size(), 2);
}

TEST(Engagements, PostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("0", "grab");

	cog.StartSession();

	cog.EndSession();

	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(Engagements, PostSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("0", "grab");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["data"][1]["engagements"].size(), 1);

	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.SetScene("tutorial");
	c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	
	EXPECT_EQ(c["data"][1]["engagements"].size(), 1); //will only update when finished
}

TEST(Engagements, PostSceneChangeMultiple) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("0", "grab");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["manifest"].size(), 1);
	EXPECT_EQ(c["data"][1]["engagements"].size(), 1);

	cog.SetScene("tutorial");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->EndEngagement("0", "grab");
	cog.GetDynamicObject()->BeginEngagement("0", "grab");

	cog.SetScene("tutorial");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->EndEngagement("0", "grab");
	cog.GetDynamicObject()->BeginEngagement("0", "hover");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["manifest"].size(), 1);
	EXPECT_EQ(c["data"][1]["engagements"].size(), 2);
}

TEST(Engagements, PostInvalidSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("0", "grab");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["manifest"].size(), 1);

	cog.SetScene("invalid"); //last scene not set, won't send
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("0", "hover");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["data"][0]["engagements"].size(), 1);
	EXPECT_EQ(c["data"][1]["engagements"].size(), 2);
}

TEST(Engagements, Values) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,2,3 };
	std::vector<float> rot = { 4,5,6,7 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("0", "grab");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);

	cognitive::nlohmann::json color;
	color["color"] = "yellow";

	cognitive::nlohmann::json size;
	size["size"] = 5;

	cognitive::nlohmann::json props = cognitive::nlohmann::json::array({ color,size });

	cog.GetDynamicObject()->EndEngagement("0", "grab");
	cog.GetDynamicObject()->EndEngagement("0", "hover");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot, props);

	auto c = cog.GetDynamicObject()->SendData();

	cognitive::nlohmann::json enabledtarget;
	enabledtarget["enabled"] = true;
	cognitive::nlohmann::json colortarget;
	colortarget["color"] = "yellow";
	cognitive::nlohmann::json sizetarget;
	sizetarget["size"] = 5;

	EXPECT_EQ(c["userid"], "travis");
	EXPECT_EQ(c["part"], 1);

	//manifest
	EXPECT_EQ(c["manifest"].size(), 1);
	EXPECT_EQ(c["data"].size(), 3);

	//snapshot 1 register
	EXPECT_EQ(c["data"][0]["id"], "0");
	EXPECT_EQ(c["data"][0]["properties"][0], enabledtarget);

	//snapshot 2 grab engagement begin
	EXPECT_EQ(c["data"][1]["id"], "0");
	EXPECT_EQ(c["data"][1]["engagements"][0]["engagementtype"], "grab");
	EXPECT_EQ(c["data"][1]["engagements"][0]["engagement_count"], 1);

	//snapshot 3 properties, grab end, hover begin/end
	EXPECT_EQ(c["data"][2]["id"], "0");
	EXPECT_EQ(c["data"][2]["properties"][1], sizetarget);
	EXPECT_EQ(c["data"][2]["properties"][0],colortarget);	
	EXPECT_EQ(c["data"][2]["engagements"][0]["engagementtype"], "grab");
	EXPECT_GE(c["data"][2]["engagements"][0]["engagement_time"], 0);
	EXPECT_EQ(c["data"][2]["engagements"][1]["engagementtype"], "hover");
	EXPECT_GE(c["data"][2]["engagements"][1]["engagement_time"], 0);
}

TEST(Engagements, LimitPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.DynamicDataLimit = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("0", "grab");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot); //limit

	cog.StartSession();

	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 4);
	EXPECT_EQ(c["manifest"].size(), 1);
	EXPECT_EQ(c["data"][1]["engagements"].size(), 1);
}

TEST(Engagements, LimitDuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.DynamicDataLimit = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("0", "grab");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot); //limit
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);

	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
	EXPECT_EQ(c["manifest"].size(), 0);
	EXPECT_EQ(c["data"][0]["engagements"].size(), 1);
}

TEST(Engagements, LimitPostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.DynamicDataLimit = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("0", "grab");
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.EndSession(); //send

	cog.GetDynamicObject()->RegisterObjectCustomId("name2", "mesh2", "2", pos, rot);
	cog.GetDynamicObject()->BeginEngagement("2", "grab");
	cog.GetDynamicObject()->RecordDynamic("2", pos, rot);
	cog.StartSession();

	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["manifest"].size(), 1);
	EXPECT_EQ(c["data"][0]["engagements"].size(), 0);
	EXPECT_EQ(c["data"][1]["engagements"].size(), 1);
}

//----------------------FIXATION

TEST(Fixation, PreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, "objectid", pos);

	auto c = cog.GetFixation()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(Fixation, DuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, "objectid", pos);

	cog.StartSession();
	auto c = cog.GetFixation()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
}

TEST(Fixation, PostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };

	cog.StartSession();
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, "objectid", pos);
	cog.EndSession();

	auto c = cog.GetFixation()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(Fixation, PostSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };

	cog.StartSession();
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, "objectid", pos);
	auto c = cog.GetFixation()->SendData();
	EXPECT_EQ(c["data"].size(), 2);

	cog.SetScene("tutorial");
	c = cog.GetFixation()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(Fixation, PostSceneChangeMultiple) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };

	cog.StartSession();
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, "objectid", pos);
	auto c = cog.GetFixation()->SendData();
	EXPECT_EQ(c["data"].size(), 2);

	cog.SetScene("tutorial");
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, "objectid", pos);

	cog.SetScene("tutorial");
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	c = cog.GetFixation()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(Fixation, PostInvalidSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };

	cog.StartSession();
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, "objectid", pos);
	auto c = cog.GetFixation()->SendData();
	EXPECT_EQ(c["data"].size(), 2);

	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.SetScene("invalid");
	c = cog.GetFixation()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(Fixation, Values) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,2,3 };
	std::string dynamicId = "asdf1234";

	cog.StartSession();
	double timestamp = cog.GetSessionTimestamp();
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.GetFixation()->RecordFixation(1005.00, 115, 0.2f, dynamicId, pos);
	auto c = cog.GetFixation()->SendData();
	EXPECT_EQ(c["userid"], "travis");
	EXPECT_EQ(c["part"], 1);
	EXPECT_EQ(c["formatversion"], "1.0");
	EXPECT_EQ(c["timestamp"], (int)timestamp);
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["data"][0].size(), 4);
	EXPECT_EQ(c["data"][0]["time"], 1000.00);
	EXPECT_EQ(c["data"][0]["duration"], 100);
	EXPECT_EQ(c["data"][0]["maxradius"], 0.5);
	EXPECT_EQ(c["data"][0]["p"][0], 1);
	EXPECT_EQ(c["data"][0]["p"][1], 2);
	EXPECT_EQ(c["data"][0]["p"][2], 3);


	EXPECT_EQ(c["data"][1].size(), 5);
	EXPECT_EQ(c["data"][1]["time"], 1005.00);
	EXPECT_EQ(c["data"][1]["duration"], 115);
	EXPECT_EQ(c["data"][1]["maxradius"], 0.2f);
	EXPECT_EQ(c["data"][1]["objectid"], "asdf1234");
	EXPECT_EQ(c["data"][1]["p"][0], 1);
	EXPECT_EQ(c["data"][1]["p"][1], 2);
	EXPECT_EQ(c["data"][1]["p"][2], 3);
}

TEST(Fixation, LimitPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.FixationBatchSize = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };

	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, "objectid", pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos); //clear

	cog.StartSession();

	auto c = cog.GetFixation()->SendData();
	EXPECT_EQ(c["data"].size(), 0);
}

TEST(Fixation, LimitDuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.FixationBatchSize = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	cog.StartSession();

	std::vector<float> pos = { 0,0,0 };

	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, "objectid", pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos); //clear
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);

	auto c = cog.GetFixation()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
}

TEST(Fixation, LimitPostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	settings.FixationBatchSize = 4;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	cog.StartSession(); //record
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.EndSession(); //send

	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, "objectid", pos);
	cog.GetFixation()->RecordFixation(1000.00, 100, 0.5, pos);
	cog.StartSession();

	auto c = cog.GetFixation()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
}

//----------------------DYNAMIC CONTROLLERS

TEST(DynamicController, PreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot,"oculustouchleft",false);
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "1", pos, rot, "vivecontroller",true);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot, "trigger", 1.0);

	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c.size(), 0);

	cog.StartSession();
}

TEST(DynamicController, DuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot, "oculustouchleft",false);

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "1", pos, rot, "vivecontroller",true);
	cog.GetDynamicObject()->RecordDynamic("0", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot, "trigger", 1.0);
	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["data"].size(), 4);
	EXPECT_EQ(c["manifest"].size(), 2);
}

TEST(DynamicController, DuringSessionOverwriteManifest) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");
	cog.StartSession();

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot, "oculustouchleft",false);
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "1", pos, rot, "vivecontroller",true);
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "1", pos, rot, "oculustouchright",true);

	cog.GetDynamicObject()->RecordDynamic("1", pos, rot, "trigger", 1.0);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);

	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["data"].size(), 5);
	EXPECT_EQ(c["manifest"].size(), 2);
}

TEST(DynamicController, MultipleManifests) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");
	cog.StartSession();

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot, "oculustouchleft",false);
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "1", pos, rot, "vivecontroller",true);

	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["manifest"].size(), 2);

	cog.GetDynamicObject()->RecordDynamic("1", pos, rot, "trigger", 1.0);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["manifest"], nullptr);
}

TEST(DynamicController, PostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot, "oculustouchleft",false);
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "1", pos, rot, "vivecontroller",true);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot, "trigger", 1.0);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);

	cog.StartSession();

	cog.EndSession();

	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c.size(), 0);
}

TEST(DynamicController, RemovePreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "1", pos, rot, "vivecontroller",true);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot, "trigger", 1.0);
	cog.GetDynamicObject()->RemoveObject("1", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c.size(), 0);
	cog.StartSession();
	c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["data"].size(), 3);
	EXPECT_EQ(c["manifest"].size(), 1);
}

TEST(DynamicController, RemoveDuringSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "1", pos, rot, "vivecontroller",true);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot, "trigger", 1.0);
	cog.GetDynamicObject()->RemoveObject("1", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();

	EXPECT_EQ(c["data"].size(), 3);
	EXPECT_EQ(c["manifest"].size(), 1);
}

TEST(DynamicController, PostSceneChange) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 4);
	EXPECT_EQ(c["manifest"].size(), 2);

	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.SetScene("tutorial"); //no previous scene. this is just setting it late
	c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 2);
	EXPECT_EQ(c["manifest"].size(), 2);
}

TEST(DynamicController, PostSceneChangeMultiple) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	std::vector<cognitive::SceneData> scenedatas;
	scenedatas.emplace_back(cognitive::SceneData("tutorial", "DELETE_ME_1", "1", 0));
	settings.AllSceneData = scenedatas;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 0,0,0 };
	std::vector<float> rot = { 0,0,0,1 };

	cog.StartSession();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "0", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	auto c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 4);
	EXPECT_EQ(c["manifest"].size(), 2);

	cog.SetScene("tutorial");
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	cog.GetDynamicObject()->RegisterObject("name", "mesh", pos, rot);

	cog.SetScene("tutorial");
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot);
	c = cog.GetDynamicObject()->SendData();
	EXPECT_EQ(c["data"].size(), 1);
	EXPECT_EQ(c["manifest"].size(), 3);
}

TEST(DynamicController, Values) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,2,3 };
	std::vector<float> rot = { 4,5,6,7 };

	cog.StartSession();
	double timestamp = cog.GetSessionTimestamp();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "1", pos, rot, "vivecontroller", false); //0
	cog.GetDynamicObject()->RegisterObject("name2", "mesh2", pos, rot, "oculustouchright", true); //1
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot, "trigger", 1.0); //2
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot, "grip", 0.5); //3

	std::vector<cognitive::ControllerInputState> states = std::vector<cognitive::ControllerInputState>();
	states.push_back(cognitive::ControllerInputState("padbtn", 0, 0.25, -0.75));
	states.push_back(cognitive::ControllerInputState("grip", 0.25));
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot, states); //4

	auto c = cog.GetDynamicObject()->SendData();

	//std::cout << c.dump() << "\n";

	cognitive::nlohmann::json enabledtarget;
	enabledtarget["enabled"] = true;

	EXPECT_EQ(c["userid"], "travis");
	EXPECT_EQ(c["part"], 1);
	EXPECT_EQ(c["formatversion"], "1.0");
	EXPECT_EQ(c["timestamp"], (int)timestamp);

	//manifest
	EXPECT_EQ(c["manifest"].size(), 2);
	EXPECT_EQ(c["manifest"]["1"]["name"], "name");
	EXPECT_EQ(c["manifest"]["1"]["mesh"], "mesh");
	EXPECT_EQ(c["manifest"]["1"]["controllerType"], "vivecontroller");
	EXPECT_EQ(c["manifest"]["1"]["properties"].size(),1);
	EXPECT_EQ(c["manifest"]["1"]["properties"][0]["controller"], "left");

	EXPECT_EQ(c["manifest"]["1000"]["controllerType"], "oculustouchright");
	EXPECT_EQ(c["manifest"]["1000"]["properties"].size(), 1);
	EXPECT_EQ(c["manifest"]["1000"]["properties"][0]["controller"], "right");

	EXPECT_EQ(c["manifest"]["1000"]["name"], "name2");
	EXPECT_EQ(c["manifest"]["1000"]["mesh"], "mesh2");

	EXPECT_EQ(c["data"].size(), 5);

	//dynamic 1
	EXPECT_EQ(c["data"][0]["id"], "1");
	EXPECT_EQ(c["data"][0]["p"][0], 1);
	EXPECT_EQ(c["data"][0]["p"][1], 2);
	EXPECT_EQ(c["data"][0]["p"][2], 3);
	EXPECT_EQ(c["data"][0]["properties"][0], enabledtarget);
	EXPECT_EQ(c["data"][0]["buttons"], nullptr);

	//dynamic 2
	EXPECT_EQ(c["data"][1]["id"], "1000");
	EXPECT_EQ(c["data"][1]["p"][0], 1);
	EXPECT_EQ(c["data"][1]["p"][1], 2);
	EXPECT_EQ(c["data"][1]["p"][2], 3);
	EXPECT_EQ(c["data"][1]["properties"][0], enabledtarget);
	EXPECT_EQ(c["data"][1]["buttons"], nullptr);

	//dynamic 1 trigger
	EXPECT_EQ(c["data"][2]["id"], "1");
	EXPECT_EQ(c["data"][2]["p"][0], 1);
	EXPECT_EQ(c["data"][2]["p"][1], 2);
	EXPECT_EQ(c["data"][2]["p"][2], 3);
	EXPECT_EQ(c["data"][2]["buttons"].size(),1);
	EXPECT_EQ(c["data"][2]["buttons"]["trigger"]["buttonPercent"], 1.0);

	//dynamic 1 grip
	EXPECT_EQ(c["data"][3]["id"], "1");
	EXPECT_EQ(c["data"][3]["p"][0], 1);
	EXPECT_EQ(c["data"][3]["p"][1], 2);
	EXPECT_EQ(c["data"][3]["p"][2], 3);
	EXPECT_EQ(c["data"][3]["buttons"].size(), 1);
	EXPECT_EQ(c["data"][3]["buttons"]["grip"]["buttonPercent"], 0.5);

	//dynamic 1 padbtn, grip
	EXPECT_EQ(c["data"][4]["id"], "1");
	EXPECT_EQ(c["data"][4]["p"][0], 1);
	EXPECT_EQ(c["data"][4]["p"][1], 2);
	EXPECT_EQ(c["data"][4]["p"][2], 3);
	EXPECT_EQ(c["data"][4]["buttons"].size(), 2);
	EXPECT_EQ(c["data"][4]["buttons"]["padbtn"]["buttonPercent"], 0.0);
	EXPECT_EQ(c["data"][4]["buttons"]["padbtn"]["x"], 0.25);
	EXPECT_EQ(c["data"][4]["buttons"]["padbtn"]["y"], -0.75);
	EXPECT_EQ(c["data"][4]["buttons"]["grip"]["buttonPercent"], 0.25);
}

TEST(DynamicController, ValuesRecordInputNonController) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.APIKey = TESTINGAPIKEY;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.SetUserName("travis");

	std::vector<float> pos = { 1,2,3 };
	std::vector<float> rot = { 4,5,6,7 };

	cog.StartSession();
	double timestamp = cog.GetSessionTimestamp();
	cog.GetDynamicObject()->RegisterObjectCustomId("name", "mesh", "1", pos, rot); //0
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot, "trigger", 1.0); //1
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot, "grip", 0.5); //2

	std::vector<cognitive::ControllerInputState> states = std::vector<cognitive::ControllerInputState>();
	states.push_back(cognitive::ControllerInputState("padbtn", 0, 0.25, -0.75));
	states.push_back(cognitive::ControllerInputState("grip", 0.25));
	cog.GetDynamicObject()->RecordDynamic("1", pos, rot, states); //3

	auto c = cog.GetDynamicObject()->SendData();

	cognitive::nlohmann::json enabledtarget;
	enabledtarget["enabled"] = true;

	EXPECT_EQ(c["userid"], "travis");
	EXPECT_EQ(c["part"], 1);
	EXPECT_EQ(c["formatversion"], "1.0");
	EXPECT_EQ(c["timestamp"], (int)timestamp);

	//manifest
	EXPECT_EQ(c["manifest"].size(), 1);
	EXPECT_EQ(c["manifest"]["1"]["name"], "name");
	EXPECT_EQ(c["manifest"]["1"]["mesh"], "mesh");
	EXPECT_EQ(c["manifest"]["1"]["properties"].size(), 0);

	EXPECT_EQ(c["data"].size(), 4);

	//dynamic 1
	EXPECT_EQ(c["data"][0]["id"], "1");
	EXPECT_EQ(c["data"][0]["p"][0], 1);
	EXPECT_EQ(c["data"][0]["p"][1], 2);
	EXPECT_EQ(c["data"][0]["p"][2], 3);
	EXPECT_EQ(c["data"][0]["properties"][0], enabledtarget);
	EXPECT_EQ(c["data"][0]["buttons"], nullptr);

	//dynamic 1 trigger
	EXPECT_EQ(c["data"][1]["id"], "1");
	EXPECT_EQ(c["data"][1]["p"][0], 1);
	EXPECT_EQ(c["data"][1]["p"][1], 2);
	EXPECT_EQ(c["data"][1]["p"][2], 3);
	EXPECT_EQ(c["data"][1]["buttons"].size(), 1);
	EXPECT_EQ(c["data"][1]["buttons"]["trigger"]["buttonPercent"], 1.0);

	//dynamic 1 grip
	EXPECT_EQ(c["data"][2]["id"], "1");
	EXPECT_EQ(c["data"][2]["p"][0], 1);
	EXPECT_EQ(c["data"][2]["p"][1], 2);
	EXPECT_EQ(c["data"][2]["p"][2], 3);
	EXPECT_EQ(c["data"][2]["buttons"].size(), 1);
	EXPECT_EQ(c["data"][2]["buttons"]["grip"]["buttonPercent"], 0.5);

	//dynamic 1 padbtn, grip
	EXPECT_EQ(c["data"][3]["id"], "1");
	EXPECT_EQ(c["data"][3]["p"][0], 1);
	EXPECT_EQ(c["data"][3]["p"][1], 2);
	EXPECT_EQ(c["data"][3]["p"][2], 3);
	EXPECT_EQ(c["data"][3]["buttons"].size(), 2);
	EXPECT_EQ(c["data"][3]["buttons"]["padbtn"]["buttonPercent"], 0.0);
	EXPECT_EQ(c["data"][3]["buttons"]["padbtn"]["x"], 0.25);
	EXPECT_EQ(c["data"][3]["buttons"]["padbtn"]["y"], -0.75);
	EXPECT_EQ(c["data"][3]["buttons"]["grip"]["buttonPercent"], 0.25);
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