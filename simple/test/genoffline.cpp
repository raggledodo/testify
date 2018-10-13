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

class JACK : public simple::TestModel {};


int main (int argc, char** argv)
{
	size_t seed = std::time(nullptr);
	retro::get_engine().seed(seed);

	simple::INIT(server_addr, "certs/server.crt", true);

	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();

	simple::SHUTDOWN();
	return ret;
}


TEST_F(JACK, Send)
{
	simple::SessionT sess = get_session("JACK::Send");

	retro::Range<double> dbrange{-10.4, 45.2};
	retro::Range<int32_t> irange{-104, 452};
	retro::Range<int32_t> srange{-25, 55};
	std::vector<double> dbs = sess->get_double("dbs", 10, dbrange);
	std::vector<int32_t> its = sess->get_int("its", 11, irange);
	int32_t scalar = sess->get_scalar("scalar", srange);
	std::string str = sess->get_string("str", 12);

	EXPECT_EQ(10, dbs.size());
	EXPECT_EQ(11, its.size());
	EXPECT_EQ(12, str.size());

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

	EXPECT_LE(srange.min_, scalar);
	EXPECT_GE(srange.max_, scalar);

	optional<std::vector<double>> exds = sess->expect_double("expect_dbs");
	optional<std::vector<int32_t>> exis = sess->expect_int("expect_its");
	optional<std::string> exstr = sess->expect_string("expect_str");

	EXPECT_FALSE((bool) exds);
	EXPECT_FALSE((bool) exis);
	EXPECT_FALSE((bool) exstr);

	sess->store_double("entry_double", {0.12, 33.2, 12});
	sess->store_int("entry_int32_t", {12, 332, 112});
	sess->store_string("entry_string", "dfq123p412m@sq#");
}
