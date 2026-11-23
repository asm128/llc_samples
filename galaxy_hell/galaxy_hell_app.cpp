#include "llc_galaxy_hell_app.h"
#include "llc_grid_copy.h"
#include "llc_raster_lh.h"
#include "llc_file.h"
#include "llc_deflate.h"
#include "llc_path.h"
#include "llc_event_screen.h"

using ::llc::get_value_namep, ::llc::get_enum_namep, ::llc::failed;
LLC_USING_TYPEINT();

static	llc::err_t	processScreenEvent		(::ghg::SGalaxyHellApp & app, const ::llc::SEventView<::llc::EVENT_SCREEN> & screenEvent) { 
	switch(screenEvent.Type) {
	default: break;
	case ::llc::EVENT_SCREEN_Close:
	case ::llc::EVENT_SCREEN_Deactivate: 
		if(app.ActiveState != ::ghg::APP_STATE_Welcome) {
			app.Save(::ghg::SAVE_MODE_AUTO);
			app.ActiveState						= ::ghg::APP_STATE_Home;
		}
		break;
	case ::llc::EVENT_SCREEN_Create:
	case ::llc::EVENT_SCREEN_Resize: {
		::llc::n2<u1_t>			newMetrics				= *(const ::llc::n2<u1_t>*)screenEvent.Data.begin();
		::llc::guiUpdateMetrics(*app.DialogDesktop.GUI, newMetrics, true);
		for(u2_t iPlayer = 0; iPlayer < app.UIPlay.PlayerUI.size(); ++iPlayer) {
			if(app.UIPlay.PlayerUI[iPlayer].DialogHome.GUI) ::llc::guiUpdateMetrics(*app.UIPlay.PlayerUI[iPlayer].DialogHome.GUI, newMetrics, true);
			if(app.UIPlay.PlayerUI[iPlayer].DialogPlay.GUI) ::llc::guiUpdateMetrics(*app.UIPlay.PlayerUI[iPlayer].DialogPlay.GUI, newMetrics, true);
		}

		app.RenderTargetPool.resize(16);
		for(u2_t iRT = 0; iRT < 16; ++iRT) {
			app.RenderTargetPool[iRT]->resize(newMetrics);
		}

		app.Game.DrawCache.RenderTargetMetrics = newMetrics;
		double						currentRatio			= app.Game.DrawCache.RenderTargetMetrics.y / (double)app.Game.DrawCache.RenderTargetMetrics.x;
		double						targetRatioY			=  9 / 16.0;
		if(currentRatio >= targetRatioY)
			app.Game.DrawCache.RenderTargetMetrics.y = (u1_t)(app.Game.DrawCache.RenderTargetMetrics.x * targetRatioY + .1f);
		else 
			app.Game.DrawCache.RenderTargetMetrics.x = (u1_t)(app.Game.DrawCache.RenderTargetMetrics.y / targetRatioY + .1f);
		}
		break;
	}
	return 0;
}

static	llc::err_t	processTextEvent		(::ghg::SGalaxyHellApp & app, const ::llc::SEventView<::llc::EVENT_TEXT> & screenEvent) { 
	switch(screenEvent.Type) {
	default: break;
	case ::llc::EVENT_TEXT_Char:
		if(screenEvent.Data[0] >= 0x20 && screenEvent.Data[0] <= 0x7F) {
			if(app.ActiveState == ::ghg::APP_STATE_Welcome)
				app.InputboxText.push_back(screenEvent.Data[0]);
		}
		break;
	}
	return 0;
}

