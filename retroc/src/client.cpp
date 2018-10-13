#include "retroc/client.hpp"

#ifdef RETRO_CLIENT_HPP

namespace retro
{

static std::unique_ptr<dora::DoraClient> client = nullptr;

// cache testname and attempt to send case to server
void cache (std::string testname, testify::GeneratedCase& gcase)
{
	// todo: implement
}

void INIT (dora::ClientConfig configs)
{
	client = std::make_unique<dora::DoraClient>(configs);
}

void SHUTDOWN (void)
{
	client.reset();
	google::protobuf::ShutdownProtobufLibrary();
}

bool can_send (void)
{
	return nullptr != client;
}

void send (std::string testname, testify::GeneratedCase& gcase)
{
	if (nullptr == client)
	{
		throw std::runtime_error("retroc client not initialized");
	}

	if (false == client->add_testcase(testname, gcase))
	{
		// log
		cache(testname, gcase);
	}
}

}

#endif
