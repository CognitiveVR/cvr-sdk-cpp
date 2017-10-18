
/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

#include "stdafx.h"
#include "network.h"
namespace cognitive {
Network::Network(::std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
    cvr = cog;
}

::std::string query = "";

//some random callback. use for debugging, otherwise nothing
void Callback(::std::string body)
{
	auto cvr = CognitiveVRAnalyticsCore::Instance();
	//check that body can be parsed to json
	if (body.empty())
	{
		cvr->log->Info("Generic callback has no body");
	}
	else
	{
		cvr->log->Info("Generic callback");
	}
}

//callback for application init. read out tuning variables
void InitCallback(::std::string body)
{
	auto cvr = CognitiveVRAnalyticsCore::Instance();

	//check that body can be parsed to json
	if (body.empty())
	{
		cvr->log->Info("Application Init response has no body");
		cvr->SetInitSuccessful(false);
		cvr->SetHasStartedSession(false);
	}
	else
	{
		//try parsing body response to json
		cvr->SetInitSuccessful(true);
		cvr->SetHasStartedSession(true);

		int errorcode = -1;

		auto jsonresponse = nlohmann::json::parse(body);

		errorcode = jsonresponse["error"].get<int>();

		if (errorcode == 0)
		{
			cvr->log->Info("Applicaiton Init callback successful");

			cvr->tuning->ReceiveValues(jsonresponse);
			nlohmann::json props;

			::std::vector<float> beginPos = { 0,0,0 };

			cvr->transaction->BeginPosition("cvr.session", beginPos, props);
		}
		else
		{
			cvr->log->Info("Init Error " + ::std::to_string(errorcode));
			cvr->SetInitSuccessful(false);
			cvr->SetHasStartedSession(false);
		}
	}
}

//exitpoll response to requesting a hook. json is question set
void ExitPollCallback(::std::string body)
{
	auto cvr = CognitiveVRAnalyticsCore::Instance();

	//check that body can be parsed to json
	if (body.empty())
	{
		cvr->log->Error("ExitPoll Callback Error: no response body");
		return;
	}

	nlohmann::json jsonresponse = nlohmann::json::parse(body);

	int errorcode = 0;
	if (jsonresponse.find("code") != jsonresponse.end())
	{
		errorcode = jsonresponse["code"].get<int>();
	}

	if (errorcode == 0)
	{
		cvr->log->Info("ExitPollCallback callback successful");

		cvr->exitpoll->ReceiveQuestionSet(body,nlohmann::json::parse(body));
	}
	else
	{
		::std::string message = jsonresponse["message"].get<::std::string>();

		cvr->log->Error("ExitPoll Callback Error: " + message);
	}
}

void Network::DashboardCall(::std::string suburl, ::std::string content)
{
	if (cvr->sendFunctionPointer == nullptr) { cvr->log->Warning("Network::DashboardCall cannot find webrequest pointer"); return; }
	cvr->log->Info("Network Dashboard call: " + suburl);

	//std::string path = "/" + cvr->config->kSsfApp + "/ws/interface/" + suburl;
	::std::string path = "/isos-personalization/ws/interface/" + suburl;

	//TEST cache this query string
	if (query.size() == 0)
	{
		query = "?";
		query.append("ssf_ws_version=");
		query.append(cvr->config->kSsfVersion);
		query.append("&ssf_cust_id=");
		query.append(cvr->GetCustomerId());
		query.append("&ssf_output=");
		query.append(cvr->config->kSsfOutput);
		query.append("&ssf_sdk=cpp");
		query.append("&ssf_sdk_version=");
		query.append(cvr->config->SdkVersion);
	}

	::std::string finalurl = cvr->config->kNetworkHost + path + query;

	WebResponse wr = nullptr;// = &Callback;

	if (suburl == "application_init")
	{
		wr = &InitCallback;
	}

	cvr->sendFunctionPointer(finalurl, content, wr);
}

void Network::APICall(::std::string suburl, ::std::string callType, ::std::string content)
{
	if (cvr->sendFunctionPointer == nullptr) { cvr->log->Warning("Network::APICall cannot find webrequest pointer"); return; }
	cvr->log->Info("API call: " + suburl);

	::std::string path = cvr->config->kNetworkHost + "/products/" + cvr->GetCustomerId() + "/" + suburl;

	WebResponse wr = nullptr;// &Callback;
	if (callType == "exitpollget")
	{
		wr = &ExitPollCallback;
	}

	cvr->sendFunctionPointer(path, content, wr);
}

bool Network::SceneExplorerCall(::std::string suburl, ::std::string content)
{
	if (cvr->sendFunctionPointer == nullptr) { cvr->log->Warning("Network::SceneExplorerCall cannot find webrequest pointer"); return false; }
	::std::string scenekey = cvr->GetSceneId();
	if (scenekey.empty())
	{
		cvr->log->Warning("SceneExplorer failed: SceneID not set");
		return false;
	}
	if (cvr->sendFunctionPointer == nullptr)
	{
		cvr->log->Warning("SceneExplorer failed: WebRequest not set");
		return false;
	}

	cvr->log->Info("SceneExplorer call: " + suburl);
	cvr->log->Info("SceneExplorer scenekey: " + scenekey);
	cvr->log->Info("SceneExplorer body " + content);

	::std::string finalurl = "https://sceneexplorer.com/api/" + suburl + "/" + scenekey;

	WebResponse wr = nullptr;// &Callback;
	cvr->sendFunctionPointer(finalurl, content, wr);
	return true;
}
}