#include "ssiege_game_handle.h"
#include "ssiege_game.h"
#include "llc_timer.h"

using ::llc::get_value_namep, ::llc::get_enum_namep, ::llc::failed;
LLC_USING_TYPEINT();

stxp	u2_t			DEFAULT_ARM_COUNT		= 6;

stxp	float				SCALE_SHIP				= .001f;	// 

stxp	float				SHIP_CORE_RADIUS		= SCALE_SHIP * .5f;	// the ship core diameter represents the size unit
// 
stxp	float				SHIP_RING_HEIGHT		= SHIP_CORE_RADIUS * 2;							// 
stxp	float				SHIP_RING_RADIUS_MAX	= SHIP_CORE_RADIUS * 10;						// 
stxp	::llc::minmaxf2_t	SHIP_RING_RADIUS		= {SHIP_RING_RADIUS_MAX - SHIP_CORE_RADIUS, SHIP_RING_RADIUS_MAX};	// 
stxp	float				SHIP_RING_BORDER		= SHIP_RING_RADIUS.Length();	// 
stxp	float				SHIP_RING_THICKNESS		= ::llc::max(SHIP_RING_HEIGHT, SHIP_RING_BORDER);	// 

stxp	float				SHIP_CRANE_BOX			= SHIP_RING_THICKNESS + SHIP_RING_THICKNESS * .5f;	// 
stxp	float				SHIP_CRANE_BOX_HALF		= SHIP_CRANE_BOX * .5f;	// 
stxp	float				SHIP_CRANE_IRON			= SHIP_CRANE_BOX_HALF * .125f;		// 
stxp	float				SHIP_CRANE_IRON_HALF	= SHIP_CRANE_IRON * .5f;		// 
stxp	float				SHIP_CRANE_WHEEL_RADIUS	= SHIP_CRANE_BOX_HALF * .5f;		// 
stxp	float				SHIP_CRANE_WHEEL_HEIGHT	= SHIP_CRANE_BOX_HALF * .5f;		// 

stxp	float				SHIP_ARM_RADIUS			= SHIP_CORE_RADIUS * .125f;	// 
stxp	float				SHIP_ARM_LENGTH			= SHIP_RING_RADIUS_MAX;	// 

stxp	float				SHIP_GRIP_HEIGHT		= SHIP_CORE_RADIUS * 5;							// 
stxp	float				SHIP_GRIP_RADIUS_MAX	= SHIP_CORE_RADIUS * 3;						// 
stxp	::llc::minmaxf2_t	SHIP_GRIP_RADIUS		= {SHIP_GRIP_RADIUS_MAX - SHIP_CORE_RADIUS * .125f, SHIP_GRIP_RADIUS_MAX};	// 

static	llc::err_t	createShipCore		(::llc::SEngine & engine) {
	::llc::SParamsSphere		sphere				= {};
	sphere.Radius			= ::SHIP_CORE_RADIUS;
	::llc::eid_t				iShip; 
	if_fail_fe(iShip = engine.CreateSphere(sphere, "Ship Core"));
	engine.SetRotation(iShip, ::llc::n3f2_t{0, 0, float(::llc::math_pi_2 * .15)});
	return iShip;
}

static	llc::err_t	createShipRing		(::llc::SEngine & engine, ::llc::vcst_t entityName = "Ship Ring") {
	::llc::eid_t				iShipRing; 
	{
		::llc::SParamsTube			ring				= {};
		ring.Origin.y			= ::SHIP_RING_HEIGHT * .5f;
		ring.Height				= ::SHIP_RING_HEIGHT;
		ring.RadiusYMin			= ::SHIP_RING_RADIUS;
		ring.RadiusYMax			= ::SHIP_RING_RADIUS;
		ring.CellCount.x		= 32;

		if_fail_fe(iShipRing = engine.CreateTube(ring, entityName));
	}
	{
		::llc::SParamsCircle		energy				= {};
		energy.Radius			= ::SHIP_RING_RADIUS.Min;
		energy.Slices			= 32;

		::llc::eid_t				iRingEnergy; 
		if_fail_fe(iRingEnergy = engine.CreateDisc(energy, "Energy Disc"));
		engine.SetColorDiffuse(iRingEnergy, {.2f, .5f, 1, .25f});
		engine.Entities.SetParent(iRingEnergy, iShipRing);
	}
	engine.SetRotation(iShipRing, ::llc::n3f2_t{0, float(::llc::math_pi * .25),});
	return iShipRing; 
}

