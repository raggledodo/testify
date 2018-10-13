#include <grpc++/grpc++.h>

#include "client/client.hpp"

#ifndef ANTERO_CLIENT_HPP
#define ANTERO_CLIENT_HPP

namespace antero
{

void INIT (ClientConfig configs = ClientConfig());

void SHUTDOWN (void);

bool get_cases (std::vector<testify::GeneratedCase>& out, std::string tname);

}

#endif /* ANTERO_CLIENT_HPP */
