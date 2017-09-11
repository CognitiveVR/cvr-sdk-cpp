/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include "stdafx.h"
#include <iostream>
#include "CognitiveVRAnalytics.h"

enum LoggingLevel
{
	All = 0,
	ErrorsOnly = -1,
	None = -2
};

class CognitiveVRAnalyticsCore;

class CognitiveLog
{
	private:
		LoggingLevel logLevel;
		CognitiveVRAnalyticsCore* cvr;

    public:
		CognitiveLog(CognitiveVRAnalyticsCore* cog);
        void Info(std::string s);
		void Warning(std::string s);
		void Error(std::string s);
		void SetLoggingLevel(LoggingLevel level);
};