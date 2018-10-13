#include "anteroc/client.hpp"

#ifdef ANTERO_CLIENT_HPP

namespace antero
{

static std::unique_ptr<dora::DoraClient> client = nullptr;

void INIT (dora::ClientConfig configs)
{
	client = std::make_unique<dora::DoraClient>(configs);
}

void SHUTDOWN (void)
{
	client.reset();
	google::protobuf::ShutdownProtobufLibrary();
}

bool get_cases (std::vector<testify::GeneratedCase>& out, std::string tname)
{
	if (nullptr == client)
	{
		throw std::runtime_error("anteroc client not initialized");
	}

	bool found = false;
	std::unordered_set<std::string> names;
	if (client->list_testcases(names))
	{
		found = names.find(tname) != names.end();
	}

	if (found)
	{
		testify::GeneratedTest test;
		client->get_testcase(test, tname);
		auto fields = test.cases();
		out = std::vector<testify::GeneratedCase>(fields.begin(), fields.end());
	}
	return found;
}

}

#endif
