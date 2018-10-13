#include "retroc/type.hpp"

#ifdef TYPE_HPP

template <>
DTYPE get_type<char> (void)
{
	return DTYPE::kDbytes;
}

template <>
DTYPE get_type<int8_t> (void)
{
	return DTYPE::kDbytes;
}

template <>
DTYPE get_type<uint8_t> (void)
{
	return DTYPE::kDbytes;
}

template <>
DTYPE get_type<double> (void)
{
	return DTYPE::kDdoubles;
}

template <>
DTYPE get_type<float> (void)
{
	return DTYPE::kDfloats;
}

template <>
DTYPE get_type<int32_t> (void)
{
	return DTYPE::kDint32S;
}

template <>
DTYPE get_type<uint32_t> (void)
{
	return DTYPE::kDuint32S;
}

template <>
DTYPE get_type<int64_t> (void)
{
	return DTYPE::kDint64S;
}

template <>
DTYPE get_type<uint64_t> (void)
{
	return DTYPE::kDuint64S;
}

#endif