static	llc::err_t	createShipCraneBox		(::llc::SEngine & engine) {
	::llc::eid_t				iArmRoot;
	{	// construct the grip around the ship ring
		::llc::SParamsBox			base					= {};
		base.HalfSizes.From(::SHIP_CRANE_BOX * .5f);
		base.Origin				= base.HalfSizes;
		if_fail_fe(iArmRoot = engine.CreateBox(base, "Crane Base"));
		engine.SetColorDiffuse(iArmRoot, {1, 1, 1, .25f});
		//engine.SetHidden(iArmRoot, true);
	}
	for(u2_t iAxis = 0; iAxis < 3; ++iAxis)
	for(u2_t iIron = 0; iIron < 4; ++iIron) {
		::llc::eid_t				iArmBaseBorder;
		::llc::SParamsBox			iron					= {};
		const u2_t				iRow					= iIron / 2;
		const u2_t				iCol					= iIron % 2;
		iron.HalfSizes.SetAxis  (iAxis, ::SHIP_CRANE_BOX_HALF);
		iron.HalfSizes.SetOthers(iAxis, ::SHIP_CRANE_IRON_HALF);
		iron.Origin				= iron.HalfSizes;
		if_fail_fe(iArmBaseBorder = engine.CreateBox(iron, "Crane Base Iron"));
	
		::llc::n3f2_t				position				= {};
		position.SetAxis((iAxis + 1) % 3, (::SHIP_CRANE_BOX_HALF - SHIP_CRANE_IRON_HALF) * (iRow ? 1 : -1));
		position.SetAxis((iAxis + 2) % 3, (::SHIP_CRANE_BOX_HALF - SHIP_CRANE_IRON_HALF) * (iCol ? 1 : -1));
		engine.SetPosition(iArmBaseBorder, position);
		if_fail_fe(engine.Entities.SetParent(iArmBaseBorder, iArmRoot));
	}
	return iArmRoot;
}

static	llc::err_t	createShipCraneBar	(::llc::SEngine & engine, ::llc::vcst_t entityName = "Crane Bar") {
	// construct the arm itself
	::llc::SParamsCylinderWall	bar					= {};
	bar.Radius				= {::SHIP_ARM_RADIUS, ::SHIP_ARM_RADIUS};
	bar.Height				= ::SHIP_ARM_LENGTH;
	::llc::eid_t				iBar;
	if_fail_fef(iBar = engine.CreateCylinder(bar, entityName), "entityName: '%.*s'", LLC_VIEW_ARGS(entityName));
	engine.SetOrientation(iBar, ::llc::quatf2_t{1, 0, 0, 1}.Normalize());
	engine.SetPosition(iBar, {0, ::SHIP_CRANE_WHEEL_RADIUS * .5f, });
	return iBar;
}

static	llc::err_t	createShipCraneWheel(::llc::SEngine & engine, ::llc::vcst_t entityName = "Crane Wheel") {
	// construct the arm itself
	::llc::SParamsCylinderWall	wheel				= {};
	wheel.Radius			= ::llc::dim2<::llc::minmaxf2_t>(::SHIP_CRANE_WHEEL_RADIUS);
	wheel.Height			= ::SHIP_CRANE_WHEEL_HEIGHT;
	::llc::eid_t				iWheel;
	if_fail_fef(iWheel = engine.CreateCylinder(wheel, entityName), "entityName: '%.*s'", LLC_VIEW_ARGS(entityName));
	engine.SetOrientation(iWheel, ::llc::quatf2_t{1, 0, 0, 1}.Normalize());
	return iWheel;
}

static	llc::err_t	createShipCraneGrip	(::llc::SEngine & engine, ::llc::vcst_t entityName = "Crane Grip") {
	::llc::eid_t				iShipRing; 
	{
		::llc::SParamsTube			ring				= {};
		ring.Origin.y			= ::SHIP_GRIP_HEIGHT * .5f;
		ring.Height				= ::SHIP_GRIP_HEIGHT;
		ring.RadiusYMin			= ::SHIP_GRIP_RADIUS;
		ring.RadiusYMax			= ::SHIP_GRIP_RADIUS;
		ring.CellCount.x		= 32;

		if_fail_fe(iShipRing = engine.CreateTube(ring, entityName));
	}
	{
		::llc::SParamsCircle		energy				= {};
		energy.Radius			= ::SHIP_RING_RADIUS.Min;
		energy.Slices			= 32;

		::llc::eid_t				iRingEnergy; 
		if_fail_fe(iRingEnergy = engine.CreateDisc(energy, "Energy Disc"));
		engine.SetColorDiffuse(iRingEnergy, {.2f, .5f, 1, .25f});
		engine.Entities.SetParent(iRingEnergy, iShipRing);
	}
	engine.SetRotation(iShipRing, ::llc::n3f2_t{0, float(::llc::math_pi * .25), -float(::llc::math_pi_2 * .15f)});
	return iShipRing; 
}

