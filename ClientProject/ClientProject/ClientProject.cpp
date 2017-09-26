// ClientProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"
#include "include\curl\curl.h"
#include "gtest\gtest.h"

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

void DoWebStuff(std::string url, std::string content, WebResponse response)
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

TEST(Initialization, MultipleStartSessions) {
	
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	bool first = cog.StartSession();
	EXPECT_EQ(first, true);
	bool second = cog.StartSession();
	EXPECT_EQ(second, false);
	bool third = cog.StartSession();
	EXPECT_EQ(third, false);
}

TEST(Initialization, MultipleStartEndSessions) {

	WebRequest fp = &DoWebStuff;
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

TEST(Initialization, SessionFullStartEnd) {
	WebRequest fp = &DoWebStuff;
	std::vector<float> pos = { 0,0,0 };
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp, "somename", 10, 10, 10, 10, std::map<std::string, std::string>());
	cog.StartSession();
	cog.transaction->BeginEndPosition("testing1", pos);
	cog.EndSession();
}

TEST(Initialization, SessionStart) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.StartSession();
}

TEST(Initialization, Initialization) {
	WebRequest fp = &DoWebStuff;
	std::vector<float> pos = { 0,0,0 };
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.transaction->BeginEndPosition("testing1", pos);
	cog.StartSession();
}

TEST(Initialization, SessionEnd) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.EndSession();
}

TEST(Initialization, SessionStartEnd) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.StartSession();
	cog.EndSession();
}

//----------------------SET USER

TEST(UserSettings, UserPreSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	nlohmann::json user = nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";
	cog.SetUser("john", user);
	cog.StartSession();
}

TEST(UserSettings, UserPostSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	nlohmann::json user = nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";
	cog.StartSession();
	cog.SetUser("john", user);
}

TEST(UserSettings, UserNullPreSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.SetUser("", nlohmann::json());
	cog.StartSession();
}

TEST(UserSettings, UserNullPostSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.StartSession();
	cog.SetUser("", nlohmann::json());
}

//----------------------SET DEVICE

TEST(DeviceSettings, DevicePreSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	nlohmann::json device = nlohmann::json();
	device["os"] = "chrome";
	device["retail value"] = 79.95;
	device["ram"] = 4;

	cog.SetDevice("chromebook", device);
	cog.StartSession();
}

TEST(DeviceSettings, DevicePostSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	nlohmann::json device = nlohmann::json();
	device["os"] = "chrome";
	device["retail value"] = 79.95;
	device["ram"] = 4;

	cog.StartSession();
	cog.SetDevice("chromebook", device);
}

TEST(DeviceSettings, DeviceNullPreSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.SetDevice("", nlohmann::json());
	cog.StartSession();
}

TEST(DeviceSettings, DeviceNullPostSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	cog.StartSession();
	cog.SetDevice("", nlohmann::json());
}

TEST(DeviceSettings, DeviceNullPreEnd) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	nlohmann::json device = nlohmann::json();
	device["os"] = "chrome";
	device["retail value"] = 79.95;
	device["ram"] = 4;

	cog.SetDevice("chromebook", device);
	cog.EndSession();
}

//----------------------SET USER DEVICE
TEST(UserDeviceSettings, UserDevicePostSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	nlohmann::json device = nlohmann::json();
	device["os"] = "chrome";
	device["retail value"] = 79.95;
	device["ram"] = 4;

	nlohmann::json user = nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";


	cog.StartSession();
	cog.SetDevice("chromebook", device);
	cog.SetUser("john", user);
	cog.EndSession();
}

TEST(UserDeviceSettings, UserDevicePreSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);
	nlohmann::json device = nlohmann::json();
	device["os"] = "chrome";
	device["retail value"] = 79.95;
	device["ram"] = 4;

	nlohmann::json user = nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";
	cog.SetUser("john", user);
	cog.SetDevice("chromebook", device);

	cog.StartSession();
	cog.EndSession();
}

//----------------------TRANSACTIONS

TEST(Transaction, PreSessionNoEnd) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);

	cog.StartSession();
}

TEST(Transaction, PreSessionEnd) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);

	cog.StartSession();
	cog.EndSession();
}

TEST(Transaction, PreSessionSend) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);

	cog.StartSession();
	cog.SendData();
}

TEST(Transaction, PreSessionPropsSend) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	std::vector<float> pos = { 0,0,0 };

	nlohmann::json user = nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";

	cog.transaction->BeginEndPosition("testing1", pos, user);

	cog.StartSession();
	cog.SendData();
}

TEST(Transaction, SessionEnd) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	std::vector<float> pos = { 0,0,0 };

	nlohmann::json user = nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";

	cog.StartSession();
	cog.transaction->BeginEndPosition("testing1", pos, user);
	cog.EndSession();
}

//----------------------TUNING

TEST(Tuning, TuningGetValue) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

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
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	auto snow_attitude = cog.tuning->GetValue("snow_attitude", "mellow", cognitive::EntityType::kEntityTypeDevice);
	std::cout << snow_attitude << std::endl;
}

TEST(Tuning, TuningGetInvalidValue) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	auto snow_attitude = cog.tuning->GetValue("snow_raditude", "mellow", cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, "mellow");
	std::cout << snow_attitude << std::endl;
	cog.EndSession();
}

TEST(Tuning, TuningGetInvalidCast) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	auto snow_attitude = cog.tuning->GetValue("snow_attitude", 500, cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, 500);
	std::cout << snow_attitude << std::endl;
	cog.EndSession();
}

TEST(Tuning, TuningGetInvalidCastBool) {
	WebRequest fp = &DoWebStuff;
	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	auto snow_attitude = cog.tuning->GetValue("snow_attitude", false, cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, false);
	std::cout << snow_attitude << std::endl;
	cog.EndSession();
}

//----------------------SETTING SCENE KEYS FOR SCENE EXPLORER

TEST(Scenes, NoScenes) {
	WebRequest fp = &DoWebStuff;
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
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.exitpoll->RequestQuestionSet("player_died");

	cog.StartSession();
}

TEST(ExitPoll, BasicRequest) {
	WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("player_died");
	cog.EndSession();
}

TEST(ExitPoll, GetThenRequest) {
	WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);


	cog.StartSession();
	cog.exitpoll->GetQuestionSet();
	cog.exitpoll->RequestQuestionSet("player_died");
	cog.EndSession();
}

TEST(ExitPoll, RequestThenGet) {
	WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);


	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("player_died");
	cog.exitpoll->GetQuestionSet();
	cog.EndSession();
}

TEST(ExitPoll, InvalidRequestThenGet) {
	WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("question-does-not-exist");
	cog.exitpoll->GetQuestionSet();
	cog.EndSession();
}

TEST(ExitPoll, RequestThenGetAnswers) {
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

	auto cog = cognitive::CognitiveVRAnalyticsCore(fp);

	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor",i);
	}

	cog.StartSession();
	cog.EndSession();
}

TEST(Sensors, SenseThenInitSetScene) {
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

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
	WebRequest fp = &DoWebStuff;

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

	RUN_ALL_TESTS();
	system("pause");
	return 0;

	//should use only this in a build
	//return RUN_ALL_TESTS();
}