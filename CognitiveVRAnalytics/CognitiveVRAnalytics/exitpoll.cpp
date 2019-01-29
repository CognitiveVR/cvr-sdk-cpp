/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

#include "stdafx.h"
#include "exitpoll.h"
namespace cognitive {
ExitPoll::ExitPoll(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
}

void ExitPoll::RequestQuestionSet(std::string Hook)
{
	//cannot request question set without successful init first!
	if (!cvr->IsSessionActive()) { cvr->GetLog()->Info("ExitPoll::RequestQuestionSet failed: no session active"); return; }

	cvr->GetNetwork()->NetworkExitpollGet(Hook);

	fullResponse.user = cvr->GetUniqueID();
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

void ExitPoll::ReceiveQuestionSet(std::string questionsetstring, nlohmann::json questionset)
{
	currentQuestionSetString = questionsetstring;

	fullResponse.questionSetId = questionset["id"].get<std::string>();
	auto splitquestionid = split(fullResponse.questionSetId, ':');
	fullResponse.questionSetName = splitquestionid[0];
	fullResponse.questionSetVersion = splitquestionid[1];
	currentQuestionSet = questionset;
}

std::string ExitPoll::GetQuestionSetString()
{
	if (currentQuestionSetString.size() == 0)
	{
		cvr->GetLog()->Warning("ExitPoll:currentQuestionSetString no active question set. Returning empty json string");
	}

	return currentQuestionSetString;
}

nlohmann::json ExitPoll::GetQuestionSet()
{
	if (currentQuestionSet.size() == 0)
	{
		cvr->GetLog()->Warning("ExitPoll:GetQuestionSet no active question set. Returning empty json");
	}

	return currentQuestionSet;
}

void ExitPoll::ClearQuestionSet()
{
	currentQuestionSetString.clear();
	currentQuestionSet.clear();

	fullResponse.answers.clear();
	fullResponse.hook.clear();
	fullResponse.questionSetId.clear();
	fullResponse.sessionId.clear();
	fullResponse.user.clear();
}

void ExitPoll::AddAnswer(ExitPollAnswer answer)
{
	fullResponse.answers.push_back(answer);
}

nlohmann::json ExitPoll::SendAllAnswers()
{
	std::vector<float> pos = { 0,0,0 };
	return SendAllAnswers(pos);
}

nlohmann::json ExitPoll::SendAllAnswers(std::vector<float> pos)
{
	if (!cvr->IsSessionActive()) { cvr->GetLog()->Info("ExitPoll::SendAllAnswers failed: no session active"); return nlohmann::json(); }

	if (fullResponse.answers.size() == 0) { cvr->GetLog()->Info("ExitPoll::SendAllAnswers failed: oustanding answers"); return nlohmann::json(); }

	nlohmann::json response = nlohmann::json();

	response["userId"] = cvr->GetUniqueID();
	response["questionSetId"] = fullResponse.questionSetId;
	response["hook"] = fullResponse.hook;
	response["sceneId"] = cvr->GetCurrentSceneId();
	response["versionNumber"] = cvr->GetCurrentSceneVersionNumber();
	response["versionId"] = cvr->GetCurrentSceneVersionId();

	for (auto& an : fullResponse.answers)
	{
		nlohmann::json tempAnswer = nlohmann::json();
		if (an.AnswerValueType == EAnswerValueTypeReturn::kBool)
		{
			tempAnswer = nlohmann::json{ { "type", an.type },{ "value", an.boolValue ? 1 : 0 } };
		}
		else if (an.AnswerValueType == EAnswerValueTypeReturn::kNumber)
		{
			tempAnswer = nlohmann::json{ { "type", an.type },{ "value", an.numberValue } };
		}
		else if (an.AnswerValueType == EAnswerValueTypeReturn::kNull)
		{
			tempAnswer = nlohmann::json{ { "type", an.type },{ "value", -32768 } };
		}
		else if (an.AnswerValueType == EAnswerValueTypeReturn::kString)
		{
			tempAnswer = nlohmann::json{ { "type", an.type },{ "value", an.stringValue } };
		}

		response["answers"].push_back(tempAnswer);
	}

	cvr->GetNetwork()->NetworkExitpollPost(fullResponse.questionSetName, fullResponse.questionSetVersion, response.dump());






	//send this as a transaction too
	nlohmann::json properties = nlohmann::json();
	properties["userId"] = cvr->GetUniqueID();
	if (!cvr->GetLobbyId().empty())
		properties["lobbyId"] = cvr->GetLobbyId();
	properties["questionSetId"] = fullResponse.questionSetId;
	properties["hook"] = fullResponse.hook;
	properties["sceneId"] = cvr->GetCurrentSceneId();
	properties["versionNumber"] = cvr->GetCurrentSceneVersionNumber();
	properties["versionId"] = cvr->GetCurrentSceneVersionId();

	//add answers as properties
	for (int i = 0; i < fullResponse.answers.size(); ++i)
	{
		if (fullResponse.answers[i].AnswerValueType == EAnswerValueTypeReturn::kString)
		{
			//strings are only for voice responses. these do not show up in dash
			properties["Answer" + std::to_string(i)] = 0;
		}
		else //bool(0-1), null(-32768),number(0-10)
		{
			properties["Answer" + std::to_string(i)] = fullResponse.answers[i].numberValue;
		}
	}

	cvr->GetCustomEvent()->RecordEvent("c3d.exitpoll", pos, properties);

	ClearQuestionSet();

	return response;
}
}