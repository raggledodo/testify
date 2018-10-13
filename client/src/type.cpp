#include "client/type.hpp"

#ifdef DORA_TYPE_HPP

namespace dora
{

template <>
DtypeT get_type<char> (void)
{
	return DtypeT::kDbytes;
}

template <>
DtypeT get_type<int8_t> (void)
{
	return DtypeT::kDbytes;
}

template <>
DtypeT get_type<uint8_t> (void)
{
	return DtypeT::kDbytes;
}

template <>
DtypeT get_type<double> (void)
{
	return DtypeT::kDdoubles;
}

template <>
DtypeT get_type<float> (void)
{
	return DtypeT::kDfloats;
}

template <>
DtypeT get_type<int32_t> (void)
{
	return DtypeT::kDint32S;
}

template <>
DtypeT get_type<uint32_t> (void)
{
	return DtypeT::kDuint32S;
}

template <>
DtypeT get_type<int64_t> (void)
{
	return DtypeT::kDint64S;
}

template <>
DtypeT get_type<uint64_t> (void)
{
	return DtypeT::kDuint64S;
}

}

#endif
