#include <thread>
#include <future>
#include <sstream>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

#include "gtest/gtest.h"

#include "proto/record.pb.h"
#include "proto/record.grpc.pb.h"

#include "retroc/generate.hpp"
#include "retroc/rand.hpp"


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
static std::string latest_name;
static testify::TestOutput latest_output;
std::condition_variable latest_update;


struct MockService final : public testify::Dora::Service
{
	grpc::Status AddTestcase (grpc::ServerContext* context,
		const testify::Testcase* tcase, testify::Nothing* nothing) override
	{
		latest_name = tcase->name();
		latest_output = tcase->results();
		latest_update.notify_one();
		return grpc::Status::OK;
	}

	grpc::Status ListTestcases (grpc::ServerContext* context,
		const testify::Nothing* nothing,
		grpc::ServerWriter<testify::Testname>* writer) override
	{
		return grpc::Status::OK;
	}

	grpc::Status GetTestcase (grpc::ServerContext* context,
		const testify::Testname* name,
		grpc::ServerWriter<testify::TestOutput>* nothing) override
	{
		return grpc::Status::OK;
	}
};


class CLIENT : public ::testing::Test
{
protected:
	virtual void SetUp (void) {}

	virtual void TearDown (void)
	{
		latest_name = "";
		latest_output.Clear();
	}
};


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
	RETRO_INIT(server_addr); // no retries

	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();

	RETRO_SHUTDOWN();

	std::chrono::system_clock::time_point deadline =
		std::chrono::system_clock::now() + std::chrono::seconds(10);
	server->Shutdown(deadline);
	server.reset(nullptr);
	th.join();

	return ret;
}


TEST_F(CLIENT, CallVecs)
{
	std::string expect_label = "vecs";
	GenIO io(expect_label);
	auto vec0 = io.get_vec<double>("double_vec", 5, Range<double>(-10.2, 24.2));
	auto vec1 = io.get_vec<float>("float_vec", 7, Range<float>(-235, 252));
	auto vec2 = io.get_vec<int32_t>("int32_vec", 3, Range<int32_t>(-100, 252));
	auto vec3 = io.get_vec<uint32_t>("uint32_vec", 2, Range<uint32_t>(100, 252));
	auto vec4 = io.get_vec<int64_t>("int64_vec", 11, Range<int64_t>(-100, 252));
	auto vec5 = io.get_vec<uint64_t>("uint64_vec", 13, Range<uint64_t>(100, 252));

	std::vector<uint8_t> out = {4, 12, 2, 19};
	io.set_output(out.begin(), out.end());

	std::mutex mtx;
	std::unique_lock<std::mutex> lck(mtx);
	latest_update.wait_for(lck,std::chrono::seconds(3));

	EXPECT_STREQ(expect_label.c_str(), latest_name.c_str());

	auto inputs = latest_output.inputs();
	ASSERT_EQ(6, inputs.size());

	EXPECT_STREQ("double_vec", inputs[0].usage().c_str());
	ASSERT_EQ(testify::DTYPE::DOUBLES, inputs[0].dtype());
	testify::Doubles ds;
	inputs[0].data().UnpackTo(&ds);
	auto d = ds.data();
	EXPECT_ARREQ(vec0, d);

	EXPECT_STREQ("float_vec", inputs[1].usage().c_str());
	ASSERT_EQ(testify::DTYPE::FLOATS, inputs[1].dtype());
	testify::Floats fs;
	inputs[1].data().UnpackTo(&fs);
	auto f = fs.data();
	EXPECT_ARREQ(vec1, f);

	EXPECT_STREQ("int32_vec", inputs[2].usage().c_str());
	ASSERT_EQ(testify::DTYPE::INT32S, inputs[2].dtype());
	testify::Int32s i32s;
	inputs[2].data().UnpackTo(&i32s);
	auto i32 = i32s.data();
	EXPECT_ARREQ(vec2, i32);

	EXPECT_STREQ("uint32_vec", inputs[3].usage().c_str());
	ASSERT_EQ(testify::DTYPE::UINT32S, inputs[3].dtype());
	testify::Uint32s u32s;
	inputs[3].data().UnpackTo(&u32s);
	auto u32 = u32s.data();
	EXPECT_ARREQ(vec3, u32);

	EXPECT_STREQ("int64_vec", inputs[4].usage().c_str());
	ASSERT_EQ(testify::DTYPE::INT64S, inputs[4].dtype());
	testify::Int64s i64s;
	inputs[4].data().UnpackTo(&i64s);
	auto i64 = i64s.data();
	EXPECT_ARREQ(vec4, i64);

	EXPECT_STREQ("uint64_vec", inputs[5].usage().c_str());
	ASSERT_EQ(testify::DTYPE::UINT64S, inputs[5].dtype());
	testify::Uint64s u64s;
	inputs[5].data().UnpackTo(&u64s);
	auto u64 = u64s.data();
	EXPECT_ARREQ(vec5, u64);

	ASSERT_EQ(testify::DTYPE::BYTES, latest_output.dtype());
	testify::Bytes outbytes;
	latest_output.output().UnpackTo(&outbytes);
	std::string outstr = outbytes.data();
	const char* optr = outstr.c_str();
	std::vector<uint8_t> outvec(optr, optr + outstr.size());
	EXPECT_ARREQ(out, outvec);
}


