#include "llc_bitmap_target.h"
#include "llc_ro_rsw.h"
#include "llc_dialog_controls.h"
#include "llc_scene.h"
#include "llc_ro_gnd.h"
#include "llc_line3.h"


namespace llc
{
	template<typename _tCoord>
	static	::llc::err_t	drawTriangle
		( ::llc::gu32					& targetDepth
		, const ::llc::minmaxf2_t		& fNearFar
		, const ::llc::tri3<_tCoord>	& triangle
		, ::llc::apod<::llc::n2s1_t>		& out_Points
		, ::llc::apod<::llc::trif2_t>	& triangleWeights
		) {
		s2_t						pixelsDrawn			= 0;
		const ::llc::n2u2_t			& _targetMetrics	= targetDepth.metrics();
		::llc::n2f2_t				areaMin				= {(float)::llc::min(::llc::min(triangle.A.x, triangle.B.x), triangle.C.x), (float)::llc::min(::llc::min(triangle.A.y, triangle.B.y), triangle.C.y)};
		::llc::n2f2_t				areaMax				= {(float)::llc::max(::llc::max(triangle.A.x, triangle.B.x), triangle.C.x), (float)::llc::max(::llc::max(triangle.A.y, triangle.B.y), triangle.C.y)};
		const float					xStop				= ::llc::min(areaMax.x, (float)_targetMetrics.x);
		for(float y = ::llc::max(areaMin.y, 0.f), yStop = ::llc::min(areaMax.y, (float)_targetMetrics.y); y < yStop; ++y)
		for(float x = ::llc::max(areaMin.x, 0.f); x < xStop; ++x) {
			const ::llc::n2s2_t			cellCurrent			= {(s2_t)x, (s2_t)y};
			const ::llc::tri2i32		triangle2D			=
				{ {(s2_t)triangle.A.x, (s2_t)triangle.A.y}
				, {(s2_t)triangle.B.x, (s2_t)triangle.B.y}
				, {(s2_t)triangle.C.x, (s2_t)triangle.C.y}
				};
			{
				s2_t						w0					= ::llc::orient2d({triangle2D.B, triangle2D.A}, cellCurrent);	// Determine barycentric coordinates
				s2_t						w1					= ::llc::orient2d({triangle2D.C, triangle2D.B}, cellCurrent);
				s2_t						w2					= ::llc::orient2d({triangle2D.A, triangle2D.C}, cellCurrent);
				if(w0 <= -1 || w1 <= -1 || w2 <= -1) // ---- If p is on or inside all edges, render pixel.
					continue;
			}
			const ::llc::n2<float>		cellCurrentF								= {x, y};
			::llc::tri<float>			proportions									=
				{ ::llc::orient2d3d({triangle.C, triangle.B}, cellCurrentF)	// notice how having to type "template" every time before "Cast" totally defeats the purpose of the template. I really find this rule very stupid and there is no situation in which the compiler is unable to resolve it from the code it already has.
				, ::llc::orient2d3d({triangle.A, triangle.C}, cellCurrentF)
				, ::llc::orient2d3d({triangle.B, triangle.A}, cellCurrentF)	// Determine barycentric coordinates
				};
			float						proportABC									= proportions.A + proportions.B + proportions.C; //(w0, w1, w2)
			if(proportABC <= 0)
				continue;
			proportions.A					/= proportABC;
			proportions.B					/= proportABC;
			proportions.C					= 1.0f - (proportions.A + proportions.B);
			float						finalZ
				= triangle.A.z * proportions.A
				+ triangle.B.z * proportions.B
				+ triangle.C.z * proportions.C
				;
			float																depth										= float((finalZ - fNearFar.Min) / (fNearFar.Max - fNearFar.Min));
			if(depth >= 1 || depth <= 0) // discard from depth planes
				continue;
			u2_t 															finalDepth									= (u2_t )(depth * 0x00FFFFFFU);
			if (targetDepth[(u2_t )y][(u2_t )x] > (u2_t )finalDepth) { // check against depth buffer
				targetDepth[(u2_t )y][(u2_t )x]							= finalDepth;
				triangleWeights.push_back({proportions.A, proportions.B, proportions.C});
				out_Points.push_back(cellCurrent.Cast<s1_t>());
				++pixelsDrawn;
			}
		}
		return pixelsDrawn;
	}
}
					::llc::err_t										drawPixelGND
	( ::llc::SRenderCache		& renderCache
	, ::llc::bgra				& targetColorCell
	, const ::llc::trif2_t		& pixelWeights
	, const ::llc::tri3f2_t		& positions
	, const ::llc::tri2f32		& uvs
	, const ::llc::g8bgra		& textureColors
	, s2_t					iTriangle
	, const ::llc::n3f3_t		& lightDir
	, const ::llc::rgbaf		& diffuseColor
	, const ::llc::rgbaf		& ambientColor
	, const ::llc::view<const ::llc::SLightInfoRSW>	& lights
	) {	// --- This function will draw some coloured symbols in each cell of the ASCII screen.
	const ::llc::tri3<float>												& normals									= renderCache.TransformedNormalsVertex[iTriangle];
	::llc::tri3<double>													weightedNormals								=
		{ normals.A.Cast<double>() * pixelWeights.A
		, normals.B.Cast<double>() * pixelWeights.B
		, normals.C.Cast<double>() * pixelWeights.C
		};
	const ::llc::n3<double>												interpolatedNormal							= (weightedNormals.A + weightedNormals.B + weightedNormals.C).Normalize();
	::llc::rgbaf															directionalColor							= diffuseColor * interpolatedNormal.Dot(lightDir);
	const ::llc::n2<u2_t >												textureMetrics								= textureColors.metrics();
	const ::llc::n2<double>												uv											=
		{ uvs.A.x * pixelWeights.A + uvs.B.x * pixelWeights.B + uvs.C.x * pixelWeights.C
		, uvs.A.y * pixelWeights.A + uvs.B.y * pixelWeights.B + uvs.C.y * pixelWeights.C
		};
	const ::llc::tri3<double>												weightedPositions							=
		{ positions.A.Cast<double>() * pixelWeights.A
		, positions.B.Cast<double>() * pixelWeights.B
		, positions.C.Cast<double>() * pixelWeights.C
		};

	const ::llc::n3<double>												interpolatedPosition						= weightedPositions.A + weightedPositions.B + weightedPositions.C;
	::llc::bgra															interpolatedBGRA;
	::llc::rgbaf															lightColor									= {0, 0, 0, 1}; //((::llc::RED * pixelWeights.A) + (::llc::GREEN * pixelWeights.B) + (::llc::BLUE * pixelWeights.C));
	if( 0 == textureMetrics.x
	 ||	0 == textureMetrics.y
	 ) {
		for(u2_t  iLight = 0; iLight < lights.size(); ++iLight) {
			const ::llc::SLightInfoRSW													& rswLight									= lights[iLight];
			::llc::n3f2_t														rswColor									= rswLight.Color * (1.0 - (rswLight.Position.Cast<double>() - interpolatedPosition).Length() / 10.0);
			lightColor																+= ::llc::rgbaf(rswColor.x, rswColor.y, rswColor.z, 1.0f) / 2.0;
		}
		interpolatedBGRA														= (directionalColor + lightColor + (ambientColor / 2.0)).Clamp();
	}
	else {
		const ::llc::n2<s2_t>												uvcoords									=
			{ (s2_t)((u2_t )(uv.x * textureMetrics.x) % textureMetrics.x)
			, (s2_t)((u2_t )(uv.y * textureMetrics.y) % textureMetrics.y)
			};
		const ::llc::rgbaf													& srcTexel									= textureColors[uvcoords.y][uvcoords.x];
		if (srcTexel == ::llc::rgbaf{ ::llc::bgra{0xFF, 0, 0xFF, 0xFF} })
			return 1;
		for(u2_t  iLight = 0, lightCount = lights.size(); iLight < lightCount; ++iLight) {
			const ::llc::SLightInfoRSW													& rswLight									= lights[rand() % lights.size()];
			double																		distFactor									= 1.0 - (rswLight.Position.Cast<double>() - interpolatedPosition).Length() / 10.0;
			if(distFactor > 0) {
				::llc::n3f2_t														rswColor									= rswLight.Color * distFactor;
				lightColor																+= srcTexel * ::llc::rgbaf(rswColor.x, rswColor.y, rswColor.z, 1.0f) / 2.0;
			}
		}
		interpolatedBGRA														= (srcTexel * directionalColor + lightColor + srcTexel * (ambientColor / 2.0)).Clamp();
	}
	if( targetColorCell == interpolatedBGRA )
		return 1;

	targetColorCell															= interpolatedBGRA;
	return 0;
}

