// ClientProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"
#include "curl.h"

#if defined(_MSC_VER)
#include "gtest.h"
#else
#include "include/gtest/gtest.h"
#endif

std::string temp;

size_t handle(char* buf, size_t size, size_t nmemb, void* up)
{
	//std::cout << "handle handle handle";

	for (int c = 0; c < size*nmemb; c++)
	{
		temp.push_back(buf[c]);
	}
	return size * nmemb;
}

void DoWebStuff(std::string url, std::string content, cognitive::WebResponse response)
{
	//std::cout << "<<<<curl url sent\n";
	//std::cout << url + "\n";

	CURL* curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	//std::cout << curl_version();

	if (curl) {
		/* First set the URL that is about to receive our POST. This URL can
		just as well be a https:// URL if that is what should receive the
		data. */
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		//verbose output
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Now specify the POST data */
		if (content.size() > 0)
		{
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str());
		}

		//if (response != NULL)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &handle);

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
	}
}

//===========================TESTS
//----------------------INITIALIZATION

TEST(DISABLED_Initialization, MultipleStartSessions) {
	
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	bool first = cog.StartSession();
	EXPECT_EQ(first, true);

	bool second = cog.StartSession();
	EXPECT_EQ(second, false);
	bool third = cog.StartSession();
	EXPECT_EQ(third, false);
}

TEST(DISABLED_Initialization, MultipleStartEndSessions) {

	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
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

TEST(DISABLED_Initialization, SessionFullStartEnd) {
	cognitive::WebRequest fp = &DoWebStuff;
	std::vector<float> pos = { 0,0,0 };
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp, "somename", 10, 10, 10, 10, std::map<std::string, std::string>());
	cog.StartSession();
	cog.transaction->BeginEndPosition("testing1", pos);
	cog.EndSession();
}

TEST(DISABLED_Initialization, SessionStart) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.StartSession();
}

TEST(DISABLED_Initialization, Initialization) {
	cognitive::WebRequest fp = &DoWebStuff;
	std::vector<float> pos = { 0,0,0 };
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.transaction->BeginEndPosition("testing1", pos);
	cog.StartSession();
}

TEST(DISABLED_Initialization, SessionEnd) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.EndSession();
}

TEST(DISABLED_Initialization, SessionStartEnd) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.StartSession();
	cog.EndSession();
}

//----------------------SET USER

TEST(UserSettings, UserPreSession) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cognitive::nlohmann::json user = cognitive::nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";
	cog.SetUser("john", user);
	cog.StartSession();
}

TEST(UserSettings, UserPostSession) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cognitive::nlohmann::json user = cognitive::nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";
	cog.StartSession();
	cog.SetUser("john", user);
}

TEST(UserSettings, UserNullPreSession) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.SetUser("", cognitive::nlohmann::json());
	cog.StartSession();
}

TEST(UserSettings, UserNullPostSession) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.StartSession();
	cog.SetUser("", cognitive::nlohmann::json());
}

//----------------------SET DEVICE

TEST(DeviceSettings, DevicePreSession) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cognitive::nlohmann::json device = cognitive::nlohmann::json();
	device["os"] = "chrome";
	device["retail value"] = 79.95;
	device["ram"] = 4;

	cog.SetDevice("chromebook", device);
	cog.StartSession();
}

TEST(DeviceSettings, DevicePostSession) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cognitive::nlohmann::json device = cognitive::nlohmann::json();
	device["os"] = "chrome";
	device["retail value"] = 79.95;
	device["ram"] = 4;

	cog.StartSession();
	cog.SetDevice("chromebook", device);
}

TEST(DeviceSettings, DeviceNullPreSession) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.SetDevice("", cognitive::nlohmann::json());
	cog.StartSession();
}

TEST(DeviceSettings, DeviceNullPostSession) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.StartSession();
	cog.SetDevice("", cognitive::nlohmann::json());
}

TEST(DeviceSettings, DeviceNullPreEnd) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cognitive::nlohmann::json device = cognitive::nlohmann::json();
	device["os"] = "chrome";
	device["retail value"] = 79.95;
	device["ram"] = 4;

	cog.SetDevice("chromebook", device);
	cog.EndSession();
}

