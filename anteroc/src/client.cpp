#include <future>

#include "anteroc/client.hpp"

#ifdef ANTERO_CLIENT_HPP

namespace antero
{

static const size_t health_interval = 30;
static ClientConfig cfg;
static std::atomic<bool> reachable(true);
static std::unique_ptr<testify::Dora::Stub> stub = nullptr;

static std::atomic<bool> keep_check(true);
static std::thread* health_job = nullptr;
static std::condition_variable stop_check;

grpc::Status unsafe_listTestcases (
	std::unordered_set<std::string>& out)
{
	google::protobuf::Empty request;

	grpc::ClientContext context;
	std::chrono::system_clock::time_point deadline =
		std::chrono::system_clock::now() + std::chrono::seconds(cfg.timeout);
	context.set_deadline(deadline);

	grpc::Status status(grpc::UNKNOWN, "obtained streaming reader is null");
	auto reader = stub->ListTestcases(&context, request);
	if (nullptr != reader)
	{
		testify::TransferName tname;
		while (reader->Read(&tname))
		{
			out.emplace(tname.name());
		}
		status = reader->Finish();
	}
	return status;
}

grpc::Status unsafe_getTestcase(
	std::vector<testify::GeneratedCase>& out,
	size_t nlimit, testify::TransferName& request)
{
	grpc::ClientContext context;
	std::chrono::system_clock::time_point deadline =
		std::chrono::system_clock::now() + std::chrono::seconds(cfg.timeout);
	context.set_deadline(deadline);

	grpc::Status status(grpc::UNKNOWN, "obtained streaming reader is null");
	auto reader = stub->GetTestcase(&context, request);
	if (nullptr != reader)
	{
		testify::GeneratedCase tout;
		for (size_t i = 0; i < nlimit && reader->Read(&tout); ++i)
		{
			out.push_back(tout);
		}
		status = reader->Finish();
	}
	return status;
}

// return true if successful
bool ListTestcases (std::unordered_set<std::string>& out)
{
	if (false == reachable)
	{
		return false;
	}

	grpc::Status status = unsafe_listTestcases(out);

	for (size_t i = 0; i < cfg.nretry && false == status.ok(); ++i)
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
	if (false == reachable)
	{
		return out;
	}

	testify::TransferName request;
	request.set_name(tname);

	grpc::Status status = unsafe_getTestcase(out, n, request);

	for (size_t i = 0; i < cfg.nretry && false == status.ok(); ++i)
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

void INIT (ClientConfig configs)
{
	cfg = configs;
	stub = testify::Dora::NewStub(grpc::CreateChannel(configs.host,
		grpc::InsecureChannelCredentials()));
	health_job = new std::thread([]()
	{
		std::mutex nothing;
		std::unique_lock<std::mutex> lock(nothing);
		while (keep_check.load())
		{
			// check health
			grpc::ClientContext context;
			std::chrono::system_clock::time_point deadline =
				std::chrono::system_clock::now() + std::chrono::seconds(cfg.timeout);
			context.set_deadline(deadline);
			google::protobuf::Empty empty;
			testify::HealthCheckResponse resp;
			auto status = stub->CheckHealth(&context, empty, &resp);
			auto ecode = status.error_code();
			reachable = ecode != grpc::StatusCode::DEADLINE_EXCEEDED &&
				ecode != grpc::StatusCode::UNAVAILABLE;
			stop_check.wait_for(lock, std::chrono::seconds(health_interval));
		}
	});
}

void SHUTDOWN (void)
{
	keep_check = false;
    stop_check.notify_all();
	health_job->join();
	delete health_job;
	stub.reset();
	google::protobuf::ShutdownProtobufLibrary();
}

std::vector<testify::GeneratedCase> get_cases (std::string tname)
{
	if (nullptr == stub)
	{
		throw std::runtime_error("client not initialized");
	}

	bool found = false;
	std::unordered_set<std::string> names;
	if (ListTestcases(names))
	{
		found = names.find(tname) != names.end();
	}

	std::vector<testify::GeneratedCase> out;
	if (found)
	{
		out = GetTestcase(cfg.grab_ncases, tname);
	}
	return out;
}

}

#endif
