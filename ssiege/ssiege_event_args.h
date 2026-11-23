#include "ssiege_id.h"

#include "llc_n3.h"

#ifndef SSG_EVENT_ARGS_H_23701
#define SSG_EVENT_ARGS_H_23701

namespace ssg
{
#pragma pack(push, 1)
	struct SArgsEvent { // TODO: This will do for now. Maybe we can use a json format later on for more flexibility customizing for each event type.
		ssiegeid_t				IdSource			= {}; 
		ssiegeid_t				IdTarget			= {}; 
		::llc::n3f2_t			CoordSource			= {}; 
		::llc::n3f2_t			CoordTarget			= {}; 

		union UValue { 
			s2_t		AsInt; 
			float		AsFloat; 
		}						Value				= {};

		ssiegeid_t				Id					= {};
	};
#pragma pack(pop)

	llc::err_t			printArgsEvent		(const ::ssg::SArgsEvent & eventArgs, bool debugOutput = false);
	llc::err_t			printArgsEvent		(const ::ssg::SArgsEvent & eventArgs, ::llc::asc_t & output);
} // namespace

#endif // SSG_EVENT_ARGS_H_23701
