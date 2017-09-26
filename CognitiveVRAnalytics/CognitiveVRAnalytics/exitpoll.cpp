// Fill out your copyright notice in the Description page of Project Settings.

#include "stdafx.h"
#include "exitpoll.h"

ExitPoll::ExitPoll(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
}

void ExitPoll::RequestQuestionSet(std::string Hook)
{
	if (!cvr->WasInitSuccessful()) { cvr->log->Info("ExitPoll::RequestQuestionSet failed: init not successful"); return; }

	if (!cvr->HasStartedSession())
	{
		cvr->log->Warning("ExitPoll::RequestQuestionSet failed: Session not started!");
		return;
	}

	cvr->network->APICall("questionSetHooks/" + Hook + "/questionSet","exitpollget");

	fullResponse.user = cvr->UserId;
	fullResponse.sessionId = cvr->GetSessionID();
	fullResponse.hook = Hook;
}

std::vector<std::string> split(const std::string &text, char sep) {
	std::vector<std::string> tokens;
	std::size_t start = 0, end = 0;
	while ((end = text.find(sep, start)) != std::string::npos) {
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));
	return tokens;
}

void ExitPoll::ReceiveQuestionSet(json questionset)
{
	fullResponse.questionSetId = questionset["id"].get<std::string>();
	auto splitquestionid = split(fullResponse.questionSetId, ':');
	fullResponse.questionSetName = splitquestionid[0];
	fullResponse.questionSetVersion = splitquestionid[1];
	currentQuestionSet = questionset;
}

json ExitPoll::GetQuestionSet()
{
	if (currentQuestionSet.size() == 0)
	{
		cvr->log->Warning("ExitPoll:GetQuestionSet no active question set. Returning empty json");
	}

	return currentQuestionSet;
}

void ExitPoll::ClearQuestionSet()
{
	currentQuestionSet.clear();

	fullResponse.answers.clear();
	fullResponse.hook.clear();
	fullResponse.questionSetId.clear();
	fullResponse.sessionId.clear();
	fullResponse.user.clear();
}

void ExitPoll::AddAnswer(FExitPollAnswer answer)
{
	fullResponse.answers.push_back(answer);
}

void ExitPoll::SendAllAnswers()
{
	//companyname1234-productname-test/questionSets/:questionset_name/:version#/responses

	json full = fullResponse.to_json();

	//cvr->log->Info(full.dump());

	cvr->network->APICall(cvr->GetCustomerId() + "/questionSets/" + fullResponse.questionSetName + "/" + fullResponse.questionSetVersion + "/responses", "answerSend", full.dump());

	//send this as a transaction too
	std::vector<float> pos = { 0,0,0 };
	json properties = json();
	properties["userId"] = cvr->UserId;
	properties["questionSetId"] = fullResponse.questionSetId;
	properties["hook"] = fullResponse.hook;

	//add answers as properties
	for (int i = 0; i < fullResponse.answers.size(); ++i)
	{
		if (fullResponse.answers[i].AnswerValueType == EAnswerValueTypeReturn::String)
		{
			//strings are only for voice responses. these do not show up in dash
			properties["Answer" + std::to_string(i)] = 0;
		}
		else //bool(0-1), null(-32768),number(0-10)
		{
			properties["Answer" + std::to_string(i)] = fullResponse.answers[i].numberValue;
		}
	}

	cvr->transaction->BeginEndPosition("cvr.exitpoll", pos, properties);


	ClearQuestionSet();
}

void ExitPoll::SendAllAnswers(std::vector<float> pos)
{
	//companyname1234-productname-test/questionSets/:questionset_name/:version#/responses

	json full = fullResponse.to_json();

	//cvr->log->Info(full.dump());

	cvr->network->APICall(cvr->GetCustomerId() + "/questionSets/" + fullResponse.questionSetName + "/" + fullResponse.questionSetVersion + "/responses", "answerSend", full.dump());

	//send this as a transaction too
	json properties = json();
	properties["userId"] = cvr->UserId;
	properties["questionSetId"] = fullResponse.questionSetId;
	properties["hook"] = fullResponse.hook;

	//add answers as properties
	for (int i = 0; i < fullResponse.answers.size(); ++i)
	{
		if (fullResponse.answers[i].AnswerValueType == EAnswerValueTypeReturn::String)
		{
			//strings are only for voice responses. these do not show up in dash
			properties["Answer" + std::to_string(i)] = 0;
		}
		else //bool(0-1), null(-32768),number(0-10)
		{
			properties["Answer" + std::to_string(i)] = fullResponse.answers[i].numberValue;
		}
	}

	cvr->transaction->BeginEndPosition("cvr.exitpoll", pos, properties);

	ClearQuestionSet();
}

