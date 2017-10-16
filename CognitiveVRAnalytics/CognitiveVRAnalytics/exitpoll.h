
// Fill out your copyright notice in the Description page of Project Settings.

//organizes all the question sets, responses and exit poll panel actors

#pragma once

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"


#if defined(_MSC_VER)
//  Microsoft 
#ifdef COGNITIVEVRANALYTICS_EXPORTS  
#define COGNITIVEVRANALYTICS_API __declspec(dllexport)
#else  
#define COGNITIVEVRANALYTICS_API __declspec(dllimport)
#endif
#elif defined(__GNUC__)
//  GCC
#ifdef COGNITIVEVRANALYTICS_EXPORTS  
#define COGNITIVEVRANALYTICS_API __attribute__((visibility("default")))
#else  
#define COGNITIVEVRANALYTICS_API
#endif
#else
//  do nothing and hope for the best?
#define COGNITIVEVRANALYTICS_EXPORTS
#pragma warning Unknown dynamic link import/export semantics.
#endif

namespace cognitive {
	//using json = nlohmann::json;
class CognitiveVRAnalyticsCore;

enum EAnswerValueTypeReturn
{
	Number,
	Bool,
	String, //used for voice
	Null
};

enum EQuestionType
{
	HAPPYSAD,
	SCALE,
	MULTIPLE,
	VOICE,
	THUMBS,
	QBOOL //BOOLEAN defined elsewhere with typedef
};

struct FExitPollAnswer
{
public:
		//question type. TODO make this an enum. HAPPYSAD,SCALE,MULTIPLE,VOICE,THUMBS,BOOLEAN
		::std::string type = "";
		EAnswerValueTypeReturn AnswerValueType = EAnswerValueTypeReturn::Null;
		int numberValue = -1;
		bool boolValue = false; //converted to 0 or 1
		::std::string stringValue = "";; //for base64 voice

		//TODO some nice converting function to json
		/*void to_json(json& j, const FExitPollAnswer& a) {
			if (AnswerValueType == EAnswerValueTypeReturn::Bool)
			{
				j = json{ { "name", a.type },{ "value", a.boolValue?1:0 }};
			}
			else if (AnswerValueType == EAnswerValueTypeReturn::Number)
			{
				j = json{ { "name", a.type },{ "value", a.numberValue } };
			}
			else if (AnswerValueType == EAnswerValueTypeReturn::Null)
			{
				j = json{ { "name", a.type },{ "value", -32768 } };
			}
			else if (AnswerValueType == EAnswerValueTypeReturn::String)
			{
				j = json{ { "name", a.type },{ "value", a.stringValue } };
			}
		}*/

		FExitPollAnswer(EQuestionType questionType, int number)
		{
			type = GetQuestionTypeString(questionType);
			AnswerValueType = EAnswerValueTypeReturn::Number;
			numberValue = number;
		}
		FExitPollAnswer(EQuestionType questionType)
		{
			type = GetQuestionTypeString(questionType);
			AnswerValueType = EAnswerValueTypeReturn::Null;
		}
		FExitPollAnswer(EQuestionType questionType, bool boolean)
		{
			type = GetQuestionTypeString(questionType);
			AnswerValueType = EAnswerValueTypeReturn::Bool;
			boolValue = boolean;
		}
		FExitPollAnswer(EQuestionType questionType, ::std::string string)
		{
			type = GetQuestionTypeString(questionType);
			AnswerValueType = EAnswerValueTypeReturn::String;
			stringValue = string;
		}

		::std::string GetQuestionTypeString(EQuestionType questionType)
		{
			if (questionType == EQuestionType::QBOOL)
				return "BOOLEAN";
			if (questionType == EQuestionType::HAPPYSAD)
				return "HAPPYSAD";
			if (questionType == EQuestionType::THUMBS)
				return "THUMBS";
			if (questionType == EQuestionType::SCALE)
				return "SCALE";
			if (questionType == EQuestionType::MULTIPLE)
				return "MULTIPLE";
			if (questionType == EQuestionType::VOICE)
				return "VOICE";
			
			return "BOOLEAN";
		}
};

struct FExitPollResponse
{
public:
		::std::string user = "";
		::std::string questionSetId = "";
		::std::string sessionId = "";
		::std::string hook = "";
		::std::vector<FExitPollAnswer> answers = ::std::vector<FExitPollAnswer>();

		::std::string questionSetName = "";
		::std::string questionSetVersion = "";

		//TODO proper override of nlohmann::to_json and from_json
		nlohmann::json to_json()
		{
			nlohmann::json j = nlohmann::json{ { "userId", user },{ "questionSetId", questionSetId },{ "sessionId", sessionId },{ "hook", hook } };
			nlohmann::json janswers = nlohmann::json::array();
			for (auto& an : answers)
			{
				nlohmann::json tempAnswer = nlohmann::json();
				if (an.AnswerValueType == EAnswerValueTypeReturn::Bool)
				{
					tempAnswer = nlohmann::json{ { "name", an.type },{ "value", an.boolValue ? 1 : 0 } };
				}
				else if (an.AnswerValueType == EAnswerValueTypeReturn::Number)
				{
					tempAnswer = nlohmann::json{ { "name", an.type },{ "value", an.numberValue } };
				}
				else if (an.AnswerValueType == EAnswerValueTypeReturn::Null)
				{
					tempAnswer = nlohmann::json{ { "name", an.type },{ "value", -32768 } };
				}
				else if (an.AnswerValueType == EAnswerValueTypeReturn::String)
				{
					tempAnswer = nlohmann::json{ { "name", an.type },{ "value", an.stringValue } };
				}

				janswers.push_back(tempAnswer);
			}
			j["answers"] = janswers;
			return j;
		};
};

class COGNITIVEVRANALYTICS_API ExitPoll
{
private:
	::std::string lastHook = "";
	::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;
	nlohmann::json currentQuestionSet = nlohmann::json();
	::std::string currentQuestionSetString = "";

	FExitPollResponse fullResponse = FExitPollResponse();

public:

	ExitPoll(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);

	void RequestQuestionSet(::std::string Hook);
	void ReceiveQuestionSet(::std::string questionsetstring, nlohmann::json questionset);

	//can return empty json if request failed
	nlohmann::json GetQuestionSet();
	::std::string GetQuestionSetString();

	bool HasQuestionSet()
	{
		return currentQuestionSet.size() > 0;
	}

	void AddAnswer(FExitPollAnswer answer);

	void SendAllAnswers();
	void SendAllAnswers(::std::vector<float> position);

	//called after SendQuestionResponse. clears the currentQuestionSetData and response
	void ClearQuestionSet();
};
}