#include "client/type.hpp"

#include "retroc/client.hpp"
#include "retroc/rand.hpp"

#ifndef RETRO_GEN_HPP
#define RETRO_GEN_HPP

namespace retro
{

struct GenIO
{
	GenIO (std::string testname) :
		testname_(testname) {}

	template <typename Iterator>
	void get_vec (std::string usage, Iterator begin, Iterator end,
		Range<IterType<Iterator>> range = Range<IterType<Iterator>>())
	{
		retro::get_vec(begin, end, range);
		testify::CaseData input;
		serialize(input, begin, end);
		gcase_.mutable_inputs()->insert({usage, input});
	}

	template <typename T>
	std::vector<T> get_vec (std::string usage, size_t len,
		Range<T> range = Range<T>())
	{
		auto out = retro::get_vec(len, range);
		testify::CaseData input;
		serialize(input, out.begin(), out.end());
		gcase_.mutable_inputs()->insert({usage, input});
		return out;
	}

	template <typename Iterator>
	void get_vec (std::string usage, Iterator obegin, Iterator oend,
		const Iterator ibegin, const Iterator iend)
	{
		retro::get_vec(obegin, oend, ibegin, iend);
		testify::CaseData input;
		serialize(input, obegin, oend);
		gcase_.mutable_inputs()->insert({usage, input});
	}

	std::string get_string (std::string usage, size_t len,
		std::string content = "0123456789!@#$%^&*"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

	std::vector<uint64_t> choose (std::string usage, uint64_t n, uint64_t k);

	template <typename Iterator>
	Iterator select (std::string usage, Iterator first, Iterator last)
	{
		auto out = retro::select(first, last);
		testify::CaseData input;
		size_t i = std::distance(first, out);
		testify::Uint64s* arr = input.mutable_duint64s();
		arr->add_data(i);
		gcase_.mutable_inputs()->insert({usage, input});
		return out;
	}

	// directed graph
	template <uint32_t N>
	size_t get_minspan_tree (std::string usage, Graph<N>& out)
	{
		retro::get_minspan_tree(out);
		testify::CaseData input;
		testify::Tree* tree = input.mutable_dtree();
		tree->set_root(0);
		serialize(*(tree->mutable_graph()), out);
		gcase_.mutable_inputs()->insert({usage, input});
		return 0;
	}

	// undirected graph
	template <uint32_t NVERT>
	void get_graph (std::string usage, Graph<NVERT>& out)
	{
		retro::get_graph(out);
		testify::CaseData input;
		testify::Graph* graph = input.mutable_dgraph();
		serialize(*graph, out);
		gcase_.mutable_inputs()->insert({usage, input});
	}

	// undirected graph
	template <uint32_t NVERT>
	void get_conn_graph (std::string usage, Graph<NVERT>& out)
	{
		retro::get_conn_graph(out);
		testify::CaseData input;
		testify::Graph* graph = input.mutable_dgraph();
		serialize(*graph, out);
		gcase_.mutable_inputs()->insert({usage, input});
	}

	template <typename Iterator>
	void set_output (std::string usage, Iterator begin, Iterator end)
	{
		testify::CaseData output;
		serialize(output, begin, end);
		gcase_.mutable_outputs()->insert({usage, output});
	}

	template <uint32_t N>
	void set_outtree (std::string usage, size_t root, Graph<N>& graph)
	{
		testify::CaseData output;
		testify::Tree* tree = output.mutable_dtree();
		tree->set_root(root);
		serialize(*(tree->mutable_graph()), graph);
		gcase_.mutable_outputs()->insert({usage, output});
	}

	template <uint32_t NVERT>
	void set_outgraph (std::string usage, Graph<NVERT>& graph)
	{
		testify::CaseData output;
		testify::Graph* g = output.mutable_dgraph();
		serialize(*g, graph);
		gcase_.mutable_outputs()->insert({usage, output});
	}

	bool can_send (void)
	{
		return gcase_.inputs().size() > 0;
	}

	void send_case (void);

private:
	template <typename Iterator>
	void serialize (testify::CaseData& out, Iterator begin, Iterator end)
	{
		dora::DtypeT type = dora::get_type<IterType<Iterator>>();
		switch (type)
		{
			case dora::DtypeT::kDbytes:
			{
				testify::Bytes* arr = out.mutable_dbytes();
				arr->set_data(std::string(begin, end));
			}
			break;
			case dora::DtypeT::kDdoubles:
			{
				testify::Doubles* arr = out.mutable_ddoubles();
				google::protobuf::RepeatedField<double> field(begin, end);
				arr->mutable_data()->Swap(&field);
			}
			break;
			case dora::DtypeT::kDfloats:
			{
				testify::Floats* arr = out.mutable_dfloats();
				google::protobuf::RepeatedField<float> field(begin, end);
				arr->mutable_data()->Swap(&field);
			}
			break;
			case dora::DtypeT::kDint32S:
			{
				testify::Int32s* arr = out.mutable_dint32s();
				google::protobuf::RepeatedField<int32_t> field(begin, end);
				arr->mutable_data()->Swap(&field);
			}
			break;
			case dora::DtypeT::kDuint32S:
			{
				testify::Uint32s* arr = out.mutable_duint32s();
				google::protobuf::RepeatedField<uint32_t> field(begin, end);
				arr->mutable_data()->Swap(&field);
			}
			break;
			case dora::DtypeT::kDint64S:
			{
				testify::Int64s* arr = out.mutable_dint64s();
				google::protobuf::RepeatedField<int64_t> field(begin, end);
				arr->mutable_data()->Swap(&field);
			}
			break;
			case dora::DtypeT::kDuint64S:
			{
				testify::Uint64s* arr = out.mutable_duint64s();
				google::protobuf::RepeatedField<uint64_t> field(begin, end);
				arr->mutable_data()->Swap(&field);
			}
			break;
			default:
				throw std::runtime_error("serializing non-vector type");
		}
	}

	template <uint32_t NVERT>
	void serialize (testify::Graph& out, Graph<NVERT>& in)
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

	testify::GeneratedCase gcase_;

	std::string testname_;
};

}

#endif // RETRO_GEN_HPP
