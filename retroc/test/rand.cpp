#include <unordered_set>
#include <unordered_map>

#include "gtest/gtest.h"

#include "retroc/rand.hpp"


#define ASSERT_ARR_BETWEEN(bot, top, arr)\
for (auto v : arr) {\
	ASSERT_LE(bot, v);\
	ASSERT_GE(top, v);\
}


int main (int argc, char** argv)
{
	size_t seed = std::time(nullptr);
	get_engine().seed(seed);

	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();
	return ret;
}


template <typename Arr>
void vec_randomness (Arr vec, Arr vec2,
	Range<IterType<typename Arr::iterator> > range,
	size_t n, double similarity_thresh)
{
	// constraint
	ASSERT_EQ(n, vec.size());
	ASSERT_EQ(n, vec2.size());
	ASSERT_ARR_BETWEEN(range.min_, range.max_, vec)

	// randomness between different arrays
	double dist = 0;
	for (uint i = 0; i < n; ++i)
	{
		double diff = (double) vec[i] - (double) vec2[i];
		dist += diff * diff;
	}
	dist = std::sqrt(dist) / (range.max_ - range.min_);
	EXPECT_LT(similarity_thresh, dist);
}


TEST(RANDO, GenDoubles)
{
	const double separation_metric = 0.01;
	const double similarity_thresh = 0.1;
	Range<double> range(-10.2, 24.2);
	size_t n = 5000;
	auto vec = get_vec<double>(n, range);
	auto vec2 = get_vec<double>(n, range);

	vec_randomness(vec, vec2, range, n, similarity_thresh);

	// randomness within a single array
	std::vector<double> svec = vec;
	std::sort(svec.begin(), svec.end());
	double diff = 0;
	for (uint i = 0; i < n - 1; ++i)
	{
		diff += (svec[i+1] - svec[i]);
	}
	double avgdiff = diff / (n - 1);
	EXPECT_GT(separation_metric, avgdiff);
}


TEST(RANDO, GenFloats)
{
	const float separation_metric = 0.1;
	const float similarity_thresh = 0.1;
	Range<float> range(-235, 252);
	size_t n = 7000;
	auto vec = get_vec<float>(n, range);
	auto vec2 = get_vec<float>(n, range);

	vec_randomness(vec, vec2, range, n, similarity_thresh);

	// randomness within a single array
	std::vector<float> svec = vec;
	std::sort(svec.begin(), svec.end());
	float diff = 0;
	for (uint i = 0; i < n - 1; ++i)
	{
		diff += (svec[i+1] - svec[i]);
	}
	float avgdiff = diff / (n - 1);
	EXPECT_GT(separation_metric, avgdiff);
}


TEST(RANDO, GenInt32s)
{
	const double separation_metric = 1;
	const double similarity_thresh = 0.1;
	Range<int32_t> range(-100, 252);
	size_t n = 3000;
	auto vec = get_vec<int32_t>(n, range);
	auto vec2 = get_vec<int32_t>(n, range);

	vec_randomness(vec, vec2, range, n, similarity_thresh);

	// randomness within a single array
	std::vector<int32_t> svec = vec;
	std::sort(svec.begin(), svec.end());
	double diff = 0;
	for (uint i = 0; i < n - 1; ++i)
	{
		diff += (svec[i+1] - svec[i]);
	}
	double avgdiff = diff / (n - 1);
	EXPECT_GT(separation_metric, avgdiff);
}


TEST(RANDO, GenUint32s)
{
	const double separation_metric = 1;
	const double similarity_thresh = 0.1;
	Range<uint32_t> range(100, 252);
	size_t n = 6000;
	auto vec = get_vec<uint32_t>(n, range);
	auto vec2 = get_vec<uint32_t>(n, range);

	vec_randomness(vec, vec2, range, n, similarity_thresh);

	// randomness within a single array
	std::vector<uint32_t> svec = vec;
	std::sort(svec.begin(), svec.end());
	double diff = 0;
	for (uint i = 0; i < n - 1; ++i)
	{
		diff += ((double) svec[i+1] - (double) svec[i]);
	}
	double avgdiff = diff / (n - 1);
	EXPECT_GT(separation_metric, avgdiff);
}


