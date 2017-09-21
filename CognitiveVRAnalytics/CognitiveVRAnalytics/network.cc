/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

#include "stdafx.h"
#include "network.h"

Network::Network(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
    cvr = cog;
}

//some random callback. use for debugging, otherwise nothing
void Callback(std::string body)
{
	auto cvr = CognitiveVRAnalyticsCore::Instance();
	//check that body can be parsed to json
	if (body.empty())
	{
		cvr->log->Info("generic callback has no body");
	}
	else
	{
		cvr->log->Info("generic callback");
	}
}

//callback for application init. read out tuning variables
void InitCallback(std::string body)
{
	auto cvr = CognitiveVRAnalyticsCore::Instance();

	//check that body can be parsed to json
	if (body.empty())
	{
		cvr->log->Info("init response has no body");
		cvr->SetInitSuccessful(false);
		cvr->SetHasStartedSession(false);
	}
	else
	{
		//try parsing body response to json
		cvr->SetInitSuccessful(true);
		cvr->SetHasStartedSession(true);

		int errorcode = -1;

		auto jsonresponse = json::parse(body);

		errorcode = jsonresponse["error"].get<int>();

		if (errorcode == 0)
		{
			cvr->log->Info("applicaiton init successful");
			
			cvr->tuning->ReceiveValues(jsonresponse);
			
		}
		else
		{
			cvr->log->Info("error " + std::to_string(errorcode));
		}
		
		//TODO begin session transaction

		//cvr->log->Info("user " + jsonresponse["data"]["userid"].get<std::string>());
	}
}

//exitpoll response to requesting a hook. json is question set
void ExitPollCallback(std::string body)
{
	auto cvr = CognitiveVRAnalyticsCore::Instance();

	cvr->log->Info("ExitPollCallback==================START");
	cvr->log->Info(body);
	cvr->log->Info("ExitPollCallback==================END");

	//check that body can be parsed to json
	if (body.empty())
	{
		return;
	}

	cvr->exitpoll->ReceiveQuestionSet(json::parse(body));

	//call cvr->exitpoll->GetQuestionSet to return json formatted to display question after this callback has been called
}

void Network::DashboardCall(std::string suburl, std::string content)
{
	std::string path = "/" + cvr->config->kSsfApp + "/ws/interface/" + suburl;

	//TODO cache this query string
	std::string query = "?";
	query.append("ssf_ws_version=");
	query.append(cvr->config->kSsfVersion);
	query.append("&ssf_cust_id=");
	query.append(cvr->GetCustomerId());
	query.append("&ssf_output=");
	query.append(cvr->config->kSsfOutput);
	query.append("&ssf_sdk=cpp");
	//query.append(COGNITIVEVR_SDK_NAME);
	query.append("&ssf_sdk_version=");
	query.append(cvr->config->SdkVersion);

	std::string finalurl = cvr->config->kNetworkHost + path + query;

	WebResponse wr = &Callback;

	cvr->sendFunctionPointer(finalurl, content, wr);
}

void Network::APICall(std::string suburl, std::string callType, std::string content)
{
	//TODO shoudl use api.networkhost from config
	std::string path = "https://api.cognitivevr.io/products/" + cvr->GetCustomerId() + "/" + suburl;

	WebResponse wr = &Callback;
	if (callType == "exitpollget") //does exitpoll call this query? does it call sceneexplorer?
	{
		wr = &ExitPollCallback;
		cvr->log->Info("get exitpoll callback");
	}

	cvr->sendFunctionPointer(path, content, wr);
}

void Network::SceneExplorerCall(std::string suburl, std::string content)
{
	std::string scenekey = cvr->GetSceneKey();
	if (scenekey.empty())
	{
		cvr->log->Warning("scene key not set. cannot upload to scene explorer");
		return;
	}

	std::string finalurl = "https://sceneexplorer.com/api/" + suburl + "/" + scenekey;

	WebResponse wr = &Callback;
	cvr->sendFunctionPointer(finalurl, content, wr);
}