//----------------------SET USER DEVICE
TEST(UserDeviceSettings, UserDevicePostSession) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cognitive::nlohmann::json device = cognitive::nlohmann::json();
	device["os"] = "chrome";
	device["retail value"] = 79.95;
	device["ram"] = 4;

	cognitive::nlohmann::json user = cognitive::nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";


	cog.StartSession();
	cog.SetDevice("chromebook", device);
	cog.SetUser("john", user);
	cog.EndSession();
}

TEST(UserDeviceSettings, UserDevicePreSession) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cognitive::nlohmann::json device = cognitive::nlohmann::json();
	device["os"] = "chrome";
	device["retail value"] = 79.95;
	device["ram"] = 4;

	cognitive::nlohmann::json user = cognitive::nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";
	cog.SetUser("john", user);
	cog.SetDevice("chromebook", device);

	cog.StartSession();
	cog.EndSession();
}

//----------------------TRANSACTIONS

TEST(Transaction, PreSessionNoEnd) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);

	cog.StartSession();
}

TEST(Transaction, PreSessionEnd) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);

	cog.StartSession();
	cog.EndSession();
}

TEST(Transaction, PreSessionSend) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);

	cog.StartSession();
	cog.SendData();
}

TEST(Transaction, PreSessionPropsSend) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	std::vector<float> pos = { 0,0,0 };

	cognitive::nlohmann::json user = cognitive::nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";

	cog.transaction->BeginEndPosition("testing1", pos, user);

	cog.StartSession();
	cog.SendData();
}

TEST(Transaction, SessionEnd) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	std::vector<float> pos = { 0,0,0 };

	cognitive::nlohmann::json user = cognitive::nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";

	cog.StartSession();
	cog.transaction->BeginEndPosition("testing1", pos, user);
	cog.EndSession();
}

//----------------------TUNING

TEST(DISABLED_Tuning, TuningGetValue) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	//DEBUG
	return;

	cog.StartSession();

	if (cog.WasInitSuccessful())
	{

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
	else
	{
		std::cout << "DEBUG could not init session for TuningGetValue!";
	}
}

TEST(DISABLED_Tuning, TuningGetValueNoSession) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	auto snow_attitude = cog.tuning->GetValue("snow_attitude", "mellow", cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, "mellow");
	std::cout << snow_attitude << std::endl;
}

TEST(DISABLED_Tuning, TuningGetInvalidValue) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	auto snow_attitude = cog.tuning->GetValue("snow_raditude", "mellow", cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, "mellow");
	std::cout << snow_attitude << std::endl;
	cog.EndSession();
}

TEST(DISABLED_Tuning, TuningGetInvalidCast) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	auto snow_attitude = cog.tuning->GetValue("snow_attitude", 500, cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, 500);
	std::cout << snow_attitude << std::endl;
	cog.EndSession();
}

TEST(DISABLED_Tuning, TuningGetInvalidCastBool) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	auto snow_attitude = cog.tuning->GetValue("snow_attitude", false, cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, false);
	std::cout << snow_attitude << std::endl;
	cog.EndSession();
}

//----------------------SETTING SCENE KEYS FOR SCENE EXPLORER

