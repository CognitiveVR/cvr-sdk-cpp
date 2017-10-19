// ClientProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"
#include "curl.h"

//for testing delay
#include <chrono>
#include <thread>

#if defined(_MSC_VER)
#include "gtest.h"
#else
#include "include/gtest/gtest.h"
#endif

std::string temp;

std::string TESTINGCUSTOMER = "altimagegames59340-cppsdkautomatedtesting-test";
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

void NORESPONSE_DoWebStuff(std::string url, std::string content, cognitive::WebResponse response)
{

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

TEST(Initialization, MultipleStartSessions) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
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
	settings.CustomerId = TESTINGCUSTOMER;
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

TEST(Initialization, SessionFullStartEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));
	
	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	//settings. //TODO try all the fields in the settings container
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.StartSession();
	cog.transaction->BeginEndPosition("testing1", pos);
	cog.EndSession();
}

TEST(Initialization, SessionStart) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
}

TEST(Initialization, SessionStartNoWeb) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.loggingLevel == cognitive::LoggingLevel::kAll;
	//settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.WasInitSuccessful(), false);
}

TEST(Initialization, SessionStartNoWebResponse) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.loggingLevel == cognitive::LoggingLevel::kAll;
	settings.webRequest = &NORESPONSE_DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.WasInitSuccessful(), false);
}

TEST(Initialization, SessionStartInvalidCustomerId) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.CustomerId = "INVALID_DELETE_NOT_A_CUSTOMERID";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	EXPECT_EQ(cog.GetCustomerId(), "INVALID_DELETE_NOT_A_CUSTOMERID");
	
	cog.StartSession();
	EXPECT_EQ(cog.GetCustomerId(), "INVALID_DELETE_NOT_A_CUSTOMERID");
	EXPECT_EQ(cog.WasInitSuccessful(), false);
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
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	cog.StartSession();
}

TEST(Initialization, SessionEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.EndSession();
}

TEST(Initialization, SessionStartEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
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
	settings.CustomerId = TESTINGCUSTOMER;
	settings.loggingLevel = cognitive::LoggingLevel::kAll;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cognitive::nlohmann::json user = cognitive::nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";
	cog.SetUserId("john");
	cog.SetUserProperties(user);

	EXPECT_EQ(cog.UserId, "john");
	EXPECT_EQ(cog.UserProperties.dump(), "{\"age\":21,\"location\":\"vancouver\"}");

	cog.UpdateUserState();
	cog.StartSession();
}

TEST(UserSettings, UserPostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cognitive::nlohmann::json user = cognitive::nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";
	cog.StartSession();
	cog.SetUserId("john");

	EXPECT_EQ(cog.UserId, "john");
	cog.SetUserProperties(user);
	EXPECT_EQ(cog.UserProperties.dump(), "{\"age\":21,\"location\":\"vancouver\"}");
	cog.UpdateUserState();
}

TEST(UserSettings, UserNullPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetUserId("");
	cog.SetUserProperties(cognitive::nlohmann::json());

	EXPECT_EQ(cog.UserId, "");
	EXPECT_EQ(cog.UserProperties, cognitive::nlohmann::json());

	cog.UpdateUserState();
	EXPECT_EQ(cog.UserId, "");
	EXPECT_EQ(cog.UserProperties, cognitive::nlohmann::json());
	cog.StartSession();
}

TEST(UserSettings, UserNullPostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.SetUserId("");
	cog.SetUserProperties(cognitive::nlohmann::json());
	cog.UpdateUserState();
	cog.SetUserId("");
	cog.SetUserProperties(cognitive::nlohmann::json());
	EXPECT_EQ(cog.UserId, "");
	EXPECT_EQ(cog.UserProperties, cognitive::nlohmann::json());
}

//----------------------SET DEVICE

