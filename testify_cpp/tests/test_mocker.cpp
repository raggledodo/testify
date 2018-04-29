#include <chrono>
#include <random>

#include "gtest/gtest.h"

#include "testify_cpp/include/fuzz/irng.hpp"
#include "testify_cpp/include/mocker/mocker.hpp"


static std::default_random_engine common_gen(std::chrono::system_clock::now().time_since_epoch().count());


class MOCK : public ::testing::Test
{
protected:
	virtual void SetUp (void)
	{
		testify::set_generator(
		[](void) -> std::default_random_engine&
		{
			return common_gen;
		});
	}

	virtual void TearDown (void) {}
};


TEST_F(MOCK, copy)
{
	testify::mocker m1;
	testify::mocker m2(m1);
	testify::mocker m3;
	m3 = m1;

	m1.label_incr("sample");
	m2.label_incr("sample");
	m3.label_incr("sample");

	EXPECT_EQ(1, testify::mocker::get_usage(&m1, "sample"));
	EXPECT_EQ(1, testify::mocker::get_usage(&m2, "sample"));
	EXPECT_EQ(1, testify::mocker::get_usage(&m3, "sample"));

	testify::mocker::clear();
}


TEST_F(MOCK, move)
{
	testify::set_generator(
	[](void) -> std::default_random_engine&
	{
		return common_gen;
	});

	testify::mocker m1;
	testify::mocker m2(std::move(m1));
	testify::mocker m3;
	m3 = std::move(m1);

	m1.label_incr("sample");
	m2.label_incr("sample");
	m3.label_incr("sample");

	EXPECT_EQ(1, testify::mocker::get_usage(&m1, "sample"));
	EXPECT_EQ(1, testify::mocker::get_usage(&m2, "sample"));
	EXPECT_EQ(1, testify::mocker::get_usage(&m3, "sample"));

	testify::mocker::clear();
}


TEST_F(MOCK, label_incr)
{
	testify::mocker m1;
	testify::mocker m2;

	m1.label_incr("sample");
	m2.label_incr("sample");

	EXPECT_EQ(1, testify::mocker::get_usage(&m1, "sample"));
	EXPECT_EQ(1, testify::mocker::get_usage(&m2, "sample"));

	testify::mocker::clear();
}


TEST_F(MOCK, set_label)
{
	testify::mocker m1;
	testify::mocker m2;
	testify::mocker m3;

	m1.set_label("sample", "value1");
	m2.set_label("sample", "value2");

	optional<std::string> v1 = testify::mocker::get_value(&m1, "sample");
	optional<std::string> v2 = testify::mocker::get_value(&m2, "sample");
	optional<std::string> v3 = testify::mocker::get_value(&m3, "sample");
	EXPECT_TRUE((bool) v1);
	EXPECT_TRUE((bool) v2);
	EXPECT_FALSE((bool) v3);
	EXPECT_STREQ("value1", v1->c_str());
	EXPECT_STREQ("value2", v2->c_str());

	testify::mocker::clear();
}