TEST(RANDO, GenInt64s)
{
	const double separation_metric = 1;
	const double similarity_thresh = 0.1;
	Range<int64_t> range(-100, 252);
	size_t n = 5500;
	auto vec = get_vec<int64_t>(n, range);
	auto vec2 = get_vec<int64_t>(n, range);

	vec_randomness(vec, vec2, range, n, similarity_thresh);

	// randomness within a single array
	std::vector<int64_t> svec = vec;
	std::sort(svec.begin(), svec.end());
	double diff = 0;
	for (uint i = 0; i < n - 1; ++i)
	{
		diff += (svec[i+1] - svec[i]);
	}
	double avgdiff = diff / (n - 1);
	EXPECT_GT(separation_metric, avgdiff);
}


TEST(RANDO, GenUint64s)
{
	const double separation_metric = 1;
	const double similarity_thresh = 0.1;
	Range<uint64_t> range(100, 252);
	size_t n = 6000;
	auto vec = get_vec<uint64_t>(n, range);
	auto vec2 = get_vec<uint64_t>(n, range);

	vec_randomness(vec, vec2, range, n, similarity_thresh);

	// randomness within a single array
	std::vector<uint64_t> svec = vec;
	std::sort(svec.begin(), svec.end());
	double diff = 0;
	for (uint i = 0; i < n - 1; ++i)
	{
		diff += ((double) svec[i+1] - (double) svec[i]);
	}
	double avgdiff = diff / (n - 1);
	EXPECT_GT(separation_metric, avgdiff);
}


TEST(RANDO, GenStr)
{
	const double similarity_thresh = 0.1;
	const std::string content = "0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	size_t n = 9300;
	std::string instr = get_string(n, content);
	std::string instr2 = get_string(n, content);

	vec_randomness(instr, instr2, Range<char>('0', 'z'), n, similarity_thresh);

	// randomness within a single string
	std::unordered_map<char,uint> occurrences;
	for (char c : instr)
	{
		if (occurrences.find(c) == occurrences.end())
		{
			occurrences[c] = 0;
		}
		occurrences[c]++;
	}
	size_t nbins = occurrences.size();
	size_t missing = content.size() - nbins;
	EXPECT_GT(0.05 * content.size(), missing);
	// stdev of occurrence should be close to 0
	double mean = 0;
	for (auto& opair : occurrences)
	{
		mean += opair.second;
	}
	mean /= nbins;
	double stdev = 0;
	for (auto& opair : occurrences)
	{
		double diff = (double) opair.second - mean;
		stdev += diff * diff;
	}
	stdev = std::sqrt(stdev / nbins);
	double rel_stdev = stdev / n;
	EXPECT_GT(0.01, rel_stdev);
}


TEST(RANDO, GenChoice)
{
	const double similarity_thresh = 0.1;
	std::vector<uint64_t> carr = choose(19, 6);
	std::vector<uint64_t> carr2 = choose(19, 6);

	// constraint
	std::unordered_set<uint64_t> carr_set(carr.begin(), carr.end());
	EXPECT_EQ(6, carr.size());
	EXPECT_EQ(carr.size(), carr_set.size()) <<
		"chosen vec doesn't contain unique values";
	uint64_t mic = *std::min_element(carr.begin(), carr.end());
	uint64_t mac = *std::max_element(carr.begin(), carr.end());
	EXPECT_LE(0, mic) << "chosen vec not within range [0, 19)";
	EXPECT_GT(19, mac) << "chosen vec not within range [0, 19)";

	// randomness between different choose ops
	double dist = 0;
	for (uint i = 0; i < 6; ++i)
	{
		double diff = (double) carr[i] - (double) carr2[i];
		dist += diff * diff;
	}
	dist = std::sqrt(dist) / 19;
	EXPECT_LT(similarity_thresh, dist);

	const uint sample_size = 3000;
	uint nbins = carr.size();
	uint occurrences[nbins];
	std::memset(occurrences, 0, sizeof(uint) * nbins);
	auto begit = carr.begin();
	auto endit = carr.end();
	for (uint i = 0; i < sample_size; ++i)
	{
		auto it = select(begit, endit);
		size_t d = std::distance(begit, it);
		occurrences[d]++;
	}
	// stdev of occurrence should be close to 0
	double mean = 0;
	for (uint i = 0; i < nbins; ++i)
	{
		mean += occurrences[i];
	}
	mean /= nbins;
	double stdev = 0;
	for (uint i = 0; i < nbins; ++i)
	{
		double diff = (double) occurrences[i] - mean;
		stdev += diff * diff;
	}
	stdev = std::sqrt(stdev / nbins);
	double rel_stdev = stdev / sample_size;
	EXPECT_GT(0.1, rel_stdev);
}


