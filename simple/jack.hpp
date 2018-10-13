#include <experimental/optional>

#include "gtest/gtest.h"

#include "retroc/generate.hpp"
#include "anteroc/testcase.hpp"

#ifndef SIMPLE_JACK_HPP
#define SIMPLE_JACK_HPP

using namespace std::experimental;

namespace simple
{

struct iSession
{
	virtual ~iSession (void) = default;

	virtual std::vector<double> get_double (std::string usage, size_t len,
		retro::Range<double> range = retro::Range<double>()) = 0;

	virtual std::vector<int32_t> get_int (std::string usage, size_t len,
		retro::Range<int32_t> range = retro::Range<int32_t>()) = 0;

	virtual int32_t get_scalar (std::string usage,
		retro::Range<int32_t> range = retro::Range<int32_t>()) = 0;

	virtual std::string get_string (std::string usage, size_t len) = 0;

	virtual std::vector<uint64_t> choose (std::string usage, uint64_t n, uint64_t k) = 0;


	virtual optional<std::vector<double>> expect_double (std::string usage) = 0;

	virtual optional<std::vector<int32_t>> expect_int (std::string usage) = 0;

	virtual optional<std::string> expect_string (std::string usage) = 0;


	virtual void store_double (std::string usage, std::vector<double> value) = 0;

	virtual void store_int (std::string usage, std::vector<int32_t> value) = 0;

	virtual void store_string (std::string usage, std::string value) = 0;

	virtual void clear (void) = 0;
};

using SessionT = std::unique_ptr<iSession>;

struct TestModel : public antero::Testament
{
	static bool GENERATE_MODE;

protected:

	void TearDown (void) override
	{
		if (::testing::Test::HasFailure())
		{
			active_->clear();
		}

		active_.reset();
	}

	SessionT get_session (std::string testname);

	SessionT active_;
};

void INIT (std::string server_addr, const char* certfile,
	bool genmode = false, size_t nretries = 3);

void SHUTDOWN (void);

}

#endif // SIMPLE_JACK_HPP
