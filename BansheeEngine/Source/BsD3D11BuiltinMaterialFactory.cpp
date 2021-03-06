//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#include "BsD3D11BuiltinMaterialFactory.h"
#include "BsRendererManager.h"
#include "BsGpuProgram.h"
#include "BsShader.h"
#include "BsTechnique.h"
#include "BsPass.h"
#include "BsMaterial.h"
#include "BsBlendState.h"
#include "BsDepthStencilState.h"

namespace BansheeEngine
{
	void D3D11BuiltinMaterialFactory::startUp()
	{
		initSpriteTextShader();
		initSpriteImageShader();
		initDebugDraw2DClipSpaceShader();
		initDebugDraw2DScreenSpaceShader();
		initDebugDraw3DShader();
		initDockDropOverlayShader();
		initDummyShader();

		SAMPLER_STATE_DESC ssDesc;
		ssDesc.magFilter = FO_POINT;
		ssDesc.minFilter = FO_POINT;
		ssDesc.mipFilter = FO_POINT;

		mGUISamplerState = SamplerState::create(ssDesc);
	}

	void D3D11BuiltinMaterialFactory::shutDown()
	{
		mSpriteTextShader = nullptr;
		mSpriteImageShader = nullptr;
		mDebugDraw2DClipSpaceShader = nullptr;
		mDebugDraw2DScreenSpaceShader = nullptr;
		mDebugDraw3DShader = nullptr;
		mDockDropOverlayShader = nullptr;
		mDummyShader = nullptr;
	}

	const String& D3D11BuiltinMaterialFactory::getSupportedRenderSystem() const
	{
		static String renderSystem = "BansheeD3D11RenderSystem";

		return renderSystem;
	}

	HMaterial D3D11BuiltinMaterialFactory::createSpriteTextMaterial() const
	{
		HMaterial newMaterial = Material::create(mSpriteTextShader);
		newMaterial->setSamplerState("mainTexSamp", mGUISamplerState);

		return newMaterial;
	}

	HMaterial D3D11BuiltinMaterialFactory::createSpriteImageMaterial() const
	{
		HMaterial newMaterial = Material::create(mSpriteImageShader);
		newMaterial->setSamplerState("mainTexSamp", mGUISamplerState);

		return newMaterial;
	}

	HMaterial D3D11BuiltinMaterialFactory::createDebugDraw2DClipSpaceMaterial() const
	{
		return Material::create(mDebugDraw2DClipSpaceShader);
	}

	HMaterial D3D11BuiltinMaterialFactory::createDebugDraw2DScreenSpaceMaterial() const
	{
		return Material::create(mDebugDraw2DScreenSpaceShader);
	}

	HMaterial D3D11BuiltinMaterialFactory::createDebugDraw3DMaterial() const
	{
		return Material::create(mDebugDraw3DShader);
	}

	HMaterial D3D11BuiltinMaterialFactory::createDockDropOverlayMaterial() const
	{
		return Material::create(mDockDropOverlayShader);
	}

	HMaterial D3D11BuiltinMaterialFactory::createDummyMaterial() const
	{
		return Material::create(mDummyShader);
	}

