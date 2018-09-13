#include "gtest/gtest.h"

#include "retroc/generate.hpp"
#include "anteroc/testcase.hpp"

struct iSession
{
	virtual std::vector<double> get_double (std::string usage, size_t len,
		Range<double> range = Range<double>()) = 0;

	virtual std::vector<int32_t> get_int (std::string usage, size_t len,
		Range<int32_t> range = Range<int32_t>()) = 0;

	virtual int32_t get_scalar (std::string usage, Range<int32_t> range = Range<int32_t>()) = 0;

	virtual std::string get_string (std::string usage, size_t len) = 0;


	virtual std::vector<double> expect_double (std::string usage) = 0;

	virtual std::vector<int32_t> expect_int (std::string usage) = 0;

	virtual std::string expect_string (std::string usage) = 0;


	virtual void store_double (std::string usage, std::vector<double> value) = 0;

	virtual void store_int (std::string usage, std::vector<int32_t> value) = 0;

	virtual void store_string (std::string usage, std::string value) = 0;

	virtual void clear (void) = 0;
};

using SESSION = std::unique_ptr<iSession>;

struct TestModel : public Testament
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

	SESSION get_session (std::string testname);

	SESSION active_;
};

namespace simple
{

void INIT (std::string server_addr,
	bool genmode = false, size_t nretries = 3);

void SHUTDOWN (void);

}
