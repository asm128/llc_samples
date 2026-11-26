#include "ssiege_game.h"

#include "llc_img_serialize.h"
#include "llc_engine.h"
#include "llc_gui.h"
#include "llc_gui_inputbox.h"
#include "llc_deflate.h"
#include "llc_array_ptr.h"
#include "llc_noise.h"
#include "llc_string_compose.h"

#include "llc_file_pack.h"
#include "llc_path.h"
#include <ctime>


#ifndef SSIEGE_H_23701
#define SSIEGE_H_23701

namespace ssg
{

	enum SAVE_MODE { SAVE_MODE_AUTO, SAVE_MODE_QUICK, SAVE_MODE_STAGE, SAVE_MODE_USER };

	struct SFileStrings {
		::llc::vcst_t				SavegameFolder				= ".";
		::llc::vcst_t				ExtensionImages				= ".png";
		::llc::vcst_t				ExtensionProfile			= ".ssg";
		::llc::vcst_t				ExtensionCredentials		= ".ssgc";
		::llc::vcst_t				ExtensionSave				= ".ssgz";
		::llc::vcst_t				ExtensionSaveUser			= ".user.ssgz";
		::llc::vcst_t				ExtensionSaveAuto			= ".auto.ssgz";
		::llc::vcst_t				ExtensionSaveStage			= ".stage.ssgz";
		::llc::vcst_t				ExtensionSaveQuick			= ".quick.ssgz";

		llc::err_t			GetSaveGameExtension		(SAVE_MODE saveMode, ::llc::vcsc_t & extension)	const	{
			switch(saveMode) {
			case SAVE_MODE_USER		: extension = ExtensionSaveUser	; break;
			case SAVE_MODE_STAGE	: extension = ExtensionSaveStage; break;
			case SAVE_MODE_QUICK	: extension = ExtensionSaveQuick; break;
			case SAVE_MODE_AUTO		: extension = ExtensionSaveAuto	; break;
			}
			return 0;
		}

		llc::err_t			GetPlayerFilePath			(const u3_t time, ::llc::vcst_t playerName, ::llc::asc_t & playerFilePath)	const	{
			::llc::string			b64PlayerName				= {};
			if_fail_fe(::llc::base64Encode(playerName, b64PlayerName));
			if_fail_fe(::llc::append_strings(b64PlayerName, time, ExtensionProfile));
			if_fail_fe(::llc::pathNameCompose(SavegameFolder, b64PlayerName, playerFilePath));
			return 0;
		}
	};

	struct SSiegeUI {
		::llc::cid_t			Root				= ::llc::CID_INVALID;
		::llc::SInputBox		UserInput			= {};
		::llc::acid				RootPerState		= {};
		::llc::apobj<string>	InputHistory		= {};
		::llc::apobj<string>	InputQueue			= {};
	};

	llc::err_t			setupSSiegeUI			(::llc::SGUI & gui, ::ssg::SSiegeUI & ssiegeUI);


	GDEFINE_ENUM_TYPE(APP_STATE, u0_t);
	GDEFINE_ENUM_VALUE(APP_STATE, Init		,  0);
	GDEFINE_ENUM_VALUE(APP_STATE, Welcome	,  1);
	GDEFINE_ENUM_VALUE(APP_STATE, Home		,  2);
	GDEFINE_ENUM_VALUE(APP_STATE, Profile	,  3);
	GDEFINE_ENUM_VALUE(APP_STATE, Shop		,  4);
	GDEFINE_ENUM_VALUE(APP_STATE, Play		,  5);
	GDEFINE_ENUM_VALUE(APP_STATE, Brief		,  6);
	GDEFINE_ENUM_VALUE(APP_STATE, Stage		,  7);
	GDEFINE_ENUM_VALUE(APP_STATE, Stats		,  8);
	GDEFINE_ENUM_VALUE(APP_STATE, Store		,  9);
	GDEFINE_ENUM_VALUE(APP_STATE, Score		, 10);
	GDEFINE_ENUM_VALUE(APP_STATE, About		, 11);
	GDEFINE_ENUM_VALUE(APP_STATE, Settings	, 12);
	GDEFINE_ENUM_VALUE(APP_STATE, Quit		, 13);

	struct SPlayer {
		ssiegeid_t					Id					= SSIEGEID_INVALID;
		::llc::vcst_t				User				= {};
		::llc::vcst_t				Name				= {};
	};

	struct SPlayerState {
		u2_t				Stage				= 0;
	};

	struct SSiegeApp {
		APP_STATE				ActiveState			= {};

		::ssg::SSiegeGame		Game;

		//SWorldView				World				= {};

		SSiegeUI				UI					= {};
		llc::apobj<EventSSiege>	EventQueue			= {};

		SFileStrings			FileStrings			= {};


		::llc::pobj<llc::SGUI>	GUI;
		::llc::prtbgra8d32		RenderTargetWorld;
		::llc::prtbgra8d32		RenderTargetUI;

		llc::err_t			StateSwitch			(APP_STATE newState);

		llc::err_t			Save				()	const	{
			if_true_vif(0, Game.Controlled >= Game.Engine.Entities.size(), "No active player character! %i >= %i", Game.Controlled, Game.Engine.Entities.size());
				
			::llc::string				playerPath;
			if_fail_fe(FileStrings.GetPlayerFilePath(::llc::timeCurrent(), Game.Players[0].Name, playerPath));
			return Save(playerPath);
		}

		llc::err_t			Save				(::llc::vcst_t fileName)	const	{
			::llc::au0_t					serialized;
			if_fail_fe(Game.World.Save(serialized));
			return ::llc::deflateFromMemory(fileName, serialized);
		}

		llc::err_t			Load				(::llc::vcst_t filename)			{
			::llc::au0_t					serialized;
			if_fail_fe(::llc::inflateToMemory(filename, serialized));
			::llc::vcu0_t					viewSerialized		= serialized;
			return Game.World.Load(viewSerialized);
		}
	};

	llc::err_t			ssiegeUpdate		(::ssg::SSiegeApp & ssiege, double lastTimeSeconds, const ::llc::pobj<::llc::SInput> & inputState, ::llc::vpobj<::llc::SEventSystem> systemEvents
		, ::llc::FBool<::llc::pobj<::ssg::EventSSiege> &, ::llc::apobj<::ssg::EventSSiege> &> funcHandleEvent
		);
	llc::err_t			ssiegeDraw			(::ssg::SSiegeApp & ssiege, ::llc::rtbgra8d32 & backBuffer, bool onlyGUI);
} // namespace ssg

#endif // SSIEGE_H_23701
