#include <grpc++/grpc++.h>

#include "proto/testify.pb.h"
#include "proto/testify.grpc.pb.h"

#ifndef ANTERO_CLIENT_HPP
#define ANTERO_CLIENT_HPP

namespace antero
{

struct ClientConfig
{
	ClientConfig (void) {}

	size_t nretry = 0;
	size_t timeout = 3;
};

void INIT (std::string host, size_t nrepeats,
    ClientConfig configs = ClientConfig());

void SHUTDOWN (void);

std::vector<testify::GeneratedCase> get_cases (std::string tname);

}

#endif /* ANTERO_CLIENT_HPP */