static	llc::err_t	processKeyboardEvent	(::ghg::SGalaxyHellApp & app, const ::llc::SEventView<::llc::EVENT_KEYBOARD> & screenEvent) { 
	switch(screenEvent.Type) {
	default: break;
	case ::llc::EVENT_KEYBOARD_Down:
		if(screenEvent.Data[0] == ::llc::SYSTEM_KEY_Escape) {
			if(app.ActiveState != ::ghg::APP_STATE_Home) {
				app.Save(::ghg::SAVE_MODE_AUTO);
				app.ActiveState				= ::ghg::APP_STATE_Home;
			}
			else { 
				bool editing = false;
				for(u2_t iPlayer = 0; iPlayer < app.TunerPlayerCount->ValueCurrent; ++iPlayer) {
					if(app.UIPlay.PlayerUI[iPlayer].InputBox.Editing) {
						editing = true;
						break;
					}
				}
				if(!editing) {
					app.Game.PlayState.GlobalState.Paused	= false;
					app.ActiveState				= ::ghg::APP_STATE_Play;
				}
			}
		}
		break;
	}
	return 0;
}

static	llc::err_t	processSystemEvent		(::ghg::SGalaxyHellApp & app, const ::llc::SEventSystem & sysEvent) { 
	switch(sysEvent.Type) {
	default: break;
	case ::llc::SYSTEM_EVENT_Screen		: if_fail_e(::llc::eventExtractAndHandle<::llc::EVENT_SCREEN	>(sysEvent, [&app](auto ev) { return processScreenEvent		(app, ev); })); break;
	case ::llc::SYSTEM_EVENT_Text		: if_fail_e(::llc::eventExtractAndHandle<::llc::EVENT_TEXT		>(sysEvent, [&app](auto ev) { return processTextEvent		(app, ev); })); break;
	case ::llc::SYSTEM_EVENT_Keyboard	: if_fail_e(::llc::eventExtractAndHandle<::llc::EVENT_KEYBOARD	>(sysEvent, [&app](auto ev) { return processKeyboardEvent	(app, ev); })); break;
	}
	return 0;
}