TEST(Scenes, NoScenes) {
	cognitive::WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.StartSession();

	std::vector<float> pos = { 1,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	pos[0] = 2;
	cog.transaction->BeginEndPosition("testing2", pos);
	pos[0] = 3;
	cog.transaction->BeginEndPosition("testing3", pos);

	cog.SendData();
	cog.EndSession();
}

TEST(Scenes, InitScenes) {
	cognitive::WebRequest fp = &DoWebStuff;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["tutorial"] = "DELETE_ME_1";
	scenes["menu"] = "DELETE_ME_2";
	scenes["finalboss"] = "DELETE_ME_3";
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp,scenes);

	cog.StartSession();

	std::vector<float> pos = { 1,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	pos[0] = 2;
	cog.transaction->BeginEndPosition("testing2", pos);
	pos[0] = 3;
	cog.transaction->BeginEndPosition("testing3", pos);

	cog.SendData();
	cog.EndSession();
}

TEST(Scenes, InitSetScenes) {
	cognitive::WebRequest fp = &DoWebStuff;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["tutorial"] = "DELETE_ME_1";
	scenes["menu"] = "DELETE_ME_2";
	scenes["finalboss"] = "DELETE_ME_3";
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp, scenes);

	cog.StartSession();
	cog.SetScene("tutorial");

	std::vector<float> pos = { 1,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	pos[0] = 2;
	cog.transaction->BeginEndPosition("testing2", pos);
	pos[0] = 3;
	cog.transaction->BeginEndPosition("testing3", pos);

	cog.SendData();
	cog.EndSession();
}

TEST(Scenes, InitSetSceneSwitch) {
	cognitive::WebRequest fp = &DoWebStuff;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["tutorial"] = "DELETE_ME_1";
	scenes["menu"] = "DELETE_ME_2";
	scenes["finalboss"] = "DELETE_ME_3";
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp, scenes);

	cog.StartSession();
	cog.SetScene("tutorial");

	std::vector<float> pos = { 1,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	cog.SetScene("menu");
	pos[0] = 2;
	cog.transaction->BeginEndPosition("testing2", pos);
	cog.SetScene("finalboss");
	pos[0] = 3;
	cog.transaction->BeginEndPosition("testing3", pos);

	cog.SendData();
	cog.EndSession();
}

TEST(Scenes, InitSetInvalidScene) {
	cognitive::WebRequest fp = &DoWebStuff;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["tutorial"] = "DELETE_ME_1";
	scenes["menu"] = "DELETE_ME_2";
	scenes["finalboss"] = "DELETE_ME_3";
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp, scenes);

	cog.StartSession();
	cog.SetScene("non-existent");

	std::vector<float> pos = { 1,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);

	cog.SendData();
	cog.EndSession();
}

TEST(Scenes, InitSetInvalidNoScene) {
	cognitive::WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	cog.SetScene("non-existent");

	std::vector<float> pos = { 1,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);

	cog.SendData();
	cog.EndSession();
}

//----------------------EXITPOLL

TEST(ExitPoll, RequestSetNoInit) {
	cognitive::WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.exitpoll->RequestQuestionSet("player_died");

	cog.StartSession();
}

TEST(ExitPoll, BasicRequest) {
	cognitive::WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("player_died");
	cog.EndSession();
}

TEST(ExitPoll, GetThenRequest) {
	cognitive::WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);


	cog.StartSession();
	cog.exitpoll->GetQuestionSet();
	cog.exitpoll->RequestQuestionSet("player_died");
	cog.EndSession();
}

TEST(ExitPoll, RequestThenGet) {
	cognitive::WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);


	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("player_died");
	cog.exitpoll->GetQuestionSet();
	cog.EndSession();
}

TEST(ExitPoll, InvalidRequestThenGet) {
	cognitive::WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("question-does-not-exist");
	cog.exitpoll->GetQuestionSet();
	cog.EndSession();
}

TEST(ExitPoll, RequestThenGetAnswers) {
	cognitive::WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("player_died");
	cog.exitpoll->GetQuestionSet();
	cog.exitpoll->AddAnswer(cognitive::FExitPollAnswer(cognitive::EQuestionType::HAPPYSAD, false));
	cog.exitpoll->AddAnswer(cognitive::FExitPollAnswer(cognitive::EQuestionType::MULTIPLE, 0));
	cog.exitpoll->AddAnswer(cognitive::FExitPollAnswer(cognitive::EQuestionType::SCALE, 1));
	cog.exitpoll->SendAllAnswers();
	cog.EndSession();
}

//----------------------GAZE

TEST(Gaze, GazeThenInit) {
	cognitive::WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	for (int i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot);
	}
	
	cog.StartSession();
	cog.EndSession();
}

TEST(Gaze, GazeThenInitSetScene) {
	cognitive::WebRequest fp = &DoWebStuff;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["gazescene"] = "DELETE_ME_3";
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp, scenes);

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	for (int i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot);
	}

	cog.StartSession();
	cog.SetScene("gazescene");
	cog.EndSession();
}

