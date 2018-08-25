#include <thread>
#include <future>
#include <sstream>
#include <fstream>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

// #include "gtest/gtest.h"

#include "proto/testify.pb.h"
#include "proto/testify.grpc.pb.h"

#include "anteroc/testcase.hpp"
#include "anteroc/client.hpp"


template <typename Iterator>
std::string to_str (Iterator begin, Iterator end)
{
	std::stringstream ss;
	ss << "[";
	if (begin != end)
	{
		ss << *begin;
	}
	for (++begin; begin != end; ++begin)
	{
		ss << "," << *begin;
	}
	ss << "]";
	return ss.str();
}


#define EXPECT_ARREQ(arr1, arr2)\
EXPECT_TRUE(std::equal(arr1.begin(), arr1.end(), arr2.begin()))\
	<< "arrays " << to_str(arr1.begin(), arr1.end()) << ","\
	<< to_str(arr2.begin(), arr2.end()) << " are not equal"

// server thread info
static const std::string server_addr = "0.0.0.0:50075";
static std::unique_ptr<grpc::Server> server;
static std::condition_variable server_started;


struct MockService final : public testify::Dora::Service
{
	MockService (void)
	{
		std::fstream input("sample/sample.pb", std::ios::in | std::ios::binary);
		if (!input)
		{
			throw std::runtime_error("sample file not found");
		}
		testify::CaseStorage storage;
		if (!storage.ParseFromIstream(&input))
		{
			throw std::runtime_error("failed to parse storage");
		}
		dmap_ = storage.storage();
	}

	grpc::Status AddTestcase (grpc::ServerContext* context,
		const testify::TransferCase* tcase, google::protobuf::Empty* nothing) override
	{
		return grpc::Status::OK;
	}

	grpc::Status RemoveTestcase (grpc::ServerContext* context,
		const testify::TransferName* tname, google::protobuf::Empty* nothing) override
	{
		return grpc::Status::OK;
	}

	grpc::Status ListTestcases (grpc::ServerContext* context,
		const google::protobuf::Empty* nothing,
		grpc::ServerWriter<testify::TransferName>* writer) override
	{
		for (auto& dpair : dmap_)
		{
			testify::TransferName tname;
			tname.set_name(dpair.first);
			writer->Write(tname);
		}
		return grpc::Status::OK;
	}

	grpc::Status GetTestcase (grpc::ServerContext* context,
		const testify::TransferName* name,
		grpc::ServerWriter<testify::GeneratedCase>* writer) override
	{
		if (name == nullptr)
		{
			return grpc::Status(grpc::UNKNOWN, "passed null name");
		}
		auto it = dmap_.find(name->name());
		if (dmap_.end() == it)
		{
			return grpc::Status(grpc::INTERNAL, "requested name not in db");
		}
		testify::Cases& gcase = it->second;
		auto& cases = gcase.cases();
		for (const testify::GeneratedCase& out : cases)
		{
			writer->Write(out);
		}
		return grpc::Status::OK;
	}

	google::protobuf::Map<std::string,testify::Cases> dmap_;
};


static MockService service;


void RunServer (void)
{
	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	server = builder.BuildAndStart();
	if (nullptr == server)
	{
		throw std::runtime_error("Failed to start server");
	}

	server_started.notify_one();
	server->Wait();
}


int main (int argc, char** argv)
{
	std::thread th(RunServer); // no recovery from bad server startup

	std::mutex mtx;
	std::unique_lock<std::mutex> lck(mtx);
	server_started.wait_for(lck,std::chrono::seconds(1));

	size_t nreps;
	char* nrepeats = getenv("GTEST_REPEAT");
	if (nrepeats == nullptr)
	{
		nreps = 100;
	}
	else
	{
		nreps = atoi(nrepeats);
	}
	ANTERO_INIT(server_addr, nreps); // no retries

	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();

	ANTERO_SHUTDOWN();

	std::chrono::system_clock::time_point deadline =
		std::chrono::system_clock::now() + std::chrono::seconds(10);
	server->Shutdown(deadline);
	server.reset(nullptr);
	th.join();

	return ret;
}

class SAMPLE : public testament {};

void EXPECT_ANY_EQ (testify::DTYPE type,
	const google::protobuf::Any& expect, const google::protobuf::Any& got)
{
	switch (type)
	{
		case testify::DOUBLES:
		{
			testify::Doubles edbs;
			testify::Doubles gdbs;
			expect.UnpackTo(&edbs);
			got.UnpackTo(&gdbs);
			auto evec = edbs.data();
			auto gvec = gdbs.data();
			EXPECT_ARREQ(evec, gvec);
		}
		break;
		case testify::INT64S:
		{
			testify::Int64s eits;
			testify::Int64s gits;
			expect.UnpackTo(&eits);
			got.UnpackTo(&gits);
			auto evec = eits.data();
			auto gvec = gits.data();
			EXPECT_ARREQ(evec, gvec);
		}
		break;
		case testify::BYTES:
		{
			testify::Bytes eits;
			testify::Bytes gits;
			expect.UnpackTo(&eits);
			got.UnpackTo(&gits);
			auto evec = eits.data();
			auto gvec = gits.data();
			EXPECT_STREQ(evec.c_str(), gvec.c_str());
		}
		break;
		default:
			std::cout << "unexpected type: " << type << " too lazy to implement" << std::endl;
	}
}

void EXPECT_DESCRIBED_EQ (testify::DescribedData& expect, testify::DescribedData& got)
{
	EXPECT_STREQ(expect.usage().c_str(), got.usage().c_str());

	testify::DTYPE dtype = expect.dtype();
	ASSERT_EQ(dtype, got.dtype());
	EXPECT_ANY_EQ(dtype, expect.data(), got.data());
}

void EXPECT_GCASE_EQ (testify::GeneratedCase& expect, testify::GeneratedCase& got)
{
	auto einputs = expect.inputs();
	auto ginputs = got.inputs();
	ASSERT_EQ(einputs.size(), ginputs.size());
	for (size_t i = 0, n = einputs.size(); i < n; ++i)
	{
		EXPECT_DESCRIBED_EQ(einputs[i], ginputs[i]);
	}

	auto eoutputs = expect.outputs();
	auto goutputs = got.outputs();
	ASSERT_EQ(eoutputs.size(), goutputs.size());
	for (size_t i = 0, n = eoutputs.size(); i < n; ++i)
	{
		EXPECT_DESCRIBED_EQ(eoutputs[i], goutputs[i]);
	}
}

TEST_F(SAMPLE, sample1)
{
	std::string tname = "sample1";
	auto expect = service.dmap_[tname].cases()[0];
	auto got = get(tname);

	EXPECT_GCASE_EQ(expect, got);
}

TEST_F(SAMPLE, sample2)
{
	std::string tname = "sample2";
	auto expect = service.dmap_[tname].cases()[0];
	auto got = get(tname);

	EXPECT_GCASE_EQ(expect, got);
}

TEST_F(SAMPLE, sample3)
{
	std::string tname = "sample3";
	auto expect = service.dmap_[tname].cases()[0];
	auto got = get(tname);

	EXPECT_GCASE_EQ(expect, got);
}

TEST_F(SAMPLE, sample4)
{
	std::string tname = "sample4";
	auto expect = service.dmap_[tname].cases()[0];
	auto got = get(tname);

	EXPECT_GCASE_EQ(expect, got);
}

TEST_F(SAMPLE, sample5)
{
	EXPECT_THROW(get("sample5"), std::exception);
}
