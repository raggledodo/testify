#include "gtest/gtest.h"

#include "anteroc/client.hpp"

class Testament : public ::testing::Test
{
protected:
	testify::GeneratedCase get (std::string testname);
};
