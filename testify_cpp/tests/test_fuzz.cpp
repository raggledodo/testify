#include "gtest/gtest.h"

#include "testify_cpp/include/fuzz/fuzz.hpp"


class FUZZ : public testify::fuzz_test {};


std::default_random_engine& get_random (void)
{
	static std::default_random_engine common_gen(12);
	return common_gen;
}


TEST_F(FUZZ, get_double)
{
	std::string expectempty = ss_.str();
	EXPECT_EQ(0, expectempty.size());
	testify::set_generator(get_random);

	std::vector<double> doub = get_double(5000, "sample1", {-0.2, 0.001});
	ASSERT_EQ(5000, doub.size());

	std::stringstream ss;
	ss << "sample1: double<";
	for (size_t i = 0; i < 5000; ++i)
	{
		EXPECT_LE(-0.21, doub[i]);
		EXPECT_GE(0.16, doub[i]);
		ss << doub[i] << ",";
	}
	ss << ">" << std::endl;

	std::string actualinfo = ss.str();
	std::string fuzzinfo = ss_.str();
	EXPECT_TRUE(0 == actualinfo.compare(fuzzinfo));
}


TEST_F(FUZZ, get_int)
{
	std::string expectempty = ss_.str();
	EXPECT_EQ(0, expectempty.size());
	testify::set_generator(get_random);

	std::vector<size_t> isize = get_int(5017, "sample2", {12, 39});
	ASSERT_EQ(5017, isize.size());

	std::stringstream ss;
	ss << "sample2: int<";
	for (size_t i = 0; i < 5017; ++i)
	{
		EXPECT_LE(12, isize[i]);
		EXPECT_GE(39, isize[i]);
		ss << isize[i] << ",";
	}
	ss << ">" << std::endl;

	std::string actualinfo = ss.str();
	std::string fuzzinfo = ss_.str();
	EXPECT_TRUE(0 == actualinfo.compare(fuzzinfo));
}


TEST_F(FUZZ, get_string)
{
	std::string expectempty = ss_.str();
	EXPECT_EQ(0, expectempty.size());
	testify::set_generator(get_random);

	std::string str = get_string(3124, "sample3", "abcdef");
	ASSERT_EQ(3124, str.size());

	std::stringstream ss;
	ss << "indices: int<";
	for (size_t i = 0; i < 3124; ++i)
	{
		EXPECT_LE('a', str[i]);
		EXPECT_GE('f', str[i]);
		ss << (int) (str[i]- 'a') << ",";
	}
	ss << ">" << std::endl;

	std::string actualinfo = ss.str() + "sample3: string<" + str + ">\n";
	std::string fuzzinfo = ss_.str();
	EXPECT_TRUE(0 == actualinfo.compare(fuzzinfo));
}


TEST_F(FUZZ, rand_select)
{
	std::string expectempty = ss_.str();
	EXPECT_EQ(0, expectempty.size());
	testify::set_generator(get_random);

	std::vector<size_t> ua(17);
	std::iota(ua.begin(), ua.end(), 0);
	auto it = rand_select(ua);
	EXPECT_GT(17, *it);

	// std::stringstream ss;
	// ss << "rand_select::n: int<" << (*it) << ",>" << std::endl;
	// std::string actualinfo = ss.str();
	// std::string fuzzinfo = ss_.str();
	// EXPECT_TRUE(0 == actualinfo.compare(fuzzinfo));
}
