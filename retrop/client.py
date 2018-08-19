''' Pass testcases to dora servers '''

import grpc

import proto.record_pb2 as record
import proto.record_pb2_grpc as record_grpc

_CLIENT = None

def init(host):
    global _CLIENT
    channel = grpc.insecure_channel(host)
    _CLIENT = record_grpc.DoraStub(channel)

def send(testname, output):
    global _CLIENT
    if _CLIENT is None:
        raise "Client not initialized yet"
    tcase = record.Testcase()
    tcase.name = testname
    tcase.results.CopyFrom(output)
    _CLIENT.AddTestcase(tcase, timeout=5)

def stop():
    global _CLIENT
    _CLIENT = None
