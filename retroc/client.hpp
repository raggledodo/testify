#include "client/client.hpp"

#ifndef RETRO_CLIENT_HPP
#define RETRO_CLIENT_HPP

namespace retro
{

void INIT (ClientConfig configs = ClientConfig());

void SHUTDOWN (void);

bool can_send (void);

// send GeneratedCase to servers, return false if output has bad format
void send (std::string testname, testify::GeneratedCase& output);

}

#endif /* RETRO_CLIENT_HPP */