static				::llc::err_t										transformTriangles
	( const ::llc::view<::llc::tri<u2_t >>	& vertexIndexList
	, const ::llc::view<::llc::n3f2_t>				& vertices
	, const ::llc::minmaxf2_t									& nearFar
	, const ::llc::m4<float>									& xWorld
	, const ::llc::m4<float>									& xWV
	, const ::llc::m4<float>									& xProjection
	, const ::llc::n2<s2_t>									& targetMetrics
	, ::llc::SRenderCache											& out_transformed
	) {	// --- This function will draw some coloured symbols in each cell of the ASCII screen.
	for(u2_t  iTriangle = 0, triCount = vertexIndexList.size(); iTriangle < triCount; ++iTriangle) {
		const ::llc::tri<u2_t >										& vertexIndices								= vertexIndexList[iTriangle];
		::llc::tri3<float>													triangle3DWorld								=
			{	vertices[vertexIndices.A]
			,	vertices[vertexIndices.B]
			,	vertices[vertexIndices.C]
			};
		::llc::tri3<float>													transformedTriangle3D						= triangle3DWorld;
		::llc::transform(transformedTriangle3D, xWV);
		// Check against far and near planes
		if(transformedTriangle3D.CulledZSpecial(nearFar))
			continue;
		float																		oldzA										= transformedTriangle3D.A.z;
		float																		oldzB										= transformedTriangle3D.B.z;
		float																		oldzC										= transformedTriangle3D.C.z;
		::llc::transform(transformedTriangle3D, xProjection);
		transformedTriangle3D.A.z												= oldzA;
		transformedTriangle3D.B.z												= oldzB;
		transformedTriangle3D.C.z												= oldzC;
		// Check against screen limits
		if(transformedTriangle3D.CulledX({0, (float)targetMetrics.x})) continue;
		if(transformedTriangle3D.CulledY({0, (float)targetMetrics.y})) continue;
		if_fail_fef(out_transformed.Triangle3dToDraw		.push_back(transformedTriangle3D)	, "Out of memory?");

		::llc::transform(triangle3DWorld, xWorld);
		if_fail_fef(out_transformed.Triangle3dWorld		.push_back(triangle3DWorld)			, "Out of memory?");
		if_fail_fef(out_transformed.Triangle3dIndices	.push_back(iTriangle)				, "Out of memory?");
	}
	return 0;
}