TEST(DeviceSettings, DevicePreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetDeviceId("7741345684915735");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceMemory, 128);
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceOS, "chrome os 16.9f");

	EXPECT_EQ(cog.DeviceId, "7741345684915735");
	EXPECT_EQ(cog.DeviceProperties.dump(), "{\"cvr.device.memory\":128,\"cvr.device.os\":\"chrome os 16.9f\"}");

	cog.UpdateDeviceState();
	cog.StartSession();
}

TEST(DeviceSettings, DevicePostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.SetDeviceId("7741345684915735");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceCPU, "i7-4770 CPU @ 3.40GHz");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceGPU, "GeForce GTX 970");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceMemory, 128);
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceOS, "chrome os 16.9f");	

	EXPECT_EQ(cog.DeviceId, "7741345684915735");
	EXPECT_EQ(cog.DeviceProperties.dump(), "{\"cvr.device.cpu\":\"i7-4770 CPU @ 3.40GHz\",\"cvr.device.gpu\":\"GeForce GTX 970\",\"cvr.device.memory\":128,\"cvr.device.os\":\"chrome os 16.9f\"}");

	cog.UpdateDeviceState();
}

TEST(DeviceSettings, DevicePostSessionOverwrite) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.SetDeviceId("7741345684915735");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceCPU, "i5");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceGPU, "GeForce GTX 170");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceMemory, 16);
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceOS, "chrome os 16.9f");

	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceCPU, "i7-4770 CPU @ 3.40GHz");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceGPU, "GeForce GTX 970");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceMemory, 128);

	EXPECT_EQ(cog.DeviceId, "7741345684915735");
	EXPECT_EQ(cog.DeviceProperties.dump(), "{\"cvr.device.cpu\":\"i7-4770 CPU @ 3.40GHz\",\"cvr.device.gpu\":\"GeForce GTX 970\",\"cvr.device.memory\":128,\"cvr.device.os\":\"chrome os 16.9f\"}");

	cog.UpdateDeviceState();
}

TEST(DeviceSettings, DevicePostSessionOutOfOrder) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.SetDeviceId("7741345684915735");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceMemory, 128);
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceGPU, "GeForce GTX 970");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceOS, "chrome os 16.9f");	
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceCPU, "i7-4770 CPU @ 3.40GHz");

	EXPECT_EQ(cog.DeviceId, "7741345684915735");
	EXPECT_EQ(cog.DeviceProperties.dump(), "{\"cvr.device.cpu\":\"i7-4770 CPU @ 3.40GHz\",\"cvr.device.gpu\":\"GeForce GTX 970\",\"cvr.device.memory\":128,\"cvr.device.os\":\"chrome os 16.9f\"}");

	cog.UpdateDeviceState();
}

TEST(DeviceSettings, DeviceNullPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.SetDeviceId("");

	EXPECT_EQ(cog.DeviceId, "");
	EXPECT_EQ(cog.DeviceProperties, cognitive::nlohmann::json());

	cog.UpdateDeviceState();
	cog.StartSession();
}

TEST(DeviceSettings, DeviceNullPostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();
	cog.SetDeviceId("");
	cog.UpdateDeviceState();
	EXPECT_EQ(cog.DeviceId, "");
	EXPECT_EQ(cog.DeviceProperties, cognitive::nlohmann::json());
}

TEST(DeviceSettings, DeviceNullPreEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);


	cog.SetDeviceId("7741345684915735");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceMemory, 128);
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceOS, "chrome os 16.9f");
	EXPECT_EQ(cog.DeviceId, "7741345684915735");
	EXPECT_EQ(cog.DeviceProperties.dump(), "{\"cvr.device.memory\":128,\"cvr.device.os\":\"chrome os 16.9f\"}");
	cog.UpdateDeviceState();
	EXPECT_EQ(cog.DeviceId, "7741345684915735");
	EXPECT_EQ(cog.DeviceProperties.dump(), "{\"cvr.device.memory\":128,\"cvr.device.os\":\"chrome os 16.9f\"}");
	cog.EndSession();
}

