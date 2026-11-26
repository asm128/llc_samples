#include "ssiege_game.h"

#ifndef SSIEGE_GAME_HANDLE_H
#define SSIEGE_GAME_HANDLE_H

namespace ssg
{
	llc::err_t			handleWORLD_EVENT(::ssg::SSiegeGame & world, const ::llc::SEventView<ssg::WORLD_EVENT> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);
	llc::err_t			handleWORLD_ADMIN(::ssg::SSiegeGame & world, const ::llc::SEventView<ssg::WORLD_ADMIN> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);
	llc::err_t			handleACTION_CHAR(::ssg::SSiegeGame & world, const ::llc::SEventView<ssg::ACTION_CHAR> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);
	stin	llc::err_t	handleACT_SAILING(::ssg::SSiegeGame & world, const ::llc::SEventView<ssg::ACT_SAILING> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { (void)world, (void)gameEvent, (void)outputEvents; return 0; };
	stin	llc::err_t	handleACT_ENGINES(::ssg::SSiegeGame & world, const ::llc::SEventView<ssg::ACT_ENGINES> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { (void)world, (void)gameEvent, (void)outputEvents; return 0; };
	stin	llc::err_t	handleACT_AIRSHIP(::ssg::SSiegeGame & world, const ::llc::SEventView<ssg::ACT_AIRSHIP> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { (void)world, (void)gameEvent, (void)outputEvents; return 0; };
	stin	llc::err_t	handleACT_WHEELED(::ssg::SSiegeGame & world, const ::llc::SEventView<ssg::ACT_WHEELED> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents) { (void)world, (void)gameEvent, (void)outputEvents; return 0; };
	llc::err_t			handleCLIENT_ASKS(::ssg::SSiegeGame & world, const ::llc::SEventView<ssg::CLIENT_ASKS> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);
	llc::err_t			handleWORLD_SETUP(::ssg::SSiegeGame & world, const ::llc::SEventView<ssg::WORLD_SETUP> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);
	llc::err_t			handleWORLD_VALUE(::ssg::SSiegeGame & world, const ::llc::SEventView<ssg::WORLD_VALUE> & gameEvent, ::llc::apobj<::ssg::EventSSiege> & outputEvents);
} // namespace

#endif // SSIEGE_GAME_HANDLE_H