static				::llc::err_t										transformNormals
	( const ::llc::view<::llc::tri<u2_t >>	& vertexIndexList
	, const ::llc::view<::llc::n3f2_t>				& normals
	, const ::llc::m4<float>									& xWorld
	, ::llc::SRenderCache											& renderCache
	) {	// --- This function will draw some coloured symbols in each cell of the ASCII screen.
		for(u2_t  iTriangle = 0, triCount = renderCache.Triangle3dIndices.size(); iTriangle < triCount; ++iTriangle) { // transform normals
			const ::llc::tri<u2_t >										& vertexIndices								= vertexIndexList[renderCache.Triangle3dIndices[iTriangle]];
			::llc::tri3<float>													triangle3DWorldNormals						=
				{ normals[vertexIndices.A]
				, normals[vertexIndices.B]
				, normals[vertexIndices.C]
				};
			::llc::tri3<float>													& vertNormalsTri							= renderCache.TransformedNormalsVertex[iTriangle];
			vertNormalsTri.A														= xWorld.TransformDirection(normals[vertexIndices.A]).Normalize(); // gndNode.Normals[vertexIndices.A]; //
			vertNormalsTri.B														= xWorld.TransformDirection(normals[vertexIndices.B]).Normalize(); // gndNode.Normals[vertexIndices.B]; //
			vertNormalsTri.C														= xWorld.TransformDirection(normals[vertexIndices.C]).Normalize(); // gndNode.Normals[vertexIndices.C]; //
			//vertNormalsTri.A.y *= -1;
			//vertNormalsTri.B.y *= -1;
			//vertNormalsTri.C.y *= -1;
		}

	return 0;
}