//----------------------SET USER DEVICE
TEST(UserDeviceSettings, UserDevicePostSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();

	cog.SetDeviceId("7741345684915735");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceMemory, 128);
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceOS, "chrome os 16.9f");
	cog.UpdateDeviceState();
	EXPECT_EQ(cog.DeviceId, "7741345684915735");
	EXPECT_EQ(cog.DeviceProperties.dump(), "{\"cvr.device.memory\":128,\"cvr.device.os\":\"chrome os 16.9f\"}");

	cognitive::nlohmann::json user = cognitive::nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";
	cog.SetUserId("john");
	EXPECT_EQ(cog.UserId, "john");
	EXPECT_EQ(cog.UserProperties, cognitive::nlohmann::json());
	cog.SetUserProperties(user);
	EXPECT_EQ(cog.UserProperties.dump(), "{\"age\":21,\"location\":\"vancouver\"}");
	cog.UpdateUserState();

	cog.EndSession();
}

TEST(UserDeviceSettings, UserDevicePreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cognitive::nlohmann::json user = cognitive::nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";
	cog.SetUserId("john");
	cog.SetUserProperties(user);
	cog.UpdateUserState();
	EXPECT_EQ(cog.UserId, "john");
	EXPECT_EQ(cog.UserProperties.dump(), "{\"age\":21,\"location\":\"vancouver\"}");

	cog.SetDeviceId("7741345684915735");
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceMemory, 128);
	cog.SetDeviceProperty(cognitive::EDeviceProperty::kDeviceOS, "chrome os 16.9f");
	EXPECT_EQ(cog.DeviceId, "7741345684915735");
	EXPECT_EQ(cog.DeviceProperties.dump(), "{\"cvr.device.memory\":128,\"cvr.device.os\":\"chrome os 16.9f\"}");
	cog.UpdateDeviceState();

	cog.StartSession();
	cog.EndSession();
}

//----------------------TRANSACTIONS

TEST(Transaction, PreSessionNoEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);

	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 1);

	cog.StartSession();
}

TEST(Transaction, PreSessionEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 1);
	cog.StartSession();
	cog.EndSession();
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
}

TEST(Transaction, PreSessionSend) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 1);
	cog.StartSession();
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 2); //testing1 and sesionbegin
	cog.SendData();
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
}

TEST(Transaction, PreSessionPropsSend) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };

	cognitive::nlohmann::json user = cognitive::nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";

	cog.transaction->BeginEndPosition("testing1", pos, user);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 1);
	cog.StartSession();
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 2);
	cog.SendData();
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
}

TEST(Transaction, SessionEnd) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };

	cognitive::nlohmann::json user = cognitive::nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";

	cog.StartSession();
	cog.transaction->BeginEndPosition("testing1", pos, user);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 2);
	cog.EndSession();
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
}

TEST(Transaction, SendLimitPreSessionThreshold) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	settings.TransactionBatchSize = 3; //on the third transaction it should send
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	cog.transaction->BeginEndPosition("testing2", pos);
	cog.transaction->BeginEndPosition("testing3", pos); //should try to send here

	cog.StartSession(); //fourth transaction. should send all
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
}

TEST(Transaction, SendLimitPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	settings.TransactionBatchSize = 3; //on the third transaction it should send
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);

	cog.StartSession();
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 2);
	cog.transaction->BeginEndPosition("testing1", pos);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
	cog.EndSession();
}

TEST(Transaction, SendLimitSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	settings.TransactionBatchSize = 3; //on the third transaction it should send
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float> pos = { 0,0,0 };
	cog.StartSession();
	cog.transaction->BeginEndPosition("testing1", pos);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 2);
	cog.transaction->BeginEndPosition("testing2", pos); //send here
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
	
	cog.transaction->BeginEndPosition("testing1", pos);
	cog.transaction->BeginEndPosition("testing1", pos);
	cog.transaction->BeginEndPosition("testing1", pos);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);

	cog.transaction->BeginEndPosition("testing1", pos);
	cog.EndSession();
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
}

