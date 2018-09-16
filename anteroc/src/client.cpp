#include <future>

#include "anteroc/client.hpp"

#ifdef ANTERO_CLIENT_HPP

namespace antero
{

static const size_t health_interval = 30;

struct DoraClient
{
	DoraClient (std::shared_ptr<grpc::Channel> channel, ClientConfig configs) :
		stub_(testify::Dora::NewStub(channel)), configs_(configs)
	{
		reachable_ = true;
	}

	~DoraClient (void)
	{
		reachable_ = false;
	}

	// return true if successful
	bool ListTestcases (std::unordered_set<std::string>& out)
	{
		if (false == reachable_)
		{
			return false;
		}

		grpc::Status status = unsafe_listTestcases(out);

		for (size_t i = 0; i < configs_.nretry && false == status.ok(); ++i)
		{
			// assert: status is not fine, so log
			// mlogger() << status.error_code() << ":" << status.error_message();
			status = unsafe_listTestcases(out);
		}

		bool good = status.ok();
		if (false == good)
		{
			// mlogger() << status.error_code() << ":" << status.error_message();
		}

		return good;
	}

	std::vector<testify::GeneratedCase> GetTestcase (size_t n, std::string tname)
	{
		std::vector<testify::GeneratedCase> out;
		if (false == reachable_)
		{
			return out;
		}

		testify::TransferName request;
		request.set_name(tname);

		grpc::Status status = unsafe_getTestcase(out, n, request);

		for (size_t i = 0; i < configs_.nretry && false == status.ok(); ++i)
		{
			// assert: status is not fine, so log
			// mlogger() << status.error_code() << ":" << status.error_message();
			status = unsafe_getTestcase(out, n, request);
		}

		if (false == status.ok())
		{
			// mlogger() << status.error_code() << ":" << status.error_message();
		}

		return out;
	}

private:
	grpc::Status unsafe_listTestcases (
		std::unordered_set<std::string>& out)
	{
		google::protobuf::Empty request;

		grpc::ClientContext context;
		std::chrono::system_clock::time_point deadline =
			std::chrono::system_clock::now() + std::chrono::seconds(configs_.timeout);
		context.set_deadline(deadline);

		grpc::Status status(grpc::UNKNOWN, "obtained streaming reader is null");
		auto reader = stub_->ListTestcases(&context, request);
		if (nullptr != reader)
		{
			testify::TransferName tname;
			while (reader->Read(&tname))
			{
				out.emplace(tname.name());
			}
			status = reader->Finish();
		}
		if (status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED)
		{
			set_health_job();
			return grpc::Status(grpc::StatusCode::DEADLINE_EXCEEDED,
				"Deadline exceeded or Client cancelled, abandoning.");
		}
		return status;
	}

	grpc::Status unsafe_getTestcase(
		std::vector<testify::GeneratedCase>& out,
		size_t nlimit, testify::TransferName& request)
	{
		grpc::ClientContext context;
		std::chrono::system_clock::time_point deadline =
			std::chrono::system_clock::now() + std::chrono::seconds(configs_.timeout);
		context.set_deadline(deadline);

		grpc::Status status(grpc::UNKNOWN, "obtained streaming reader is null");
		auto reader = stub_->GetTestcase(&context, request);
		if (nullptr != reader)
		{
			testify::GeneratedCase tout;
			for (size_t i = 0; i < nlimit && reader->Read(&tout); ++i)
			{
				out.push_back(tout);
			}
			status = reader->Finish();
		}
		if (status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED)
		{
			set_health_job();
			return grpc::Status(grpc::StatusCode::DEADLINE_EXCEEDED,
				"Deadline exceeded or Client cancelled, abandoning.");
		}
		return status;
	}

	void set_health_job (void)
	{
		reachable_ = false;
		std::async(std::launch::async,
		[this]()
		{
			while (false == reachable_)
			{
				std::this_thread::sleep_for(std::chrono::seconds(health_interval));
				// check health
				grpc::ClientContext context;
				std::chrono::system_clock::time_point deadline =
					std::chrono::system_clock::now() + std::chrono::seconds(configs_.timeout);
				context.set_deadline(deadline);
				google::protobuf::Empty empty;
				testify::HealthCheckResponse resp;
				auto status = stub_->CheckHealth(&context, empty, &resp);
				reachable_ = status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED;
			}
		});
	}

	std::atomic<bool> reachable_;

	std::unique_ptr<testify::Dora::Stub> stub_;

	ClientConfig configs_;
};

static DoraClient* client = nullptr;
static size_t ntests = 50;

void INIT (std::string host, size_t nrepeats, ClientConfig configs)
{
	ntests = nrepeats;
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

std::vector<testify::GeneratedCase> get_cases (std::string tname)
{
	if (nullptr == client)
	{
		throw std::runtime_error("client not initialized");
	}

	bool found = false;
	std::unordered_set<std::string> names;
	if (client->ListTestcases(names))
	{
		found = names.find(tname) != names.end();
	}

	std::vector<testify::GeneratedCase> out;
	if (found)
	{
		out = client->GetTestcase(ntests, tname);
	}
	return out;
}

}

#endif