llc::err_t			ghg::galaxyHellUpdate	(::ghg::SGalaxyHellApp & app, double lastTimeSeconds, const ::llc::pobj<::llc::SInput> & inputState, ::llc::vpobj<::llc::SEventSystem> systemEventsNew) {
	if_true_vi(1, app.ActiveState == ::ghg::APP_STATE_Quit);

	if_fail_fe(systemEventsNew.for_each([&app](const ::llc::pobj<::llc::SEventSystem> & sysEvent) { return ::processSystemEvent(app, *sysEvent); }));

	switch(app.ActiveState) {
	case APP_STATE_Init		: {
		::ghg::solarSystemSetup(app.Game, app.Game.DrawCache.RenderTargetMetrics);
		::ghg::guiSetup(app, inputState);

		::llc::aobj<::llc::string>						fileNames					= {};
		::llc::pathList(app.SavegameFolder, fileNames, app.ExtensionSaveAuto);
		if(fileNames.size()) {
			if(failed(::ghg::solarSystemLoad(app.Game, fileNames[0]))) {
				app.Game.PilotsReset();
			}
		}
		fileNames					= {};
		::llc::pathList(app.SavegameFolder, fileNames, app.ExtensionProfile);
		app.Players.resize(fileNames.size());

		for(u2_t iPlayer = 0; iPlayer < fileNames.size(); ++iPlayer) {
			try {
				if_fail_fef(app.Players[iPlayer].Load(fileNames[iPlayer]), "fileNames[iPlayer]: %.*s", LLC_VIEW_ARGS(fileNames[iPlayer]));
			}
			catch (const sc_t * ) {
				app.Players[iPlayer] = {};
			}

		}
		app.TunerPlayerCount->SetValue((u0_t)app.Game.PlayState.Constants.Players);
		for(u2_t iPilot = 0; iPilot < app.Game.PlayState.Constants.Players; ++iPilot) {
			const ::llc::vcsc_t namePilot	= app.Game.Pilots[iPilot].Name;
			for(u2_t iPlayer = 0; iPlayer < app.Game.PlayState.Constants.Players; ++iPlayer) {
				if(iPlayer >= app.Players.size())
					app.AddNewPlayer(namePilot);

				const ::llc::vcsc_t namePlayer	= app.Players[iPlayer].Name;
				if(namePilot == namePlayer) {
					::std::swap(app.Players[iPlayer], app.Players[iPilot]);
					break;
				}
			}
			for(u2_t iPlayer = app.Game.PlayState.Constants.Players; iPlayer < app.Players.size(); ++iPlayer) {
				const ::llc::vcsc_t namePlayer	= app.Players[iPlayer].Name;
				if(namePilot == namePlayer) {
					::std::swap(app.Players[iPlayer], app.Players[iPilot]);
					break;
				}
			}
		}
		app.ActiveState					= APP_STATE_Welcome;
		break;
	}
	case  APP_STATE_Welcome:
		if(app.Players.size())
			app.ActiveState					= APP_STATE_Home;
		else {
			//app.DialogDesktop.GUI->Controls.States[app.VirtualKeyboard.IdRoot].Hidden = false;
		}
		break;
	}	 

	const bool inGame = app.ActiveState == ::ghg::APP_STATE_Play;

	::llc::view<::ghg::SShipController>		controllerPlayer	= app.Game.ShipControllers;


	const ::llc::SInput								& input				= *app.DialogDesktop.Input;
	if(controllerPlayer.size() && app.Game.PlayState.Constants.Players == 1) {
		controllerPlayer[0].Forward					= input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Up		] || input.KeyboardCurrent.KeyState['W'];
		controllerPlayer[0].Back					= input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Down	] || input.KeyboardCurrent.KeyState['S'];
		controllerPlayer[0].Left					= input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Left	] || input.KeyboardCurrent.KeyState['A'];
		controllerPlayer[0].Right					= input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Right	] || input.KeyboardCurrent.KeyState['D'];
		controllerPlayer[0].Turbo					= input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_RControl] || input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Control] || input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_LShift] || input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Shift];
	}
	else if(controllerPlayer.size() > 1) {
		controllerPlayer[0].Forward					= input.KeyboardCurrent.KeyState['W'];
		controllerPlayer[0].Back					= input.KeyboardCurrent.KeyState['S'];
		controllerPlayer[0].Left					= input.KeyboardCurrent.KeyState['A'];
		controllerPlayer[0].Right					= input.KeyboardCurrent.KeyState['D'];
		controllerPlayer[0].Turbo					= input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_LShift] || input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Shift];

		controllerPlayer[1].Forward					= input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Up		];
		controllerPlayer[1].Back					= input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Down	];
		controllerPlayer[1].Left					= input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Left	];
		controllerPlayer[1].Right					= input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Right	];
		controllerPlayer[1].Turbo					= input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_RControl] || input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Control];
	}

	::ghg::solarSystemUpdate(app.Game, (false == inGame) ? 0 : lastTimeSeconds, *inputState, systemEventsNew);
	for(u2_t iShip = 0; iShip < app.Game.ShipState.SpaceshipManager.ShipOrbiterActionQueue.size(); ++iShip)
		if(iShip < app.Game.PlayState.Constants.Players) {
			if(app.Game.ShipState.SpaceshipManager.ShipOrbiterActionQueue[iShip]) {
				::llc::apod<::llc::SHIP_ACTION> & actionQueue = *app.Game.ShipState.SpaceshipManager.ShipOrbiterActionQueue[iShip];
				for(u2_t iEvent = 0; iEvent < actionQueue.size(); ++iEvent)
					if(actionQueue[iEvent] == ::llc::SHIP_ACTION_Spawn) {
						app.Save(::ghg::SAVE_MODE_STAGE);
						actionQueue[iEvent] = (::llc::SHIP_ACTION)-1;
					}
			}
		}
	//::ghg::overlayUpdate(app.Overlay, app.World.PlayState.Stage, app.World.ShipState.ShipCores.size() ? app.World.ShipState.ShipCores[0].Score : 0, app.World.PlayState.TimeWorld);
	// Galaxy3l
	app.ActiveState					= (::ghg::APP_STATE)::ghg::guiUpdate(app, systemEventsNew);
	return 0;
}

