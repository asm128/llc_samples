#include "ssiege.h"
#include "ssiege_event.h"

#ifndef SSIEGE_CLIENT_H_23701
#define SSIEGE_CLIENT_H_23701

namespace ssg
{
	struct SSiegeClient : ::ssg::SSiegeApp {
		::llc::apobj<::ssg::EventSSiege>	EventsToSend;
		::llc::apobj<::ssg::EventSSiege>	EventsReceived;
	};

	llc::err_t			handleWORLD_EVENT		(::ssg::SSiegeClient & world, const ::llc::SEventView<ssg::WORLD_EVENT> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents); // { (void)world, (void)gameEvent, (void)outputEvents; return 0; }
	llc::err_t			handleWORLD_ADMIN		(::ssg::SSiegeClient & world, const ::llc::SEventView<ssg::WORLD_ADMIN> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents); // { (void)world, (void)gameEvent, (void)outputEvents; return 0; }
	llc::err_t			handleACTION_CHAR		(::ssg::SSiegeClient & world, const ::llc::SEventView<ssg::ACTION_CHAR> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents); // { (void)world, (void)gameEvent, (void)outputEvents; return 0; }
	stin	llc::err_t	handleACT_SAILING		(::ssg::SSiegeClient & world, const ::llc::SEventView<ssg::ACT_SAILING> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { (void)world, (void)gameEvent, (void)outputEvents; return 0; }
	stin	llc::err_t	handleACT_ENGINES		(::ssg::SSiegeClient & world, const ::llc::SEventView<ssg::ACT_ENGINES> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { (void)world, (void)gameEvent, (void)outputEvents; return 0; }
	stin	llc::err_t	handleACT_AIRSHIP		(::ssg::SSiegeClient & world, const ::llc::SEventView<ssg::ACT_AIRSHIP> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { (void)world, (void)gameEvent, (void)outputEvents; return 0; }
	stin	llc::err_t	handleACT_WHEELED		(::ssg::SSiegeClient & world, const ::llc::SEventView<ssg::ACT_WHEELED> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { (void)world, (void)gameEvent, (void)outputEvents; return 0; }
	llc::err_t			handleCLIENT_ASKS		(::ssg::SSiegeClient & world, const ::llc::SEventView<ssg::CLIENT_ASKS> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);// { (void)world, (void)gameEvent, (void)outputEvents; return 0; }
	llc::err_t			handleWORLD_SETUP		(::ssg::SSiegeClient & world, const ::llc::SEventView<ssg::WORLD_SETUP> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);// { (void)world, (void)gameEvent, (void)outputEvents; return 0; }
	llc::err_t			handleWORLD_VALUE		(::ssg::SSiegeClient & world, const ::llc::SEventView<ssg::WORLD_VALUE> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);// { (void)world, (void)gameEvent, (void)outputEvents; return 0; }

	llc::err_t		ssiegeClientUpdate		(SSiegeClient & ssiege, double lastTimeSeconds, const ::llc::pobj<::llc::SInput> & inputState, ::llc::vpobj<::llc::SEventSystem> systemEvents);
} // namespace 

#endif // SSIEGE_CLIENT_H_23701
