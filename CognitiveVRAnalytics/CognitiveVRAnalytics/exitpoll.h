/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

//Get question sets, add player answers and send answers to dashboard

#pragma once

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"


#if defined(_MSC_VER)
//  Microsoft 
#ifdef COGNITIVEVRANALYTICS_EXPORTS  
#define COGNITIVEVRANALYTICS_API __declspec(dllexport)
#else  
#define COGNITIVEVRANALYTICS_API
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

namespace cognitive
{
class CognitiveVRAnalyticsCore;

//the type of answer for a question
enum class EAnswerValueTypeReturn
{
	kNumber,
	kBool,
	kString, //used for voice
	kNull
};

//the type of question
enum class EQuestionType
{
	kHappySad,
	kScale,
	kMultiple,
	kVoice,
	kThumbs,
	kBoolean
};

//the value of an answer
struct COGNITIVEVRANALYTICS_API ExitPollAnswer
{
	friend class ExitPoll;
	friend struct ExitPollResponse;

private:
	//this is only used internally. use one of the constructors and an enum
	std::string type = "";
	EAnswerValueTypeReturn AnswerValueType = EAnswerValueTypeReturn::kNull;
	int numberValue = -1;
	bool boolValue = false; //converted to 0 or 1
	std::string stringValue = "";; //for base64 voice

	std::string GetQuestionTypeString(EQuestionType questionType)
	{
		if (questionType == EQuestionType::kBoolean)
			return "BOOLEAN";
		if (questionType == EQuestionType::kHappySad)
			return "HAPPYSAD";
		if (questionType == EQuestionType::kThumbs)
			return "THUMBS";
		if (questionType == EQuestionType::kScale)
			return "SCALE";
		if (questionType == EQuestionType::kMultiple)
			return "MULTIPLE";
		if (questionType == EQuestionType::kVoice)
			return "VOICE";

		return "BOOLEAN";
	}

public:
		ExitPollAnswer(EQuestionType questionType, int number)
		{
			type = GetQuestionTypeString(questionType);
			AnswerValueType = EAnswerValueTypeReturn::kNumber;
			numberValue = number;
		}
		ExitPollAnswer(EQuestionType questionType, std::string string)
		{
			type = GetQuestionTypeString(questionType);
			AnswerValueType = EAnswerValueTypeReturn::kString;
			stringValue = string;
		}
};

//a collection of answers. this is sent to the web api
struct ExitPollResponse
{
	friend class ExitPoll;

private:
		std::string user = "";
		std::string questionSetId = "";
		std::string sessionId = "";
		std::string hook = "";
		std::vector<ExitPollAnswer> answers = std::vector<ExitPollAnswer>();

		std::string questionSetName = "";
		std::string questionSetVersion = "";
};

class COGNITIVEVRANALYTICS_API ExitPoll
{
	friend class Network;

private:
	std::string lastHook = "";
	double startTime;
	std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;
	nlohmann::json currentQuestionSet = nlohmann::json();
	std::string currentQuestionSetString = "";

	ExitPollResponse fullResponse = ExitPollResponse();

public:
	ExitPoll(std::shared_ptr<CognitiveVRAnalyticsCore> cog);

	/** Get a set of questions from the web api

	@param std::string hook
	*/
	void RequestQuestionSet(std::string hook);

	//Receive question set as raw string and json
	void ReceiveQuestionSet(std::string questionsetstring, nlohmann::json questionset);

	//can return empty json if request failed
	nlohmann::json GetQuestionSet();
	//can return empty string if request failed
	std::string GetQuestionSetString();

	//returns true if question set successfully recieved
	bool HasQuestionSet()
	{
		return currentQuestionSet.size() > 0;
	}

	/** Add an answer to the collection of responses for a question set

		@param FExitPollAnswer position
	*/
	void AddAnswer(ExitPollAnswer answer);

	/** Send the collection of answers from a question set to the web api
	*/
	nlohmann::json SendAllAnswers();
	/** Send the collection of answers from a question set to the web api
	@param std::vector<float> position
	*/
	nlohmann::json SendAllAnswers(std::vector<float> position);

	//called after SendQuestionResponse. clears the currentQuestionSetData and response
	void ClearQuestionSet();
};
}