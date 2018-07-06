/*
Copyright (c) 2017 CognitiveVR, Inc. All rights reserved.
*/

//All comments from Cognitive Analytics. Can be set to different logging level to disable comments by type

#pragma once

#include "stdafx.h"
#include <iostream>
#include "CognitiveVRAnalytics.h"

namespace cognitive
{
class CognitiveLog
{
	friend class Network;
	friend class CognitiveVRAnalyticsCore;

	private:
		LoggingLevel logLevel = LoggingLevel::kAll;
		std::shared_ptr<CognitiveVRAnalyticsCore> cvr = nullptr;

public:
		CognitiveLog(std::shared_ptr<CognitiveVRAnalyticsCore> cog);
        void Info(std::string s);
		void Warning(std::string s);
		void Error(std::string s);
		void SetLoggingLevel(LoggingLevel level);
};
}