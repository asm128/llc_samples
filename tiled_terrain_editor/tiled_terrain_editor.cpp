#include "tiled_terrain_editor.h"
#include "llc_png.h"
#include "llc_grid_copy.h"
#include "llc_grid_scale.h"
#include "llc_bitmap_file.h"

#include "llc_dialog_controls.h"
//#define LLC_AVOID_LOCAL_APPLICATION_MODULE_MODEL_EXECUTABLE_RUNTIME
#include "llc_app_impl.h"

 LLC_DEFINE_APPLICATION_ENTRY_POINT(::gme::SApplication, "Tiled Terrain Editor v0.01");

::llc::err_t			cleanup		(::gme::SApplication & app)						{ return ::llc::mainWindowDestroy(app.Framework.RootWindow); }
::llc::err_t			setup		(::gme::SApplication & app)						{
	::llc::SFramework				& framework					= app.Framework;
	::llc::SWindow					& mainWindow				= framework.RootWindow;
	framework.Settings.GUIZoom											= false;
	app.Framework.GUI													= app.DialogMain.GUI;
	app.DialogMain.Input												= mainWindow.Input;
	mainWindow.Size														= {1024, 768};
	if_true_e(::llc::failed(::llc::mainWindowCreate(mainWindow, framework.RuntimeValues.PlatformDetail, mainWindow.Input)));
	::llc::SGUI						& gui						= *framework.GUI;
	gui.ColorModeDefault												= ::llc::GUI_COLOR_MODE_3D;
	gui.ThemeDefault													= ::llc::ASCII_COLOR_DARKGREEN * 16 + 7;
	{
		app.IdExit															= ::llc::controlCreate(gui);
		::llc::SControlPlacement															& controlExit								= gui.Controls.Placement[app.IdExit];
		controlExit.Area													= {{}, {64, 20}};
		controlExit.Border													= {10, 10, 10, 10};
		controlExit.Margin													= {1, 1, 1, 1};
		controlExit.Align													= ::llc::ALIGN_CENTER_BOTTOM;
		::llc::SControlText														& controlText								= gui.Controls.Text[app.IdExit];
		controlText.Text													= "Exit";
		controlText.Align													= ::llc::ALIGN_CENTER;
		::llc::controlSetParent(gui, app.IdExit, -1);
	}

	{
		app.IdFrameRateUpdate												= ::llc::controlCreate(gui);
		::llc::SControlPlacement															& controlExit								= gui.Controls.Placement[app.IdFrameRateUpdate];
		controlExit.Area													= {{}, {384, 20}};
		controlExit.Border													= {10, 10, 10, 10};
		controlExit.Margin													= {1, 1, 1, 1};
		controlExit.Align													= ::llc::ALIGN_BOTTOM_LEFT;
		::llc::SControlText														& controlText								= gui.Controls.Text[app.IdFrameRateUpdate];
		controlText.Align													= ::llc::ALIGN_CENTER;
		::llc::controlSetParent(gui, app.IdFrameRateUpdate, -1);
	}

	{
		app.IdFrameRateRender												= ::llc::controlCreate(gui);
		::llc::SControlPlacement															& controlExit								= gui.Controls.Placement[app.IdFrameRateRender];
		controlExit.Area													= {{}, {384, 20}};
		controlExit.Border													= {10, 10, 10, 10};
		controlExit.Margin													= {1, 1, 1, 1};
		controlExit.Align													= ::llc::ALIGN_BOTTOM_RIGHT;
		::llc::SControlText														& controlText								= gui.Controls.Text[app.IdFrameRateRender];
		controlText.Align													= ::llc::ALIGN_CENTER;
		::llc::controlSetParent(gui, app.IdFrameRateRender, -1);
	}

	::llc::pobj<::llc::SDialogTuner<u0_t >>								tuner						= {};
	app.NumericTuner													= ::llc::tunerCreate(app.DialogMain, tuner);
	tuner->ValueLimits.Min												= 100;
	tuner->ValueLimits.Max												= 200;
	::llc::SControlTable													& controlTable				= gui.Controls;
	controlTable.Placement[tuner->IdGUIControl].Area.Offset				= {128, 192};
	controlTable.Placement[tuner->IdGUIControl].Area.Size.x				= 128;
	controlTable.Placement[tuner->IdGUIControl].Area.Size.y				=  20;
	tuner->SetValue(0);

	::llc::pobj<::llc::SDialogCheckBox>									checkbox					= {};
	app.CheckBox														= ::llc::checkBoxCreate(app.DialogMain, checkbox);
	controlTable.Placement[checkbox->IdGUIControl].Area.Offset			= {128, 256};


	::llc::pobj<::llc::SDialogViewport>									viewport					= {};
	app.Viewport														= ::llc::viewportCreate(app.DialogMain, viewport);
	controlTable.Placement[viewport->IdGUIControl].Area.Offset		= {320, 128};
	controlTable.Placement[viewport->IdGUIControl].Area.Size			= {640, 480};
	controlTable.Images[viewport->IdClient].ImageInvertY			= true;


	//---------------------------

	stxp char													ragnaPath	[]								= "\\data_2005\\data\\";

	char																		temp		[512]							= {};
	::llc::SRSWFileContents														& rswData									= app.RSWData;
	//sprintf_s(temp, "%s%s%s", ragnaPath, "", "in_sphinx1.rsw");						if_fail_fef(::llc::rswFileLoad(rswData						, ::llc::view_const_string(temp)), "Error");
	sprintf_s(temp, "%s%s%s", ragnaPath, "", "alberta.rsw");					if_fail_fef(::llc::rswFileLoad(rswData		, ::llc::view_const_string(temp)), "Error");
	sprintf_s(temp, "%s%s%s", ragnaPath, "", &rswData.GNDFilename[0]);			if_fail_fef(::llc::gndFileLoad(app.GNDData	, ::llc::view_const_string(temp)), "Error");
	app.RSMData.resize(rswData.RSWModels.size());
	for(u2_t  iRSM = 0; iRSM < (u2_t )rswData.RSWModels.size(); ++iRSM){
		::llc::SRSMFileContents														& rsmData									= app.RSMData[iRSM];
		sprintf_s(temp, "%s%s%s", ragnaPath, "model\\", &rswData.RSWModels[iRSM].Filename[0]);
		ef_if(::llc::failed(::llc::rsmFileLoad(rsmData, ::llc::vcst_t(temp))), "Failed to load file: %s.", temp);
	}
	for(u2_t  iLight = 0; iLight < rswData.RSWLights.size(); ++iLight) {
		rswData.RSWLights[iLight].Position										*= 1.0 / app.GNDData.Metrics.TileScale;
		rswData.RSWLights[iLight].Position										+= ::llc::n3f2_t{app.GNDData.Metrics.Size.x / 2.0f, 0.0f, (app.GNDData.Metrics.Size.y / 2.0f)};
		rswData.RSWLights[iLight].Position.y									*= -1;
	}

	app.TexturesGND.resize(app.GNDData.TextureNames.size());
	for(u2_t  iTex = 0; iTex < app.GNDData.TextureNames.size(); ++ iTex) {
		sprintf_s(temp, "%s%s%s", ragnaPath, "texture\\", &app.GNDData.TextureNames[iTex][0]);
		ef_if(::llc::failed(::llc::bmpFileLoad(::llc::view_const_string(temp), app.TexturesGND[iTex])), "Not found? %s.", temp);
	}

	app.GNDModel.Nodes.resize(app.GNDData.TextureNames.size() * 6);
	app.GNDModel.TileMapping.resize(app.GNDData.Metrics.Size);
	::llc::grid<::llc::STileGeometryGND>									tileGeometryView							= {app.GNDData.lstTileGeometryData.begin(), app.GNDData.Metrics.Size};
	for(u2_t  iTex = 0, texCount = app.GNDData.TextureNames.size(); iTex < texCount; ++iTex) {
		s2_t indexTop	= iTex + app.GNDData.TextureNames.size() * ::llc::TILE_FACE_FACING_TOP		; if_fail_fef(::llc::gndGenerateFaceGeometry(app.GNDData.lstTileTextureData, app.GNDData.lstTileGeometryData, app.GNDData.Metrics,::llc::TILE_FACE_FACING_TOP	, iTex, app.GNDModel.Nodes[indexTop		], app.GNDModel.TileMapping.View), "");
		s2_t indexFront	= iTex + app.GNDData.TextureNames.size() * ::llc::TILE_FACE_FACING_FRONT	; if_fail_fef(::llc::gndGenerateFaceGeometry(app.GNDData.lstTileTextureData, app.GNDData.lstTileGeometryData, app.GNDData.Metrics,::llc::TILE_FACE_FACING_FRONT	, iTex, app.GNDModel.Nodes[indexFront	], app.GNDModel.TileMapping.View), "");
		s2_t indexRight	= iTex + app.GNDData.TextureNames.size() * ::llc::TILE_FACE_FACING_RIGHT	; if_fail_fef(::llc::gndGenerateFaceGeometry(app.GNDData.lstTileTextureData, app.GNDData.lstTileGeometryData, app.GNDData.Metrics,::llc::TILE_FACE_FACING_RIGHT	, iTex, app.GNDModel.Nodes[indexRight	], app.GNDModel.TileMapping.View), "");
		//s2_t indexBottom	= iTex + app.GNDData.TextureNames.size() * ::llc::TILE_FACE_FACING_BOTTOM	; if_fail_fef(::llc::gndGenerateFaceGeometry(app.GNDData.lstTileTextureData, app.GNDData.lstTileGeometryData, app.GNDData.Metrics,::llc::TILE_FACE_FACING_BOTTOM	, iTex, app.GNDModel.Nodes[indexBottom	], app.GNDModel.TileMapping.View), "");
		s2_t indexBack	= iTex + app.GNDData.TextureNames.size() * ::llc::TILE_FACE_FACING_BACK		; if_fail_fef(::llc::gndGenerateFaceGeometry(app.GNDData.lstTileTextureData, app.GNDData.lstTileGeometryData, app.GNDData.Metrics,::llc::TILE_FACE_FACING_BACK	, iTex, app.GNDModel.Nodes[indexBack	], app.GNDModel.TileMapping.View), "");
		s2_t indexLeft	= iTex + app.GNDData.TextureNames.size() * ::llc::TILE_FACE_FACING_LEFT		; if_fail_fef(::llc::gndGenerateFaceGeometry(app.GNDData.lstTileTextureData, app.GNDData.lstTileGeometryData, app.GNDData.Metrics,::llc::TILE_FACE_FACING_LEFT	, iTex, app.GNDModel.Nodes[indexLeft	], app.GNDModel.TileMapping.View), "");
	}
	// Blend normals.
	for(u2_t  y = 0; y < app.GNDData.Metrics.Size.y - 1; ++y)
	for(u2_t  x = 0; x < app.GNDData.Metrics.Size.x - 1; ++x) {
		const ::llc::STileGeometryGND	& tileGeometry0			= tileGeometryView[y][x];
		const ::llc::STileGeometryGND	& tileGeometry1			= tileGeometryView[y][x + 1];
		const ::llc::STileGeometryGND	& tileGeometry2			= tileGeometryView[y + 1][x];
		const ::llc::STileGeometryGND	& tileGeometry3			= tileGeometryView[y + 1][x + 1];
		const ::llc::STileMapping		& tileMapping0			= app.GNDModel.TileMapping.View[y + 0][x + 0];
		const ::llc::STileMapping		& tileMapping1			= app.GNDModel.TileMapping.View[y + 0][x + 1];
		const ::llc::STileMapping		& tileMapping2			= app.GNDModel.TileMapping.View[y + 1][x + 0];
		const ::llc::STileMapping		& tileMapping3			= app.GNDModel.TileMapping.View[y + 1][x + 1];
		const bool						processTile0			= (tileGeometry0.SkinMapping.SkinIndexTop != -1);// && (app.GNDData.lstTileTextureData[tileGeometry0.SkinMapping.SkinIndexTop].TextureIndex != -1);
		const bool						processTile1			= (tileGeometry1.SkinMapping.SkinIndexTop != -1) && tileGeometry0.SkinMapping.SkinIndexFront == -1;// && (app.GNDData.lstTileTextureData[tileGeometry1.SkinMapping.SkinIndexTop].TextureIndex != -1);
		const bool						processTile2			= (tileGeometry2.SkinMapping.SkinIndexTop != -1) && tileGeometry0.SkinMapping.SkinIndexRight == -1;// && (app.GNDData.lstTileTextureData[tileGeometry2.SkinMapping.SkinIndexTop].TextureIndex != -1);
		const bool						processTile3			= (tileGeometry3.SkinMapping.SkinIndexTop != -1) && (tileGeometry0.SkinMapping.SkinIndexFront == -1 && tileGeometry0.SkinMapping.SkinIndexRight == -1);// && (app.GNDData.lstTileTextureData[tileGeometry3.SkinMapping.SkinIndexTop].TextureIndex != -1);
		const s2_t					texIndex0				= processTile0 ? app.GNDData.lstTileTextureData[tileGeometry0.SkinMapping.SkinIndexTop].TextureIndex : -1;
		const s2_t					texIndex1				= processTile1 ? app.GNDData.lstTileTextureData[tileGeometry1.SkinMapping.SkinIndexTop].TextureIndex : -1;
		const s2_t					texIndex2				= processTile2 ? app.GNDData.lstTileTextureData[tileGeometry2.SkinMapping.SkinIndexTop].TextureIndex : -1;
		const s2_t					texIndex3				= processTile3 ? app.GNDData.lstTileTextureData[tileGeometry3.SkinMapping.SkinIndexTop].TextureIndex : -1;
		::llc::n3f2_t					normal					= {};
		u2_t 						divisor					= 0;
		if(processTile0) { ++divisor; ::llc::SModelNodeGND & gndNode0 = app.GNDModel.Nodes[texIndex0]; normal += gndNode0.Normals[tileMapping0.VerticesTop[3]]; }
		if(processTile1) { ++divisor; ::llc::SModelNodeGND & gndNode1 = app.GNDModel.Nodes[texIndex1]; normal += gndNode1.Normals[tileMapping1.VerticesTop[2]]; }
		if(processTile2) { ++divisor; ::llc::SModelNodeGND & gndNode2 = app.GNDModel.Nodes[texIndex2]; normal += gndNode2.Normals[tileMapping2.VerticesTop[1]]; }
		if(processTile3) { ++divisor; ::llc::SModelNodeGND & gndNode3 = app.GNDModel.Nodes[texIndex3]; normal += gndNode3.Normals[tileMapping3.VerticesTop[0]]; }
		if(divisor) {
			(normal /= divisor).Normalize();
			if(processTile0) { ::llc::SModelNodeGND	& gndNode0 = app.GNDModel.Nodes[texIndex0]; gndNode0.Normals[tileMapping0.VerticesTop[3]] = normal; }
			if(processTile1) { ::llc::SModelNodeGND	& gndNode1 = app.GNDModel.Nodes[texIndex1]; gndNode1.Normals[tileMapping1.VerticesTop[2]] = normal; }
			if(processTile2) { ::llc::SModelNodeGND	& gndNode2 = app.GNDModel.Nodes[texIndex2]; gndNode2.Normals[tileMapping2.VerticesTop[1]] = normal; }
			if(processTile3) { ::llc::SModelNodeGND	& gndNode3 = app.GNDModel.Nodes[texIndex3]; gndNode3.Normals[tileMapping3.VerticesTop[0]] = normal; }
		}
	}
	app.Scene.Camera.Points.Position	= {0, 30, -20};
	app.Scene.Camera.NearFar.Max		= 1000.0f;
	app.Scene.Camera.NearFar.Min		= 0.001f;

	::llc::SGNDFileContents					& gndData									= app.GNDData;
		// -- Generate minimap
	::llc::minmax<float>					heightMinMax								= {};
	for(u2_t  iTile = 0; iTile < gndData.lstTileGeometryData.size(); ++iTile)
		if(gndData.lstTileGeometryData[iTile].SkinMapping.SkinIndexTop != -1) {
			for(u2_t  iHeight = 0; iHeight < 4; ++iHeight) {
				heightMinMax.Max			= ::llc::max(gndData.lstTileGeometryData[iTile].fHeight[iHeight] * -1, heightMinMax.Max);
				heightMinMax.Min			= ::llc::min(gndData.lstTileGeometryData[iTile].fHeight[iHeight] * -1, heightMinMax.Min);
			}
		}
	::llc::img8bgra				minimapTemp;
	minimapTemp.resize(gndData.Metrics.Size);

	::llc::g8bgra				& minimapView								= minimapTemp.View;
	const float					heightRange									= heightMinMax.Max - heightMinMax.Min;
	for(u2_t  y = 0, yMax = minimapView.metrics().y; y < yMax; ++y)
	for(u2_t  x = 0, xMax = minimapView.metrics().x; x < xMax; ++x) {
		float						fAverageHeight								= 0;
		for(u2_t  iHeight = 0; iHeight < 4; ++iHeight)
			fAverageHeight			+= tileGeometryView[y][x].fHeight[iHeight] * -1;
		fAverageHeight			*= .25f;
		float						colorRatio										= (fAverageHeight - heightMinMax.Min) / heightRange;
		//minimapView[y][minimapView.metrics().x - 1 - x]							= ((fAverageHeight < 0) ? ::llc::WHITE : ::llc::BLUE) * colorRatio;
		minimapView[minimapView.metrics().y - 1 - y][x]							= ::llc::WHITE * colorRatio;
	}

	app.TextureMinimap.resize(gndData.Metrics.Size * 4);
	::llc::grid_scale(app.TextureMinimap.View, minimapTemp.View);

	app.ViewportMinimap												= ::llc::viewportCreate(app.DialogMain, viewport);
	controlTable.Placement	[viewport->IdGUIControl].Area.Offset	= {0, 0};
	controlTable.Placement	[viewport->IdGUIControl].Align			= ::llc::ALIGN_RIGHT;
	controlTable.Images	[viewport->IdClient].ImageInvertY		= true;
	::llc::viewportAdjustSize(controlTable.Placement[viewport->IdGUIControl].Area.Size, minimapTemp.metrics().Cast<s1_t>() * 2);
	controlTable.Images	[viewport->IdClient].Image				= app.TextureMinimap.View;
	controlTable.Images	[viewport->IdClient].ImageAlign			= ::llc::ALIGN_TOP_LEFT;

	::llc::pobj<::llc::SDialogSlider>									sliderH						= {};
	app.SliderH															= ::llc::sliderCreate(app.DialogMain, sliderH);
	sliderH->ValueLimits.Min											= 0;
	sliderH->ValueLimits.Max											= app.TextureMinimap.View.metrics().x / 2;
	controlTable.Placement[sliderH->IdGUIControl].Area.Offset			= {0, 0};
	controlTable.Placement[sliderH->IdGUIControl].Area.Size.x			= controlTable.Placement[viewport->IdGUIControl].Area.Size.x;
	controlTable.Placement[sliderH->IdGUIControl].Area.Size.y			= 8;
	controlTable.Constraints[sliderH->IdGUIControl].DockToControl.Bottom= viewport->IdGUIControl;
	controlTable.Constraints[sliderH->IdGUIControl].DockToControl.Left	= viewport->IdGUIControl;
	//::llc::controlSetParent(gui, sliderH->IdGUIControl, viewport->IdGUIControl);

	::llc::pobj<::llc::SDialogSlider>									sliderV						= {};
	app.SliderV															= ::llc::sliderCreate(app.DialogMain, sliderV);
	sliderV->ValueLimits.Min											= 0;
	sliderV->ValueLimits.Max											= app.TextureMinimap.View.metrics().y / 2;
	sliderV->Vertical													= true;
	controlTable.Placement[sliderV->IdGUIControl].Area.Offset			= {0, 20};
	controlTable.Placement[sliderV->IdGUIControl].Area.Size.x			= 8;
	controlTable.Placement[sliderV->IdGUIControl].Area.Size.y			= controlTable.Placement[viewport->IdGUIControl].Area.Size.y - 20;
	controlTable.Constraints[sliderV->IdGUIControl].DockToControl.Top	= viewport->IdGUIControl;
	controlTable.Constraints[sliderV->IdGUIControl].DockToControl.Right	= viewport->IdGUIControl;
	::llc::sliderSetValue(*sliderV, 0);
	return 0;
}

