#include "retroc/include/type.hpp"

template <typename Iterator>
void GenIO::get_vec (std::string usage,
	Iterator begin, Iterator end,
	Range<IterType<Iterator>> range)
{
	::get_vec(begin, end, range);
	testify::Input* input = output_.mutable_inputs()->Add();
	input->set_usage(usage);
	input->set_dtype(serialize(input->mutable_data(), begin, end));
}

template <typename T>
std::vector<T> GenIO::get_vec (std::string usage,
	size_t len, Range<T> range)
{
	auto out = ::get_vec(len, range);
	testify::Input* input = output_.mutable_inputs()->Add();
	input->set_usage(usage);
	input->set_dtype(serialize(input->mutable_data(),
		out.begin(), out.end()));
	return out;
}

template <typename Iterator>
void GenIO::get_vec (std::string usage,
	Iterator obegin, Iterator oend,
	const Iterator ibegin, const Iterator iend)
{
	::get_vec(obegin, oend, ibegin, iend);
	testify::Input* input = output_.mutable_inputs()->Add();
	input->set_usage(usage);
	input->set_dtype(serialize(input->mutable_data(), obegin, oend));
	}

template <typename Iterator>
Iterator GenIO::select (std::string usage, Iterator first, Iterator last)
{
	auto out = ::select(first, last);
	testify::Input* input = output_.mutable_inputs()->Add();
	input->set_usage("select_" + usage);
	size_t i = std::distance(first, out);
	testify::Uint64s arr;
	arr.add_data(i);
	input->mutable_data()->PackFrom(arr);
	input->set_dtype(testify::UINT64S);
	return out;
}

template <uint32_t N>
size_t GenIO::get_minspan_tree (std::string usage, Graph<N>& out)
{
	::get_minspan_tree(out);
	testify::Input* input = output_.mutable_inputs()->Add();
	input->set_usage(usage);
	testify::Tree tree;
	tree.set_root(0);
	serialize(*(tree.mutable_graph()), out);
	input->mutable_data()->PackFrom(tree);
	input->set_dtype(testify::NTREE);
	return 0;
}

template <uint32_t NVERT>
void GenIO::get_graph (std::string usage, Graph<NVERT>& out)
{
	::get_graph(out);
	testify::Input* input = output_.mutable_inputs()->Add();
	input->set_usage(usage);
	testify::Graph graph;
	serialize(graph, out);
	input->mutable_data()->PackFrom(graph);
	input->set_dtype(testify::GRAPH);
}

template <uint32_t NVERT>
void GenIO::get_conn_graph (std::string usage, Graph<NVERT>& out)
{
	::get_conn_graph(out);
	testify::Input* input = output_.mutable_inputs()->Add();
	input->set_usage(usage);
	testify::Graph graph;
	serialize(graph, out);
	input->mutable_data()->PackFrom(graph);
	input->set_dtype(testify::GRAPH);
}

template <typename Iterator>
void GenIO::set_output (Iterator begin, Iterator end)
{
	output_.set_dtype(serialize(output_.mutable_output(),
		begin, end));
	if (send(testname_, output_))
	{
		output_.Clear();
	}
}

template <uint32_t N>
void GenIO::set_outtree (size_t root, Graph<N>& graph)
{
	testify::Tree tree;
	tree.set_root(root);
	serialize(*(tree.mutable_graph()), graph);
	output_.mutable_output()->PackFrom(tree);
	output_.set_dtype(testify::NTREE);
	if (send(testname_, output_))
	{
		output_.Clear();
	}
}

template <uint32_t NVERT>
void GenIO::set_outgraph (Graph<NVERT>& graph)
{
	testify::Graph g;
	serialize(g, graph);
	output_.mutable_output()->PackFrom(g);
	output_.set_dtype(testify::GRAPH);
	if (send(testname_, output_))
	{
		output_.Clear();
	}
}

#define SET_DATA(TYPE)\
google::protobuf::RepeatedField<TYPE> field(begin, end);\
arr.mutable_data()->Swap(&field);\
out->PackFrom(arr);

template <typename Iterator>
testify::DTYPE GenIO::serialize (google::protobuf::Any* out,
	Iterator begin, Iterator end)
{
	assert(nullptr != out);
	testify::DTYPE type = get_type<IterType<Iterator>>();
	switch (type)
	{
		case testify::BYTES:
		{
			testify::Bytes arr;
			arr.set_data(std::string(begin, end));\
			out->PackFrom(arr);
		}
		break;
		case testify::DOUBLES:
		{
			testify::Doubles arr;
			SET_DATA(double)
		}
		break;
		case testify::FLOATS:
		{
			testify::Floats arr;
			SET_DATA(float)
		}
		break;
		case testify::INT32S:
		{
			testify::Int32s arr;
			SET_DATA(int32_t)
		}
		break;
		case testify::UINT32S:
		{
			testify::Uint32s arr;
			SET_DATA(uint32_t)
		}
		break;
		case testify::INT64S:
		{
			testify::Int64s arr;
			SET_DATA(int64_t)
		}
		break;
		case testify::UINT64S:
		{
			testify::Uint64s arr;
			SET_DATA(uint64_t)
		}
		break;
		default:
			throw std::exception();
	}
	return type;
}

template <uint32_t NVERT>
void GenIO::serialize (testify::Graph& out, Graph<NVERT>& in)
{
	double nelems = NVERT * NVERT;
	std::string bitfield(std::ceil(nelems / 8), 0);
	for (size_t i = 0; i < nelems; ++i)
	{
		size_t x = i % NVERT;
		size_t y = i / NVERT;
		if (in.get(y, x))
		{
			bitfield[i / 8] |= (1 << (i % 8));
		}
	}
	out.set_matrix(bitfield);
	out.set_nverts(NVERT);
}
