// ClientProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"
#include "include\curl\curl.h"

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
	std::cout << url + "\n";
	std::cout << content + "\n";

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
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str());

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

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
}

int main()
{
	WebRequest fp = &DoWebStuff;

	auto cog = CognitiveVRAnalyticsCore(fp);

	nlohmann::json user = nlohmann::json();
	user["age"] = 21;
	user["location"] = "vancouver";

	cog.SetUser("john", &user);


	nlohmann::json device = nlohmann::json();
	device["os"] = "chrome";
	device["retail value"] = 79.95;
	device["ram"] = 4;

	cog.SetDevice("chromebook", &device);

	cog.StartSession();

	std::vector<float> pos = { 0,0,0 };
	cog.transaction->BeginEndPosition("testing1", pos, nullptr);
	cog.transaction->BeginEndPosition("testing2", pos, nullptr);
	cog.transaction->BeginEndPosition("testing3", pos, nullptr);

	/*cog.sensor->RecordSensor("comfort", 9);
	cog.sensor->RecordSensor("fps", 60);
	cog.sensor->RecordSensor("fps", 55);
	cog.sensor->RecordSensor("comfort", 8);
	cog.sensor->RecordSensor("fps", 30);
	cog.sensor->RecordSensor("fps", 25);
	cog.sensor->RecordSensor("comfort", 5);*/

	cog.SendData();

    return 0;
}