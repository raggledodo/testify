#include "retroc/type.hpp"

#ifdef TYPE_HPP

template <>
testify::DTYPE get_type<char> (void)
{
	return testify::BYTES;
}

template <>
testify::DTYPE get_type<int8_t> (void)
{
	return testify::BYTES;
}

template <>
testify::DTYPE get_type<uint8_t> (void)
{
	return testify::BYTES;
}

template <>
testify::DTYPE get_type<double> (void)
{
	return testify::DOUBLES;
}

template <>
testify::DTYPE get_type<float> (void)
{
	return testify::FLOATS;
}

template <>
testify::DTYPE get_type<int32_t> (void)
{
	return testify::INT32S;
}

template <>
testify::DTYPE get_type<uint32_t> (void)
{
	return testify::UINT32S;
}

template <>
testify::DTYPE get_type<int64_t> (void)
{
	return testify::INT64S;
}

template <>
testify::DTYPE get_type<uint64_t> (void)
{
	return testify::UINT64S;
}

#endif
