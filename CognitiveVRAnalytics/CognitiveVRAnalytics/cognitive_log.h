/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include "stdafx.h"
#include <iostream>
#include "CognitiveVRAnalytics.h"

namespace cognitive{
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
		LoggingLevel logLevel = LoggingLevel::All;
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = NULL;

    public:
		CognitiveLog(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);
        void Info(::std::string s);
		void Warning(::std::string s);
		void Error(::std::string s);
		void SetLoggingLevel(LoggingLevel level);
};
}