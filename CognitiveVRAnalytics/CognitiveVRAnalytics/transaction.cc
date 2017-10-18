
/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "stdafx.h"
#include "transaction.h"
namespace cognitive {
Transaction::Transaction(::std::shared_ptr<CognitiveVRAnalyticsCore> cog)
{
	cvr = cog;

	BatchedTransactions = nlohmann::json::array();
}

void Transaction::BeginPosition(::std::string category, ::std::vector<float> &Position, ::std::string transaction_id)
{
	BeginPosition(category, Position, nlohmann::json(), transaction_id);
}

void Transaction::BeginPosition(::std::string category, ::std::vector<float> &Position, nlohmann::json properties, ::std::string transaction_id)
{
	if (!cvr->WasInitSuccessful() && !cvr->IsPendingInit()) { cvr->log->Info("Transaction::BeginPosition failed: init not successful"); return; }

	::std::string trans = std::string("TXN");

	double ts = cvr->GetTimestamp();

	nlohmann::json args = nlohmann::json::array();

	args.emplace_back(ts);
	args.emplace_back(ts);
	args.emplace_back(cvr->UserId);
	args.emplace_back(cvr->DeviceId);
	args.emplace_back(category);
	args.emplace_back(trans);
	args.emplace_back(cvr->config->kNetworkTimeout);
	args.emplace_back(transaction_id);
	args.emplace_back(properties);

	AddToBatch("datacollector_beginTransaction", args);

	//TODO conversion for xyz = -xzy or whatever

	nlohmann::json se = nlohmann::json();
	se["name"] = category;
	se["time"] = ts;
	se["point"] = { Position[0],Position[1] ,Position[2] };
	if (properties.size() > 0)
	{
		se["properties"] = properties;
	}

	BatchedTransactionsSE.emplace_back(se);
}

void Transaction::UpdatePosition(::std::string category, ::std::vector<float> &Position, ::std::string transaction_id, double progress)
{
	UpdatePosition(category, Position, nlohmann::json(), transaction_id, progress);
}

void Transaction::UpdatePosition(::std::string category, ::std::vector<float> &Position, nlohmann::json properties, ::std::string transaction_id, double progress)
{
	if (!cvr->WasInitSuccessful() && !cvr->IsPendingInit()) { cvr->log->Info("Transaction::UpdatePosition failed: init not successful"); return; }

	double ts = cvr->GetTimestamp();

	nlohmann::json args = nlohmann::json::array();

	args.emplace_back(ts);
	args.emplace_back(ts);
	args.emplace_back(cvr->UserId);
	args.emplace_back(cvr->DeviceId);
	args.emplace_back(category);
	args.emplace_back(progress);
	args.emplace_back(transaction_id);
	args.emplace_back(properties);

	AddToBatch("datacollector_updateTransaction", args);

	//TODO conversion for xyz = -xzy or whatever

	nlohmann::json se = nlohmann::json();
	se["name"] = category;
	se["time"] = ts;
	se["point"] = { Position[0],Position[1] ,Position[2] };
	if (properties.size() > 0)
	{
		se["properties"] = properties;
	}

	BatchedTransactionsSE.emplace_back(se);
}

void Transaction::EndPosition(::std::string category, ::std::vector<float> &Position, ::std::string transaction_id, ::std::string result)
{
	EndPosition(category, Position, nlohmann::json(), transaction_id, result);
}

void Transaction::EndPosition(::std::string category, ::std::vector<float> &Position, nlohmann::json properties, ::std::string transaction_id, ::std::string result)
{
	if (!cvr->WasInitSuccessful() && !cvr->IsPendingInit()) { cvr->log->Info("Transaction::EndPosition failed: init not successful"); return; }

	double ts = cvr->GetTimestamp();

	nlohmann::json args = nlohmann::json::array();

	args.emplace_back(ts);
	args.emplace_back(ts);
	args.emplace_back(cvr->UserId);
	args.emplace_back(cvr->DeviceId);
	args.emplace_back(category);
	args.emplace_back(result);
	args.emplace_back(transaction_id);
	args.emplace_back(properties);

	AddToBatch("datacollector_endTransaction", args);

	//TODO conversion for xyz = -xzy or whatever

	nlohmann::json se = nlohmann::json();
	se["name"] = category;
	se["time"] = ts;
	se["point"] = { Position[0],Position[1] ,Position[2] };
	if (properties.size() > 0)
	{
		se["properties"] = properties;
	}

	BatchedTransactionsSE.emplace_back(se);
}

void Transaction::BeginEndPosition(::std::string category, ::std::vector<float> &Position, ::std::string transaction_id, ::std::string result)
{
	BeginEndPosition(category, Position, nlohmann::json(), transaction_id, result);
}

void Transaction::BeginEndPosition(::std::string category, ::std::vector<float> &Position, nlohmann::json properties, ::std::string transaction_id, ::std::string result)
{
	if (!cvr->WasInitSuccessful() && !cvr->IsPendingInit()) { cvr->log->Info("Transaction::BeginEndPosition failed: init not successful"); return; }

	this->EndPosition(category, Position, properties, transaction_id, result);
}

void Transaction::AddToBatch(::std::string method, nlohmann::json args)
{
	nlohmann::json batchObject = nlohmann::json();

	batchObject["method"] = method;
	batchObject["args"] = args;

	BatchedTransactions.emplace_back(batchObject);

	if (BatchedTransactions.size() >= cvr->config->TransactionBatchSize)
	{
		SendData();
	}
}

void Transaction::SendData()
{
	if (cvr->IsPendingInit()) { cvr->log->Info("Transaction::SendData failed: init pending"); return; }
	if (!cvr->WasInitSuccessful()) { cvr->log->Info("Transaction::SendData failed: init not successful"); return; }

	if (BatchedTransactions.size() > 0)
	{
		//send to dashboard
		nlohmann::json data = nlohmann::json::array();
		data.emplace_back(cvr->GetTimestamp());
		data.emplace_back(BatchedTransactions);
		cvr->network->DashboardCall("datacollector_batch", data.dump());

		BatchedTransactions.clear();
	}


	if (BatchedTransactionsSE.size() > 0)
	{
		//send to sceneexplorer
		nlohmann::json se = nlohmann::json();
		se["userid"] = cvr->UserId;
		se["timestamp"] = (int)cvr->GetTimestamp();
		se["sessionid"] = cvr->GetSessionID();
		se["part"] = jsonPart;
		jsonPart++;
		se["data"] = BatchedTransactionsSE;
		if (cvr->network->SceneExplorerCall("events", se.dump()))
		{
			BatchedTransactionsSE.clear();
		}
	}
}
void Transaction::EndSession()
{
	BatchedTransactionsSE.clear();
}
}