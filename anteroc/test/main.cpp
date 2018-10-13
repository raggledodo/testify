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
static const std::string server_addr = "localhost:50075";
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
		testify::TestStorage storage;
		if (!storage.ParseFromIstream(&input))
		{
			throw std::runtime_error("failed to parse storage");
		}
		dmap_ = storage.storage();
		std::cout << dmap_.size() << std::endl;
	}

	grpc::Status ListTestcases (grpc::ServerContext* context,
		const testify::ListRequest* request, testify::ListResponse* response) override
	{
		auto testnames = request->test_names();
		auto testouts = response->mutable_tests();
		if (testnames.size() > 0)
		{
			for (auto testname : testnames)
			{
				auto test = dmap_[testname];
				testouts->insert({testname, test});
			}
		}
		else
		{
			for (auto& dpair : dmap_)
			{
				testouts->insert(dpair);
			}
		}
		return grpc::Status::OK;
	}

	grpc::Status AddTestcase (grpc::ServerContext* context,
		const testify::AddRequest* request, ::google::protobuf::Empty*) override
	{
		return grpc::Status::OK;
	}

	grpc::Status RemoveTestcase (grpc::ServerContext* context,
		const testify::RemoveRequest*, ::google::protobuf::Empty*) override
	{
		return grpc::Status::OK;
	}

	grpc::Status CheckHealth (grpc::ServerContext* context,
		const google::protobuf::Empty*, testify::HealthCheckResponse* response) override
	{
		response->set_status(testify::HealthCheckResponse::SERVING);
		return grpc::Status::OK;
	}

	google::protobuf::Map<std::string,testify::GeneratedTest> dmap_;
};


static MockService service;


void RunServer (void)
{
	std::string servercert = read_keycert("certs/server.crt");
	std::string serverkey = read_keycert("certs/server.key");

	grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp;
	pkcp.private_key = serverkey;
	pkcp.cert_chain = servercert;

	grpc::SslServerCredentialsOptions ssl_opts;
	ssl_opts.pem_key_cert_pairs.push_back(pkcp);
	auto creds = grpc::SslServerCredentials(ssl_opts);

	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_addr, creds);
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

	ClientConfig cfg;
	size_t grab_ncases;
	cfg.host = server_addr;
	cfg.cert = read_keycert("certs/server.crt");
	antero::INIT(cfg);

	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();

	antero::SHUTDOWN();

	server.reset(nullptr);
	th.join();

	return ret;
}


class SAMPLE : public Testament {};


void EXPECT_DESCRIBED_EQ (testify::CaseData& expect, testify::CaseData& got)
{
	DTYPE dtype = expect.data_case();
	ASSERT_EQ(dtype, got.data_case());
	switch (dtype)
	{
		case DTYPE::kDdoubles:
		{
			const testify::Doubles& edbs = expect.ddoubles();
			const testify::Doubles& gdbs = got.ddoubles();
			auto evec = edbs.data();
			auto gvec = gdbs.data();
			EXPECT_ARREQ(evec, gvec);
		}
		break;
		case DTYPE::kDint64S:
		{
			const testify::Int64s& eits = expect.dint64s();
			const testify::Int64s& gits = got.dint64s();
			auto evec = eits.data();
			auto gvec = gits.data();
			EXPECT_ARREQ(evec, gvec);
		}
		break;
		case DTYPE::kDbytes:
		{
			const testify::Bytes& eits = expect.dbytes();
			const testify::Bytes& gits = got.dbytes();
			auto evec = eits.data();
			auto gvec = gits.data();
			EXPECT_STREQ(evec.c_str(), gvec.c_str());
		}
		break;
		default:
			std::cout << "unexpected type: " << dtype << " too lazy to implement" << std::endl;
	}
}


void EXPECT_GCASE_EQ (testify::GeneratedCase& expect, testify::GeneratedCase& got)
{
	auto einputs = expect.inputs();
	auto ginputs = got.inputs();
	ASSERT_EQ(einputs.size(), ginputs.size());
	for (auto epair : einputs)
	{
		EXPECT_DESCRIBED_EQ(epair.second, ginputs[epair.first]);
	}

	auto eoutputs = expect.outputs();
	auto goutputs = got.outputs();
	ASSERT_EQ(eoutputs.size(), goutputs.size());
	for (auto epair : eoutputs)
	{
		EXPECT_DESCRIBED_EQ(epair.second, goutputs[epair.first]);
	}
}


TEST_F(SAMPLE, Sample1)
{
	std::string tname = "sample1";
	auto expect = service.dmap_[tname].cases()[0];
	// auto got = get(tname);

	// EXPECT_GCASE_EQ(expect, got);
}


// TEST_F(SAMPLE, Sample2)
// {
// 	std::string tname = "sample2";
// 	auto expect = service.dmap_[tname].cases()[0];
// 	auto got = get(tname);

// 	EXPECT_GCASE_EQ(expect, got);
// }


// TEST_F(SAMPLE, Sample3)
// {
// 	std::string tname = "sample3";
// 	auto expect = service.dmap_[tname].cases()[0];
// 	auto got = get(tname);

// 	EXPECT_GCASE_EQ(expect, got);
// }


// TEST_F(SAMPLE, Sample4)
// {
// 	std::string tname = "sample4";
// 	auto expect = service.dmap_[tname].cases()[0];
// 	auto got = get(tname);

// 	EXPECT_GCASE_EQ(expect, got);
// }


// TEST_F(SAMPLE, Sample5)
// {
// 	EXPECT_THROW(get("sample5"), std::runtime_error);
// }
