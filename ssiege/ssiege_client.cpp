#include "ssiege_client.h"
#include "ssiege_event_args.h"

using ::llc::get_value_namep, ::llc::get_enum_namep, ::llc::failed;
LLC_USING_TYPEINT();

llc::err_t		ssg::ssiegeClientUpdate		(::ssg::SSiegeClient & app, double lastTimeSeconds, const ::llc::pobj<::llc::SInput> & inputState, ::llc::vpobj<::llc::SEventSystem> systemEvents) { 
	if_fail_fe(::ssg::ssiegeUpdate(app, lastTimeSeconds, inputState, systemEvents, [&app](::llc::pobj<::ssg::EventSSiege> & _eventToProcess, ::llc::apobj<::ssg::EventSSiege> & outputEvents) {
		if(!_eventToProcess)
			return false;

		const ::ssg::EventSSiege	& eventToProcess		= *_eventToProcess;
		info_printf("%s", ::llc::get_value_namep(eventToProcess.Type));

		llc::err_t			result			= 0; 
		switch(eventToProcess.Type) {
		CASE_SSG_EVENT(result, app, WORLD_EVENT, outputEvents); 
		CASE_SSG_EVENT(result, app, WORLD_ADMIN, outputEvents); 
		CASE_SSG_EVENT(result, app, ACT_SAILING, outputEvents); 
		CASE_SSG_EVENT(result, app, ACTION_CHAR, outputEvents); 
		CASE_SSG_EVENT(result, app, ACT_ENGINES, outputEvents); 
		CASE_SSG_EVENT(result, app, ACT_AIRSHIP, outputEvents); 
		CASE_SSG_EVENT(result, app, ACT_WHEELED, outputEvents); 
		CASE_SSG_EVENT(result, app, CLIENT_ASKS, outputEvents); 
		CASE_SSG_EVENT(result, app, WORLD_SETUP, outputEvents); 
		CASE_SSG_EVENT(result, app, WORLD_VALUE, outputEvents); 
		default: 
			llc_warning_unhandled_event(eventToProcess); 
			break;
		}
		return bool(result == 1);
	})); 
	return 0; 
}

llc::err_t	ssg::handleACTION_CHAR		(::ssg::SSiegeClient & app, const ::ssg::EViewMe & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { 
	(void)outputEvents; (void)app; 
	const ::ssg::SArgsEvent	& eventArgs				= *(const ::ssg::SArgsEvent*)gameEvent.Data.begin();
	::ssg::printArgsEvent(eventArgs);
	switch(gameEvent.Type) { 
	default: llc_warning_unhandled_event(gameEvent); break; 
	case ::ssg::ACTION_CHAR_Walk	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Turn	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Rush	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Hide	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Dash	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Jump	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Duck	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Warp	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Pick	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Grab	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Wear	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Use		: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Spawn	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Shoot	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Throw	: llc_warning_not_implemented(gameEvent); break;
	case ::ssg::ACTION_CHAR_Sleep	: llc_warning_not_implemented(gameEvent); break;
	} 
	return 0; 
}

llc::err_t	ssg::handleWORLD_ADMIN		(::ssg::SSiegeClient & app, const ::ssg::EViewAdminWorld & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { 
	(void)outputEvents; (void)app; 
	const ::ssg::SArgsEvent	& eventArgs				= *(const ::ssg::SArgsEvent*)gameEvent.Data.begin();
	::ssg::printArgsEvent(eventArgs);
	switch(gameEvent.Type) { 
	default: llc_warning_unhandled_event(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Create	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Rename	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Delete	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Locate	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Rotate	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Resize	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Reskin	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Relocate	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Generate	: llc_warning_not_implemented(gameEvent); break; 
	} 
	return 0; 
}

llc::err_t	ssg::handleWORLD_EVENT		(::ssg::SSiegeClient & app, const ::ssg::EViewWorld & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { 
	(void)outputEvents; (void)app; 
	const ::ssg::SArgsEvent	& eventArgs				= *(const ::ssg::SArgsEvent*)gameEvent.Data.begin();
	::ssg::printArgsEvent(eventArgs);
	switch(gameEvent.Type) { 
	default: llc_warning_unhandled_event(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Rain			: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Thunderstorm	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Earthquake	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Tornado		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Wave			: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Lightning		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Wildfire		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Flood			: llc_warning_not_implemented(gameEvent); break; 
	} 
	return 0; 
}

llc::err_t	ssg::handleCLIENT_ASKS		(::ssg::SSiegeClient & app, const ::ssg::EViewClient & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { 
	(void)outputEvents; (void)app; 
	switch(gameEvent.Type) { 
	default: llc_warning_unhandled_event(gameEvent); break; 
	case ::ssg::CLIENT_ASKS_Join: 
	case ::ssg::CLIENT_ASKS_Quit: 
		if_fail_fe(::llc::eventEnqueueChild(app.EventsToSend, ::ssg::SSG_EVENT_CLIENT_ASKS, gameEvent.Type, gameEvent.Data));
		return 1; 
	} 
	return 0; 
}

llc::err_t	ssg::handleWORLD_SETUP		(::ssg::SSiegeClient & world, const ::ssg::EViewWorldSetup & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { 
	(void)outputEvents; (void)world; 
	switch(gameEvent.Type) { 
	default: llc_warning_unhandled_event(gameEvent); break; 
	case ::ssg::WORLD_SETUP_Seed		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_SETUP_MaxPlayers: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_SETUP_BlockSize	: llc_warning_not_implemented(gameEvent); break; 
	}
	return 0;
}

llc::err_t	ssg::handleWORLD_VALUE		(::ssg::SSiegeClient & world, const ::ssg::EViewWorldValue & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { 
	(void)outputEvents; (void)world; 
	switch(gameEvent.Type) { 
	default: llc_warning_unhandled_event(gameEvent); break; 
	case ::ssg::WORLD_VALUE_TimeCreated			: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_VALUE_TimeOffset			: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_VALUE_DaylightOffsetMinutes	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_VALUE_DaylightRatioExtra	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_VALUE_TimeLastSaved			: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_VALUE_WorldSize				: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_VALUE_TimeScale				: llc_warning_not_implemented(gameEvent); break; 
	}
	return 0;
}
