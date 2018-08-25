#include "proto/testify.pb.h"

#ifndef TYPE_HPP
#define TYPE_HPP

template <typename T>
testify::DTYPE get_type (void)
{
	throw std::bad_function_call();
}

template <>
testify::DTYPE get_type<char> (void);

template <>
testify::DTYPE get_type<int8_t> (void);

template <>
testify::DTYPE get_type<uint8_t> (void);

template <>
testify::DTYPE get_type<double> (void);

template <>
testify::DTYPE get_type<float> (void);

template <>
testify::DTYPE get_type<int32_t> (void);

template <>
testify::DTYPE get_type<uint32_t> (void);

template <>
testify::DTYPE get_type<int64_t> (void);

template <>
testify::DTYPE get_type<uint64_t> (void);

#endif /* TYPE_HPP */
