//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#include "BsCoreThreadAccessor.h"
#include "BsCommandQueue.h"
#include "BsRenderSystem.h"
#include "BsBlendState.h"
#include "BsRasterizerState.h"
#include "BsDepthStencilState.h"
#include "BsGpuResourceData.h"
#include "BsVideoModeInfo.h"
#include "BsGpuParams.h"
#include "BsPass.h"
#include "BsMaterial.h"
#include "BsCoreThread.h"

namespace BansheeEngine
{
	CoreThreadAccessorBase::CoreThreadAccessorBase(CommandQueueBase* commandQueue)
		:mCommandQueue(commandQueue)
	{

	}

	CoreThreadAccessorBase::~CoreThreadAccessorBase()
	{
		bs_delete(mCommandQueue);
	}

	void CoreThreadAccessorBase::disableTextureUnit(GpuProgramType gptype, UINT16 texUnit)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::disableTextureUnit, RenderSystem::instancePtr(), gptype, texUnit));
	}

	void CoreThreadAccessorBase::setTexture(GpuProgramType gptype, UINT16 unit, bool enabled, const TexturePtr &texPtr)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::setTexture, RenderSystem::instancePtr(), gptype, unit, enabled, texPtr));
	}

	void CoreThreadAccessorBase::setSamplerState(GpuProgramType gptype, UINT16 texUnit, const SamplerStatePtr& samplerState)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::setSamplerState, RenderSystem::instancePtr(), gptype, texUnit, samplerState));
	}

	void CoreThreadAccessorBase::setBlendState(const BlendStatePtr& blendState)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::setBlendState, RenderSystem::instancePtr(), blendState));
	}

	void CoreThreadAccessorBase::setRasterizerState(const RasterizerStatePtr& rasterizerState)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::setRasterizerState, RenderSystem::instancePtr(), rasterizerState));
	}

	void CoreThreadAccessorBase::setDepthStencilState(const DepthStencilStatePtr& depthStencilState, UINT32 stencilRefValue)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::setDepthStencilState, RenderSystem::instancePtr(), depthStencilState, stencilRefValue));
	}

	void CoreThreadAccessorBase::setViewport(Viewport vp)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::setViewport, RenderSystem::instancePtr(), vp));
	}

	void CoreThreadAccessorBase::setDrawOperation(DrawOperationType op)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::setDrawOperation, RenderSystem::instancePtr(), op));
	}

	void CoreThreadAccessorBase::setClipPlanes(const PlaneList& clipPlanes)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::setClipPlanes, RenderSystem::instancePtr(), clipPlanes));
	}

	void CoreThreadAccessorBase::addClipPlane(const Plane& p)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::addClipPlane, RenderSystem::instancePtr(), p));
	}

	void CoreThreadAccessorBase::resetClipPlanes()
	{
		mCommandQueue->queue(std::bind(&RenderSystem::resetClipPlanes, RenderSystem::instancePtr()));
	}

	void CoreThreadAccessorBase::setScissorTest(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::setScissorRect, RenderSystem::instancePtr(), left, top, right, bottom));
	}

	void CoreThreadAccessorBase::setRenderTarget(RenderTargetPtr target)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::setRenderTarget, RenderSystem::instancePtr(), target));
	}

	void CoreThreadAccessorBase::bindGpuProgram(HGpuProgram prg)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::bindGpuProgram, RenderSystem::instancePtr(), prg));
	}

	void CoreThreadAccessorBase::unbindGpuProgram(GpuProgramType gptype)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::unbindGpuProgram, RenderSystem::instancePtr(), gptype));
	}

	void CoreThreadAccessorBase::bindGpuParams(GpuProgramType gptype, const GpuParamsPtr& params)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::bindGpuParams, RenderSystem::instancePtr(), gptype, params->_cloneForCore(gCoreThread().getFrameAlloc())));
	}

	void CoreThreadAccessorBase::beginRender()
	{
		mCommandQueue->queue(std::bind(&RenderSystem::beginFrame, RenderSystem::instancePtr()));
	}

	void CoreThreadAccessorBase::endRender()
	{
		mCommandQueue->queue(std::bind(&RenderSystem::endFrame, RenderSystem::instancePtr()));
	}

	void CoreThreadAccessorBase::clearRenderTarget(UINT32 buffers, const Color& color, float depth, UINT16 stencil)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::clearRenderTarget, RenderSystem::instancePtr(), buffers, color, depth, stencil));
	}

	void CoreThreadAccessorBase::clearViewport(UINT32 buffers, const Color& color, float depth, UINT16 stencil)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::clearViewport, RenderSystem::instancePtr(), buffers, color, depth, stencil));
	}

	void CoreThreadAccessorBase::swapBuffers(RenderTargetPtr target)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::swapBuffers, RenderSystem::instancePtr(), target));
	}

	void CoreThreadAccessorBase::render(const MeshBasePtr& mesh, UINT32 indexOffset, UINT32 indexCount, bool useIndices, DrawOperationType drawOp)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::render, RenderSystem::instancePtr(), mesh, indexOffset, indexCount, useIndices, drawOp));
	}

	void CoreThreadAccessorBase::draw(UINT32 vertexOffset, UINT32 vertexCount)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::draw, RenderSystem::instancePtr(), vertexOffset, vertexCount));
	}

	void CoreThreadAccessorBase::drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount)
	{
		mCommandQueue->queue(std::bind(&RenderSystem::drawIndexed, RenderSystem::instancePtr(), startIndex, indexCount, vertexOffset, vertexCount));
	}

	AsyncOp CoreThreadAccessorBase::writeSubresource(GpuResourcePtr resource, UINT32 subresourceIdx, const GpuResourceDataPtr& data, bool discardEntireBuffer)
	{
		data->_lock();

		resource->_writeSubresourceSim(subresourceIdx, *data, discardEntireBuffer);
		return mCommandQueue->queueReturn(std::bind(&RenderSystem::writeSubresource, RenderSystem::instancePtr(), resource, 
			subresourceIdx, data, discardEntireBuffer, std::placeholders::_1));
	}

	AsyncOp CoreThreadAccessorBase::readSubresource(GpuResourcePtr resource, UINT32 subresourceIdx, const GpuResourceDataPtr& data)
	{
		data->_lock();

		return mCommandQueue->queueReturn(std::bind(&RenderSystem::readSubresource, RenderSystem::instancePtr(), 
			resource, subresourceIdx, data, std::placeholders::_1));
	}

	void CoreThreadAccessorBase::resizeWindow(RenderWindowPtr& renderWindow, UINT32 width, UINT32 height)
	{
		mCommandQueue->queue(std::bind(&RenderWindow::resize, renderWindow.get(), width, height));
	}

	void CoreThreadAccessorBase::moveWindow(RenderWindowPtr& renderWindow, INT32 left, INT32 top)
	{
		mCommandQueue->queue(std::bind(&RenderWindow::move, renderWindow.get(), left, top));
	}

	void CoreThreadAccessorBase::hideWindow(RenderWindowPtr& renderWindow)
	{
		mCommandQueue->queue(std::bind(&RenderWindow::setHidden, renderWindow.get(), true));
	}

	void CoreThreadAccessorBase::showWindow(RenderWindowPtr& renderWindow)
	{
		mCommandQueue->queue(std::bind(&RenderWindow::setHidden, renderWindow.get(), false));
	}

	void CoreThreadAccessorBase::setFullscreen(RenderWindowPtr& renderWindow, UINT32 width, UINT32 height, 
		float refreshRate, UINT32 monitorIdx)
	{
		void(RenderWindow::*funcPtr)(UINT32, UINT32, float, UINT32) = &RenderWindow::setFullscreen;

		mCommandQueue->queue(std::bind(funcPtr, renderWindow.get(), width, height, refreshRate, monitorIdx));
	}

	void CoreThreadAccessorBase::setFullscreen(RenderWindowPtr& renderWindow, const VideoMode& mode)
	{
		void(RenderWindow::*funcPtr)(const VideoMode&) = &RenderWindow::setFullscreen;

		mCommandQueue->queue(std::bind(funcPtr, renderWindow.get(), std::cref(mode)));
	}

	void CoreThreadAccessorBase::setWindowed(RenderWindowPtr& renderWindow, UINT32 width, UINT32 height)
	{
		mCommandQueue->queue(std::bind(&RenderWindow::setWindowed, renderWindow.get(), width, height));
	}

	AsyncOp CoreThreadAccessorBase::queueReturnCommand(std::function<void(AsyncOp&)> commandCallback)
	{
		return mCommandQueue->queueReturn(commandCallback);
	}

	void CoreThreadAccessorBase::queueCommand(std::function<void()> commandCallback)
	{
		mCommandQueue->queue(commandCallback);
	}

	void CoreThreadAccessorBase::submitToCoreThread(bool blockUntilComplete)
	{
		Queue<QueuedCommand>* commands = mCommandQueue->flush();

		gCoreThread().queueCommand(std::bind(&CommandQueueBase::playback, mCommandQueue, commands), blockUntilComplete);
	}

	void CoreThreadAccessorBase::cancelAll()
	{
		// Note that this won't free any Frame data allocated for all the canceled commands since
		// frame data will only get cleared at frame start
		mCommandQueue->cancelAll();
	}
}