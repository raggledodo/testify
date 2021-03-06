#include <thread>
#include <future>
#include <sstream>

#include <grpc/grpc.h>

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

#include "gtest/gtest.h"

#include "proto/testify.pb.h"
#include "proto/testify.grpc.pb.h"

#include "simple/jack.hpp"


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
std::condition_variable server_started;

// server-client comms
// generated inputs
static std::vector<double> dbs;
static std::vector<int32_t> its;
static int32_t scalar;
static std::string str;
static std::vector<uint64_t> choices;

// generated outputs
static std::vector<double> global_exdbs = {0.12, 33.2, 12};
static std::vector<int32_t> global_exlng = {12, 332, 112};
static std::string global_exstr = "dfq123p412m@sq#";

// expected outputs
static std::vector<double> dubs = {123.22, 252.3, 1.552};
static std::vector<int32_t> lngs = {1232, 2523, 1552};
static std::string strs = "1o29@34%sslw";

struct MockService final : public testify::Dora::Service
{
	grpc::Status ListTestcases (grpc::ServerContext*,
		const testify::ListRequest*, testify::ListResponse* res) override
	{
		testify::GeneratedTest test;
		testify::GeneratedCase* out = test.add_cases();
		auto& outputs = *out->mutable_outputs();
		testify::CaseData dcase;
		testify::CaseData icase;
		testify::CaseData strcase;
		testify::Doubles* dd = dcase.mutable_ddoubles();
		google::protobuf::RepeatedField<double> pb_dubs(dubs.begin(), dubs.end());
		dd->mutable_data()->Swap(&pb_dubs);
		testify::Int32s* di = icase.mutable_dint32s();
		google::protobuf::RepeatedField<int> pb_lngs(lngs.begin(), lngs.end());
		di->mutable_data()->Swap(&pb_lngs);
		testify::Bytes* dstr = strcase.mutable_dbytes();
		dstr->set_data(strs);

		outputs.insert({"expect_dbs", dcase});
		outputs.insert({"expect_its", icase});
		outputs.insert({"expect_str", strcase});

		auto testmap = res->mutable_tests();
		testmap->insert({"JACK::Send", test});
		return grpc::Status::OK;
	}

	grpc::Status AddTestcase (grpc::ServerContext*,
		const testify::AddRequest* tcase, google::protobuf::Empty*) override
	{
		std::string key = tcase->name();
		assert(0 == key.compare("JACK::Send"));

		testify::GeneratedCase gcase = tcase->payload();
		auto& inputs = *gcase.mutable_inputs();
		auto& outputs = *gcase.mutable_outputs();

		auto dbdata = inputs["dbs"];
		auto itsdata = inputs["its"];
		auto scalardata = inputs["scalar"];
		auto strdata = inputs["str"];

		assert(dbdata.has_ddoubles());
		assert(itsdata.has_dint32s());
		assert(scalardata.has_dint32s());
		assert(strdata.has_dbytes());

		testify::Doubles got_dbs = dbdata.ddoubles();
		testify::Int32s got_its = itsdata.dint32s();
		testify::Int32s got_sclr = scalardata.dint32s();
		testify::Bytes got_strs = strdata.dbytes();

		assert(std::equal(dbs.begin(), dbs.end(), got_dbs.data().begin()));
		assert(std::equal(its.begin(), its.end(), got_its.data().begin()));
		assert(scalar == got_sclr.data()[0] && 1 == got_sclr.data().size());
		assert(0 == str.compare(got_strs.data()));

		auto entry_dbdata = outputs["entry_double"];
		auto entry_lngdata = outputs["entry_int32_t"];
		auto entry_strdata = outputs["entry_string"];

		assert(entry_dbdata.has_ddoubles());
		assert(entry_lngdata.has_dint32s());
		assert(entry_strdata.has_dbytes());

		testify::Doubles got_entrydbs = entry_dbdata.ddoubles();
		testify::Int32s got_entryits = entry_lngdata.dint32s();
		testify::Bytes got_entrystrs = entry_strdata.dbytes();

		assert(std::equal(global_exdbs.begin(), global_exdbs.end(), got_entrydbs.data().begin()));
		assert(std::equal(global_exlng.begin(), global_exlng.end(), got_entryits.data().begin()));
		assert(0 == global_exstr.compare(got_entrystrs.data()));

		return grpc::Status::OK;
	}

