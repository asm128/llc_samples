#include "ssiege_handle.h"
#include "ssiege_event_args.h"

using ::llc::get_value_namep, ::llc::get_enum_namep, ::llc::failed;
LLC_USING_TYPEINT();

llc::err_t	ssg::handleACTION_CHAR		(::ssg::SSiegeApp & app, const ::ssg::EViewMe & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { 
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

llc::err_t	ssg::handleWORLD_ADMIN		(::ssg::SSiegeApp & app, const ::ssg::EViewAdminWorld & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { 
	(void)outputEvents; (void)app; 
	const ::ssg::SArgsEvent	& eventArgs				= *(const ::ssg::SArgsEvent*)gameEvent.Data.begin();
	::ssg::printArgsEvent(eventArgs);
	switch(gameEvent.Type) { 
	default: llc_warning_unhandled_event(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Create		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Rename		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Delete		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Locate		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Rotate		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Resize		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Reskin		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Relocate	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Generate	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Initialize	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_ADMIN_Reset		: llc_warning_not_implemented(gameEvent); break; 
	} 
	return 0; 
}

llc::err_t	ssg::handleWORLD_EVENT		(::ssg::SSiegeApp & app, const ::ssg::EViewWorld & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { 
	(void)outputEvents; (void)app; 
	const ::ssg::SArgsEvent	& eventArgs				= *(const ::ssg::SArgsEvent*)gameEvent.Data.begin();
	::ssg::printArgsEvent(eventArgs);
	switch(gameEvent.Type) { 
	default: llc_warning_unhandled_event(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Tick			: 
		//llc_warning_not_implemented(gameEvent); 
		break;
	case ::ssg::WORLD_EVENT_Rain			: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Thunderstorm	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Earthquake		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Tornado			: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Wave			: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Lightning		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Wildfire		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_EVENT_Flood			: llc_warning_not_implemented(gameEvent); break; 
	} 
	return 0; 
}

llc::err_t	ssg::handleCLIENT_ASKS		(::ssg::SSiegeApp & app, const ::ssg::EViewClient & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { 
	(void)outputEvents; (void)app; 
	switch(gameEvent.Type) { 
	default: llc_warning_unhandled_event(gameEvent); break; 
	case ::ssg::CLIENT_ASKS_Join: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::CLIENT_ASKS_Quit: llc_warning_not_implemented(gameEvent); break; 
	} 
	return 0; 
}

llc::err_t	ssg::handleWORLD_SETUP		(::ssg::SSiegeApp & world, const ::ssg::EViewWorldSetup & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { 
	(void)outputEvents; (void)world; 
	switch(gameEvent.Type) { 
	default: llc_warning_unhandled_event(gameEvent); break; 
	case ::ssg::WORLD_SETUP_Seed		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_SETUP_MaxPlayers	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_SETUP_BlockSize	: llc_warning_not_implemented(gameEvent); break; 
	}
	return 0;
}

llc::err_t	ssg::handleWORLD_VALUE		(::ssg::SSiegeApp & world, const ::ssg::EViewWorldValue & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { 
	(void)outputEvents; (void)world; 
	switch(gameEvent.Type) { 
	default: llc_warning_unhandled_event(gameEvent); break; 
	case ::ssg::WORLD_VALUE_TimeCreated				: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_VALUE_TimeOffset				: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_VALUE_DaylightOffsetMinutes	: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_VALUE_DaylightRatioExtra		: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_VALUE_TimeLastSaved			: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_VALUE_WorldSize				: llc_warning_not_implemented(gameEvent); break; 
	case ::ssg::WORLD_VALUE_TimeScale				: llc_warning_not_implemented(gameEvent); break; 
	}
	return 0;
}
