#include <future>
#include <iostream>
#include <fstream>
#include <sstream>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "proto/testify.pb.h"
#include "proto/testify.grpc.pb.h"

#ifndef CLIENT_HPP
#define CLIENT_HPP

using DTYPE = testify::CaseData::DataCase;

struct ClientConfig
{
	std::string host;
	std::string cert;
	size_t nretry = 0;
	size_t timeout = 3;
	size_t health_interval = 30;
};

bool is_reachable (grpc::Status status);

void log_status (grpc::Status status);

std::string read_keycert (const char* filename);

struct DoraClient
{
	DoraClient(ClientConfig configs) :
		configs_(configs), keep_check_(true), reachable_(true)
	{
		grpc::SslCredentialsOptions ssl_opts;
		ssl_opts.pem_root_certs = configs.cert;

		auto channel_creds = grpc::SslCredentials(ssl_opts);
		auto channel = grpc::CreateChannel(configs.host, channel_creds);
		stub_ = testify::Dora::NewStub(channel);

		health_job_ = new std::thread([this]()
		{
			std::mutex nothing;
			std::unique_lock<std::mutex> lock(nothing);
			while (this->keep_check_.load())
			{
				// check health
				this->check_reachable();
				this->stop_check_.wait_for(lock,
					std::chrono::seconds(this->configs_.health_interval));
			}
		});
	}

	virtual ~DoraClient (void)
	{
		keep_check_ = false;
		stop_check_.notify_all();
		health_job_->join();
		delete health_job_;
	}

	bool reachable (void)
	{
		return reachable_.load();
	}

	// return all testcases names
	bool list_testcases (std::unordered_set<std::string>& out)
	{
		if (false == check_reachable())
		{
			return false;
		}

		testify::ListRequest request;

		std::unordered_map<std::string,testify::GeneratedTest> tests;
		grpc::Status status = unsafe_list_testcases(request, tests);
		log_status(status);

		for (size_t i = 0; i < configs_.nretry && false == status.ok(); ++i)
		{
			status = unsafe_list_testcases(request, tests);
			log_status(status);
		}

		for (auto tpair : tests)
		{
			out.emplace(tpair.first);
		}

		return status.ok();
	}

	// return one of the testcases
	bool get_testcase (testify::GeneratedTest& out, std::string testname)
	{
		if (false == check_reachable())
		{
			return false;
		}

		testify::ListRequest request;
		request.add_test_names(testname);

		std::vector<testify::GeneratedCase> vec;

		std::unordered_map<std::string,testify::GeneratedTest> tests;
		grpc::Status status = unsafe_list_testcases(request, tests);
		log_status(status);

		for (size_t i = 0; i < configs_.nretry && false == status.ok(); ++i)
		{
			status = unsafe_list_testcases(request, tests);
			log_status(status);
		}

		auto it = tests.find(testname);
		if (tests.end() != it)
		{
			out = it->second;
		}

		return status.ok();
	}

	// return true if successful
	bool add_testcase (std::string testname, testify::GeneratedCase& gcase)
	{
		if (false == check_reachable())
		{
			return false;
		}

		testify::AddRequest request;
		request.set_name(testname);
		request.mutable_payload()->Swap(&gcase);
		grpc::Status status = unsafe_add_testcase(request);
		log_status(status);

		for (size_t i = 0; i < configs_.nretry && false == status.ok(); ++i)
		{
			status = unsafe_add_testcase(request);
			log_status(status);
		}

		return status.ok();
	}

	// return true if successful
	bool remove_testcase (std::string testname)
	{
		if (false == check_reachable())
		{
			return false;
		}

		testify::RemoveRequest request;
		request.add_names(testname);
		grpc::Status status = unsafe_remove_testcase(request);
		log_status(status);

		for (size_t i = 0; i < configs_.nretry && false == status.ok(); ++i)
		{
			status = unsafe_remove_testcase(request);
			log_status(status);
		}

		return status.ok();
	}

	ClientConfig configs_;

private:
	grpc::Status unsafe_list_testcases (testify::ListRequest& request,
		std::unordered_map<std::string,testify::GeneratedTest>& out)
	{
		testify::ListResponse response;
		grpc::ClientContext context;
		this->get_context(context);

		grpc::Status status = stub_->ListTestcases(
			&context, request, &response);
		if (is_reachable(status))
		{
			auto tests = response.tests();
			for (auto tpair : tests)
			{
				out[tpair.first] = tpair.second;
			}
		}
		return status;
	}

	grpc::Status unsafe_add_testcase (testify::AddRequest& request)
	{
		google::protobuf::Empty reply;
		grpc::ClientContext context;
		this->get_context(context);

		return stub_->AddTestcase(&context, request, &reply);
	}

	grpc::Status unsafe_remove_testcase (testify::RemoveRequest& request)
	{
		google::protobuf::Empty reply;
		grpc::ClientContext context;
		this->get_context(context);

		return stub_->RemoveTestcase(&context, request, &reply);
	}

	bool check_reachable (void)
	{
		// avoid recoverying sporadically,
		// allow health thread to recover status
		if (false == reachable_.load())
		{
			return false;
		}

		grpc::ClientContext context;
		this->get_context(context);

		google::protobuf::Empty empty;
		testify::HealthCheckResponse resp;
		auto status = stub_->CheckHealth(&context, empty, &resp);
		bool reachable = is_reachable(status) &&
			resp.status() == testify::HealthCheckResponse::SERVING;
		this->reachable_ = reachable;
		return reachable;
	}

	void get_context (grpc::ClientContext& context)
	{
		std::chrono::system_clock::time_point deadline =
			std::chrono::system_clock::now() +
			std::chrono::seconds(configs_.timeout);
		context.set_deadline(deadline);
	}

	std::unique_ptr<testify::Dora::Stub> stub_;
	std::thread* health_job_ = nullptr;

	std::atomic<bool> keep_check_;
	std::atomic<bool> reachable_;
	std::condition_variable stop_check_;
};

#endif /* CLIENT_HPP */