//----------------------TUNING

TEST(Tuning, TuningGetValue) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
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
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	auto snow_attitude = cog.tuning->GetValue("snow_attitude", "mellow", cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, "mellow");
}

TEST(Tuning, TuningGetInvalidValue) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
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
	settings.CustomerId = TESTINGCUSTOMER;
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
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	auto snow_attitude = cog.tuning->GetValue("snow_attitude", false, cognitive::EntityType::kEntityTypeDevice);
	EXPECT_EQ(snow_attitude, false);
	cog.EndSession();
}

//----------------------SETTING SCENE KEYS FOR SCENE EXPLORER

TEST(Scenes, NoScenes) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();

	std::vector<float> pos = { 1,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	pos[0] = 2;
	cog.transaction->BeginEndPosition("testing2", pos);
	pos[0] = 3;
	cog.transaction->BeginEndPosition("testing3", pos);
	EXPECT_EQ(cog.CurrentSceneId, "");
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 4);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 4);
	cog.SendData();
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 4);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
	cog.EndSession();
}

TEST(Scenes, InitScenes) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["tutorial"] = "DELETE_ME_1";
	scenes["menu"] = "DELETE_ME_2";
	scenes["finalboss"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();

	std::vector<float> pos = { 1,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	pos[0] = 2;
	cog.transaction->BeginEndPosition("testing2", pos);
	pos[0] = 3;
	cog.transaction->BeginEndPosition("testing3", pos);

	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 4);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 4);
	cog.SendData();
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 4);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
	cog.EndSession();
}

TEST(Scenes, InitSetScenes) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["tutorial"] = "DELETE_ME_1";
	scenes["menu"] = "DELETE_ME_2";
	scenes["finalboss"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.CurrentSceneId, "");
	cog.SetScene("tutorial");
	EXPECT_EQ(cog.CurrentSceneId, "DELETE_ME_1");

	std::vector<float> pos = { 1,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	pos[0] = 2;
	cog.transaction->BeginEndPosition("testing2", pos);
	pos[0] = 3;
	cog.transaction->BeginEndPosition("testing3", pos);
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 4);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 4);
	cog.SendData();
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 0);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
	cog.EndSession();
}

TEST(Scenes, InitSetSceneSwitch) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["tutorial"] = "DELETE_ME_1";
	scenes["menu"] = "DELETE_ME_2";
	scenes["finalboss"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.CurrentSceneId,"");
	cog.SetScene("tutorial");
	EXPECT_EQ(cog.CurrentSceneId, "DELETE_ME_1");

	std::vector<float> pos = { 1,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 2);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 2);
	cog.SetScene("menu");
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 0);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
	pos[0] = 2;
	cog.transaction->BeginEndPosition("testing2", pos);
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 1);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 1);
	cog.SetScene("finalboss");
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 0);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
	pos[0] = 3;
	cog.transaction->BeginEndPosition("testing3", pos);
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 1);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 1);

	cog.SendData();
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 0);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
	cog.EndSession();
}

TEST(Scenes, InitSetInvalidScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["tutorial"] = "DELETE_ME_1";
	scenes["menu"] = "DELETE_ME_2";
	scenes["finalboss"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.CurrentSceneId, "");
	cog.SetScene("non-existent");
	EXPECT_EQ(cog.CurrentSceneId, "");

	std::vector<float> pos = { 1,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 2);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 2);

	cog.SendData();
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 2);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
	cog.EndSession();
}

