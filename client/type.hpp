#include <functional>

#include "proto/testify.pb.h"

#ifndef DORA_TYPE_HPP
#define DORA_TYPE_HPP

namespace dora
{

using DtypeT = testify::CaseData::DataCase;

template <typename T>
DtypeT get_type (void)
{
	throw std::bad_function_call();
}

template <>
DtypeT get_type<char> (void);

template <>
DtypeT get_type<int8_t> (void);

template <>
DtypeT get_type<uint8_t> (void);

template <>
DtypeT get_type<double> (void);

template <>
DtypeT get_type<float> (void);

template <>
DtypeT get_type<int32_t> (void);

template <>
DtypeT get_type<uint32_t> (void);

template <>
DtypeT get_type<int64_t> (void);

template <>
DtypeT get_type<uint64_t> (void);

}

#endif // DORA_TYPE_HPP 
