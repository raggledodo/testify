#include "anteroc/client.hpp"
#include "retroc/client.hpp"

#include "simple/jack.hpp"

bool TestModel::GENERATE_MODE = false;

static std::vector<double> unpack_double (const google::protobuf::Any& data, testify::DTYPE type)
{
	std::vector<double> out;
	switch (type)
	{
		case testify::DOUBLES:
		{
			testify::Doubles arr;
			data.UnpackTo(&arr);
			auto temp = arr.data();
			out = std::vector<double>(temp.begin(), temp.end());
		}
		break;
		case testify::FLOATS:
		{
			testify::Floats arr;
			data.UnpackTo(&arr);
			auto temp = arr.data();
			out = std::vector<double>(temp.begin(), temp.end());
		}
		default:
			throw std::runtime_error("incorrect (non-decimal) type");
	}
	return out;
}

static std::vector<int32_t> unpack_int (const google::protobuf::Any& data, testify::DTYPE type)
{
	std::vector<int32_t> out;
	switch (type)
	{
		case testify::INT64S:
		{
			testify::Int64s arr;
			data.UnpackTo(&arr);
			auto temp = arr.data();
			out = std::vector<int32_t>(temp.begin(), temp.end());
		}
		break;
		case testify::UINT64S:
		{
			testify::Uint64s arr;
			data.UnpackTo(&arr);
			auto temp = arr.data();
			out = std::vector<int32_t>(temp.begin(), temp.end());
		}
		break;
		case testify::INT32S:
		{
			testify::Int32s arr;
			data.UnpackTo(&arr);
			auto temp = arr.data();
			out = std::vector<int32_t>(temp.begin(), temp.end());
		}
		break;
		case testify::UINT32S:
		{
			testify::Uint32s arr;
			data.UnpackTo(&arr);
			auto temp = arr.data();
			out = std::vector<int32_t>(temp.begin(), temp.end());
		}
		break;
		case testify::BYTES:
		{
			testify::Bytes arr;
			data.UnpackTo(&arr);
			auto temp = arr.data();
			out = std::vector<int32_t>(temp.begin(), temp.end());
		}
		break;
		default:
			throw std::runtime_error("incorrect (non-integer) type");
	}
	return out;
}

static std::string unpack_string (const google::protobuf::Any& data, testify::DTYPE type)
{
	std::string out;
	switch (type)
	{
		case testify::BYTES:
		{
			testify::Bytes arr;
			data.UnpackTo(&arr);
			auto temp = arr.data();
			out = std::string(temp.begin(), temp.end());
		}
		break;
		default:
			throw std::runtime_error(
				"incorrect (non-string) type");
	}
	return out;
}

struct ModelledSession : public iSession
{
	ModelledSession (testify::GeneratedCase gcase) : gcase_(gcase) {}

	virtual ~ModelledSession (void) {}

	optional<std::vector<double>> expect_double (std::string usage) override
	{
		optional<std::vector<double>> out;
		auto& outputs = gcase_.outputs();
		auto it = outputs.find(usage);
		if (outputs.end() != it)
		{
			out = unpack_double(it->second.data(), it->second.dtype());
		}
		return out;
	}

	optional<std::vector<int32_t>> expect_int (std::string usage) override
	{
		optional<std::vector<int32_t>> out;
		auto& outputs = gcase_.outputs();
		auto it = outputs.find(usage);
		if (outputs.end() != it)
		{
			out = unpack_int(it->second.data(), it->second.dtype());
		}
		return out;
	}

	optional<std::string> expect_string (std::string usage) override
	{
		optional<std::string> out;
		auto& outputs = gcase_.outputs();
		auto it = outputs.find(usage);
		if (outputs.end() != it)
		{
			out = unpack_string(it->second.data(), it->second.dtype());
		}
		return out;
	}

protected:
	testify::GeneratedCase gcase_;
};

struct GenSession final : public ModelledSession
{
	GenSession (testify::GeneratedCase gcase, std::string testname) :
		ModelledSession(gcase), io_(testname) {}

	~GenSession (void)
	{
		if (io_.can_send() && retro::can_send() && can_send_)
		{
			io_.send_case();
		}
	}

	std::vector<double> get_double (std::string usage, size_t len,
		Range<double> range = Range<double>()) override
	{
		return io_.get_vec<double>(usage, len, range);
	}

	std::vector<int32_t> get_int (std::string usage, size_t len,
		Range<int32_t> range = Range<int32_t>()) override
	{
		return io_.get_vec<int32_t>(usage, len, range);
	}

	int32_t get_scalar (std::string usage, Range<int32_t> range = Range<int32_t>()) override
	{
		return io_.get_vec<int32_t>(usage, 1, range)[0];
	}