TEST(Scenes, InitSetInvalidNoScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.CurrentSceneId, "");
	cog.SetScene("non-existent");
	EXPECT_EQ(cog.CurrentSceneId, "");

	std::vector<float> pos = { 1,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 2);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 2);

	cog.SendData();
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 2);
	EXPECT_EQ(cog.transaction->BatchedTransactions.size(), 0);
	cog.EndSession();
}

TEST(Scenes, SetSceneMiddle) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["finalboss"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	EXPECT_EQ(cog.CurrentSceneId, "");
	cog.SetScene("non-existent");
	EXPECT_EQ(cog.CurrentSceneId, "");

	std::vector<float> pos = { 1,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos);
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 2);

	cog.SendData();
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 2);
	cog.SetScene("finalboss");
	cog.SendData();
	EXPECT_EQ(cog.transaction->BatchedTransactionsSE.size(), 0);

	cog.EndSession();
}
//----------------------EXITPOLL

TEST(ExitPoll, RequestSetNoInit) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.exitpoll->RequestQuestionSet("arquestions");

	EXPECT_EQ(cog.exitpoll->GetQuestionSetString(), "");

	cog.StartSession();
}

TEST(ExitPoll, BasicRequest) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	settings.loggingLevel = cognitive::LoggingLevel::kAll;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("testing_questions");
	EXPECT_EQ(cog.exitpoll->GetQuestionSetString(), "{\"customerId\":\"altimagegames59340-cppsdkautomatedtesting-test\",\"id\":\"testingquestionsetbool:1\",\"name\":\"testingquestionsetbool\",\"version\":1,\"title\":\"Questionnaire\",\"status\":\"active\",\"questions\":[{\"title\":\"Are you having fun with this experience?\",\"type\":\"BOOLEAN\"}]}");
	cog.EndSession();
}

TEST(ExitPoll, GetThenRequest) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);


	cog.StartSession();
	cog.exitpoll->GetQuestionSet();
	cog.exitpoll->RequestQuestionSet("testing_questions");
	cog.EndSession();
}

TEST(ExitPoll, RequestThenGet) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);


	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("testing_questions");
	cog.exitpoll->GetQuestionSet();
	cog.EndSession();
}

TEST(ExitPoll, InvalidRequestThenGet) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("question-does-not-exist");
	cog.exitpoll->GetQuestionSet();
	cog.EndSession();
}

TEST(ExitPoll, RequestThenGetAnswersJson) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("testing_questions");
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
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	cog.exitpoll->RequestQuestionSet("testing_questions");
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
	settings.CustomerId = TESTINGCUSTOMER;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	for (float i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot);
	}

	EXPECT_EQ(cog.gaze->BatchedGazeSE.size(), 10);
	
	cog.StartSession();
	cog.EndSession();
	EXPECT_EQ(cog.gaze->BatchedGazeSE.size(), 0); //no scene to send to. endsession clears everything
}

TEST(Gaze, GazeThenInitSetScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["gazescene"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);


	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	for (float i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot);
	}
	EXPECT_EQ(cog.gaze->BatchedGazeSE.size(), 10);

	cog.StartSession();
	cog.SetScene("gazescene");
	cog.EndSession();
	EXPECT_EQ(cog.gaze->BatchedGazeSE.size(), 0);
}

TEST(Gaze, InitThenGazeThenSetScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["gazescene"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	for (float i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot);
	}
	EXPECT_EQ(cog.gaze->BatchedGazeSE.size(), 10);

	cog.SetScene("gazescene");
	cog.EndSession();
	EXPECT_EQ(cog.gaze->BatchedGazeSE.size(), 0);
}

TEST(Gaze, InitThenGazeThenSendThenSetScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["gazescene"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	for (float i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot);
	}
	EXPECT_EQ(cog.gaze->BatchedGazeSE.size(), 10);

	cog.SendData();
	EXPECT_EQ(cog.gaze->BatchedGazeSE.size(), 10);
	cog.SetScene("gazescene");
	cog.SendData();
	EXPECT_EQ(cog.gaze->BatchedGazeSE.size(), 0);
	cog.EndSession();
	EXPECT_EQ(cog.gaze->BatchedGazeSE.size(), 0);
}

