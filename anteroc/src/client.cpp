#include "anteroc/client.hpp"

#ifdef ANTERO_CLIENT_HPP

namespace antero
{

static size_t grab_ncases = 30;
static std::unique_ptr<DoraClient> client = nullptr;

void INIT (size_t ncases, ClientConfig configs)
{
	client = std::make_unique<DoraClient>(configs);
	grab_ncases = ncases;
}

void SHUTDOWN (void)
{
	client.reset();
	google::protobuf::ShutdownProtobufLibrary();
}

optional<std::vector<testify::GeneratedCase>> get_cases (std::string tname)
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

	optional<std::vector<testify::GeneratedCase>> out;
	if (found)
	{
		out = client->get_testcase(grab_ncases, tname);
	}
	return out;
}

}

#endif
