
/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include "stdafx.h"
#include "CognitiveVRAnalytics.h"


#ifdef COGNITIVEVRANALYTICS_EXPORTS  
#define COGNITIVEVRANALYTICS_API __declspec(dllexport)   
#else  
#define COGNITIVEVRANALYTICS_API __declspec(dllimport)
#endif


namespace cognitive {
	//using json = nlohmann::json;
class CognitiveVRAnalyticsCore;

class COGNITIVEVRANALYTICS_API Transaction
{
	//friend class CognitiveVRAnalyticsCore;

    private:
		::std::shared_ptr<CognitiveVRAnalyticsCore> cvr = NULL;
		//the array of objects {method:"whatever",args:json} to send to dashboard
		nlohmann::json BatchedTransactions;
		//the array of objects {"name":"whatever","time":100,"point":[0,1,2]} to send to dashboard
		nlohmann::json BatchedTransactionsSE;

		

		int jsonPart = 1;
		int transactionCount = 0;

    public:
        Transaction(::std::shared_ptr<CognitiveVRAnalyticsCore> cog);

        /** Begin a new transaction.

			@param std::string category
			@param FVector position - Optional.
            @param Json::Value properties - Optional.
			@param std::string transaction_id - Optional.
                
            @throws CognitiveVR_exception
        */
		//void Begin(std::string category, std::shared_ptr<json> properties = NULL, std::string transaction_id = "");
		void BeginPosition(::std::string category, ::std::vector<float> &Position, ::std::string transaction_id = "");
		void BeginPosition(::std::string category, ::std::vector<float> &Position, nlohmann::json properties, ::std::string transaction_id = "");

        /** Update an existing transaction.

			@param std::string category
			@param FVector position - Optional.
			@param Json::Value properties - Optional.
            @param std::string transaction_id - Optional.
            @param double progress - Optional.

            @throws CognitiveVR_exception
        */
		//void Update(std::string category, std::shared_ptr<json> properties = NULL, std::string transaction_id = "", double progress = 0);
		void UpdatePosition(::std::string category, ::std::vector<float> &Position, ::std::string transaction_id = "", double progress = 0);
		void UpdatePosition(::std::string category, ::std::vector<float> &Position, nlohmann::json properties, ::std::string transaction_id = "", double progress = 0);

        /** End an existing transaction.

			@param std::string category
			@param FVector position - Optional.
			@param Json::Value properties - Optional.
            @param std::string transaction_id - Optional.
            @param std::string result - Optional.

            @throws cognitivevr_exception
        */
		//void End(std::string category, std::shared_ptr<json> properties = NULL, std::string transaction_id = "", std::string result = "");
		void EndPosition(::std::string category, ::std::vector<float> &Position, ::std::string transaction_id = "", ::std::string result = "");
		void EndPosition(::std::string category, ::std::vector<float> &Position, nlohmann::json properties, ::std::string transaction_id = "", ::std::string result = "");

        /** Begin and end new transaction.

			@param std::string category
			@param FVector position - Optional.
			@param Json::Value properties - Optional.
            @param std::string transaction_id - Optional.
            @param std::string result - Optional.

            @throws cognitivevr_exception
        */
		//void BeginEnd(std::string category, std::shared_ptr<json> properties = NULL, std::string transaction_id = "", std::string result = "");
		void BeginEndPosition(::std::string category, ::std::vector<float> &Position, ::std::string transaction_id = "", ::std::string result = "");
		void BeginEndPosition(::std::string category, ::std::vector<float> &Position, nlohmann::json properties, ::std::string transaction_id = "", ::std::string result = "");

		void AddToBatch(::std::string method, nlohmann::json args);

		void SendData();
};
}