	void D3D11BuiltinMaterialFactory::initSpriteTextShader()
	{
		String vsCode = "										\
			float invViewportWidth;								\
			float invViewportHeight;							\
			float4x4 worldTransform;							\
																\
			void vs_main(										\
			in float3 inPos : POSITION,							\
			in float2 uv : TEXCOORD0,							\
			out float4 oPosition : SV_Position,					\
			out float2 oUv : TEXCOORD0)							\
			{													\
				float4 tfrmdPos = mul(worldTransform, float4(inPos.xy, 0, 1));	\
																				\
				float tfrmdX = -1.0f + (tfrmdPos.x * invViewportWidth);			\
				float tfrmdY = 1.0f - (tfrmdPos.y * invViewportHeight);			\
																\
				oPosition = float4(tfrmdX, tfrmdY, 0, 1);		\
				oUv = uv;										\
			}													\
			";

		String psCode = "																			\
			SamplerState mainTexSamp : register(s0);												\
			Texture2D mainTexture : register(t0);													\
			float4 tint;																			\
																									\
			float4 ps_main(in float4 inPos : SV_Position, float2 uv : TEXCOORD0) : SV_Target		\
			{																						\
				float4 color = float4(tint.rgb, mainTexture.Sample(mainTexSamp, uv).r * tint.a);	\
				return color;																		\
			}";

		HGpuProgram vsProgram = GpuProgram::create(vsCode, "vs_main", "hlsl", GPT_VERTEX_PROGRAM, GPP_VS_4_0);
		HGpuProgram psProgram = GpuProgram::create(psCode, "ps_main", "hlsl", GPT_FRAGMENT_PROGRAM, GPP_PS_4_0);

		vsProgram.synchronize();
		psProgram.synchronize();

		mSpriteTextShader = Shader::create("TextSpriteShader");

		mSpriteTextShader->addParameter("worldTransform", "worldTransform", GPDT_MATRIX_4X4);
		mSpriteTextShader->addParameter("invViewportWidth", "invViewportWidth", GPDT_FLOAT1);
		mSpriteTextShader->addParameter("invViewportHeight", "invViewportHeight", GPDT_FLOAT1);
		mSpriteTextShader->addParameter("mainTexSamp", "mainTexSamp", GPOT_SAMPLER2D);
		mSpriteTextShader->addParameter("mainTexture", "mainTexture", GPOT_TEXTURE2D);
		mSpriteTextShader->addParameter("tint", "tint", GPDT_FLOAT4);

		TechniquePtr newTechnique = mSpriteTextShader->addTechnique("D3D11RenderSystem", RendererManager::getCoreRendererName()); 
		PassPtr newPass = newTechnique->addPass();
		newPass->setVertexProgram(vsProgram);
		newPass->setFragmentProgram(psProgram);

		BLEND_STATE_DESC desc;
		desc.renderTargetDesc[0].blendEnable = true;
		desc.renderTargetDesc[0].srcBlend = BF_SOURCE_ALPHA;
		desc.renderTargetDesc[0].dstBlend = BF_INV_SOURCE_ALPHA;
		desc.renderTargetDesc[0].blendOp = BO_ADD;
		desc.renderTargetDesc[0].renderTargetWriteMask = 0x7; // Don't write to alpha

		HBlendState blendState = BlendState::create(desc);
		newPass->setBlendState(blendState);

		DEPTH_STENCIL_STATE_DESC depthStateDesc;
		depthStateDesc.depthReadEnable = false;
		depthStateDesc.depthWriteEnable = false;

		HDepthStencilState depthState = DepthStencilState::create(depthStateDesc);
		newPass->setDepthStencilState(depthState);
	}

	void D3D11BuiltinMaterialFactory::initSpriteImageShader()
	{
		String vsCode = "										\
						float invViewportWidth;								\
						float invViewportHeight;							\
						float4x4 worldTransform;							\
						\
						void vs_main(										\
						in float3 inPos : POSITION,							\
						in float2 uv : TEXCOORD0,							\
						out float4 oPosition : SV_Position,					\
						out float2 oUv : TEXCOORD0)							\
						{													\
						float4 tfrmdPos = mul(worldTransform, float4(inPos.xy, 0, 1));	\
						\
						float tfrmdX = -1.0f + (tfrmdPos.x * invViewportWidth);			\
						float tfrmdY = 1.0f - (tfrmdPos.y * invViewportHeight);			\
						\
						oPosition = float4(tfrmdX, tfrmdY, 0, 1);		\
						oUv = uv;										\
						}													\
						";

		String psCode = "																					\
						SamplerState mainTexSamp : register(s0);											\
						Texture2D mainTexture : register(t0);												\
						float4 tint;																		\
																											\
						float4 ps_main(in float4 inPos : SV_Position, float2 uv : TEXCOORD0) : SV_Target	\
						{																					\
						float4 color = mainTexture.Sample(mainTexSamp, uv);									\
						return color * tint;																\
						}";

		HGpuProgram vsProgram = GpuProgram::create(vsCode, "vs_main", "hlsl", GPT_VERTEX_PROGRAM, GPP_VS_4_0);
		HGpuProgram psProgram = GpuProgram::create(psCode, "ps_main", "hlsl", GPT_FRAGMENT_PROGRAM, GPP_PS_4_0);

		vsProgram.synchronize();
		psProgram.synchronize();

		mSpriteImageShader = Shader::create("ImageSpriteShader");

		mSpriteImageShader->addParameter("worldTransform", "worldTransform", GPDT_MATRIX_4X4);
		mSpriteImageShader->addParameter("invViewportWidth", "invViewportWidth", GPDT_FLOAT1);
		mSpriteImageShader->addParameter("invViewportHeight", "invViewportHeight", GPDT_FLOAT1);
		mSpriteImageShader->addParameter("mainTexSamp", "mainTexSamp", GPOT_SAMPLER2D);
		mSpriteImageShader->addParameter("mainTexture", "mainTexture", GPOT_TEXTURE2D);
		mSpriteImageShader->addParameter("tint", "tint", GPDT_FLOAT4);

		TechniquePtr newTechnique = mSpriteImageShader->addTechnique("D3D11RenderSystem", RendererManager::getCoreRendererName()); 
		PassPtr newPass = newTechnique->addPass();
		newPass->setVertexProgram(vsProgram);
		newPass->setFragmentProgram(psProgram);

		BLEND_STATE_DESC desc;
		desc.renderTargetDesc[0].blendEnable = true;
		desc.renderTargetDesc[0].srcBlend = BF_SOURCE_ALPHA;
		desc.renderTargetDesc[0].dstBlend = BF_INV_SOURCE_ALPHA;
		desc.renderTargetDesc[0].blendOp = BO_ADD;
		desc.renderTargetDesc[0].renderTargetWriteMask = 0x7; // Don't write to alpha

		HBlendState blendState = BlendState::create(desc);
		newPass->setBlendState(blendState);

		DEPTH_STENCIL_STATE_DESC depthStateDesc;
		depthStateDesc.depthReadEnable = false;
		depthStateDesc.depthWriteEnable = false;

		HDepthStencilState depthState = DepthStencilState::create(depthStateDesc);
		newPass->setDepthStencilState(depthState);
	}