TEST_F(CLIENT, CallStr)
{
	std::string expect_label = "strings";
	GenIO io(expect_label);
	std::string instr = io.get_string("simplestring", 13);

	std::vector<uint8_t> out = {5, 13, 3, 20};
	io.set_output(out.begin(), out.end());

	std::mutex mtx;
	std::unique_lock<std::mutex> lck(mtx);
	latest_update.wait_for(lck,std::chrono::seconds(3));

	EXPECT_STREQ(expect_label.c_str(), latest_name.c_str());

	auto inputs = latest_output.inputs();
	ASSERT_EQ(1, inputs.size());

	EXPECT_STREQ("simplestring", inputs[0].usage().c_str());
	ASSERT_EQ(testify::DTYPE::BYTES, inputs[0].dtype());
	testify::Bytes bs;
	inputs[0].data().UnpackTo(&bs);
	std::string b = bs.data();
	EXPECT_STREQ(instr.c_str(), b.c_str());

	ASSERT_EQ(testify::DTYPE::BYTES, latest_output.dtype());
	testify::Bytes outbytes;
	latest_output.output().UnpackTo(&outbytes);
	std::string outstr = outbytes.data();
	const char* optr = outstr.c_str();
	std::vector<uint8_t> outvec(optr, optr + outstr.size());
	EXPECT_ARREQ(out, outvec);
}


TEST_F(CLIENT, CallChoice)
{
	std::string expect_label = "choices";
	GenIO io(expect_label);
	std::vector<uint64_t> carr = io.choose("choosevec", 19, 6);
	auto it = io.select("carr", carr.begin(), carr.end());
	size_t dist = std::distance(carr.begin(), it);

	std::vector<uint8_t> out = {6, 14, 4, 21};
	io.set_output(out.begin(), out.end());

	std::mutex mtx;
	std::unique_lock<std::mutex> lck(mtx);
	latest_update.wait_for(lck,std::chrono::seconds(3));

	EXPECT_STREQ(expect_label.c_str(), latest_name.c_str());

	auto inputs = latest_output.inputs();
	ASSERT_EQ(2, inputs.size());

	EXPECT_STREQ("choosevec", inputs[0].usage().c_str());
	ASSERT_EQ(testify::DTYPE::UINT64S, inputs[0].dtype());
	testify::Uint64s u64s;
	inputs[0].data().UnpackTo(&u64s);
	auto u64 = u64s.data();
	EXPECT_ARREQ(carr, u64);

	EXPECT_STREQ("select_carr", inputs[1].usage().c_str());
	ASSERT_EQ(testify::DTYPE::UINT64S, inputs[1].dtype());
	testify::Uint64s ss;
	inputs[1].data().UnpackTo(&ss);
	auto s = ss.data();
	ASSERT_EQ(1, s.size());
	EXPECT_EQ(dist, s[0]);

	ASSERT_EQ(testify::DTYPE::BYTES, latest_output.dtype());
	testify::Bytes outbytes;
	latest_output.output().UnpackTo(&outbytes);
	std::string outstr = outbytes.data();
	const char* optr = outstr.c_str();
	std::vector<uint8_t> outvec(optr, optr + outstr.size());
	EXPECT_ARREQ(out, outvec);
}