::llc::err_t			update		(::gme::SApplication & app, bool exitSignal)	{
	::llc::STimer															timer;
	if_true_vi(::llc::APPLICATION_STATE_EXIT, exitSignal);
	{
		::std::lock_guard														lock						(app.LockRender);
		app.Framework.RootWindow.BackBuffer									= app.Offscreen;
	}
	::llc::SFramework			& framework		= app.Framework;
	if_true_vi(::llc::APPLICATION_STATE_EXIT, ::llc::APPLICATION_STATE_EXIT == ::llc::updateFramework(app.Framework));

	::llc::SGUI					& gui			= *framework.GUI;
	::llc::acid					toProcess		= {};
	if(1 == ::llc::guiProcessControls(gui, [&app](::llc::cid_t iControl) { return one_if(iControl == app.IdExit); }))
		return 1;

	app.DialogMain.Update();

	//----------------------------------------------
	::llc::SFrameInfo			& frameInfo			= framework.FrameInfo;
	bool						updateProjection	= false;
	::llc::SWindow				& mainWindow		= framework.RootWindow;
	if(mainWindow.Input->KeyboardCurrent.KeyState[VK_ADD		])	{ updateProjection = true; app.Scene.Camera.Angle += frameInfo.Seconds.LastFrame; }
	if(mainWindow.Input->KeyboardCurrent.KeyState[VK_SUBTRACT	])	{ updateProjection = true; app.Scene.Camera.Angle -= frameInfo.Seconds.LastFrame; }
	if(mainWindow.Input->KeyboardCurrent.KeyState['Q'])				{ updateProjection = true; app.Scene.Camera.Points.Position.x += (float)frameInfo.Seconds.LastFrame * 10; }
	if(mainWindow.Input->KeyboardCurrent.KeyState['E'])				{ updateProjection = true; app.Scene.Camera.Points.Position.x -= (float)frameInfo.Seconds.LastFrame * 10; }
	if(mainWindow.Input->KeyboardCurrent.KeyState['W'])				{ updateProjection = true; app.Scene.Camera.Points.Position.y += (float)frameInfo.Seconds.LastFrame * 10; }
	if(mainWindow.Input->KeyboardCurrent.KeyState['S'])				{ updateProjection = true; app.Scene.Camera.Points.Position.y -= (float)frameInfo.Seconds.LastFrame * 10; }
	if(mainWindow.Input->KeyboardCurrent.KeyState['A'])				{ updateProjection = true; app.Scene.Camera.Points.Position.z += (float)frameInfo.Seconds.LastFrame * 10; }
	if(mainWindow.Input->KeyboardCurrent.KeyState['D'])				{ updateProjection = true; app.Scene.Camera.Points.Position.z -= (float)frameInfo.Seconds.LastFrame * 10; }

	//------------------------------------------------ Camera
	::llc::SCameraPoints														& camera									= app.Scene.Camera.Points;
	//camera.Position.RotateY(framework.Input->MouseCurrent.Deltas.x / 20.0f / (framework.Input->KeyboardCurrent.KeyState[VK_CONTROL] ? 2.0 : 1));
	//if(framework.Input->MouseCurrent.Deltas.z) {
	//	::llc::n3f2_t														zoomVector									= camera.Position;
	//	zoomVector.Normalize();
	//	const double																zoomWeight									= framework.Input->MouseCurrent.Deltas.z * (framework.Input->KeyboardCurrent.KeyState[VK_SHIFT] ? 10 : 1) / 240.;
	//	camera.Position															+= zoomVector * zoomWeight * .5;
	//}
	camera.Target																= {app.TextureMinimap.metrics().x / 2.0f, 0, -(s2_t)app.TextureMinimap.metrics().y / 2.0f};
	//------------------------------------------------ Lights
	::llc::n3f2_t														& lightDir									= app.LightDirection;
	lightDir.RotateY(frameInfo.Microseconds.LastFrame / 1000000.0f);
	lightDir.Normalize();

	::llc::SRSWFileContents														& rswData									= app.RSWData;
	const ::llc::n3f2_t													halfMapDir									= ::llc::n3f2_t{app.GNDData.Metrics.Size.x / 2.0f, 0.0f, (app.GNDData.Metrics.Size.y / 2.0f)};
	for(u2_t  iLight = 0; iLight < rswData.RSWLights.size(); ++iLight) {
		rswData.RSWLights[iLight].Position										-= halfMapDir;
		rswData.RSWLights[iLight].Position.RotateY(frameInfo.Seconds.LastFrame);
		rswData.RSWLights[iLight].Position										+= halfMapDir;
	}
	//------------------------------------------------
	//app.GridPivot.Scale										= {2.f, 4.f, 2.f};
	app.GridPivot.Scale										= {1, -1, -1};
	app.GridPivot.Orientation.Normalize();

	::llc::pobj<::llc::SDialogSlider>									slider									= {};
	app.DialogMain.Controls[app.SliderH].as(slider);
	::llc::pobj<::llc::SDialogViewport>									viewport									= {};
	app.DialogMain.Controls[app.ViewportMinimap].as(viewport);
	gui.Controls.Images[viewport->IdClient].ImageOffset.x				= (s1_t)slider->ValueCurrent;
	::llc::controlMetricsInvalidate(gui, slider->IdGUIControl);

	app.DialogMain.Controls[app.SliderV].as(slider);
	gui.Controls.Images[viewport->IdClient].ImageOffset.y				= (s1_t)slider->ValueCurrent;
	::llc::controlMetricsInvalidate(gui, slider->IdGUIControl);


	timer.Frame();
	sprintf_s(app.StringFrameRateUpdate, "Last frame time (update): %fs.", (float)timer.LastTimeSeconds);
	//warning_printf("Update time: %f.", (float)timer.LastTimeSeconds);
	gui.Controls.Text[app.IdFrameRateUpdate].Text							= app.StringFrameRateUpdate;
	::llc::controlMetricsInvalidate(gui, app.IdFrameRateUpdate);

	return 0;
}