TEST(Gaze, GazeOnDynamic) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["gazescene"] = "DELETE_ME_3";
	settings.sceneIds = scenes;

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

	EXPECT_EQ(cog.gaze->BatchedGazeSE.size(), 10);

	cog.dynamicobject->RegisterObjectCustomId("object1", "block", 1);

	for (float i = 0; i < 10; ++i)
	{
		pos[1] = i;
		cog.gaze->RecordGaze(pos, rot, point,1);
	}

	EXPECT_EQ(cog.gaze->BatchedGazeSE.size(), 20);

	cog.SendData();
	EXPECT_EQ(cog.gaze->BatchedGazeSE.size(), 0);

	cog.EndSession();
}

//----------------------SENSORS

TEST(Sensors, SenseThenInit) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor",i);	
	}

	cog.StartSession();
	EXPECT_EQ(cog.sensor->sensorCount, 10);
	cog.SendData();
	EXPECT_EQ(cog.sensor->sensorCount, 10);
	cog.EndSession();
	EXPECT_EQ(cog.sensor->sensorCount, 0);
}

TEST(Sensors, EndSessionThenSense) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;

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
	settings.CustomerId = TESTINGCUSTOMER;
	settings.SensorDataLimit = 10;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["sensescene"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
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
	settings.CustomerId = TESTINGCUSTOMER;
	settings.SensorDataLimit = 15;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["sensescene"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
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
	settings.CustomerId = TESTINGCUSTOMER;
	
	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["sensescene"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
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
	EXPECT_EQ(cog.sensor->sensorCount, 6);
}

TEST(Sensors, SenseThenInitSetScene) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["sensescene"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
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
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["sensescene"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
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
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["sensescene"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);

	cog.StartSession();
	for (int i = 0; i < 10; ++i)
	{
		cog.sensor->RecordSensor("test-sensor", i);
	}
	EXPECT_EQ(cog.sensor->sensorCount, 10);
	cog.SendData();
	cog.SetScene("sensescene");
	EXPECT_EQ(cog.sensor->sensorCount, 10);
	cog.EndSession();
	EXPECT_EQ(cog.sensor->sensorCount, 0);
}

TEST(Sensors, ManySensors) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["sensescene"] = "DELETE_ME_3";
	settings.sceneIds = scenes;
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
	settings.CustomerId = TESTINGCUSTOMER;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	int object1id = cog.dynamicobject->RegisterObject("object1", "lamp");
	int object2id = cog.dynamicobject->RegisterObject("object2", "lamp");

	EXPECT_EQ(cog.dynamicobject->fullManifest.size(), 2);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 2);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 0);

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	pos = { 0,0,5 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,1,6 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 2);

	pos = { 0,0,7 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,2,8 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 4);

	pos = { 0,0,9 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,3,10 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 6);

	cog.SendData();
	EXPECT_EQ(cog.dynamicobject->fullManifest.size(), 2);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 2);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 6);
	cog.EndSession();
}

TEST(Dynamics, InitRegisterSceneSend) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["dynamicscene"] = "DELETE_ME_2";
	settings.sceneIds = scenes;
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	int object1id = cog.dynamicobject->RegisterObject("object1", "lamp");
	int object2id = cog.dynamicobject->RegisterObject("object2", "lamp");

	EXPECT_EQ(cog.dynamicobject->fullManifest.size(), 2);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 2);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 0);

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	pos = { 0,0,5 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,1,6 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 2);

	pos = { 0,0,7 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,2,8 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 4);

	pos = { 0,0,9 };
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	pos = { 0,3,10 };
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 6);
	
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
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["one"] = "DELETE_ME_1";
	scenes["two"] = "DELETE_ME_2";
	settings.sceneIds = scenes;
	settings.DefaultSceneName = "one";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	int object1id = cog.dynamicobject->RegisterObject("object1", "lamp");
	int object2id = cog.dynamicobject->RegisterObject("object2", "lamp");

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

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
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["one"] = "DELETE_ME_1";
	scenes["two"] = "DELETE_ME_2";
	settings.sceneIds = scenes;
	settings.DefaultSceneName = "one";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	int object1id = cog.dynamicobject->RegisterObjectCustomId("object1", "lamp",1);
	int object2id = cog.dynamicobject->RegisterObjectCustomId("object2", "lamp",2);

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	EXPECT_EQ(2, cog.dynamicobject->manifestEntries.size());
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object2id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 3);

	cog.SendData();
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 0);
	cog.EndSession();
}

