#include <random>
#include <memory>
#include <cstring>
#include <queue>
#include <sstream>

#ifndef RAND_HPP
#define RAND_HPP

using ENGINE = std::default_random_engine;

template <typename Iterator>
using IterType = typename std::iterator_traits<Iterator>::value_type;

ENGINE& get_engine (void);

template <typename T>
struct Range
{
	Range (void) :
		min_(std::numeric_limits<T>::min()),
		max_(std::numeric_limits<T>::max()) {}

	Range (T a, T b) :
		min_(std::min(a, b)), max_(std::max(a, b))
	{
		if (a == b)
		{
			min_ = std::numeric_limits<T>::min();
			max_ = std::numeric_limits<T>::max();
		}
	}

	T min_;
	T max_;
};

template <typename Iterator, typename T>
struct ItHelper
{
	static void get_vec (Iterator begin, Iterator end, Range<T> range)
	{
		ENGINE& gen = get_engine();
		std::uniform_int_distribution<T> dis(range.min_, range.max_);
		std::generate(begin, end, [&]()
		{
			return dis(gen);
		});
	}
};

template <typename Iterator>
struct ItHelper<Iterator,double>
{
	static void get_vec (Iterator begin, Iterator end, Range<double> range)
	{
		ENGINE& gen = get_engine();
		std::uniform_real_distribution<double> dis(range.min_, range.max_);
		std::generate(begin, end, [&]()
		{
			return dis(gen);
		});
	}
};

template <typename Iterator>
struct ItHelper<Iterator,float>
{
	static void get_vec (Iterator begin, Iterator end, Range<float> range)
	{
		ENGINE& gen = get_engine();
		std::uniform_real_distribution<float> dis(range.min_, range.max_);
		std::generate(begin, end, [&]()
		{
			return dis(gen);
		});
	}
};

template <typename Iterator>
void get_vec (Iterator begin, Iterator end, Range<IterType<Iterator> > range)
{
	ItHelper<Iterator,IterType<Iterator> >::get_vec(begin, end, range);
}

template <typename T>
std::vector<T> get_vec (size_t len, Range<T> range = Range<T>())
{
	std::vector<T> out(len);
	get_vec(out.begin(), out.end(), range);
	return out;
}

template <typename Iterator>
void get_vec (Iterator obegin, Iterator oend,
	const Iterator ibegin, const Iterator iend)
{
	size_t len = std::distance(obegin, oend);
	size_t ncontent = std::distance(ibegin, iend);
	std::vector<size_t> hash = get_vec(len, Range<size_t>(0, ncontent-1));
	std::transform(hash.begin(), hash.end(), obegin,
	[&ibegin](size_t index)
	{
		return *(ibegin + index);
	});
}

std::string get_string (size_t len, std::string content =
	"0123456789!@#$%^&*ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz");

std::vector<uint64_t> choose (uint64_t n, uint64_t k);

template <typename Iterator>
Iterator select (Iterator first, Iterator last)
{
	size_t n = std::distance(first, last);
	size_t i = get_vec(1, Range<size_t>(0, n-1))[0];
	return first + i;
}

using NodeCb = std::function<void(uint32_t,std::vector<uint32_t>&)>;

template <uint32_t N>
struct Graph
{
	Graph (bool ones = false) :
		data_(N * N, ones) {}

	void zeroes (void)
	{
		std::fill(data_.begin(), data_.end(), false);
	}

	void connect (uint32_t src, uint32_t dest)
	{
		data_[index(src, dest)] = true;
	}

	void disconnect (uint32_t src, uint32_t dest)
	{
		data_[index(src, dest)] = false;
	}

	bool get (uint32_t src, uint32_t dest) const
	{
		return data_[index(src, dest)];
	}

	void breadth_traverse (uint32_t root, NodeCb cb)
	{
		bool visited[N];
		std::memset(visited, false, N);

		std::queue<uint32_t> q;
		q.push(root);

		while (false == q.empty())
		{
			root = q.front();
			q.pop();
			if (visited[root]) continue;
			std::vector<uint32_t> children;
			for (uint32_t child = 0; child < N; ++child)
			{
				if (get(root, child))
				{
					q.push(child);
					children.push_back(child);
				}
			}
			cb(root, children);
			visited[root] = true;
		}
	}

private:
	size_t index (uint32_t src, uint32_t dest) const
	{
		size_t out = src * N + dest;
		if (out >= data_.size())
		{
			std::stringstream ss;
			ss << "src=" << src << ",dest=" << dest << " yields " << out
				<< " out of range=" << data_.size();
			throw std::out_of_range(ss.str());
		}
		return out;
	}

	std::vector<bool> data_;
};

template <uint32_t N>
void get_minspan_tree (Graph<N>& out)
{
	static_assert(N > 0, "creating empty tree");
	if (N > 1)
	{
		out.connect(0, 1);
	}
	if (N > 2)
	{
		std::vector<uint32_t> parents = get_vec(N - 2, Range<uint32_t>(0, N-1));
		for (uint32_t i = 2; i < N; ++i)
		{
			uint32_t parent = parents[i - 2] % i;
			out.connect(parent, i);
		}
	}
}

template <uint32_t NVERT>
void get_graph (Graph<NVERT>& out)
{
	static_assert(NVERT > 0, "creating empty graph");
	size_t nedges = NVERT * (NVERT - 1);
	uint8_t edges[nedges];
	get_vec(edges, edges + nedges, Range<uint8_t>(0, 1));
	size_t srci = 0;
	for (size_t i = 0; i < NVERT; ++i)
	{
		for (size_t j = 0; j < NVERT; ++j)
		{
			if (j != i)
			{
				if (edges[srci] > 0)
				{
					out.connect(i, j);
				}
				++srci;
			}
		}
	}
}

template <uint32_t NVERT>
void get_conn_graph (Graph<NVERT>& out)
{
	static_assert(NVERT > 0, "creating empty graph");
	get_minspan_tree<NVERT>(out);
	get_graph<NVERT>(out);
}

#endif /* RAND_HPP */