::llc::err_t				drawGND
	( ::llc::SRenderCache						& renderCache
	, ::llc::SSceneTransforms					& transforms
	, ::llc::SSceneCamera						& camera
	, ::llc::SWindow::TOffscreen				& target
	, const ::llc::SModelPivot<float>			& modelPivot
	, const ::llc::n3f2_t						& lightDir
	, const ::llc::SModelGND					& modelGND
	, const ::llc::SRSWWorldLight				& directionalLight
	, const ::llc::view<const ::llc::img8bgra>	& textures
	, const ::llc::view<const ::llc::SLightInfoRSW>	& lights
	, bool											wireframe
	);
::llc::err_t			draw		(::gme::SApplication & app)							{
	::llc::STimer															timer;
	::llc::SFramework				& framework									= app.Framework;
	::llc::SGUI						& gui										= *framework.GUI;
	static ::llc::rtbgra8d32				buffer3D;
	::llc::pobj<::llc::SDialogViewport>									viewport									= {};
	app.DialogMain.Controls[app.Viewport].as(viewport);
	const ::llc::n2<u2_t >											& offscreenMetrics							= gui.Controls.Placement[viewport->IdClient].Area.Size.Cast<u2_t >();

	buffer3D.resize(offscreenMetrics, {0, 0, 0, 0}, 0xFFFFFFFF);
	gui.Controls.Images[viewport->IdClient].Image						= buffer3D.Color.View;
	::llc::pobj<::llc::SDialogCheckBox>			checkbox;
	app.DialogMain.Controls[app.CheckBox].as(checkbox);

	s2_t 																pixelsDrawn0								= drawGND(app.RenderCache, app.Scene.Transforms, app.Scene.Camera, buffer3D, app.GridPivot, app.LightDirection, app.GNDModel, app.RSWData.Light, app.TexturesGND, app.RSWData.RSWLights, checkbox->Checked);
	if_true_e(::llc::failed(pixelsDrawn0));
	::llc::pobj<::llc::rtbgra8d32>	target;
	target.create();
	target->resize(app.Framework.RootWindow.Size, ::llc::LIGHTGRAY, 0xFFFFFFFFU);
	{
		::std::lock_guard														lock					(app.Framework.LockGUI);
		::llc::guiDraw(*app.Framework.GUI, target->Color.View);
	}
	{
		::std::lock_guard														lock					(app.Framework.LockGUI);
		::llc::guiDraw(*app.DialogMain.GUI, target->Color.View);
	}
	{
		::std::lock_guard														lock					(app.LockRender);
		app.Offscreen														= target;
	}
	timer.Frame();
	sprintf_s(app.StringFrameRateRender, "Last frame time (render): %fs.", (float)timer.LastTimeSeconds);
	gui.Controls.Text[app.IdFrameRateRender].Text							= app.StringFrameRateRender;
	::llc::controlMetricsInvalidate(gui, app.IdFrameRateRender);
	return 0;
}
