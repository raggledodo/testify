#include <grpcpp/grpcpp.h>

#include "retroc/client.hpp"

#include "proto/record.pb.h"
#include "proto/record.grpc.pb.h"

#ifdef CLIENT_HPP

static const int client_timeout = 3;

struct DoraClient
{
	DoraClient(std::shared_ptr<grpc::Channel> channel, size_t nretry = 0) :
	stub_(testify::Dora::NewStub(channel)), nretry_(nretry) {}

	bool AddTestcase(testify::Testcase& request)
	{
		testify::Nothing reply;
		grpc::ClientContext context;

		std::chrono::system_clock::time_point deadline =
			std::chrono::system_clock::now() + std::chrono::seconds(client_timeout);
		context.set_deadline(deadline);

		grpc::Status status = stub_->AddTestcase(&context,
			request, &reply);
		bool keepsending = false == status.ok();

		for (size_t i = 0; i < nretry_ && keepsending; ++i)
		{
			// errlog << "Retry(" << i << ") Failed to send testcase: "
			// 	<< status.error_code() << ": " << status.error_message() << "\n";
			status = stub_->AddTestcase(&context,
				request, &reply);
			keepsending = false == status.ok();
		}
		// log

		return keepsending;
	}

private:
	std::unique_ptr<testify::Dora::Stub> stub_;

	size_t nretry_;
};

static DoraClient* client = nullptr;

void RETRO_INIT (std::string host, size_t nretry)
{
	client = new DoraClient(grpc::CreateChannel(host,
		grpc::InsecureChannelCredentials()), nretry);
}

void RETRO_SHUTDOWN (void)
{
	delete client;
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