static	llc::err_t	createShipCrane		(::llc::SEngine & engine) {
	::llc::eid_t				iCrane; 
	{
		::llc::SParamsSphere		sphere				= {};
		sphere.Radius			= ::SHIP_CORE_RADIUS;
		if_fail_fe(iCrane = engine.CreateSphere(sphere, "Ship Crane"));
	}
	engine.SetColorDiffuse(iCrane, {.25f, 1, 1, .5f});
	{
		::llc::eid_t				iBox;
		if_fail_fe(iBox = ::createShipCraneBox(engine));	
		if_fail_fe(engine.Entities.SetParent(iBox, iCrane));
	}
	stxp	float				OFFSET_CORNER		= ::SHIP_CRANE_BOX_HALF + ::SHIP_CRANE_IRON_HALF;
	for(u2_t iArm = 0; iArm < 2; ++iArm) {
		const bool					isLeft				= (1 + iArm) % 2;
		if(isLeft)
			continue;

		::llc::eid_t				iWheel;
		if_fail_fe(iWheel = ::createShipCraneWheel(engine, isLeft ? ::llc::vcst_t{"Ship Crane Wheel Left?"} : ::llc::vcst_t{"Ship Crane Wheel Right?"}));
		::llc::n3f2_t				barPosition			= {OFFSET_CORNER, -::SHIP_CRANE_BOX_HALF, OFFSET_CORNER * (isLeft ? -1 : 1)};
		engine.SetPosition(iWheel, barPosition);
		engine.SetRotation(iWheel, ::llc::n3f2_t{0, 0, float(::llc::math_pi_2 * .15)});
		if_fail_fe(engine.Entities.SetParent(iWheel, iCrane));

		::llc::eid_t				iBar;	// Create bar
		if_fail_fe(iBar = ::createShipCraneBar(engine, isLeft ? ::llc::vcst_t{"Ship Crane Bar Left?"} : ::llc::vcst_t{"Ship Crane Bar Right?"}));
		if_fail_fe(engine.Entities.SetParent(iBar, iWheel));

		::llc::eid_t				iGrip;	// Create bar
		if_fail_fe(iGrip = ::createShipCraneGrip(engine, isLeft ? ::llc::vcst_t{"Ship Crane Grip Left?"} : ::llc::vcst_t{"Ship Crane Grip Right?"}));
		::llc::n3f2_t				tubePosition			= {::SHIP_GRIP_RADIUS_MAX, ::SHIP_ARM_LENGTH, OFFSET_CORNER * (isLeft ? -1 : 1)};
		engine.SetPosition(iGrip, tubePosition);
		if_fail_fe(engine.Entities.SetParent(iGrip, iBar));
	}
	return iCrane; 
}

////static	llc::err_t	shipEngineCreate	(::llc::SEngine & engine) {
////	::llc::SParamsRing			ring			= {};
////	ring.Height				= .0005f;
////	ring.RadiusYMin.Max		*= .005f;
////	ring.RadiusYMin.Min		*= .005f;
////	ring.RadiusYMax.Max		*= .005f;
////	ring.RadiusYMax.Min		*= .005f;
////
////	::llc::eid_t				iShipRing; 
////	if_fail_fe(iShipRing = engine.CreateRing(ring));
////	engine.SetRotation(iShipRing, ::llc::n3f2_t{0, float(::llc::math_pi * .25),});
////	return iShipRing; 
////}

