''' Pass testcases to dora servers '''

import google.protobuf
import grpc

from google.protobuf import empty_pb2

import proto.testify_pb2 as testify
import proto.testify_pb2_grpc as testify_grpc

_CLIENT = None

def init(host, cert):
    global _CLIENT
    credentials = grpc.ssl_channel_credentials(cert)
    channel = grpc.secure_channel(host, credentials)
    _CLIENT = testify_grpc.DoraStub(channel)
    print(_CLIENT.CheckHealth(empty_pb2.Empty()))

def stop():
    global _CLIENT
    _CLIENT = None

def send(testname, output):
    global _CLIENT
    if _CLIENT is None:
        raise "Client not initialized yet"

    req = testify.AddRequest()
    req.name = testname
    req.payload.CopyFrom(output)
    _CLIENT.AddTestcase(req, timeout=5)
