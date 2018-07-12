#pragma once

#include <SonyMath/vectormath_aos.h>

#include <Structures/sTexture.h>
#include <Structures/sResourceID.h>
#include <Structures/sVertex.h>
#include <Render/Shader.h>

using namespace vmath;

enum eWindowMode
{
    WINDOWED,
    MAXIMIZED,
    WINDOWED_FULLSCREEN,
    FULLSCREEN
};

enum eBufferType
{
    STATIC,
    DYNAMIC
};

enum eBlendMode
{
    BLENDMODE_ZERO,
    BLENDMODE_ONE,
    BLENDMODE_SRC_COLOR,
    BLENDMODE_INV_SRC_COLOR,
    BLENDMODE_SRC_ALPHA,
    BLENDMODE_INV_SRC_ALPHA,
    BLENDMODE_DEST_ALPHA,
    BLENDMODE_INV_DEST_ALPHA,
    BLENDMODE_DEST_COLOR,
    BLENDMODE_INV_DEST_COLOR,
    BLENDMODE_SRC_ALPHA_SAT,
    BLENDMODE_BLEND_FACTOR,
    BLENDMODE_INV_BLEND_FACTOR,
    BLENDMODE_SRC1_COLOR,
    BLENDMODE_INV_SRC1_COLOR,
    BLENDMODE_SRC1_ALPHA,
    BLENDMODE_INV_SRC1_ALPHA,
    BLENDMODE_COUNT
};

enum eBlendOP
{
	BLENDOP_ADD,
	BLENDOP_SUBTRACT,
	BLENDOP_REV_SUBTRACT,
	BLENDOP_MIN,
	BLENDOP_MAX,
	BLENDOP_COUNT
};

enum eFillMode
{
	FILLMODE_WIREFRAME,
	FILLMODE_SOLID,
	FILLMODE_COUNT
};

enum eCullMode
{
	CULLMODE_BACK,
	CULLMODE_FRONT,
	CULLMODE_NONE,
	CULLMODE_COUNT
};

enum eTextureDimension
{
    TEXTUREDIMENSION_1D,
    TEXTUREDIMENSION_2D,
    TEXTUREDIMENSION_3D,
    TEXTUREDIMENSION_COUNT
};

enum eTextureFormat
{
	TEXTUREFORMAT_R8,
	TEXTUREFORMAT_R8G8,
	TEXTUREFORMAT_R8G8B8A8,
	TEXTUREFORMAT_R16G16B16A16,
	TEXTUREFORMAT_R8G8B8A8SNORM,
	TEXTUREFORMAT_R8G8SNORM,
    TEXTUREFORMAT_R32,
    TEXTUREFORMAT_R32G32,
    TEXTUREFORMAT_R32G32B32,
    TEXTUREFORMAT_R32G32B32A32,
    TEXTUREFORMAT_COUNT
};

struct sRenderTargetBlendDesc
{
	bool mBlendEnabled;
	eBlendMode mSrcBlend;
	eBlendMode mDestBlend;
	eBlendOP mBlendOP;
	eBlendMode mSrcBlendAlpha;
	eBlendMode mDestBlendAlpha;
	eBlendOP mBlendOPAlpha;
};

class RenderWindow
{
public:
    RenderWindow(int aWidth, int aHeight, char* aTitle, eWindowMode aMode) {}
    virtual ~RenderWindow() {}

    virtual void ChangeMode(eWindowMode aMode) = 0;
    virtual void ChangeName(char* aName) = 0;

    virtual int GetWidth() = 0;
    virtual int GetHeight() = 0;
    virtual void ChangeSize(int aWidth, int aHeight) = 0;

    virtual bool ShouldClose() = 0;

    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual void Update() = 0;

	virtual char* GetKey() = 0;
    virtual eWindowMode GetMode() = 0;
};

class Renderer
{
public:
    Renderer() {}
    virtual ~Renderer() {}

    virtual RenderWindow* CreateRenderWindow(int aWidth, int aHeight, char* aTitle, eWindowMode aMode) = 0;

    virtual sResourceID LoadTexture(int aWidth, int aHeight, char* aRawData, eTextureDimension dimension = eTextureDimension::TEXTUREDIMENSION_2D, eTextureFormat format = eTextureFormat::TEXTUREFORMAT_R8G8B8A8, bool aRenderTargetBindable = false) = 0;
    virtual sResourceID CompileVertexShader(char* aData) = 0;
    virtual sResourceID CompilePixelShader(char* aData) = 0;
    virtual sResourceID CompileGeometryShader(char* aData) = 0;
    virtual sResourceID CreateVertexBuffer(eBufferType aType, void* aData, int aStride, int aCount) = 0;
    virtual sResourceID CreateIndexBuffer(eBufferType aType, int* aData, int aCount) = 0;
    virtual sResourceID CreateConstantBuffer(eBufferType aType, void* aData, int aStructSize, int aCount) = 0;
	virtual sResourceID CreateRasterizerState(eFillMode aFillMode, eCullMode aCullMode) = 0;
	virtual sResourceID CreateBlendState(sRenderTargetBlendDesc aBlendDesc) = 0;

    virtual sResourceID CreateRenderTarget(int aWidth, int aHeight, eTextureDimension dimension, eTextureFormat format, bool aScaleToWindow = false) = 0;

    virtual void UpdateConstantBuffer(sResourceID aConstantBufferID, void* aData, int aStructSize, int aCount) = 0;
    virtual void UpdateTexture(sResourceID aTextureID, char* aRawData, int aWidth = -1, int aHeight = -1) = 0;

    virtual void BindTexture(sResourceID aTextureID, int aTextureSlot) = 0;
    virtual void BindVertexShader(sResourceID aVertexShaderID) = 0;
    virtual void BindPixelShader(sResourceID aPixelShaderID) = 0;
    virtual void BindGeometryShader(sResourceID aGeometryShaderID) = 0;
    virtual void BindVertexBuffer(sResourceID aVertexBufferID) = 0;
    virtual void BindIndexBuffer(sResourceID aIndexBufferID) = 0;
    virtual void BindConstantBuffer(eShaderType aShaderType, sResourceID aShaderID, sResourceID aConstantBufferID, int aBindSlot) = 0;
    virtual void BindRenderTarget(sResourceID aRenderTargetID = { 0 }) = 0;
    virtual void BindRenderTargets(sResourceID* aRenderTargetIDs, int aCount) = 0;

	virtual void SetRasterizerState(sResourceID aRasterizer) = 0;
	virtual void SetBlendState(sResourceID aBlendState) = 0;

    virtual sResourceID GetTextureFromRenderTarget(sResourceID aRenderTargetID) = 0;
    virtual int ObtainShaderBindingSlot(eShaderType aShaderType, sResourceID aShaderID, char* aBindingName) = 0;

    virtual char* GetShaderExtension() = 0;
    virtual char* GetShaderPreFlags() = 0;
    virtual char* GetRendererName() = 0;

    virtual void SetDepthTestEnabled(bool mEnabled) = 0;

    virtual void ClearColorBuffer(Vec4 aColor) = 0;
    virtual void ClearDepthBuffer(float aDepth) = 0;
	virtual void Draw(int aVertexCount, int aStartVertexLocation = 0) = 0;
    virtual void DrawIndexed(int aStart, int aCount) = 0;
    virtual void Swap() = 0;
};