//
// Created by Mingkai Chen on 2017-03-09.
//

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cstdlib>
#include <limits>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <functional>

#include "gtest/gtest.h"

#include "testify_cpp/include/fuzz/irng.hpp"

#ifndef TESTIFY_FUZZ_HPP
#define TESTIFY_FUZZ_HPP

namespace testify
{

class fuzz_test : public ::testing::Test
{
protected:
	virtual void SetUp (void);

	virtual void TearDown (void);

	bool print_enabled = true;

public:
	std::vector<double> get_double (size_t len,
		std::string purpose = "get_double",
		std::pair<double,double> range={0,0});

	std::vector<size_t> get_int (size_t len,
		std::string purpose = "get_integer",
		std::pair<size_t,size_t> range={0,0});

	std::string get_string (size_t len,
		std::string purpose = "get_string",
		std::string alphanum =
		"0123456789!@#$%^&*"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz");

	template <typename T>
	typename T::iterator rand_select (T& container)
	{
		size_t size = container.size();
		size_t i = get_int(1, "rand_select::n", {0, size-1})[0];
		auto out = container.begin();
		if (i > 0)
		{
			out += i;
		}
		return out;
	}

	// generate a binary tree structure in serial form
	template <typename N>
	N* build_ntree (size_t n, size_t nnodes,
		std::function<N*()> buildleaf,
		std::function<N*(std::vector<N*>)> connect)
	{
		std::vector<size_t> nstrlen = get_int(nnodes, "nstrlen", {14, 29});

		std::vector<size_t> preds = {0, 0};
		std::unordered_set<size_t> leaves = {1};
		std::vector<size_t> nargs = {1, 0};
		{
			// randomly generate predecessor nodes
			std::default_random_engine& generator = get_generator();
			std::vector<size_t> concount = {0, 1};

			ss_ << "tree<";
			for (size_t i = 2; i < nnodes; i++)
			{
				std::uniform_int_distribution<size_t> dis(0, concount.size()-1);
				size_t pidx = dis(generator);

				auto it = concount.begin() + pidx;
				size_t p = *it;
				// remove nodes that are not leaves
				leaves.erase(p);

				// update the potential connectors list
				if (++nargs[p]>= n)
				{
					concount.erase(it);
				}

				concount.push_back(i);
				leaves.insert(i);
				nargs.push_back(0);
				// update predecessors
				preds.push_back(p);
				ss_ << p << "," << std::endl;
			}
			ss_ << ">" << std::endl;
		}
		std::unordered_map<size_t,std::vector<N*> > deps;
		std::queue<size_t> pq;
		for (size_t l : leaves)
		{
			N* nn = buildleaf();
			// preds
			size_t pp = preds[l];
			deps[pp].push_back(nn);
			if (deps[pp].size() == nargs[pp])
			{
				pq.push(pp);
			}
		}

		N* root = nullptr;
		while (pq.empty() == false)
		{
			size_t nidx = pq.front();
			pq.pop();
			root = connect(deps[nidx]);
			if (nidx)
			{
				// preds
				size_t pidx = preds[nidx];
				deps[pidx].push_back(root);
				if (deps[pidx].size() == nargs[pidx])
				{
					pq.push(pidx);
				}
			}
		}

		return root;
	}

	std::stringstream ss_;
};

}

#endif /* TESTIFY_FUZZ_HPP */
