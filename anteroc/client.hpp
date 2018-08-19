#include <grpcpp/grpcpp.h>

#include "proto/record.pb.h"
#include "proto/record.grpc.pb.h"

#ifndef CLIENT_HPP
#define CLIENT_HPP

struct ClientConfig
{
	ClientConfig (void) {}

	size_t nretry = 0;
	size_t timeout = 3;
};

void ANTERO_INIT (std::string host, size_t nrepeats,
    ClientConfig configs = ClientConfig());

void ANTERO_SHUTDOWN (void);

std::vector<testify::TestOutput> get_outputs (std::string tname);

#endif /* CLIENT_HPP */
