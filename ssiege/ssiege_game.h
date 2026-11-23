#include "llc_game.h"

#include "ssiege_world_view.h"

#include "llc_engine_planetary_system.h"

#ifndef SSIEGE_GAME_H
#define SSIEGE_GAME_H

namespace ssg
{
#pragma pack(push, 1)
#pragma pack(pop)
	//GDEFINE_ENUM_TYPE(SCENE_TYPE, u0_t);Likely not needed
	//GDEFINE_ENUM_VALUE(SCENE_TYPE, OuterSpace	, 0);
	//GDEFINE_ENUM_VALUE(SCENE_TYPE, Planetary	, 1);
	//GDEFINE_ENUM_VALUE(SCENE_TYPE, Submarine	, 2);
	//struct SGameObject {
	//	s1_t							Definition			=  0;
	//	s1_t							Modifier			=  0;
	//	s1_t							Level				=  1;
	//	s1_t							Owner				= -1;
	//};

	struct SSiegeGame {
		::llc::SGame					Global				= {};
		::llc::SEnginePlanetarySystem	SolarSystem			= {};
		::ssg::SWorldView				World				= {};
		::llc::SCamera					Camera				= {};

		::llc::eid_t					Controlled			= ::llc::EID_INVALID;
		::llc::aobj<::llc::SPlayer>		Players;

		::llc::eid_t					ShipCore			= ::llc::EID_INVALID;
		::llc::eid_t					ShipRing			= ::llc::EID_INVALID;
		::llc::aeid						ShipEngines			= {};
		::llc::aeid						ShipWeapons			= {};

		::llc::SEngine					Engine				= {};

		llc::err_t					Save				(::llc::au0_t & output)	const	{ 
			if_fail_fe(Global.Save(output));
			if_fail_fe(SolarSystem.Save(output));
			if_fail_fe(World.Save(output));
			if_fail_fe(Engine.Save(output));
			return 0;
		}

		llc::err_t					Load				(::llc::vcu0_t & input)	{ 
			if_fail_fe(Global.Load(input));
			if_fail_fe(SolarSystem.Load(input));
			if_fail_fe(World.Load(input));
			if_fail_fe(Engine.Load(input));
			return 0;
		}

	};

	llc::err_t					ssiegeGameSetup		(::ssg::SSiegeGame & world);
	llc::err_t					ssiegeGameUpdate	(::ssg::SSiegeGame & world, ::llc::vpobj<::ssg::EventSSiege> inputEvents, ::llc::apobj<::ssg::EventSSiege> & outputEvents, double secondsElapsed);
	llc::err_t					ssiegeGameDraw		(::ssg::SSiegeGame & world
		, ::llc::rtbgra8d32			& backBuffer
		, const ::llc::n3f2_t		& cameraPosition
		, const ::llc::n3f2_t		& cameraTarget
		, const ::llc::n3f2_t		& cameraUp = {0, 1, 0}
		, const ::llc::minmaxf2_t	& nearFar = {.1f, 10000.f}
		);
} // namespace

#endif // SSIEGE_GAME_H
