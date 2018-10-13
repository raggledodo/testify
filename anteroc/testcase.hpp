#include "gtest/gtest.h"

#include "anteroc/client.hpp"

#ifndef ANTERO_TESTAMENT_HPP
#define ANTERO_TESTAMENT_HPP

namespace antero
{

class Testament : public ::testing::Test
{
protected:
	testify::GeneratedCase get (std::string testname);
};

}

#endif // ANTERO_TESTAMENT_HPP