/*
void ExitPoll::OnResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!Response.IsValid())
	{
		CognitiveLog::Error("ExitPoll::OnResponseReceivedAsync - No valid Response. Check internet connection");

		if (r.IsBound())
		{
			r.Execute(FExitPollQuestionSet());
		}
		return;
	}

	currentSet = FExitPollQuestionSet();

	std::string UE4Str = Response->GetContentAsString();
	std::string content(TCHAR_TO_UTF8(*UE4Str));
	CognitiveLog::Info("ExitPoll::OnResponseReceivedAsync - Response: " + content);

	//CognitiveVRResponse response = Network::ParseResponse(content);

	FExitPollQuestionSet set = FExitPollQuestionSet();
	std::shared_ptr<json> jobject;
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(UE4Str);
	if (FJsonSerializer::Deserialize(Reader, jobject))
	{
		if (!jobject->HasField(TEXT("customerId")))
		{
			CognitiveLog::Info("ExitPoll::OnResponseReceivedAsync - no customerId in response - fail");
			if (r.IsBound())
			{
				r.Execute(FExitPollQuestionSet());
			}
			return;
		}

		set.customerId = jobject->GetStringField(TEXT("customerId"));
		set.id = jobject->GetStringField(TEXT("id"));
		set.name = jobject->GetStringField(TEXT("name"));
		set.version = jobject->GetNumberField(TEXT("version"));
		set.title = jobject->GetStringField(TEXT("title"));
		set.status = jobject->GetStringField(TEXT("status"));

		TArray<std::shared_ptr<FJsonValue>> questions = jobject->GetArrayField(TEXT("questions"));
		for (int i = 0; i < questions.Num(); i++)
		{
			//replace with trygets
			auto qobject = questions[i]->AsObject();
			FExitPollQuestion q = FExitPollQuestion();
			q.title = qobject->GetStringField(TEXT("title"));
			q.type = qobject->GetStringField(TEXT("type"));

			//voice
			int maxResponseLength;
			if (qobject->TryGetNumberField(TEXT("maxResponseLength"), maxResponseLength))
			{
				q.maxResponseLength = maxResponseLength;
			}

			//scale
			std::string minLabel;
			if (qobject->TryGetStringField(TEXT("minLabel"), minLabel))
			{
				q.minLabel = minLabel;
			}
			std::string maxLabel;
			if (qobject->TryGetStringField(TEXT("maxLabel"), maxLabel))
			{
				q.maxLabel = maxLabel;
			}
			const std::shared_ptr<json>* range;
			if (qobject->TryGetObjectField(TEXT("range"), range))
			{
				int start = 0;
				if (range->Get()->TryGetNumberField(TEXT("start"), start))
				{
					
				}
				int end = 10;
				if (range->Get()->TryGetNumberField(TEXT("end"), end))
				{
					
				}
				q.range = FExitPollScaleRange();
				q.range.start = start;
				q.range.end = end;
			}

			
			//multiple choice
			const TArray<std::shared_ptr<FJsonValue>>* answers;
			if (qobject->TryGetArrayField(TEXT("answers"), answers))
			{
				for (int j = 0; j < answers->Num(); j++)
				{
					FExitPollMultipleChoice choice = FExitPollMultipleChoice();
					choice.answer = (*answers)[j]->AsObject()->GetStringField(TEXT("answer"));
					//choice.icon = (*answers)[j]->AsObject()->GetBoolField(TEXT("icon"));
					q.answers.Add(choice);
				}
			}

			set.questions.Add(q);
		}

		currentSet = set;
		if (r.IsBound())
		{
			r.Execute(currentSet);
		}
	}
	else
	{
		r.Execute(FExitPollQuestionSet());
	}
}
*/