	void D3D11BuiltinMaterialFactory::initDebugDraw2DClipSpaceShader()
	{
		String vsCode = "						\
		void vs_main(							\
			in float2 inPos : POSITION,			\
			in float4 color : COLOR0,			\
			out float4 oPosition : SV_Position, \
			out float4 oColor : COLOR0)			\
		{										\
			oPosition = float4(inPos.xy, 0, 1); \
			oColor = color;						\
		}";

		String psCode = "																		\
		float4 ps_main(in float4 inPos : SV_Position, in float4 color : COLOR0) : SV_Target		\
		{																						\
			return color;																		\
		}																						\
		";	

		HGpuProgram vsProgram = GpuProgram::create(vsCode, "vs_main", "hlsl", GPT_VERTEX_PROGRAM, GPP_VS_4_0);
		HGpuProgram psProgram = GpuProgram::create(psCode, "ps_main", "hlsl", GPT_FRAGMENT_PROGRAM, GPP_PS_4_0);

		vsProgram.synchronize();
		psProgram.synchronize();

		mDebugDraw2DClipSpaceShader = Shader::create("DebugDraw2DClipSpaceShader");

		TechniquePtr newTechnique = mDebugDraw2DClipSpaceShader->addTechnique("D3D11RenderSystem", RendererManager::getCoreRendererName()); 
		PassPtr newPass = newTechnique->addPass();
		newPass->setVertexProgram(vsProgram);
		newPass->setFragmentProgram(psProgram);

		BLEND_STATE_DESC desc;
		desc.renderTargetDesc[0].blendEnable = true;
		desc.renderTargetDesc[0].srcBlend = BF_SOURCE_ALPHA;
		desc.renderTargetDesc[0].dstBlend = BF_INV_SOURCE_ALPHA;
		desc.renderTargetDesc[0].blendOp = BO_ADD;

		HBlendState blendState = BlendState::create(desc);
		newPass->setBlendState(blendState);

		DEPTH_STENCIL_STATE_DESC depthStateDesc;
		depthStateDesc.depthReadEnable = false;
		depthStateDesc.depthWriteEnable = false;

		HDepthStencilState depthState = DepthStencilState::create(depthStateDesc);
		newPass->setDepthStencilState(depthState);
	}