TEST_F(CLIENT, CallTree)
{
	std::string expect_label = "tree";
	GenIO io(expect_label);

	Graph<13> tree;
	size_t root = io.get_minspan_tree("mstree", tree);

	io.set_outtree(root, tree);

	std::mutex mtx;
	std::unique_lock<std::mutex> lck(mtx);
	latest_update.wait_for(lck,std::chrono::seconds(3));

	EXPECT_STREQ(expect_label.c_str(), latest_name.c_str());

	auto inputs = latest_output.inputs();
	ASSERT_EQ(1, inputs.size());

	EXPECT_STREQ("mstree", inputs[0].usage().c_str());
	ASSERT_EQ(testify::DTYPE::NTREE, inputs[0].dtype());
	ASSERT_EQ(testify::DTYPE::NTREE, latest_output.dtype());

	testify::Tree ntree;
	inputs[0].data().UnpackTo(&ntree);
	EXPECT_EQ(root, ntree.root());
	ASSERT_EQ(13, ntree.graph().nverts());

	testify::Tree otree;
	latest_output.output().UnpackTo(&otree);
	EXPECT_EQ(root, otree.root());
	ASSERT_EQ(13, otree.graph().nverts());

	std::string encoding = ntree.graph().matrix();
	EXPECT_EQ(22, encoding.size()); // ceil(13 * 13 / 8)

	std::string outcoding = otree.graph().matrix();
	EXPECT_EQ(22, outcoding.size()); // ceil(13 * 13 / 8)

	for (uint y = 0; y < 13; ++y)
	{
		for (uint x = 0; x < 13; ++x)
		{
			uint ibit = x + y * 13;
			bool set = 1 & (encoding[ibit / 8] >> (ibit % 8));
			EXPECT_EQ(tree.get(y, x), set) << "bit " << ibit;

			bool oset = 1 & (outcoding[ibit / 8] >> (ibit % 8));
			EXPECT_EQ(tree.get(y, x), oset) << "bit " << ibit;
		}
	}
}


TEST_F(CLIENT, CallGraph)
{
	std::string expect_label = "graph";
	GenIO io(expect_label);

	Graph<17> graph;
	io.get_graph("reggraph", graph);

	io.set_outgraph(graph);

	std::mutex mtx;
	std::unique_lock<std::mutex> lck(mtx);
	latest_update.wait_for(lck,std::chrono::seconds(3));

	EXPECT_STREQ(expect_label.c_str(), latest_name.c_str());

	auto inputs = latest_output.inputs();
	ASSERT_EQ(1, inputs.size());

	EXPECT_STREQ("reggraph", inputs[0].usage().c_str());
	ASSERT_EQ(testify::DTYPE::GRAPH, inputs[0].dtype());
	ASSERT_EQ(testify::DTYPE::GRAPH, latest_output.dtype());

	testify::Graph rgraph;
	inputs[0].data().UnpackTo(&rgraph);
	ASSERT_EQ(17, rgraph.nverts());

	testify::Graph ograph;
	latest_output.output().UnpackTo(&ograph);
	ASSERT_EQ(17, ograph.nverts());

	std::string encoding = rgraph.matrix();
	EXPECT_EQ(37, encoding.size()); // ceil(17 * 17 / 8)

	std::string outcoding = ograph.matrix();
	EXPECT_EQ(37, outcoding.size()); // ceil(17 * 17 / 8)

	for (uint y = 0; y < 17; ++y)
	{
		for (uint x = 0; x < 17; ++x)
		{
			uint ibit = x + y * 17;
			bool set = 1 & (encoding[ibit / 8] >> (ibit % 8));
			EXPECT_EQ(graph.get(y, x), set) << "bit " << ibit;

			bool oset = 1 & (outcoding[ibit / 8] >> (ibit % 8));
			EXPECT_EQ(graph.get(y, x), oset) << "bit " << ibit;
		}
	}
}


TEST_F(CLIENT, CallCGraph)
{
	std::string expect_label = "cgraph";
	GenIO io(expect_label);

	Graph<23> graph;
	io.get_conn_graph("congraph", graph);

	io.set_outgraph(graph);

	std::mutex mtx;
	std::unique_lock<std::mutex> lck(mtx);
	latest_update.wait_for(lck,std::chrono::seconds(3));

	EXPECT_STREQ(expect_label.c_str(), latest_name.c_str());

	auto inputs = latest_output.inputs();
	ASSERT_EQ(1, inputs.size());

	EXPECT_STREQ("congraph", inputs[0].usage().c_str());
	ASSERT_EQ(testify::DTYPE::GRAPH, inputs[0].dtype());
	ASSERT_EQ(testify::DTYPE::GRAPH, latest_output.dtype());

	testify::Graph cgraph;
	inputs[0].data().UnpackTo(&cgraph);
	ASSERT_EQ(23, cgraph.nverts());

	testify::Graph ograph;
	latest_output.output().UnpackTo(&ograph);
	ASSERT_EQ(23, ograph.nverts());

	std::string encoding = cgraph.matrix();
	EXPECT_EQ(67, encoding.size()); // ceil(23 * 23 / 8)

	std::string outcoding = ograph.matrix();
	EXPECT_EQ(67, outcoding.size()); // ceil(23 * 23 / 8)

	for (uint y = 0; y < 23; ++y)
	{
		for (uint x = 0; x < 23; ++x)
		{
			uint ibit = x + y * 23;
			bool set = 1 & (encoding[ibit / 8] >> (ibit % 8));
			EXPECT_EQ(graph.get(y, x), set) << "bit " << ibit;

			bool oset = 1 & (outcoding[ibit / 8] >> (ibit % 8));
			EXPECT_EQ(graph.get(y, x), oset) << "bit " << ibit;
		}
	}
}
