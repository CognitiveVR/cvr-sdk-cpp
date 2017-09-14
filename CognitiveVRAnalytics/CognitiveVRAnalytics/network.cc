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
	//check that body can be parsed to json
	if (body.empty())
	{
		auto cvr = CognitiveVRAnalyticsCore::Instance();
		
		cvr->log->Info("generic callback has no body");
	}
}

//callback for application init. read out tuning variables
void InitCallback(std::string body)
{
	auto cvr = CognitiveVRAnalyticsCore::Instance();

	cvr->log->Info("ANY RESPONSE");

	//check that body can be parsed to json
	if (body.empty())
	{
		cvr->log->Info("init response has no body");
		cvr->SetInitSuccessful(false);
		cvr->SetHasStartedSession(false);
	}
	else
	{
		cvr->log->Info("NEW SPECIAL AND AMAZING RESPONSE\n"+body);
		//try parsing body response to json
		cvr->SetInitSuccessful(true);
		cvr->SetHasStartedSession(true);

		int errorcode = -1;

		

		auto jsonresponse = json::parse(body);

		errorcode = jsonresponse["error"].get<int>();

		if (errorcode == 0)
		{
			cvr->log->Info("error 0"); //tostring
			
			cvr->tuning->ReceiveValues(jsonresponse);
			
		}
		else
		{
			cvr->log->Info("error " + std::to_string(errorcode));
		}
		
		//cvr->log->Info("user " + jsonresponse["data"]["userid"].get<std::string>());
	}
}

//exitpoll response to requesting a hook. json is question set
void ExitPollCallback(std::string body)
{
	//check that body can be parsed to json
	if (body.empty())
	{
		auto cvr = CognitiveVRAnalyticsCore::Instance();

		cvr->log->Info("stuff");
		return;
	}

	//call cvr->exitpoll->GetQuestionSet to return json formatted to display question after this callback has been called
}