static	llc::err_t	shipCreate			(::ssg::SSiegeGame & world) { 
	if_fail_fe(world.ShipCore = ::createShipCore(world.Engine));
	if_fail_fe(world.ShipRing = ::createShipRing(world.Engine));
	const int					armCount			= ::DEFAULT_ARM_COUNT;
	for(u2_t iArm = 0; iArm < armCount; ++iArm) {
		::llc::eid_t				iArmRoot			= ::createShipCrane(world.Engine);
		::llc::n3f2_t				position			= {::SHIP_RING_RADIUS.Middle()};
		position.RotateY(::llc::math_2pi * (1.0f / armCount) * iArm);
		world.Engine.SetOrientation(iArmRoot, ::llc::quatf2_t{}.MakeFromEuler(0, -(float)::llc::math_2pi * (1.0f / armCount) * iArm, 0).Normalize());
		world.Engine.SetPosition(iArmRoot, position);
		world.Engine.Entities.SetParent(iArmRoot, world.ShipCore);
	}
	
	world.Engine.SetPosition	(world.ShipCore, (world.Camera.Position * .95) + ::llc::n3f2_t{-.005f});
	world.Engine.SetOrientation	(world.ShipCore, ::llc::quatf2_t{}.MakeFromEuler(::llc::math_pi_2 * .5, .25f, ::llc::math_pi_2));
	world.Engine.Entities.SetParent(world.ShipRing, world.ShipCore);

	return 0; 
}

llc::err_t			ssg::ssiegeGameSetup	(::ssg::SSiegeGame & world) {
	if_fail_fe(::llc::planetarySystemSetup(world.SolarSystem, world.Engine, "llc_solar_system.json"));
	if_fail_fe(::shipCreate(world));
	return 0;
}

llc::err_t			ssg::ssiegeGameUpdate	(::ssg::SSiegeGame & world, ::llc::vpobj<::ssg::EventSSiege> inputEvents, ::llc::apobj<::ssg::EventSSiege> & outputEvents, double secondsElapsed) {
	::llc::FBool<::llc::pobj<::ssg::EventSSiege> &, ::llc::apobj<::ssg::EventSSiege> &>	funcHandleEvent 
		= [&world](::llc::pobj<::ssg::EventSSiege> & _eventToProcess, ::llc::apobj<::ssg::EventSSiege> & worldOutputEvents) { 
			if(!_eventToProcess)
				return false;

			const ::ssg::EventSSiege	& eventToProcess		= *_eventToProcess;
			info_printf("%s", ::llc::get_value_namep(eventToProcess.Type));

			llc::err_t			result			= 0; 
			switch(eventToProcess.Type) {
			CASE_SSG_EVENT(result, world, WORLD_EVENT, worldOutputEvents); 
			CASE_SSG_EVENT(result, world, WORLD_ADMIN, worldOutputEvents); 
			CASE_SSG_EVENT(result, world, ACT_SAILING, worldOutputEvents); 
			CASE_SSG_EVENT(result, world, ACTION_CHAR, worldOutputEvents); 
			CASE_SSG_EVENT(result, world, ACT_ENGINES, worldOutputEvents); 
			CASE_SSG_EVENT(result, world, ACT_AIRSHIP, worldOutputEvents); 
			CASE_SSG_EVENT(result, world, ACT_WHEELED, worldOutputEvents); 
			CASE_SSG_EVENT(result, world, CLIENT_ASKS, worldOutputEvents); 
			CASE_SSG_EVENT(result, world, WORLD_SETUP, worldOutputEvents); 
			CASE_SSG_EVENT(result, world, WORLD_VALUE, worldOutputEvents); 
			default: 
				llc_warning_unhandled_event(eventToProcess); 
				break;
			}
			return bool(result == 1);
		};

	if_fail_fe(inputEvents.for_each([&outputEvents, &funcHandleEvent](::llc::pobj<::ssg::EventSSiege> & _eventToProcess){ 
		if(funcHandleEvent(_eventToProcess, outputEvents))
			_eventToProcess.clear();
		return 0;
	}));

	if_fail_fe(world.Engine.Update(secondsElapsed));
	return 0;
}

llc::err_t			ssg::ssiegeGameDraw				
	( ::ssg::SSiegeGame			& world
	, ::llc::rtbgra8d32			& backBuffer
	, const ::llc::n3f2_t		& cameraPosition
	, const ::llc::n3f2_t		& cameraTarget
	, const ::llc::n3f2_t		& cameraUp
	, const ::llc::minmaxf2_t	& nearFar
	) {
	::llc::STimer				timer				= {};
	::llc::SEngine				& engine			= world.Engine;

	if_fail_fe(::llc::drawScene(backBuffer.Color.View, backBuffer.DepthStencil.View, engine.Scene->RenderCache, *engine.Scene, cameraPosition, cameraTarget, cameraUp, nearFar));

	timer.Frame();
	info_printf("Render scene in %f seconds", timer.LastTimeSeconds);
	return 0;
}
