/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

#include "stdafx.h"
#include "network.h"
namespace cognitive {
Network::Network(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
    cvr = cog;
	headers.push_back("Content-Type:application/json");
	headers.push_back("Authorization:APIKEY:DATA "+ cvr->config->APIKey);
}

//a default callback. use for debugging, otherwise nothing
void Callback(std::string body)
{
	auto cvr = CognitiveVRAnalyticsCore::Instance();
	//check that body can be parsed to json
	if (body.empty())
	{
		//cvr->log->Info("Generic callback has no body");
	}
	else
	{
		//cvr->log->Info("Generic callback");
	}
}

//exitpoll response to requesting a hook. json is question set
void ExitPollCallback(std::string body)
{
	auto cvr = CognitiveVRAnalyticsCore::Instance();

	//check that body can be parsed to json
	if (body.empty())
	{
		//cvr->log->Error("ExitPoll Callback Error: no response body");
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
		//cvr->log->Info("ExitPollCallback callback successful");

		cvr->GetExitPoll()->ReceiveQuestionSet(body, jsonresponse);
	}
	else
	{
		std::string message = jsonresponse["message"].get<std::string>();

		//cvr->log->Error("ExitPoll Callback Error: " + message);
	}
}

void Network::NetworkCall(std::string suburl, std::string content)
{
	if (cvr->sendFunctionPointer == nullptr) { cvr->log->Warning("Network::NetworkCall cannot find webrequest pointer"); return; }
	if (cvr->CurrentSceneId == "") { cvr->log->Warning("Network::NetworkCall does not have valid scenename"); return; }
	if (cvr->CurrentSceneVersionNumber == "") { cvr->log->Warning("Network::NetworkCall does not have valid scene version number"); return; }

	std::string path;

	if (cvr->CurrentSceneVersionNumber.empty())
		path = "https://" + cvr->config->kNetworkHost + "/v" + cvr->config->networkVersion + "/" + suburl + "/" + cvr->CurrentSceneId;
	else
		path = "https://" + cvr->config->kNetworkHost + "/v" + cvr->config->networkVersion + "/" + suburl + "/" + cvr->CurrentSceneId + "?version=" + cvr->CurrentSceneVersionNumber;
	

	cvr->GetLog()->Info("API call: " + path);

	cvr->sendFunctionPointer(path, content, headers, nullptr);
}

void Network::NetworkExitpollGet(std::string hook)
{
	if (cvr->sendFunctionPointer == nullptr) { cvr->log->Warning("Network::NetworkExitpollGet cannot find webrequest pointer"); return; }

	std::string path = "https://" + cvr->config->kNetworkHost + "/v" + cvr->config->networkVersion + "/questionSetHooks/" + hook + "/questionSet";

	cvr->GetLog()->Info("Network::NetworkExitpollGet: " + path);

	cvr->sendFunctionPointer(path, "", headers, &ExitPollCallback);
}

void Network::NetworkExitpollPost(std::string questionsetname, std::string questionsetversion, std::string content)
{
	if (cvr->sendFunctionPointer == nullptr) { cvr->log->Warning("Network::NetworkExitpollPost cannot find webrequest pointer"); return; }

	std::string path = "https://" + cvr->config->kNetworkHost + "/v" + cvr->config->networkVersion + "/questionSets/" + questionsetname + "/" +questionsetversion + "/responses";

	cvr->sendFunctionPointer(path, content, headers, nullptr);
}
}