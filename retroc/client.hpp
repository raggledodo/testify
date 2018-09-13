#include <grpc/grpc.h>

#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "proto/testify.pb.h"

#ifndef RETRO_CLIENT_HPP
#define RETRO_CLIENT_HPP

namespace retro
{

struct ClientConfig
{
    ClientConfig (void) {}

    size_t nretry = 0;
    size_t timeout = 3;
};

void INIT (std::string host,
    ClientConfig configs = ClientConfig());

void SHUTDOWN (void);

// send GeneratedCase to servers, return false if output has bad format
void send (std::string testname, testify::GeneratedCase& output);

}

#endif /* RETRO_CLIENT_HPP */
