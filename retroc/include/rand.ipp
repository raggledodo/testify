template <typename T>
Range<T>::Range (void) :
		min_(std::numeric_limits<T>::min()),
		max_(std::numeric_limits<T>::max()) {}

template <typename T>
Range<T>::Range (T a, T b) :
	min_(std::min(a, b)), max_(std::max(a, b))
{
	if (a == b)
	{
		min_ = std::numeric_limits<T>::min();
		max_ = std::numeric_limits<T>::max();
	}
}

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
std::vector<T> get_vec (size_t len, Range<T> range)
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

template <typename Iterator>
Iterator select (Iterator first, Iterator last)
{
	size_t n = std::distance(first, last);
	size_t i = get_vec(1, Range<size_t>(0, n-1))[0];
	return first + i;
}