/*void ExitPoll::SendAllAnswers()
{
	//turn fullresponse into json
	//dump json into string
	//send to api somewhere

	//TODO send this as a transaction too?

	ClearQuestionSet();

	if (cogProvider.Get() == NULL)
	{
		cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	}

	std::shared_ptr<json> ResponseObject = MakeShareable(new json);

	//ResponseObject->SetStringField("user", s->GetUserID());
	//ResponseObject->SetStringField("questionSetId", currentSet.id);
	//ResponseObject->SetStringField("sessionId", s->GetSessionID());
	//ResponseObject->SetStringField("hook", lastHook);

	ResponseObject->SetStringField("userId", Responses.user);
	ResponseObject->SetStringField("questionSetId", Responses.questionSetId);
	ResponseObject->SetStringField("sessionId", Responses.sessionId);
	ResponseObject->SetStringField("hook", Responses.hook);

	TArray<std::shared_ptr<FJsonValue>> answerValues;

	for (int i = 0; i < Responses.answers.Num(); i++)
	{
		std::shared_ptr<json> answerObject = MakeShareable(new json);
		answerObject->SetStringField("type",Responses.answers[i].type);
		if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::String)
		{
			answerObject->SetStringField("value", Responses.answers[i].stringValue);
		}
		else if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Number)
		{
			answerObject->SetNumberField("value", Responses.answers[i].numberValue);
		}
		else if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Bool)
		{
			if (Responses.answers[i].boolValue == true)
			{
				answerObject->SetNumberField("value", 1);
			}
			else
			{
				answerObject->SetNumberField("value", 0);
			}
		}
		else if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Null)
		{
			answerObject->SetNumberField("value", -32768);
		}
		std::shared_ptr<FJsonValueObject> ao = MakeShareable(new FJsonValueObject(answerObject));
		answerValues.Add(ao);
	}
	ResponseObject->SetArrayField("answers", answerValues);

	std::string OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(ResponseObject.ToSharedRef(), Writer);
	
	//serialize and send this json
	
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	std::string ValueReceived = cogProvider->CustomerId;// = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "CognitiveVRApiKey", false);
	std::string url = "https://api.cognitivevr.io/products/" + ValueReceived + "/questionSets/" + currentSet.name + "/" + std::string::FromInt(currentSet.version) + "/responses";

	HttpRequest->SetURL(url);
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetVerb("POST");
	HttpRequest->SetContentAsString(OutputString);
	//HttpRequest->OnProcessRequestComplete().BindStatic(ExitPoll::OnQuestionResponse);
	HttpRequest->ProcessRequest();

	//send this as a transaction too

	std::shared_ptr<json> properties = MakeShareable(new json);
	//properties->SetStringField("userId", s->GetUserID());
	//properties->SetStringField("questionSetId", currentSet.id);
	//properties->SetStringField("hook", lastHook);

	properties->SetStringField("userId", Responses.user);
	properties->SetStringField("questionSetId", Responses.questionSetId);
	properties->SetStringField("hook", Responses.hook);

	for (int i = 0; i < Responses.answers.Num(); i++)
	{
		if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Number)
		{
			properties->SetNumberField("Answer" + std::string::FromInt(i), Responses.answers[i].numberValue);
		}
		else if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Bool) //bool as number
		{
			if (Responses.answers[i].boolValue == true)
			{
				properties->SetNumberField("Answer" + std::string::FromInt(i), 1);
			}
			else
			{
				properties->SetNumberField("Answer" + std::string::FromInt(i), 0);
			}
		}
		else if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Null)
		{
			//skipped answer
			properties->SetNumberField("Answer" + std::string::FromInt(i), -32768);
		}
		else if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::String)
		{
			//voice answer. don't display on dashboard, but not skipped
			properties->SetNumberField("Answer" + std::string::FromInt(i), 0);
		}
	}

	if (!cogProvider.IsValid() || !bHasSessionStarted)
	{
		CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	
	cogProvider.Get()->transaction->BeginEnd("cvr.exitpoll", properties);

	//then flush transactions
	cogProvider.Get()->FlushEvents();
}*/