	void D3D11BuiltinMaterialFactory::initDebugDraw2DScreenSpaceShader()
	{
		String vsCode = "									\
						float invViewportWidth;				\
						float invViewportHeight;			\
															\
						void vs_main(						\
						in float2 inPos : POSITION,			\
						in float4 color : COLOR0,			\
						out float4 oPosition : SV_Position, \
						out float4 oColor : COLOR0)			\
						{														\
						float tfrmdX = -1.0f + (inPos.x * invViewportWidth);	\
						float tfrmdY = 1.0f - (inPos.y * invViewportHeight);	\
																				\
						oPosition = float4(tfrmdX, tfrmdY, 0, 1);				\
						oColor = color;											\
						}";

		String psCode = "																		\
						float4 ps_main(in float4 inPos : SV_Position, in float4 color : COLOR0) : SV_Target		\
						{																						\
						return color;																		\
						}																						\
						";	

		HGpuProgram vsProgram = GpuProgram::create(vsCode, "vs_main", "hlsl", GPT_VERTEX_PROGRAM, GPP_VS_4_0);
		HGpuProgram psProgram = GpuProgram::create(psCode, "ps_main", "hlsl", GPT_FRAGMENT_PROGRAM, GPP_PS_4_0);

		vsProgram.synchronize();
		psProgram.synchronize();

		mDebugDraw2DScreenSpaceShader = Shader::create("DebugDraw2DScreenSpaceShader");

		mDebugDraw2DScreenSpaceShader->addParameter("invViewportWidth", "invViewportWidth", GPDT_FLOAT1);
		mDebugDraw2DScreenSpaceShader->addParameter("invViewportHeight", "invViewportHeight", GPDT_FLOAT1);

		TechniquePtr newTechnique = mDebugDraw2DScreenSpaceShader->addTechnique("D3D11RenderSystem", RendererManager::getCoreRendererName()); 
		PassPtr newPass = newTechnique->addPass();
		newPass->setVertexProgram(vsProgram);
		newPass->setFragmentProgram(psProgram);

		BLEND_STATE_DESC desc;
		desc.renderTargetDesc[0].blendEnable = true;
		desc.renderTargetDesc[0].srcBlend = BF_SOURCE_ALPHA;
		desc.renderTargetDesc[0].dstBlend = BF_INV_SOURCE_ALPHA;
		desc.renderTargetDesc[0].blendOp = BO_ADD;

		HBlendState blendState = BlendState::create(desc);
		newPass->setBlendState(blendState);

		DEPTH_STENCIL_STATE_DESC depthStateDesc;
		depthStateDesc.depthReadEnable = false;
		depthStateDesc.depthWriteEnable = false;

		HDepthStencilState depthState = DepthStencilState::create(depthStateDesc);
		newPass->setDepthStencilState(depthState);
	}

	void D3D11BuiltinMaterialFactory::initDebugDraw3DShader()
	{
		String vsCode = "float4x4 matViewProj;					\
															\
						void vs_main(						\
						in float3 inPos : POSITION,			\
						in float4 color : COLOR0,			\
						out float4 oPosition : SV_Position, \
						out float4 oColor : COLOR0)			\
						{										\
						oPosition = mul(matViewProj, float4(inPos.xyz, 1)); \
						oColor = color;						\
						}";

		String psCode = "																		\
						float4 ps_main(in float4 inPos : SV_Position, in float4 color : COLOR0) : SV_Target		\
						{																						\
						return color;																		\
						}																						\
						";	

		HGpuProgram vsProgram = GpuProgram::create(vsCode, "vs_main", "hlsl", GPT_VERTEX_PROGRAM, GPP_VS_4_0);
		HGpuProgram psProgram = GpuProgram::create(psCode, "ps_main", "hlsl", GPT_FRAGMENT_PROGRAM, GPP_PS_4_0);

		vsProgram.synchronize();
		psProgram.synchronize();

		mDebugDraw3DShader = Shader::create("DebugDraw3DShader");

		mDebugDraw3DShader->addParameter("matViewProj", "matViewProj", GPDT_MATRIX_4X4);

		TechniquePtr newTechnique = mDebugDraw3DShader->addTechnique("D3D11RenderSystem", RendererManager::getCoreRendererName()); 
		PassPtr newPass = newTechnique->addPass();
		newPass->setVertexProgram(vsProgram);
		newPass->setFragmentProgram(psProgram);

		BLEND_STATE_DESC desc;
		desc.renderTargetDesc[0].blendEnable = true;
		desc.renderTargetDesc[0].srcBlend = BF_SOURCE_ALPHA;
		desc.renderTargetDesc[0].dstBlend = BF_INV_SOURCE_ALPHA;
		desc.renderTargetDesc[0].blendOp = BO_ADD;

		HBlendState blendState = BlendState::create(desc);
		newPass->setBlendState(blendState);
	}

