#include "client/client.hpp"

#ifdef DORA_CLIENT_HPP

namespace dora
{

bool is_reachable (grpc::Status status)
{
	auto ecode = status.error_code();
	return ecode != grpc::StatusCode::DEADLINE_EXCEEDED &&
		ecode != grpc::StatusCode::UNAVAILABLE;
}

void log_status (grpc::Status status)
{
	bool good = status.ok();
	if (false == good)
	{
		std::cerr << status.error_code() << ":" << status.error_message() << std::endl;
	}
}

std::string read_keycert (const char* filename)
{
	std::stringstream ss;
	std::ifstream fs(filename);
    if (fs)
    {
        ss << fs.rdbuf();
        fs.close();
    }
    return ss.str();
}

}

#endif
