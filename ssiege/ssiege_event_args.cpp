#include "ssiege_event_args.h"
#include "llc_string_compose.h"

llc::err_t			ssg::printArgsEvent		(const ::ssg::SArgsEvent & eventArgs, bool debugOutput) {
	if(debugOutput)
		info_printf(
			"\nCoordSource  : " N3_F2
			"\nCoordTarget  : " N3_F2
			"\nIdSource     : %i"
			"\nIdTarget     : %i"
			"\nValue.AsInt  : %i"
			"\nValue.AsFloat: %f"
			"\nId           : %i"
			, llc_xyz(eventArgs.CoordSource)
			, llc_xyz(eventArgs.CoordTarget)
			, eventArgs.IdSource		
			, eventArgs.IdTarget		
			, eventArgs.Value.AsInt		
			, eventArgs.Value.AsFloat	
			, eventArgs.Id	
			);
	else
		printf(
			"\nCoordSource  : " N3_F2
			"\nCoordTarget  : " N3_F2
			"\nIdSource     : %i"
			"\nIdTarget     : %i"
			"\nValue.AsInt  : %i"
			"\nValue.AsFloat: %f"
			"\nId           : %i"
			, llc_xyz(eventArgs.CoordSource)
			, llc_xyz(eventArgs.CoordTarget)
			, eventArgs.IdSource		
			, eventArgs.IdTarget		
			, eventArgs.Value.AsInt		
			, eventArgs.Value.AsFloat	
			, eventArgs.Id	
			);
	return 0;
}

llc::err_t			ssg::printArgsEvent		(const ::ssg::SArgsEvent & eventArgs, ::llc::asc_t & output) {
	sc_t						temp	[1024]				= {};
	::llc::sprintf_s(temp, 
		"\nCoordSource  : " N3_F2
		"\nCoordTarget  : " N3_F2
		"\nIdSource     : %i"
		"\nIdTarget     : %i"
		"\nValue.AsInt  : %i"
		"\nValue.AsFloat: %f"
		"\nId           : %i"
		, llc_xyz(eventArgs.CoordSource)
		, llc_xyz(eventArgs.CoordTarget)
		, eventArgs.IdSource		
		, eventArgs.IdTarget		
		, eventArgs.Value.AsInt		
		, eventArgs.Value.AsFloat	
		, eventArgs.Id	
		);
	if_fail_fe(::llc::append_strings(output, temp));
	return 0;
}
