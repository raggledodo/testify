#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "proto/testdata.pb.h"

#ifndef CLIENT_HPP
#define CLIENT_HPP

struct ClientConfig
{
    ClientConfig (void) {}

    size_t nretry = 0;
    size_t timeout = 3;
};

void RETRO_INIT (std::string host,
    ClientConfig configs = ClientConfig());

void RETRO_SHUTDOWN (void);

// send testoutput to servers, return false if output has bad format
bool send (std::string testname, testify::TestOutput& output);

#endif /* CLIENT_HPP */
