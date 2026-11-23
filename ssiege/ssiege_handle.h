#include "ssiege.h"

#ifndef SSIEGE_HANDLE_H_23701
#define SSIEGE_HANDLE_H_23701

namespace ssg
{
	llc::err_t			handleWORLD_EVENT(::ssg::SSiegeApp & world, const ::llc::SEventView<ssg::WORLD_EVENT> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);
	llc::err_t			handleWORLD_ADMIN(::ssg::SSiegeApp & world, const ::llc::SEventView<ssg::WORLD_ADMIN> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);
	llc::err_t			handleACTION_CHAR(::ssg::SSiegeApp & world, const ::llc::SEventView<ssg::ACTION_CHAR> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);
	stin	llc::err_t	handleACT_SAILING(::ssg::SSiegeApp & world, const ::llc::SEventView<ssg::ACT_SAILING> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { (void)world, (void)gameEvent, (void)outputEvents; return 0; };
	stin	llc::err_t	handleACT_ENGINES(::ssg::SSiegeApp & world, const ::llc::SEventView<ssg::ACT_ENGINES> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { (void)world, (void)gameEvent, (void)outputEvents; return 0; }
	stin	llc::err_t	handleACT_AIRSHIP(::ssg::SSiegeApp & world, const ::llc::SEventView<ssg::ACT_AIRSHIP> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { (void)world, (void)gameEvent, (void)outputEvents; return 0; }
	stin	llc::err_t	handleACT_WHEELED(::ssg::SSiegeApp & world, const ::llc::SEventView<ssg::ACT_WHEELED> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { (void)world, (void)gameEvent, (void)outputEvents; return 0; }
	llc::err_t			handleCLIENT_ASKS(::ssg::SSiegeApp & world, const ::llc::SEventView<ssg::CLIENT_ASKS> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);
	llc::err_t			handleWORLD_SETUP(::ssg::SSiegeApp & world, const ::llc::SEventView<ssg::WORLD_SETUP> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);
	llc::err_t			handleWORLD_VALUE(::ssg::SSiegeApp & world, const ::llc::SEventView<ssg::WORLD_VALUE> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);
} // namespace

#endif // SSIEGE_HANDLE_H_23701