	grpc::Status RemoveTestcase (grpc::ServerContext*,
		const testify::RemoveRequest*, google::protobuf::Empty*) override
	{
		return grpc::Status::OK;
	}

	grpc::Status CheckHealth (grpc::ServerContext* context,
		const google::protobuf::Empty*, testify::HealthCheckResponse* response) override
	{
		response->set_status(testify::HealthCheckResponse::SERVING);
		return grpc::Status::OK;
	}
};


class JACK : public simple::TestModel {};


void RunServer (void)
{
	std::string servercert = dora::read_keycert("certs/server.crt");
	std::string serverkey = dora::read_keycert("certs/server.key");

	grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp;
	pkcp.private_key = serverkey;
	pkcp.cert_chain = servercert;

	grpc::SslServerCredentialsOptions ssl_opts;
	ssl_opts.pem_key_cert_pairs.push_back(pkcp);
	auto creds = grpc::SslServerCredentials(ssl_opts);

	MockService service;
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
	size_t seed = std::time(nullptr);
	retro::get_engine().seed(seed);

	std::thread th(RunServer); // no recovery from bad server startup

	std::mutex mtx;
	std::unique_lock<std::mutex> lck(mtx);
	server_started.wait_for(lck, std::chrono::seconds(1));
	simple::INIT(server_addr, "certs/server.crt", true);

	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();

	simple::SHUTDOWN();

	server.reset(nullptr);
	th.join();

	return ret;
}


TEST_F(JACK, Send)
{
	simple::SessionT sess = get_session("JACK::Send");

	retro::Range<double> dbrange{-10.4, 45.2};
	retro::Range<int32_t> irange{-104, 452};
	retro::Range<int32_t> srange{-25, 55};
	dbs = sess->get_double("dbs", 10, dbrange);
	its = sess->get_int("its", 11, irange);
	scalar = sess->get_scalar("scalar", srange);
	str = sess->get_string("str", 12);
	choices = sess->choose("choices", 52, 13);

	EXPECT_EQ(10, dbs.size());
	EXPECT_EQ(11, its.size());
	EXPECT_EQ(12, str.size());
	EXPECT_EQ(13, choices.size());

	for (double d : dbs)
	{
		EXPECT_LE(dbrange.min_, d);
		EXPECT_GE(dbrange.max_, d);
	}

	for (int32_t l : its)
	{
		EXPECT_LE(irange.min_, l);
		EXPECT_GE(irange.max_, l);
	}

	for (uint64_t l : choices)
	{
		EXPECT_GT(52, l);
	}

	EXPECT_LE(srange.min_, scalar);
	EXPECT_GE(srange.max_, scalar);

	optional<std::vector<double>> exds = sess->expect_double("expect_dbs");
	optional<std::vector<int32_t>> exis = sess->expect_int("expect_its");
	optional<std::string> exstr = sess->expect_string("expect_str");

	ASSERT_TRUE((bool) exds);
	ASSERT_TRUE((bool) exis);
	ASSERT_TRUE((bool) exstr);

	std::vector<double> exdsvec = *exds;
	std::vector<int32_t> exisvec = *exis;
	std::string exstrvec = *exstr;

	EXPECT_ARREQ(dubs, exdsvec);
	EXPECT_ARREQ(lngs, exisvec);
	EXPECT_STREQ(strs.c_str(), exstrvec.c_str());

	sess->store_double("entry_double", global_exdbs);
	sess->store_int("entry_int32_t", global_exlng);
	sess->store_string("entry_string", global_exstr);
}