	void D3D11BuiltinMaterialFactory::initDockDropOverlayShader()
	{
		String vsCode = "									\
						float invViewportWidth;				\
						float invViewportHeight;			\
						\
						float4 tintColor;					\
						float4 highlightColor;				\
						float4 highlightActive;				\
						\
						void vs_main(						\
						in float2 inPos : POSITION,			\
						in float4 color : COLOR0,			\
						out float4 oPosition : SV_Position, \
						out float4 oColor : COLOR0)			\
						{														\
						float tfrmdX = -1.0f + (inPos.x * invViewportWidth);	\
						float tfrmdY = 1.0f - (inPos.y * invViewportHeight);	\
						\
						oPosition = float4(tfrmdX, tfrmdY, 0, 1);				\
																				\
						float4 highlight = highlightActive * color;				\
						float highlightSum = highlight.x + highlight.y +		\
							highlight.z + highlight.a;							\
																				\
						oColor = (1.0f - highlightSum) * tintColor +			\
							highlightSum * highlightColor;						\
						}";

		String psCode = "																		\
						float4 ps_main(in float4 inPos : SV_Position, in float4 color : COLOR0) : SV_Target		\
						{																						\
						return color;																		\
						}																						\
						";	

		HGpuProgram vsProgram = GpuProgram::create(vsCode, "vs_main", "hlsl", GPT_VERTEX_PROGRAM, GPP_VS_4_0);
		HGpuProgram psProgram = GpuProgram::create(psCode, "ps_main", "hlsl", GPT_FRAGMENT_PROGRAM, GPP_PS_4_0);

		vsProgram.synchronize();
		psProgram.synchronize();

		mDockDropOverlayShader = Shader::create("DockDropOverlayShader");

		mDockDropOverlayShader->addParameter("invViewportWidth", "invViewportWidth", GPDT_FLOAT1);
		mDockDropOverlayShader->addParameter("invViewportHeight", "invViewportHeight", GPDT_FLOAT1);

		mDockDropOverlayShader->addParameter("tintColor", "tintColor", GPDT_FLOAT4);
		mDockDropOverlayShader->addParameter("highlightColor", "highlightColor", GPDT_FLOAT4);
		mDockDropOverlayShader->addParameter("highlightActive", "highlightActive", GPDT_FLOAT4);

		TechniquePtr newTechnique = mDockDropOverlayShader->addTechnique("D3D11RenderSystem", RendererManager::getCoreRendererName()); 
		PassPtr newPass = newTechnique->addPass();
		newPass->setVertexProgram(vsProgram);
		newPass->setFragmentProgram(psProgram);

		BLEND_STATE_DESC desc;
		desc.renderTargetDesc[0].blendEnable = true;
		desc.renderTargetDesc[0].srcBlend = BF_SOURCE_ALPHA;
		desc.renderTargetDesc[0].dstBlend = BF_INV_SOURCE_ALPHA;
		desc.renderTargetDesc[0].blendOp = BO_ADD;

		HBlendState blendState = BlendState::create(desc);
		newPass->setBlendState(blendState);

		DEPTH_STENCIL_STATE_DESC depthStateDesc;
		depthStateDesc.depthReadEnable = false;
		depthStateDesc.depthWriteEnable = false;

		HDepthStencilState depthState = DepthStencilState::create(depthStateDesc);
		newPass->setDepthStencilState(depthState);
	}

	void D3D11BuiltinMaterialFactory::initDummyShader()
	{
		String vsCode = "float4x4 matWorldViewProj;				\
																\
						 void vs_main(							\
						 in float3 inPos : POSITION,			\
						 out float4 oPosition : SV_Position)	\
						 {										\
							 oPosition = mul(matWorldViewProj, float4(inPos.xyz, 1)); \
						 }";

		String psCode = "float4 ps_main() : SV_Target				\
						 {											\
							 return float4(0.5f, 0.5f, 0.5f, 0.5f);	\
						 }";	

		HGpuProgram vsProgram = GpuProgram::create(vsCode, "vs_main", "hlsl", GPT_VERTEX_PROGRAM, GPP_VS_4_0);
		HGpuProgram psProgram = GpuProgram::create(psCode, "ps_main", "hlsl", GPT_FRAGMENT_PROGRAM, GPP_PS_4_0);

		vsProgram.synchronize();
		psProgram.synchronize();

		mDummyShader = Shader::create("DummyShader");

		mDummyShader->addParameter("matWorldViewProj", "matWorldViewProj", GPDT_MATRIX_4X4);

		TechniquePtr newTechnique = mDummyShader->addTechnique("D3D11RenderSystem", RendererManager::getCoreRendererName());
		PassPtr newPass = newTechnique->addPass();
		newPass->setVertexProgram(vsProgram);
		newPass->setFragmentProgram(psProgram);
	}
}