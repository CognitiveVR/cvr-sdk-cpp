/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "stdafx.h"
#include "cognitive_log.h"

CognitiveLog::CognitiveLog(std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;
	std::cout << "log constructor\n";
	SetLoggingLevel(LoggingLevel::All);
}

void CognitiveLog::SetLoggingLevel(LoggingLevel level)
{
	logLevel = level;
}

void CognitiveLog::Info(std::string s)
{
	if (logLevel != LoggingLevel::All) { return; }
	std::cout << "[cognitiveVR::Info] " + s + "\n";
}

void CognitiveLog::Warning(std::string s)
{
	if (logLevel != LoggingLevel::All) { return; }
	std::cout << "[cognitiveVR::Warning] " + s + "\n";
}

void CognitiveLog::Error(std::string s)
{
	if (logLevel == LoggingLevel::None) { return; }
	std::cerr << "[cognitiveVR::Error] " + s + "\n";
}
