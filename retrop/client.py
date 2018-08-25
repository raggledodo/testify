''' Pass testcases to dora servers '''

import grpc

import proto.testify_pb2 as testify
import proto.testify_pb2_grpc as testify_grpc

_CLIENT = None

def init(host):
    global _CLIENT
    channel = grpc.insecure_channel(host)
    _CLIENT = testify_grpc.DoraStub(channel)

def send(testname, output):
    global _CLIENT
    if _CLIENT is None:
        raise "Client not initialized yet"
    tcase = testify.TransferCase()
    tcase.name = testname
    tcase.results.CopyFrom(output)
    _CLIENT.AddTestcase(tcase, timeout=5)

def stop():
    global _CLIENT
    _CLIENT = None
