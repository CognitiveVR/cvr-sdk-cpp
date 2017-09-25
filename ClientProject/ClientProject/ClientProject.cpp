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
		//curl_easy_setopt(curl, CURLOPT_URL, "https://www.example.com/");

		//verbose output
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Now specify the POST data */
		if (content.size() > 0)
		{
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str());
		}

		//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &handle);
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, &temp);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);

		//call response
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

TEST(Initialization, SessionFullStartEnd) {
	WebRequest fp = &DoWebStuff;
	std::vector<float> pos = { 0,0,0 };
	auto cog = CognitiveVRAnalyticsCore(fp, "somename", 10, 10, 10, 10, std::map<std::string, std::string>());
	cog.StartSession();
	cog.transaction->BeginEndPosition("testing1", pos);
	cog.EndSession();
}

TEST(Initialization, SessionStart) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);
	cog.StartSession();
}

TEST(Initialization, Initialization) {
	WebRequest fp = &DoWebStuff;
	std::vector<float> pos = { 0,0,0 };
	auto cog = CognitiveVRAnalyticsCore(fp);
	cog.transaction->BeginEndPosition("testing1", pos);
	cog.StartSession();
}

TEST(Initialization, SessionEnd) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);
	cog.EndSession();
}

TEST(Initialization, SessionStartEnd) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);
	cog.StartSession();
	cog.EndSession();
}

//----------------------SET USER

TEST(UserSettings, UserPreSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);
	nlohmann::json user = nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";
	cog.SetUser("john", user);
	cog.StartSession();
}

TEST(UserSettings, UserPostSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);
	nlohmann::json user = nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";
	cog.StartSession();
	cog.SetUser("john", user);
}

TEST(UserSettings, UserNullPreSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);
	cog.SetUser("", nlohmann::json());
	cog.StartSession();
}

TEST(UserSettings, UserNullPostSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);
	cog.StartSession();
	cog.SetUser("", nlohmann::json());
}

//----------------------SET DEVICE

TEST(DeviceSettings, DevicePreSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);
	nlohmann::json device = nlohmann::json();
	device["os"] = "chrome";
	device["retail value"] = 79.95;
	device["ram"] = 4;

	cog.SetDevice("chromebook", device);
	cog.StartSession();
}

TEST(DeviceSettings, DevicePostSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);
	nlohmann::json device = nlohmann::json();
	device["os"] = "chrome";
	device["retail value"] = 79.95;
	device["ram"] = 4;

	cog.StartSession();
	cog.SetDevice("chromebook", device);
}

TEST(DeviceSettings, DeviceNullPreSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);

	cog.SetDevice("", nlohmann::json());
	cog.StartSession();
}

TEST(DeviceSettings, DeviceNullPostSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);
	cog.StartSession();
	cog.SetDevice("", nlohmann::json());
}

TEST(DeviceSettings, DeviceNullPreEnd) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);

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
	auto cog = CognitiveVRAnalyticsCore(fp);
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
	auto cog = CognitiveVRAnalyticsCore(fp);
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
	auto cog = CognitiveVRAnalyticsCore(fp);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);

	cog.StartSession();
}

TEST(Transaction, PreSessionEnd) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);

	cog.StartSession();
	cog.EndSession();
}

TEST(Transaction, PreSessionSend) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);

	cog.StartSession();
	cog.SendData();
}

TEST(Transaction, PreSessionPropsSend) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);

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
	auto cog = CognitiveVRAnalyticsCore(fp);

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
	auto cog = CognitiveVRAnalyticsCore(fp);

	cog.StartSession();

	auto snow_attitude = cog.tuning->GetValue("snow_attitude", "mellow", EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, "ferocious");
	std::cout << snow_attitude << std::endl;

	auto blockPosition = cog.tuning->GetValue("vinegar_volume", 0, EntityType::kEntityTypeDevice);
	EXPECT_EQ(blockPosition, 50);
	std::cout << blockPosition << std::endl;

	auto ExitPollActivated = cog.tuning->GetValue("ExitPollActivated", false, EntityType::kEntityTypeDevice);
	EXPECT_EQ(ExitPollActivated, true);
	std::cout << ExitPollActivated << std::endl;

	auto pi = cog.tuning->GetValue("pi", (float)3.0, EntityType::kEntityTypeDevice);
	EXPECT_FLOAT_EQ(pi, 3.1415927);
	std::cout << pi << std::endl;


	cog.EndSession();
}

TEST(Tuning, TuningGetValueNoSession) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);

	auto snow_attitude = cog.tuning->GetValue("snow_attitude", "mellow", EntityType::kEntityTypeDevice);
	std::cout << snow_attitude << std::endl;
}

TEST(Tuning, TuningGetInvalidValue) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	auto snow_attitude = cog.tuning->GetValue("snow_raditude", "mellow", EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, "mellow");
	std::cout << snow_attitude << std::endl;
	cog.EndSession();
}

TEST(Tuning, TuningGetInvalidCast) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	auto snow_attitude = cog.tuning->GetValue("snow_attitude", 500, EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, 500);
	std::cout << snow_attitude << std::endl;
	cog.EndSession();
}

TEST(Tuning, TuningGetInvalidCastBool) {
	WebRequest fp = &DoWebStuff;
	auto cog = CognitiveVRAnalyticsCore(fp);

	cog.StartSession();
	auto snow_attitude = cog.tuning->GetValue("snow_attitude", false, EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, false);
	std::cout << snow_attitude << std::endl;
	cog.EndSession();
}

//----------------------EXITPOLL

//----------------------GAZE

//----------------------SENSORS

//----------------------DYNAMICS


int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);

	RUN_ALL_TESTS();

	system("pause");

	return 0;
}

/*
WebRequest fp = &DoWebStuff;

auto cog = CognitiveVRAnalyticsCore(fp);

nlohmann::json user = nlohmann::json();
user["age"] = 21;
user["location"] = "vancouver";

cog.SetUser("john", user);


nlohmann::json device = nlohmann::json();
device["os"] = "chrome";
device["retail value"] = 79.95;
device["ram"] = 4;

cog.SetDevice("chromebook", device);

cog.StartSession();

std::vector<float> pos = { 0,0,0 };
cog.transaction->BeginEndPosition("testing1", pos);
cog.transaction->BeginEndPosition("testing2", pos);
cog.transaction->BeginEndPosition("testing3", pos);

cog.exitpoll->RequestQuestionSet("pre_experience_questions");

cog.sensor->RecordSensor("comfort", 9);
cog.sensor->RecordSensor("fps", 60);
cog.sensor->RecordSensor("fps", 55);
cog.sensor->RecordSensor("comfort", 8);
cog.sensor->RecordSensor("fps", 30);
cog.sensor->RecordSensor("fps", 25);
cog.sensor->RecordSensor("comfort", 5);

cog.SendData();
*/