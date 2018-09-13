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
static const std::string server_addr = "0.0.0.0:50075";
static std::unique_ptr<grpc::Server> server;
std::condition_variable server_started;

// server-client comms
static std::string unknown_register;

// generated inputs
static std::vector<double> dbs;
static std::vector<int32_t> its;
static int32_t scalar;
static std::string str;

// generated outputs
static std::vector<double> exdbs = {0.12, 33.2, 12};
static std::vector<int32_t> exlng = {12, 332, 112};
static std::string exstr = "dfq123p412m@sq#";

// expected outputs
static std::vector<double> dubs = {123.22, 252.3, 1.552};
static std::vector<int32_t> lngs = {1232, 2523, 1552};
static std::string strs = "1o29@34%sslw";

struct MockService final : public testify::Dora::Service
{
	grpc::Status ListTestcases (grpc::ServerContext*,
		const google::protobuf::Empty*,
		grpc::ServerWriter<testify::TransferName>* writer) override
	{
		testify::TransferName name;
		name.set_name("JACK::Send");
		writer->Write(name);
		return grpc::Status::OK;
	}

	grpc::Status GetTestcase (grpc::ServerContext*,
		const testify::TransferName* name,
		grpc::ServerWriter<testify::GeneratedCase>* writer) override
	{
		std::string key = name->name();
		if (0 == key.compare("JACK::Send"))
		{
			testify::GeneratedCase out;
			auto& outputs = *out.mutable_outputs();
			testify::CaseData dcase;
			testify::CaseData icase;
			testify::CaseData strcase;
			testify::Doubles dd;
			google::protobuf::RepeatedField<double> pb_dubs(dubs.begin(), dubs.end());
			dd.mutable_data()->Swap(&pb_dubs);
			testify::Int32s di;
			google::protobuf::RepeatedField<int> pb_lngs(lngs.begin(), lngs.end());
			di.mutable_data()->Swap(&pb_lngs);
			testify::Bytes dstr;
			dstr.set_data(strs);

			dcase.set_dtype(testify::DOUBLES);
			dcase.mutable_data()->PackFrom(dd);
			icase.set_dtype(testify::INT32S);
			icase.mutable_data()->PackFrom(di);
			strcase.set_dtype(testify::BYTES);
			strcase.mutable_data()->PackFrom(dstr);

			outputs.insert({"expect_dbs", dcase});
			outputs.insert({"expect_its", icase});
			outputs.insert({"expect_str", strcase});
			writer->Write(out);
		}
		return grpc::Status::OK;
	}

	grpc::Status AddTestcase (grpc::ServerContext*,
		const testify::TransferCase* tcase, google::protobuf::Empty*) override
	{
		std::string key = tcase->name();
		assert(0 == key.compare("JACK::Send"));

		testify::GeneratedCase gcase = tcase->results();
		auto& inputs = *gcase.mutable_inputs();
		auto& outputs = *gcase.mutable_outputs();

		auto dbdata = inputs["dbs"];
		auto itsdata = inputs["its"];
		auto scalardata = inputs["scalar"];
		auto strdata = inputs["str"];

		assert(dbdata.dtype() == testify::DOUBLES);
		assert(itsdata.dtype() == testify::INT32S);
		assert(scalardata.dtype() == testify::INT32S);
		assert(strdata.dtype() == testify::BYTES);

		testify::Doubles got_dbs;
		testify::Int32s got_its;
		testify::Int32s got_sclr;
		testify::Bytes got_strs;
		dbdata.mutable_data()->UnpackTo(&got_dbs);
		itsdata.mutable_data()->UnpackTo(&got_its);
		scalardata.mutable_data()->UnpackTo(&got_sclr);
		strdata.mutable_data()->UnpackTo(&got_strs);

		assert(std::equal(dbs.begin(), dbs.end(), got_dbs.data().begin()));
		assert(std::equal(its.begin(), its.end(), got_its.data().begin()));
		assert(scalar == got_sclr.data()[0] && 1 == got_sclr.data().size());
		assert(0 == str.compare(got_strs.data()));

		auto entry_dbdata = outputs["entry_double"];
		auto entry_lngdata = outputs["entry_int32_t"];
		auto entry_strdata = outputs["entry_string"];

		assert(entry_dbdata.dtype() == testify::DOUBLES);
		assert(entry_lngdata.dtype() == testify::INT32S);
		assert(entry_strdata.dtype() == testify::BYTES);

		testify::Doubles got_entrydbs;
		testify::Int32s got_entryits;
		testify::Bytes got_entrystrs;
		entry_dbdata.mutable_data()->UnpackTo(&got_entrydbs);
		entry_lngdata.mutable_data()->UnpackTo(&got_entryits);
		entry_strdata.mutable_data()->UnpackTo(&got_entrystrs);

		assert(std::equal(exdbs.begin(), exdbs.end(), got_entrydbs.data().begin()));
		assert(std::equal(exlng.begin(), exlng.end(), got_entryits.data().begin()));
		assert(0 == exstr.compare(got_entrystrs.data()));

		return grpc::Status::OK;
	}

	grpc::Status RemoveTestcase (grpc::ServerContext*,
		const testify::TransferName*, google::protobuf::Empty*) override
	{
		return grpc::Status::OK;
	}
};


class JACK : public TestModel {};


void RunServer (void)
{
	MockService service;
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
	size_t seed = std::time(nullptr);
	get_engine().seed(seed);

	std::thread th(RunServer); // no recovery from bad server startup

	std::mutex mtx;
	std::unique_lock<std::mutex> lck(mtx);
	server_started.wait_for(lck, std::chrono::seconds(1));
	simple::INIT(server_addr, true);

	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();

	simple::SHUTDOWN();

	std::chrono::system_clock::time_point deadline =
		std::chrono::system_clock::now() + std::chrono::seconds(10);
	server->Shutdown(deadline);
	server.reset(nullptr);
	th.join();

	return ret;
}


TEST_F(JACK, Send)
{
	SESSION sess = get_session("JACK::Send");

	Range<double> dbrange{-10.4, 45.2};
	Range<int32_t> irange{-104, 452};
	Range<int32_t> srange{-25, 55};
	dbs = sess->get_double("dbs", 10, dbrange);
	its = sess->get_int("its", 11, irange);
	scalar = sess->get_scalar("scalar", srange);
	str = sess->get_string("str", 12);

	EXPECT_EQ(10, dbs.size());
	EXPECT_EQ(11, its.size());
	EXPECT_EQ(12, str.size());

	for (double d : dbs)
	{
		EXPECT_LT(dbrange.min_, d);
		EXPECT_GT(dbrange.max_, d);
	}

	for (int32_t l : its)
	{
		EXPECT_LT(irange.min_, l);
		EXPECT_GT(irange.max_, l);
	}

	EXPECT_LT(srange.min_, scalar);
	EXPECT_GT(srange.max_, scalar);

	std::vector<double> exds = sess->expect_double("expect_dbs");
	std::vector<int32_t> exis = sess->expect_int("expect_its");
	std::string exstr = sess->expect_string("expect_str");

	EXPECT_ARREQ(dubs, exds);
	EXPECT_ARREQ(lngs, exis);
	EXPECT_STREQ(strs.c_str(), exstr.c_str());

	sess->store_double("entry_double", exdbs);
	sess->store_int("entry_int32_t", exlng);
	sess->store_string("entry_string", exstr);
}