void Network::DashboardCall(std::string suburl, std::string content)
{
	//cvr->WebRequestDelegate();
	std::string path = "/" + cvr->config->kSsfApp + "/ws/interface/" + suburl;

	//cache this query string?
	//Build query string.
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
	if (suburl == "application_init")
	{
		wr = &InitCallback;
		cvr->log->Info("set init callback");
	}
	else if (suburl == "exitpoll") //does exitpoll call this query? does it call sceneexplorer?
	{
		wr = &ExitPollCallback;
		cvr->log->Info("set exitpoll callback");
	}
	else
	{
		cvr->log->Info("generic callback");
	}

	cvr->sendFunctionPointer(finalurl, content, wr);
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

/*Network::~Network()
{

}*/

/*void Network::Init(HttpInterface* a, NetworkCallback callback)
{
    this->httpint = a;
	CognitiveLog::Info("CognitiveVR::Network - Init");

	//applicaiton init

	TArray< std::shared_ptr<FJsonValue> > ObjArray;
	std::shared_ptr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));
	//std::shared_ptr<json> json = MakeShareable(new json);

	std::string fs = std::string::SanitizeFloat(Util::GetTimestamp());
	std::string empty = "";

	//std::string user = s->GetUserID();
	//std::string device = s->GetDeviceID();

	if (s == NULL)
	{
		CognitiveLog::Warning("CognitiveVR::Network Init cannot find provider");
		return;
	}

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	s->AppendUD(jsonArray);
	Util::AppendToJsonArray(jsonArray, empty);

	if (s->initProperties.Get() == NULL)
	{
		CognitiveLog::Warning("CognitiveVR::Network Init initproperties is null");
		return;
	}
	std::shared_ptr<json>deviceProperties = Util::DeviceScraper(s->initProperties);

	Util::AppendToJsonArray(jsonArray, deviceProperties);

	TArray<std::shared_ptr<FJsonValue>> arr = jsonArray.Get()->AsArray();

	Network::DirectCall("application_init", arr, callback);
}*/

/*void Network::Call(std::string sub_path, std::shared_ptr<FJsonValueArray> content, NetworkCallback callback)
{
	const TArray<std::shared_ptr<FJsonValue>> arr = content.Get()->AsArray();

	Network::Call(sub_path, arr, callback);
}

//'application_init' and json'd sendtimestamp, eventtimestamp, userid, deviceid, userprops, deviceprops
void Network::Call(std::string sub_path, TArray<std::shared_ptr<FJsonValue>> content, NetworkCallback callback)
{
    if(!this->httpint)
	{
		CognitiveLog::Warning("Network::Call - No HTTP implementation available. Did you call cognitivevr::Init()?");
    }

	std::string ValueReceived;

	bool moduleIsAvailable = FAnalytics::IsAvailable();
	if (!moduleIsAvailable)
	{
		CognitiveLog::Warning("Network::Call - analyticsModule is not available");
		return;
	}

	ValueReceived = s->CustomerId;// FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "CognitiveVRApiKey", false);

	std::string customerid(TCHAR_TO_UTF8(*ValueReceived));

	std::string path = "/" + Config::kSsfApp + "/ws/interface/datacollector_batch";// +sub_path;

    //Build query string.
    std::string query = "?";
    query.append("&ssf_output=");
    query.append(Config::kSsfOutput);
    query.append("&ssf_cust_id=");
	query.append(customerid);
    query.append("&ssf_ws_version=");
    query.append(Config::kSsfVersion);
    query.append("&ssf_sdk=");
    query.append(COGNITIVEVR_SDK_NAME);
    query.append("&ssf_sdk_version=");
    query.append(COGNITIVEVR_SDK_VERSION);
    query.append("&ssf_sdk_contextname=");
    query.append("defaultContext"); //context is rarely/never used?

    std::string headers[] = {
        "ssf-use-positional-post-params: true",
        "ssf-contents-not-url-encoded: true"
    };

    std::string str_response = "";

	std::string OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(content, Writer);

	OutputString = "[" + std::string::SanitizeFloat(Util::GetTimestamp()) + "," + OutputString + "]";

	str_response = this->httpint->Post(Config::kNetworkHost, path + query, headers, 2, TCHAR_TO_UTF8(*OutputString), Config::kNetworkTimeout, callback);
}

//'application_init' and json'd sendtimestamp, eventtimestamp, userid, deviceid, userprops, deviceprops
void Network::DirectCall(std::string sub_path, TArray<std::shared_ptr<FJsonValue>> content, NetworkCallback callback)
{
	if (!this->httpint)
	{
		CognitiveLog::Warning("Network::Call - No HTTP implementation available. Did you call cognitivevr::Init()?");
	}

	std::string ValueReceived;

	bool moduleIsAvailable = FAnalytics::IsAvailable();
	if (!moduleIsAvailable)
	{
		CognitiveLog::Warning("Network::Call - analyticsModule is not available");
		return;
	}

	ValueReceived = s->CustomerId;// = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "CognitiveVRApiKey", false);

	std::string customerid(TCHAR_TO_UTF8(*ValueReceived));

	std::string path = "/" + Config::kSsfApp + "/ws/interface/"+sub_path;

																				   //Build query string.
	std::string query = "?";
	query.append("&ssf_output=");
	query.append(Config::kSsfOutput);
	query.append("&ssf_cust_id=");
	query.append(customerid);
	query.append("&ssf_ws_version=");
	query.append(Config::kSsfVersion);
	query.append("&ssf_sdk=");
	query.append(COGNITIVEVR_SDK_NAME);
	query.append("&ssf_sdk_version=");
	query.append(COGNITIVEVR_SDK_VERSION);
	//query.append("&ssf_sdk_contextname=");
	//query.append("defaultContext"); //context is rarely/never used?

	std::string headers[] = {
		"ssf-use-positional-post-params: true",
		"ssf-contents-not-url-encoded: true"
	};

	std::string str_response = "";

	std::string OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(content, Writer);

	//OutputString = "[" + std::string::SanitizeFloat(Util::GetTimestamp()) + "," + OutputString + "]";

	str_response = this->httpint->Post(Config::kNetworkHost, path + query, headers, 2, TCHAR_TO_UTF8(*OutputString), Config::kNetworkTimeout, callback);
}
*/

/*
CognitiveVRResponse Network::ParseResponse(std::string str_response)
{
	std::shared_ptr<json> root;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(str_response.c_str());

	if (FJsonSerializer::Deserialize(reader,root))
	{
		//TODO where is this used? what data does it expect with "error" and "data" json fields?
		int error_code = root.Get()->GetIntegerField("error");
		bool success = (error_code == kErrorSuccess);

		CognitiveVRResponse response(success);

		if (!success) {
			std::string err = Network::InterpretError(error_code);
			CognitiveLog::Error(err);
			response.SetErrorMessage(err);
			CognitiveLog::Warning("Network::ParseResponse - Failed");
		}
		else {
			if (root.Get()->GetObjectField("data").IsValid())
			{
				/*
				{
					"error":0,
					"description" : "",
					"data" :
					{
						"userid":"test username",
						"usertuning" :
						{
							"Hungry":"true",
							"FavouriteFood" : "Burritos",
						},
							"usernew" : false,
							"deviceid" : "test device id",
							"devicetuning" :
						{
							"Hungry":"true",
							"FavouriteFood" : "Burritos",
						},
						"devicenew" : false
					}
				}
				*//*

				response.SetContent(*root.Get()->GetObjectField("data").Get());
			}
			else
			{
				response.SetContent(*root.Get());
			}

			/*
			if (root["data"].isNull()) {
				response.SetContent(root);
			}
			else {
				response.SetContent(root["data"]);
			}*//*
		}

		return response;
	}
	else
	{
		CognitiveVRResponse response(false);
		response.SetErrorMessage("Failed to parse JSON response.");
		CognitiveLog::Error("Network::ParseResponse - Failed to parse JSON response.");
		return response;
	}

	CognitiveVRResponse resp(false);
	resp.SetErrorMessage("Unknown error.");

	return resp;
}

std::string Network::InterpretError(int code)
{
    switch (code) {
        case kErrorSuccess:
            return "Success.";
        case kErrorGeneric:
            return "Generic error.";
        case kErrorNotInitialized:
            return "CognitiveVR not initialized.";
        case kErrorNotFound:
            return "Path not found.";
        case kErrorInvalidArgs:
            return "Invalid arguments or invalid JSON format.";
        case kErrorMissingId:
            return "Missing ID.";
        case kErrorRequestTimedOut:
            return "Request timed out.";
    }

    return "Unknown error.";
}*/