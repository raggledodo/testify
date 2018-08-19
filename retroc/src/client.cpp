#include <grpcpp/grpcpp.h>

#include "retroc/client.hpp"

#include "proto/record.pb.h"
#include "proto/record.grpc.pb.h"

#ifdef CLIENT_HPP

struct DoraClient
{
	DoraClient(std::shared_ptr<grpc::Channel> channel, ClientConfig configs) :
		stub_(testify::Dora::NewStub(channel)), configs_(configs) {}

	// return true if successful
	bool AddTestcase(testify::Testcase& testcase)
	{
		grpc::Status status = unsafe_addTestcase(testcase);

		for (size_t i = 0; i < configs_.nretry && false == status.ok(); ++i)
		{
			// assert: status is not fine, so log
			// mlogger() << status.error_code() << ":" << status.error_message();
			status = unsafe_addTestcase(testcase);
		}

		bool good = status.ok();
		if (false == good)
		{
			// mlogger() << status.error_code() << ":" << status.error_message();
		}

		return good;
	}

private:
	grpc::Status unsafe_addTestcase (testify::Testcase& request)
	{
		testify::Nothing reply;

		grpc::ClientContext context;
		std::chrono::system_clock::time_point deadline =
			std::chrono::system_clock::now() + std::chrono::seconds(configs_.timeout);
		context.set_deadline(deadline);

		return stub_->AddTestcase(&context, request, &reply);
	}

	std::unique_ptr<testify::Dora::Stub> stub_;

	ClientConfig configs_;
};

static DoraClient* client = nullptr;

void RETRO_INIT (std::string host, ClientConfig configs)
{
	client = new DoraClient(grpc::CreateChannel(host,
		grpc::InsecureChannelCredentials()), configs);
}

void RETRO_SHUTDOWN (void)
{
	if (client != nullptr)
	{
		delete client;
	}
	client = nullptr;
}

bool send (std::string testname, testify::TestOutput& output)
{
	if (nullptr == client)
	{
		throw std::runtime_error("client not initialized");
	}

	if (output.inputs().empty())
	{
		return false;
	}

	testify::Testcase tcase;
	tcase.set_name(testname);
	tcase.mutable_results()->Swap(&output);

	if (false == client->AddTestcase(tcase))
	{
		// log
		// cache output locally
		// add to reattempt job queue
	}
	return true;
}

#endif
