// Fill out your copyright notice in the Description page of Project Settings.

//organizes all the question sets, responses and exit poll panel actors

#pragma once

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"
using json = nlohmann::json;

#ifdef COGNITIVEVRANALYTICS_EXPORTS  
#define COGNITIVEVRANALYTICS_API __declspec(dllexport)   
#else  
#define COGNITIVEVRANALYTICS_API __declspec(dllimport)
#endif

class CognitiveVRAnalyticsCore;

enum EAnswerValueTypeReturn
{
	Number,
	Bool,
	String, //used for voice
	Null
};

struct FExitPollAnswer
{
public:
		std::string type; //question type
		EAnswerValueTypeReturn AnswerValueType;
		int numberValue;
		bool boolValue; //converted to 0 or 1
		std::string stringValue; //for base64 voice
};

struct FExitPollResponse
{
public:
		std::string user;
		std::string questionSetId;
		std::string sessionId;
		std::string hook;
		std::vector<FExitPollAnswer> answers;
};

class COGNITIVEVRANALYTICS_API ExitPoll
{
private:
	std::string lastHook;
	std::shared_ptr<CognitiveVRAnalyticsCore> cvr;
public:

	ExitPoll(std::shared_ptr<CognitiveVRAnalyticsCore> cog);

	//TODO figure out simple delegate
	void GetQuestionSet(std::string Hook);

	void SendQuestionResponse(FExitPollResponse responses);
};