llc::err_t					ghg::galaxyHellDraw				(::ghg::SGalaxyHellApp & app, ::llc::n2u1_t renderTargetSize) {
	if(app.ActiveState < 2)
		return 0;

	::llc::pobj<::ghg::TRenderTarget>		target	= {};
	{
		::std::lock_guard			rtLock(app.RenderTargetLockPool);
		if(app.RenderTargetPool.size()) {
			target					= app.RenderTargetPool[0];
			app.RenderTargetPool.remove_unordered(0);
		}
	}
	if(!target) {
		::std::lock_guard			rtLock(app.RenderTargetLockQueue);
		if(app.RenderTargetQueue.size() > 1) {
			target					= app.RenderTargetQueue[0];
			app.RenderTargetQueue.remove_unordered(0);
		}
	}

	target->resize(renderTargetSize.u2(), {0, 0, 0, 1}, 0xFFFFFFFFU);

	::llc::g8bgra							targetPixels			= target->Color.View;
	::llc::grid<u2_t>									depthBuffer				= target->DepthStencil.View;
	switch(app.ActiveState) {
	default					: 
	case APP_STATE_Play		: 
		::ghg::solarSystemDraw(app.Game, app.Game.DrawCache, app.Game.LockUpdate);
		::llc::pobj<::ghg::TRenderTarget>		sourceRT	= {};
		sourceRT								= app.Game.DrawCache.RenderTarget;
		if(!sourceRT)
			break;

		::llc::g8bgra			cameraView			= sourceRT->Color.View;
		::llc::n2s1_t						cameraViewMetrics	= cameraView.metrics().s1();
		::llc::grid_copy(targetPixels, cameraView, ::llc::n2<u2_t>
				{ (targetPixels.metrics().x >> 1) - (cameraView.metrics().x >> 1)
				, (targetPixels.metrics().y >> 1) - (cameraView.metrics().y >> 1)
				}
			);

		const ::llc::n2s1_t		targetCenter		= targetPixels.metrics().s1() / 2;
		const ::llc::n2s1_t		cameraCenter		= cameraView.metrics().s1() / 2;
		::llc::n2s1_t				cornerTopLeft		= targetCenter + ::llc::n2s1_t{s1_t(-cameraCenter.x), s1_t(-cameraCenter.y)};
		::llc::n2s1_t				cornerTopRight		= targetCenter + ::llc::n2s1_t{s1_t( cameraCenter.x), s1_t(-cameraCenter.y)};
		::llc::n2s1_t				cornerBottomLeft	= targetCenter + ::llc::n2s1_t{s1_t(-cameraCenter.x), s1_t( cameraCenter.y)};
		::llc::n2s1_t				cornerBottomRight	= targetCenter + ::llc::n2s1_t{s1_t( cameraCenter.x), s1_t( cameraCenter.y)};

		cornerTopLeft		.InPlaceClamp({}, {(s1_t)(targetPixels.metrics().x - 1), (s1_t)(targetPixels.metrics().y - 1)});
		cornerTopRight		.InPlaceClamp({}, {(s1_t)(targetPixels.metrics().x - 1), (s1_t)(targetPixels.metrics().y - 1)});
		cornerBottomLeft	.InPlaceClamp({}, {(s1_t)(targetPixels.metrics().x - 1), (s1_t)(targetPixels.metrics().y - 1)});
		cornerBottomRight	.InPlaceClamp({}, {(s1_t)(targetPixels.metrics().x - 1), (s1_t)(targetPixels.metrics().y - 1)});

		::llc::drawLine(targetPixels, ::llc::line2s1_t{cornerTopLeft		, cornerTopRight	}, ::llc::DARKGRAY);
		::llc::drawLine(targetPixels, ::llc::line2s1_t{cornerTopLeft		, cornerBottomLeft	}, ::llc::DARKGRAY);
		::llc::drawLine(targetPixels, ::llc::line2s1_t{cornerTopRight	, cornerBottomRight	}, ::llc::GRAY * 1.1);
		::llc::drawLine(targetPixels, ::llc::line2s1_t{cornerBottomLeft	, cornerBottomRight	}, ::llc::GRAY * 1.1);

		//::ghg::overlayDraw(app.Overlay, app.World.DrawCache, app.World.PlayState.TimeWorld, depthBuffer, targetPixels);
		break;
	}

	{
		::std::lock_guard							lockUpdate			(app.Game.LockUpdate);
		::llc::guiDraw(*app.DialogPerState[app.ActiveState].GUI, targetPixels);
	}
	if(app.ActiveState != ::ghg::APP_STATE_Play) {
		::std::lock_guard							lockUpdate			(app.Game.LockUpdate);
		::llc::guiDraw(*app.DialogDesktop.GUI, targetPixels);
	}
	for(u2_t iPlayer = 0; iPlayer < app.UIPlay.PlayerUI.size(); ++iPlayer) {
		::std::lock_guard							lockUpdate			(app.Game.LockUpdate);
		::llc::guiDraw((app.ActiveState == ::ghg::APP_STATE_Play)
			? *app.UIPlay.PlayerUI[iPlayer].DialogPlay.GUI
			: *app.UIPlay.PlayerUI[iPlayer].DialogHome.GUI
			, targetPixels
		);
	}
	{
		::std::lock_guard			rtLock(app.RenderTargetLockQueue);
		app.RenderTargetQueue.push_back(target);
	}
	return 0;
}