TEST(RANDO, GenTree)
{
	const uint32_t N = 147;
	Graph<N> tree;
	get_minspan_tree(tree);
	Graph<N> tree2;
	get_minspan_tree(tree2);

	// constraint
	// let's go through the tree, assertion:
	// MUST visit every node once and only once
	bool visited[N];
	std::memset(visited, false, N);
	tree.breadth_traverse(0,
	[&](uint32_t node, std::vector<uint32_t>&)
	{
		ASSERT_FALSE(visited[node]) << "node=" << node << " is visited twice";
		visited[node] = true;
	});
	for (uint i = 0; i < N; ++i)
	{
		EXPECT_TRUE(visited[i]) << "tree is disconnected from node " << i;
	}

	// randomness between trees
	uint32_t nand = 0;
	uint32_t nor = 0;
	for (uint32_t i = 0; i < N; ++i)
	{
		for (uint32_t j = 0; j < N; ++j)
		{
			nand += (tree.get(i, j) && tree2.get(i, j));
			nor += (tree.get(i, j) || tree2.get(i, j));
		}
	}

	double sim_frac = (double) nand / nor;
	EXPECT_GT(0.05, sim_frac);
}


TEST(RANDO, GenGraph)
{
	const uint32_t N = 137;
	Graph<N> graph;
	get_graph(graph);
	Graph<N> graph2;
	get_graph(graph2);

	// randomness between graphs
	uint32_t nand = 0;
	uint32_t nor = 0;
	for (uint32_t i = 0; i < N; ++i)
	{
		for (uint32_t j = 0; j < N; ++j)
		{
			nand += (graph.get(i, j) && graph2.get(i, j));
			nor += (graph.get(i, j) || graph2.get(i, j));
		}
	}

	double sim_frac = (double) nand / nor;
	EXPECT_GT(0.5, sim_frac);
}


TEST(RANDO, GenCGraph)
{
	const uint32_t N = 123;
	Graph<N> graph;
	get_conn_graph(graph);
	Graph<N> graph2;
	get_conn_graph(graph2);

	// constraint
	// let's go through the graph, assertion:
	// MUST visit every node
	bool visited[N];
	std::memset(visited, false, N);
	graph.breadth_traverse(0,
	[&](uint32_t node, std::vector<uint32_t>&)
	{
		visited[node] = true;
	});
	for (size_t i = 0; i < N; ++i)
	{
		EXPECT_TRUE(visited[i]) << "connected graph is disconnected from node " << i;
	}

	// randomness between graphs
	uint32_t nand = 0;
	uint32_t nor = 0;
	for (uint32_t i = 0; i < N; ++i)
	{
		for (uint32_t j = 0; j < N; ++j)
		{
			nand += (graph.get(i, j) && graph2.get(i, j));
			nor += (graph.get(i, j) || graph2.get(i, j));
		}
	}

	double sim_frac = (double) nand / nor;
	EXPECT_GT(0.5, sim_frac);
}
