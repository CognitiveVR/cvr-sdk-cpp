/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/
#include "stdafx.h"
#include "cognitive_log.h"

namespace cognitive {
CognitiveLog::CognitiveLog(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
	SetLoggingLevel(LoggingLevel::kAll);
}

void CognitiveLog::SetLoggingLevel(LoggingLevel level)
{
	logLevel = level;
}

void CognitiveLog::Info(std::string s)
{
	if (logLevel != LoggingLevel::kAll) { return; }
	std::cout << "[cognitiveVR::Info] " + s + "\n";
}

void CognitiveLog::Warning(std::string s)
{
	if (logLevel != LoggingLevel::kAll) { return; }
	std::cout << "[cognitiveVR::Warning] " + s + "\n";
}

void CognitiveLog::Error(std::string s)
{
	if (logLevel == LoggingLevel::kNone) { return; }
	std::cerr << "[cognitiveVR::Error] " + s + "\n";
}
}