#include <functional>

#include "client/client.hpp"

#ifndef TYPE_HPP
#define TYPE_HPP

template <typename T>
DTYPE get_type (void)
{
	throw std::bad_function_call();
}

template <>
DTYPE get_type<char> (void);

template <>
DTYPE get_type<int8_t> (void);

template <>
DTYPE get_type<uint8_t> (void);

template <>
DTYPE get_type<double> (void);

template <>
DTYPE get_type<float> (void);

template <>
DTYPE get_type<int32_t> (void);

template <>
DTYPE get_type<uint32_t> (void);

template <>
DTYPE get_type<int64_t> (void);

template <>
DTYPE get_type<uint64_t> (void);

#endif /* TYPE_HPP */