TEST(Gaze, InitThenGazeThenSetScene) {
	cognitive::WebRequest fp = &DoWebStuff;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["gazescene"] = "DELETE_ME_3";
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp, scenes);

	cog.StartSession();
	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	for (int i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot);
	}

	cog.SetScene("gazescene");
	cog.EndSession();
}

TEST(Gaze, InitThenGazeThenSendThenSetScene) {
	cognitive::WebRequest fp = &DoWebStuff;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["gazescene"] = "DELETE_ME_3";
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp, scenes);

	cog.StartSession();
	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	for (int i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot);
	}
	cog.SendData();
	cog.SetScene("gazescene");
	cog.EndSession();
}

//----------------------SENSORS

TEST(Sensors, SenseThenInit) {
	cognitive::WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor",i);
	}

	cog.StartSession();
	cog.EndSession();
}

TEST(Sensors, SenseThenInitSetScene) {
	cognitive::WebRequest fp = &DoWebStuff;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["sensescene"] = "DELETE_ME_3";
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp, scenes);

	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}

	cog.StartSession();
	cog.SetScene("sensescene");
	cog.EndSession();
}

TEST(Sensors, InitThenGazeThenSetScene) {
	cognitive::WebRequest fp = &DoWebStuff;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["sensescene"] = "DELETE_ME_3";
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp, scenes);

	cog.StartSession();
	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}

	cog.SetScene("sensescene");
	cog.EndSession();
}

TEST(Sensors, InitThenGazeThenSendThenSetScene) {
	cognitive::WebRequest fp = &DoWebStuff;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["sensescene"] = "DELETE_ME_3";
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp, scenes);

	cog.StartSession();
	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}
	cog.SendData();
	cog.SetScene("sensescene");
	cog.EndSession();
}

//----------------------DYNAMICS

TEST(DISABLED_Dynamics, InitRegisterSend) {
	cognitive::WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.StartSession();

	int object1id = cog.dynamicobject->RegisterObject("object1", "lamp");
	int object2id = cog.dynamicobject->RegisterObject("object2", "lamp");

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	pos = { 0,0,5 };
	cog.dynamicobject->Snapshot(object1id, pos, rot);
	pos = { 0,1,6 };
	cog.dynamicobject->Snapshot(object2id, pos, rot);

	pos = { 0,0,7 };
	cog.dynamicobject->Snapshot(object1id, pos, rot);
	pos = { 0,2,8 };
	cog.dynamicobject->Snapshot(object2id, pos, rot);

	pos = { 0,0,9 };
	cog.dynamicobject->Snapshot(object1id, pos, rot);
	pos = { 0,3,10 };
	cog.dynamicobject->Snapshot(object2id, pos, rot);

	cog.SendData();
	cog.EndSession();
}

TEST(Dynamics, InitRegisterSceneSend) {
	cognitive::WebRequest fp = &DoWebStuff;

	

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["dynamicscene"] = "DELETE_ME_2";
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp, scenes);
	cog.StartSession();

	int object1id = cog.dynamicobject->RegisterObject("object1", "lamp");
	int object2id = cog.dynamicobject->RegisterObject("object2", "lamp");

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	pos = { 0,0,5 };
	cog.dynamicobject->Snapshot(object1id, pos, rot);
	pos = { 0,1,6 };
	cog.dynamicobject->Snapshot(object2id, pos, rot);

	pos = { 0,0,7 };
	cog.dynamicobject->Snapshot(object1id, pos, rot);
	pos = { 0,2,8 };
	cog.dynamicobject->Snapshot(object2id, pos, rot);

	pos = { 0,0,9 };
	cog.dynamicobject->Snapshot(object1id, pos, rot);
	pos = { 0,3,10 };
	cog.dynamicobject->Snapshot(object2id, pos, rot);
	
	cog.SetScene("dynamicscene");
	cog.SendData();
	cog.EndSession();
}



int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);

	
	return RUN_ALL_TESTS();

	//should use only this in a build
	//return RUN_ALL_TESTS();
}