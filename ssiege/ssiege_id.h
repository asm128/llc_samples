#include "llc_array_pod.h"

#ifndef SSIEGE_ID_H_23701
#define SSIEGE_ID_H_23701

namespace ssg
{
#pragma pack(push, 1)
	LLC_USING_TYPEINT();
	tydf	u2_t							ssiegeid_t;
	tydf	const ::ssg::ssiegeid_t			ssiegeid_c;
	tydf	::llc::apod<::ssg::ssiegeid_t>	assiegeid;
	tydf	::llc::view<::ssg::ssiegeid_t>	vssiegeid;
	tydf	::llc::view<ssiegeid_c>			vssiegecid;

	stxp ssiegeid_t	SSIEGEID_INVALID		= (ssiegeid_t)-1;
#pragma pack(pop)
} // namespace

#endif // SSIEGE_ID_H_23701