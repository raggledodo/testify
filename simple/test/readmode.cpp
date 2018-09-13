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
static std::vector<double> dbs = {221.1, 234.21, 274.2};
static std::vector<int32_t> its = {11112, 7742, 90322, 43};
static int32_t scalar = 7059432;
static std::string input_str = "1123SSSS1\\*^";

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
            auto& inputs = *(out.mutable_inputs());
            testify::CaseData case_dbs;
            testify::CaseData case_its;
            testify::CaseData case_scalar;
            testify::CaseData case_str;
			testify::Doubles input_dd;
			google::protobuf::RepeatedField<double> input_pb_dubs(dbs.begin(), dbs.end());
			input_dd.mutable_data()->Swap(&input_pb_dubs);
			testify::Int32s input_di;
			google::protobuf::RepeatedField<int> input_pb_lngs(its.begin(), its.end());
			input_di.mutable_data()->Swap(&input_pb_lngs);
			testify::Int32s input_scalar;
			input_scalar.add_data(scalar);
			testify::Bytes input_dstr;
			input_dstr.set_data(input_str);

			case_dbs.set_dtype(testify::DOUBLES);
			case_dbs.mutable_data()->PackFrom(input_dd);
			case_its.set_dtype(testify::INT32S);
			case_its.mutable_data()->PackFrom(input_di);
			case_scalar.set_dtype(testify::INT32S);
			case_scalar.mutable_data()->PackFrom(input_scalar);
			case_str.set_dtype(testify::BYTES);
			case_str.mutable_data()->PackFrom(input_dstr);

			inputs.insert({"dbs", case_dbs});
			inputs.insert({"its", case_its});
			inputs.insert({"scalar", case_scalar});
			inputs.insert({"str", case_str});

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
        assert(false);
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
	server_started.wait_for(lck,std::chrono::seconds(1));
	simple::INIT(server_addr, false);

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

	Range<double> dbrange{-10.4, 453.2};
	Range<int32_t> irange{-104, 452242};
	Range<int32_t> srange{-25, 55222222};
	std::vector<double> local_dbs = sess->get_double("dbs", 3, dbrange);
	std::vector<int32_t> local_its = sess->get_int("its", 4, irange);
	int32_t local_scalar = sess->get_scalar("scalar", srange);
	std::string local_str = sess->get_string("str", 12);

	EXPECT_ARREQ(dbs, local_dbs);
	EXPECT_ARREQ(its, local_its);
	EXPECT_EQ(scalar, local_scalar);
	EXPECT_STREQ(input_str.c_str(), local_str.c_str());

	std::vector<double> exds = sess->expect_double("expect_dbs");
	std::vector<int32_t> exis = sess->expect_int("expect_its");
	std::string exstr = sess->expect_string("expect_str");

	EXPECT_ARREQ(dubs, exds);
	EXPECT_ARREQ(lngs, exis);
	EXPECT_STREQ(strs.c_str(), exstr.c_str());

    static std::vector<double> entry_exdbs = {0.12, 33.2, 12};
    static std::vector<int32_t> entry_exlng = {12, 332, 112};
    static std::string entry_exstr = "dfq123p412m@sq#";

	sess->store_double("entry_double", entry_exdbs);
	sess->store_int("entry_int32_t", entry_exlng);
	sess->store_string("entry_string", entry_exstr);
}
