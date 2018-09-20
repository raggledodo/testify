#include <grpc++/grpc++.h>

#include "client/client.hpp"

#ifndef ANTERO_CLIENT_HPP
#define ANTERO_CLIENT_HPP

namespace antero
{

void INIT (size_t grab_ncases = 50, ClientConfig configs = ClientConfig());

void SHUTDOWN (void);

optional<std::vector<testify::GeneratedCase>> get_cases (std::string tname);

}

#endif /* ANTERO_CLIENT_HPP */
