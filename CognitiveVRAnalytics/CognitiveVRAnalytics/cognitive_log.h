/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include "stdafx.h"
#include <iostream>
#include "CognitiveVRAnalytics.h"

namespace cognitive{

class CognitiveVRAnalyticsCore;

class CognitiveLog
{
	private:
		LoggingLevel logLevel = LoggingLevel::All;
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;

    public:
		CognitiveLog(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);
        void Info(::std::string s);
		void Warning(::std::string s);
		void Error(::std::string s);
		void SetLoggingLevel(LoggingLevel level);
};
}