static	::llc::err_t		drawTriangles
	( const ::llc::view<::llc::triu2_t>				& vertexIndexList
	, const ::llc::view<::llc::n3f2_t>				& vertices
	, const ::llc::view<::llc::n2f2_t>				& uvs
	, const ::llc::g8bgra							& textureView
	, const ::llc::minmaxf2_t						& nearFar
	, const ::llc::n3f2_t							& lightDir
	, ::llc::SRenderCache							& renderCache
	, ::llc::gu32									& targetDepthView
	, ::llc::g8bgra									& targetView
	, const ::llc::rgbaf							& diffuseColor
	, const ::llc::rgbaf							& ambientColor
	, const ::llc::view<const ::llc::SLightInfoRSW>	& lights
	, u2_t 										* pixelsDrawn
	, u2_t 										* pixelsSkipped
	, bool											wireframe
	) {	// ---
		//const ::llc::n3f2_t			& lightDir									= app.LightDirection;
		for(u2_t  iTriangle = 0, triCount = renderCache.Triangle3dIndices.size(); iTriangle < triCount; ++iTriangle) { //
			renderCache.TrianglePixelCoords.clear();
			renderCache.TrianglePixelWeights.clear();
			const ::llc::tri3<float>												& tri3DToDraw								= renderCache.Triangle3dToDraw[iTriangle];
			if_true_e(::llc::failed(::llc::drawTriangle(targetDepthView, nearFar, tri3DToDraw, renderCache.TrianglePixelCoords, renderCache.TrianglePixelWeights)));
			++renderCache.TrianglesDrawn;
			const ::llc::tri<u2_t >										& vertexIndices								= vertexIndexList[renderCache.Triangle3dIndices[iTriangle]];
			const ::llc::tri3<float>												triangle3DPositions							=
				{ vertices[vertexIndices.A]
				, vertices[vertexIndices.B]
				, vertices[vertexIndices.C]
				};
			const ::llc::tri2<float>												triangle3DUVs								=
				{ uvs[vertexIndices.A]
				, uvs[vertexIndices.B]
				, uvs[vertexIndices.C]
				};

			for(u2_t  iPixel = 0, pixCount = renderCache.TrianglePixelCoords.size(); iPixel < pixCount; ++iPixel) {
				const ::llc::n2<s1_t>												& pixelCoord								= renderCache.TrianglePixelCoords	[iPixel];
				const ::llc::tri<float>												& pixelWeights								= renderCache.TrianglePixelWeights	[iPixel];
				if(false == wireframe) {
					if(0 == ::drawPixelGND(renderCache, targetView[pixelCoord.y][pixelCoord.x], pixelWeights, triangle3DPositions, triangle3DUVs, textureView, iTriangle, lightDir.Cast<double>(), diffuseColor, ambientColor, ::llc::view<const ::llc::SLightInfoRSW>{lights.begin(), ::llc::min(lights.size(), 8U)}))
						++*pixelsDrawn;
					else
						++*pixelsSkipped;
				}
			}
			if(wireframe) {
				if_true_e(::llc::failed(::llc::drawLine(targetView.metrics().Cast<u1_t>(), ::llc::line3<float>{renderCache.Triangle3dToDraw[iTriangle].A, renderCache.Triangle3dToDraw[iTriangle].B}, renderCache.WireframePixelCoords)));
				if_true_e(::llc::failed(::llc::drawLine(targetView.metrics().Cast<u1_t>(), ::llc::line3<float>{renderCache.Triangle3dToDraw[iTriangle].B, renderCache.Triangle3dToDraw[iTriangle].C}, renderCache.WireframePixelCoords)));
				if_true_e(::llc::failed(::llc::drawLine(targetView.metrics().Cast<u1_t>(), ::llc::line3<float>{renderCache.Triangle3dToDraw[iTriangle].C, renderCache.Triangle3dToDraw[iTriangle].A}, renderCache.WireframePixelCoords)));
			}
		}
	return 0;
}

					::llc::err_t										updateTransforms							(::llc::SSceneTransforms & transforms, ::llc::SSceneCamera& camera, ::llc::n2<u2_t > targetMetrics)											{	// --- This function will draw some coloured symbols in each cell of the ASCII screen.
	::llc::m4<float>														& finalProjection							= transforms.FinalProjection	;
	::llc::m4<float>														& fieldOfView								= transforms.FieldOfView		;
	::llc::m4<float>														& mviewport									= transforms.Viewport			;
	fieldOfView.FieldOfView(camera.Angle * ::llc::math_pi, targetMetrics.x / (double)targetMetrics.y, camera.NearFar);
	mviewport.ViewportRH(targetMetrics.Cast<u1_t>());
	const ::llc::n2<s2_t>												screenCenter								= {(s2_t)targetMetrics.x / 2, (s2_t)targetMetrics.y / 2};
	finalProjection															= fieldOfView * mviewport;
	transforms.FinalProjectionInverse										= finalProjection.GetInverse();

	::llc::m4<float>														& viewMatrix								= transforms.View;
	::llc::SCameraVectors														& cameraVectors								= camera.Vectors;
	cameraVectors.Up														= {0, 1, 0};
	cameraVectors.Front														= (camera.Points.Target - camera.Points.Position).Normalize();
	cameraVectors.Right														= cameraVectors.Up		.Cross(cameraVectors.Front).Normalize();
	cameraVectors.Up														= cameraVectors.Front	.Cross(cameraVectors.Right).Normalize();
	viewMatrix.View3D(camera.Points.Position, cameraVectors.Right, cameraVectors.Up, cameraVectors.Front);
	//viewMatrix.LookAt(camera.Points.Position, {(app.GNDData.Metrics.Size.x / 2.0f), 0, -(app.GNDData.Metrics.Size.y / 2.0f)}, {0, 1, 0});
	return 0;
}

					::llc::err_t										drawGND
	( ::llc::SRenderCache							& renderCache
	, ::llc::SSceneTransforms						& transforms
	, ::llc::SSceneCamera							& camera
	, ::llc::rtbgra8d32								& target
	, const ::llc::SModelPivot<float>				& modelPivot
	, const ::llc::n3f2_t							& lightDir
	, const ::llc::SModelGND						& modelGND
	, const ::llc::SRSWWorldLight					& directionalLight
	, const ::llc::view<const ::llc::img8bgra>		& textures
	, const ::llc::view<const ::llc::SLightInfoRSW>	& lights
	, bool											wireframe
	) {	// --- This function will draw some coloured symbols in each cell of the ASCII screen.
	::llc::g8bgra											& offscreen									= target.Color.View;
	const ::llc::n2<u2_t >												& offscreenMetrics							= offscreen.metrics();
	::updateTransforms(transforms, camera, offscreenMetrics);

	const ::llc::m4<float>												& projection								= transforms.FinalProjection	;
	const ::llc::m4<float>												& viewMatrix								= transforms.View				;

	::llc::m4<float>														xRotation									= {};
	xRotation.Identity();
	::llc::m4<float>														xWorld										= {};
	xWorld.Identity();
	const ::llc::minmaxf2_t														& nearFar									= camera.NearFar;
	u2_t 																	& pixelsDrawn								= renderCache.PixelsDrawn	= 0;
	u2_t 																	& pixelsSkipped								= renderCache.PixelsSkipped	= 0;
	renderCache.WireframePixelCoords.clear();
	renderCache.TrianglesDrawn												= 0;
	const ::llc::rgbaf													ambient										= {directionalLight.Ambient.x, directionalLight.Ambient.y, directionalLight.Ambient.z, 1};
	const ::llc::rgbaf													diffuse										= {directionalLight.Diffuse.x, directionalLight.Diffuse.y, directionalLight.Diffuse.z, 1};
	xWorld		.Scale			(modelPivot.Scale, true);
	xRotation	.Identity();
	xRotation	.SetOrientation	(modelPivot.Orientation);
	xWorld																	= xWorld * xRotation;
	xWorld		.SetTranslation	(modelPivot.Position, false);
	for(u2_t  iGNDTexture = 0; iGNDTexture < textures.size(); ++iGNDTexture) {
		for(u2_t  iFacingDirection = 0; iFacingDirection < 6; ++iFacingDirection) {
			const ::llc::g8bgra									& gndNodeTexture							= textures[iGNDTexture].View;
			const ::llc::SModelNodeGND													& gndNode									= modelGND.Nodes[textures.size() * iFacingDirection + iGNDTexture];
			::llc::clear
				( renderCache.Triangle3dWorld
				, renderCache.Triangle3dToDraw
				, renderCache.Triangle3dIndices
				);
			const ::llc::m4<float>												xWV											= xWorld * viewMatrix;
			transformTriangles	(gndNode.VertexIndices, gndNode.Vertices, nearFar, xWorld, xWV, projection, offscreenMetrics.Cast<s2_t>(), renderCache);
			if_fail_fef(renderCache.TransformedNormalsVertex.resize(renderCache.Triangle3dIndices.size()), "Out of memory?");
			transformNormals	(gndNode.VertexIndices, gndNode.Normals, xWorld, renderCache);
			drawTriangles		(gndNode.VertexIndices, gndNode.Vertices, gndNode.UVs, gndNodeTexture, nearFar, lightDir, renderCache, target.DepthStencil.View, offscreen, diffuse, ambient, lights, &pixelsDrawn, &pixelsSkipped, wireframe);
		}
	}

	static	const ::llc::bgra												color										= ::llc::YELLOW;
	for(u2_t  iPixel = 0, pixCount = renderCache.WireframePixelCoords.size(); iPixel < pixCount; ++iPixel) {
		const ::llc::n2<s1_t>												& pixelCoord								= renderCache.WireframePixelCoords[iPixel];
		if( offscreen[pixelCoord.y][pixelCoord.x] != color ) {
			offscreen[pixelCoord.y][pixelCoord.x]								= color;
			++pixelsDrawn;
		}
		else
			++pixelsSkipped;
	}
	return (::llc::err_t)pixelsDrawn;
}