TEST(Dynamics, CustomIdMultiples) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["one"] = "DELETE_ME_1";
	settings.sceneIds = scenes;
	settings.DefaultSceneName = "one";
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.CustomerId = TESTINGCUSTOMER;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	int object1id = cog.dynamicobject->RegisterObjectCustomId("object1", "lamp", 1);
	int object2id = cog.dynamicobject->RegisterObjectCustomId("object2", "lamp", 1);
	int object3id = cog.dynamicobject->RegisterObjectCustomId("object3", "lamp", 1);
	
	EXPECT_EQ(cog.dynamicobject->objectIds.size(), 3);

	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 3);
	EXPECT_EQ(cog.dynamicobject->fullManifest.size(), 3);
}

TEST(Dynamics, LimitSnapshots) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["one"] = "DELETE_ME_1";
	settings.sceneIds = scenes;
	settings.DefaultSceneName = "one";
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.DynamicDataLimit = 5;
	settings.CustomerId = TESTINGCUSTOMER;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	int object1id = cog.dynamicobject->RegisterObjectCustomId("object1", "lamp", 1);

	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 1);

	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 2);
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 3);
	cog.dynamicobject->AddSnapshot(object1id, pos, rot); //manifest + snapshots = limit
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	EXPECT_EQ(cog.dynamicobject->snapshots.size(), 1);
}

TEST(Dynamics, LimitRegister) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["one"] = "DELETE_ME_1";
	settings.sceneIds = scenes;
	settings.DefaultSceneName = "one";
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.DynamicDataLimit = 5;
	settings.CustomerId = TESTINGCUSTOMER;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	cog.dynamicobject->RegisterObjectCustomId("object1", "lamp", 1);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 1);
	cog.dynamicobject->RegisterObjectCustomId("object2", "lamp", 2);
	cog.dynamicobject->RegisterObjectCustomId("object3", "lamp", 3);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 3);
	cog.dynamicobject->RegisterObjectCustomId("object4", "lamp", 4);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 4);
	cog.dynamicobject->RegisterObjectCustomId("object5", "lamp", 5);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 0);
}

TEST(Dynamics, LimitPreSession) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["one"] = "DELETE_ME_1";
	settings.sceneIds = scenes;
	settings.DefaultSceneName = "one";
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.DynamicDataLimit = 5;
	settings.CustomerId = TESTINGCUSTOMER;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);


	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	cog.dynamicobject->RegisterObjectCustomId("object1", "lamp", 1);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 1);
	cog.dynamicobject->RegisterObjectCustomId("object2", "lamp", 2);
	cog.dynamicobject->RegisterObjectCustomId("object3", "lamp", 3);
	cog.dynamicobject->RegisterObjectCustomId("object4", "lamp", 4);
	cog.dynamicobject->RegisterObjectCustomId("object5", "lamp", 5);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 5);
	cog.StartSession();
	cog.dynamicobject->RegisterObjectCustomId("object5", "lamp", 6);
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 0);
}

