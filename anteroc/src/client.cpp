#include "anteroc/client.hpp"

#ifdef CLIENT_HPP

struct DoraClient
{
	DoraClient (std::shared_ptr<grpc::Channel> channel, ClientConfig configs) :
		stub_(testify::Dora::NewStub(channel)), configs_(configs) {}

	// return true if successful
	bool ListTestcases (std::unordered_set<std::string>& out)
	{
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

	std::vector<testify::TestOutput> GetTestcase (size_t n, std::string tname)
	{
		testify::Testname request;
		request.set_name(tname);

		std::vector<testify::TestOutput> out;

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
		testify::Nothing request;

		grpc::ClientContext context;
		std::chrono::system_clock::time_point deadline =
			std::chrono::system_clock::now() + std::chrono::seconds(configs_.timeout);
		context.set_deadline(deadline);

		grpc::Status status(grpc::UNKNOWN, "obtained streaming reader is null");
		auto reader = stub_->ListTestcases(&context, request);
		if (nullptr != reader)
		{
			testify::Testname tname;
			while (reader->Read(&tname))
			{
				out.emplace(tname.name());
			}
			status = reader->Finish();
		}
		return status;
	}

	grpc::Status unsafe_getTestcase(
		std::vector<testify::TestOutput>& out,
		size_t nlimit, testify::Testname& request)
	{
		grpc::ClientContext context;
		std::chrono::system_clock::time_point deadline =
			std::chrono::system_clock::now() + std::chrono::seconds(configs_.timeout);
		context.set_deadline(deadline);

		grpc::Status status(grpc::UNKNOWN, "obtained streaming reader is null");
		auto reader = stub_->GetTestcase(&context, request);
		if (nullptr != reader)
		{
			testify::TestOutput tout;
			for (size_t i = 0; i < nlimit && reader->Read(&tout); ++i)
			{
				out.push_back(tout);
			}
			status = reader->Finish();
		}
		return status;
	}

	std::unique_ptr<testify::Dora::Stub> stub_;

	ClientConfig configs_;
};

static DoraClient* client = nullptr;
static size_t ntests = 50;

void ANTERO_INIT (std::string host, size_t nrepeats, ClientConfig configs)
{
	ntests = nrepeats;
	client = new DoraClient(grpc::CreateChannel(host,
		grpc::InsecureChannelCredentials()), configs);
}

void ANTERO_SHUTDOWN (void)
{
	if (client != nullptr)
	{
		delete client;
	}
	client = nullptr;
}

std::vector<testify::TestOutput> get_outputs (std::string tname)
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

	std::vector<testify::TestOutput> out;
	if (found)
	{
		out = client->GetTestcase(ntests, tname);
	}
	return out;
}

#endif