llc::err_t					ghg::listFilesSavegame		(::ghg::SGalaxyHellApp & app, const ::llc::vcsc_t & saveGameFolder, ::llc::aobj<::llc::vcst_t> & savegameFilenames) {
	::llc::aobj<::llc::string>		fileNames;
	if_fail_fef(::llc::pathList(saveGameFolder, fileNames, app.ExtensionSave), "%.*s", LLC_VIEW_ARGS(saveGameFolder));

	::llc::aobj<::llc::vcst_t>				pathFileNames;
	for(u2_t iFile = 0; iFile < fileNames.size(); ++iFile)
		pathFileNames.push_back(fileNames[iFile]);

	::llc::aobj<::llc::vcst_t>				pathFileNamesSaveAuto;
	::llc::aobj<::llc::vcst_t>				pathFileNamesSaveUser;
	::llc::aobj<::llc::vcst_t>				pathFileNamesSaveStage;
	::llc::filterPostfix(pathFileNames, app.ExtensionSaveAuto	, pathFileNamesSaveAuto);
	::llc::filterPostfix(pathFileNames, app.ExtensionSaveUser	, pathFileNamesSaveUser);
	::llc::filterPostfix(pathFileNames, app.ExtensionSaveStage	, pathFileNamesSaveStage);
		
	for(u2_t iPath = 0; iPath < pathFileNamesSaveAuto .size(); ++iPath) savegameFilenames.push_back(::llc::label(pathFileNamesSaveAuto [iPath]));
	for(u2_t iPath = 0; iPath < pathFileNamesSaveUser .size(); ++iPath) savegameFilenames.push_back(::llc::label(pathFileNamesSaveUser [iPath]));
	for(u2_t iPath = 0; iPath < pathFileNamesSaveStage.size(); ++iPath) savegameFilenames.push_back(::llc::label(pathFileNamesSaveStage[iPath]));
	return 0;
}

llc::err_t					ghg::listFilesProfile		(::ghg::SGalaxyHellApp & app, const ::llc::vcsc_t & saveGameFolder, ::llc::aobj<::llc::vcst_t> & savegameFilenames) {
	::llc::aobj<::llc::string>		fileNames;
	if_fail_fef(::llc::pathList(saveGameFolder, fileNames, app.ExtensionProfile), "%.*s", LLC_VIEW_ARGS(saveGameFolder));

	::llc::aobj<::llc::vcsc_t>				pathFileNames;
	for(u2_t iFile = 0; iFile < fileNames.size(); ++iFile)
		savegameFilenames.push_back(::llc::label(::llc::vcsc_t{fileNames[iFile]}));
	return 0;
}