	std::string get_string (std::string usage, size_t len) override
	{
		return io_.get_string(usage, len);
	}


	void store_double (std::string usage, std::vector<double> value) override
	{
		io_.set_output(usage, value.begin(), value.end());
	}

	void store_int (std::string usage, std::vector<int32_t> value) override
	{
		io_.set_output(usage, value.begin(), value.end());
	}

	void store_string (std::string usage, std::string value) override
	{
		io_.set_output(usage, value.begin(), value.end());
	}

	void clear (void) override
	{
		can_send_ = false;
	}

private:
	GenIO io_;

	bool can_send_ = true;
};

struct OutSession final : public ModelledSession
{
	OutSession (testify::GeneratedCase gcase) : ModelledSession(gcase) {}

	std::vector<double> get_double (std::string usage, size_t len,
		Range<double> range = Range<double>()) override
	{
		try
		{
			auto& inputs = gcase_.inputs();
			auto it = inputs.find(usage);
			if (inputs.end() == it)
			{
				throw std::runtime_error(usage + " not found");
			}
			std::vector<double> out = unpack_double(it->second.data(), it->second.dtype());
			if (out.size() != len)
			{
				throw std::runtime_error("extracted int32_t vec invalid length");
			}
			for (double e : out)
			{
				if (e < range.min_ || e > range.max_)
				{
					throw std::runtime_error("extracted double value out of range");
				}
			}
			return out;
		}
		catch (...)
		{
			return get_vec<double>(len, range);
		}
	}

	std::vector<int32_t> get_int (std::string usage, size_t len,
		Range<int32_t> range = Range<int32_t>()) override
	{
		try
		{
			auto& inputs = gcase_.inputs();
			auto it = inputs.find(usage);
			if (inputs.end() == it)
			{
				throw std::runtime_error(usage + " not found");
			}
			std::vector<int32_t> out = unpack_int(it->second.data(), it->second.dtype());
			if (out.size() != len)
			{
				throw std::runtime_error("extracted int32_t vec invalid length");
			}
			for (int32_t e : out)
			{
				if (e < range.min_ || e > range.max_)
				{
					throw std::runtime_error("extracted int32_t value out of range");
				}
			}
			return out;
		}
		catch (...)
		{
			return get_vec<int32_t>(len, range);
		}
	}

	int32_t get_scalar (std::string usage, Range<int32_t> range = Range<int32_t>()) override
	{
		return get_int(usage, 1, range)[0];
	}

	std::string get_string (std::string usage, size_t len) override
	{
		try
		{
			auto& inputs = gcase_.inputs();
			auto it = inputs.find(usage);
			if (inputs.end() == it)
			{
				throw std::runtime_error(usage + " not found");
			}
			std::string out = unpack_string(it->second.data(), it->second.dtype());
			if (out.size() != len)
			{
				throw std::runtime_error("extracted int32_t vec invalid length");
			}
			return out;
		}
		catch (...)
		{
			return ::get_string(len);
		}
	}


	void store_double (std::string usage, std::vector<double> value) override {}

	void store_int (std::string usage, std::vector<int32_t> value) override {}

	void store_string (std::string usage, std::string value) override {}

	void clear (void) override {}
};

SESSION TestModel::get_session (std::string testname)
{
	testify::GeneratedCase model;
	try
	{
		model = get(testname);
		if (false == GENERATE_MODE)
		{
			return SESSION(new OutSession(model));
		}
	}
	catch (...) // fall back on generation mode
	{
		// log
	}
	TestModel::GENERATE_MODE = true;
	return SESSION(new GenSession(model, testname));
}

namespace simple
{

void INIT (std::string server_addr, bool genmode, size_t nretries)
{
	size_t seed = std::time(nullptr);
	get_engine().seed(seed);

	TestModel::GENERATE_MODE = genmode;
	ClientConfig config;
	config.host = server_addr;
	config.nretry = nretries;
	try
	{
		size_t grab_ncases;
		char* nrepeats = getenv("GTEST_REPEAT");
		if (nrepeats == nullptr)
		{
			grab_ncases = 100;
		}
		else
		{
			grab_ncases = atoi(nrepeats);
		}
		antero::INIT(grab_ncases, config);
	}
	catch (...)
	{
		// fall back on generation mode
		TestModel::GENERATE_MODE = true;
	}

	if (TestModel::GENERATE_MODE)
	{
		try
		{
			retro::INIT(config);
		}
		catch (...) {} // it's ok gen session cache data locally
	}
}

void SHUTDOWN (void)
{
	antero::SHUTDOWN();
	if (TestModel::GENERATE_MODE)
	{
		retro::SHUTDOWN();
	}
}

}
