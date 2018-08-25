#include "retroc/client.hpp"
#include "retroc/rand.hpp"

#ifndef GEN_HPP
#define GEN_HPP

struct GenIO
{
	GenIO (std::string testname) :
		testname_(testname) {}

	template <typename Iterator>
	void get_vec (std::string usage, Iterator begin, Iterator end,
		Range<IterType<Iterator>> range = Range<IterType<Iterator>>());

	template <typename T>
	std::vector<T> get_vec (std::string usage, size_t len,
		Range<T> range = Range<T>());

	template <typename Iterator>
	void get_vec (std::string usage, Iterator obegin, Iterator oend,
		const Iterator ibegin, const Iterator iend);

	std::string get_string (std::string usage, size_t len,
		std::string content = "0123456789!@#$%^&*"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

	std::vector<uint64_t> choose (std::string usage, uint64_t n, uint64_t k);

	template <typename Iterator>
	Iterator select (std::string usage, Iterator first, Iterator last);

	// directed graph
	template <uint32_t N>
	size_t get_minspan_tree (std::string usage, Graph<N>& out);

	// undirected graph
	template <uint32_t NVERT>
	void get_graph (std::string usage, Graph<NVERT>& out);

	// undirected graph
	template <uint32_t NVERT>
	void get_conn_graph (std::string usage, Graph<NVERT>& out);

	template <typename Iterator>
	void set_output (std::string usage, Iterator begin, Iterator end);

	template <uint32_t N>
	void set_outtree (std::string usage, size_t root, Graph<N>& graph);

	template <uint32_t NVERT>
	void set_outgraph (std::string usage, Graph<NVERT>& graph);

	void send_case (void);

private:
	template <typename Iterator>
	testify::DTYPE serialize (google::protobuf::Any* out,
		Iterator begin, Iterator end);

	template <uint32_t NVERT>
	void serialize (testify::Graph& out, Graph<NVERT>& in);

	testify::GeneratedCase gcase_;

	std::string testname_;
};

#include "retroc/include/generate.ipp"

#endif /* GEN_HPP */
