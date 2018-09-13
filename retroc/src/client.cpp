#include <grpc/grpc.h>

#include "retroc/client.hpp"

#include "proto/testify.pb.h"
#include "proto/testify.grpc.pb.h"

#ifdef RETRO_CLIENT_HPP

namespace retro
{

struct DoraClient
{
	DoraClient(std::shared_ptr<grpc::Channel> channel, ClientConfig configs) :
		stub_(testify::Dora::NewStub(channel)), configs_(configs) {}

	// return true if successful
	bool AddTestcase (std::string testname, testify::GeneratedCase& gcase)
	{
		testify::TransferCase tcase;
		tcase.set_name(testname);
		tcase.mutable_results()->Swap(&gcase);

		grpc::Status status = unsafe_addTestcase(tcase);

		for (size_t i = 0; i < configs_.nretry && false == status.ok(); ++i)
		{
			// assert: status is not fine, so log
			// mlogger() << status.error_code() << ":" << status.error_message();
			status = unsafe_addTestcase(tcase);
		}

		bool good = status.ok();
		if (false == good)
		{
			// mlogger() << status.error_code() << ":" << status.error_message();
		}

		return good;
	}

private:
	grpc::Status unsafe_addTestcase (testify::TransferCase& request)
	{
		google::protobuf::Empty reply;

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

// cache testname and attempt to send case to server
void cache (std::string testname, testify::GeneratedCase& gcase)
{
	// todo: implement
}

void INIT (std::string host, ClientConfig configs)
{
	client = new DoraClient(grpc::CreateChannel(host,
		grpc::InsecureChannelCredentials()), configs);
}

void SHUTDOWN (void)
{
	if (client != nullptr)
	{
		delete client;
	}
	client = nullptr;
	google::protobuf::ShutdownProtobufLibrary();
}

void send (std::string testname, testify::GeneratedCase& gcase)
{
	if (nullptr == client)
	{
		throw std::runtime_error("client not initialized");
	}

	if (false == client->AddTestcase(testname, gcase))
	{
		// log
		cache(testname, gcase);
	}
}

}

#endif