TEST(Dynamics, ReuseObjectId) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["one"] = "DELETE_ME_1";
	settings.sceneIds = scenes;
	settings.DefaultSceneName = "one";
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.CustomerId = TESTINGCUSTOMER;

	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);


	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };
	cog.StartSession();

	int object1id = cog.dynamicobject->RegisterObject("object1", "lamp");
	int object2id = cog.dynamicobject->RegisterObject("object2", "lamp");
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 2);

	cog.dynamicobject->RemoveObject(object1id, pos, rot);
	int object3id = cog.dynamicobject->RegisterObject("object3", "block");
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 3);
	int object4id = cog.dynamicobject->RegisterObject("object4", "lamp");
	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 3);
	cog.SendData();
}

TEST(Dynamics, EngagementBeforeRegister) {
	if (TestDelay > 0)
		std::this_thread::sleep_for(std::chrono::seconds(TestDelay));

	cognitive::CoreSettings settings;
	settings.webRequest = &DoWebStuff;
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["one"] = "DELETE_ME_1";
	scenes["two"] = "DELETE_ME_2";
	settings.sceneIds = scenes;
	settings.DefaultSceneName = "one";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(),0);

	cog.dynamicobject->BeginEngagement(1, "grab");
	cog.dynamicobject->AddSnapshot(1, pos, rot);
	cog.dynamicobject->EndEngagement(1, "grab");
	EXPECT_EQ(cog.dynamicobject->activeEngagements.size(), 1);

	EXPECT_EQ(cog.dynamicobject->manifestEntries.size(), 0);
	cog.dynamicobject->RegisterObjectCustomId("object1", "lamp",1);
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
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["one"] = "DELETE_ME_1";
	scenes["two"] = "DELETE_ME_2";
	settings.sceneIds = scenes;
	settings.DefaultSceneName = "one";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();



	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	cog.dynamicobject->BeginEngagement(1, "grab");
	cog.dynamicobject->AddSnapshot(1, pos, rot);
	cog.dynamicobject->EndEngagement(1, "grab");
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
	//settings.loggingLevel = cognitive::LoggingLevel::kAll;
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["one"] = "DELETE_ME_1";
	scenes["two"] = "DELETE_ME_2";
	settings.sceneIds = scenes;
	settings.DefaultSceneName = "one";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	int object1id = cog.dynamicobject->RegisterObject("object1", "lamp");
	int object2id = cog.dynamicobject->RegisterObject("object2", "lamp");

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	EXPECT_EQ(2, cog.dynamicobject->manifestEntries.size());

	cog.dynamicobject->BeginEngagement(object1id, "grab");
	cog.dynamicobject->AddSnapshot(object1id, pos, rot);
	cog.dynamicobject->EndEngagement(object1id, "grab");
	EXPECT_EQ(cog.dynamicobject->activeEngagements.size(),1);
	EXPECT_EQ(cog.dynamicobject->allEngagements.size(),1);

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
	settings.CustomerId = TESTINGCUSTOMER;

	std::map<std::string, std::string> scenes = std::map<std::string, std::string>();
	scenes["one"] = "DELETE_ME_1";
	settings.sceneIds = scenes;
	settings.DefaultSceneName = "one";
	auto cog = cognitive::CognitiveVRAnalyticsCore(settings);
	cog.StartSession();

	std::vector<float>pos = { 0,0,0 };
	std::vector<float>rot = { 0,0,0,1 };

	
	cog.dynamicobject->RegisterObjectCustomId("object1", "lamp", 1);
	
	cog.dynamicobject->BeginEngagement(1, "grab");
	cog.dynamicobject->AddSnapshot(1, pos, rot);
	cog.dynamicobject->RemoveObject(1, pos, rot);
	EXPECT_EQ(cog.dynamicobject->activeEngagements.size(), 1);


	cog.SendData();
	EXPECT_EQ(cog.dynamicobject->activeEngagements.size(), 1);
	EXPECT_EQ(cog.dynamicobject->allEngagements.size(), 1);
}


int main(int argc, char **argv)
{
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