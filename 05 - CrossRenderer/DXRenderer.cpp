#include "DXRenderer.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include "DXRenderWindow.h"

#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

#include <d3d11_1.h>
#include <directxcolors.h>
#include <d3dcompiler.h>
#include <Windows.h>

using namespace DirectX;

struct sDXContext
{
	// Window data
	HWND mHwnd = nullptr;
	DXRenderWindow* mWindow = nullptr;

	// Device data
	D3D_DRIVER_TYPE         mDriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       mFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*           mPD3DDevice = nullptr;
	ID3D11Device1*          mPD3DDevice1 = nullptr;
	ID3D11DeviceContext*    mImmediateContext = nullptr;
	ID3D11DeviceContext1*   mImmediateContext1 = nullptr;
	IDXGISwapChain*         mSwapChain = nullptr;
	IDXGISwapChain1*        mSwapChain1 = nullptr;

	ID3D11Texture2D*        mDepthStencil = nullptr;

	ID3D11RenderTargetView* mRenderTargetView = nullptr;
	ID3D11DepthStencilView* mDepthStencilView = nullptr;

	ID3D11RenderTargetView* mBackRenderTargetView = nullptr;
	ID3D11DepthStencilView* mBackDepthStencilView = nullptr;

	ID3D11DepthStencilState* mDepthStencilStateEnabled = nullptr;
	ID3D11DepthStencilState* mDepthStencilStateDisabled = nullptr;
};

struct sVertexBuffer
{
	ID3D11Buffer* mBuffer = nullptr;
	UINT mStride = 0;
	UINT mOffset = 0;
};

struct sConstantBuffer
{
	ID3D11Buffer* mBuffer = nullptr;
	eShaderType mShaderType;
};

struct sCompiledVertexShader
{
	ID3D11VertexShader* mShader = nullptr;
	ID3DBlob* mData = nullptr;

	int mElementCount = 0;
	ID3D11InputLayout* mInputLayout = nullptr;
};

struct sCompiledPixelShader
{
	ID3D11PixelShader* mShader = nullptr;
	ID3DBlob* mData = nullptr;
};

struct sDXTexture
{
	ID3D11ShaderResourceView* mShaderResourceView = nullptr;
	ID3D11Resource* mResource = nullptr;
	ID3D11Texture1D* texture1D = nullptr;
	ID3D11Texture2D* texture2D = nullptr;
	ID3D11Texture3D* texture3D = nullptr;

	int mWidth;
	int mHeight;
	eTextureDimension mDimension;
	eTextureFormat mFormat;
	bool mRenderTargetBindable;
};

struct sRenderTarget
{
	sResourceID mTextureID;
	ID3D11RenderTargetView* mRenderTargetView = nullptr;
	ID3D11DepthStencilView* mDepthStencilView = nullptr;

	bool mScaleToWindow;
	int mOriginalTextureWidth;
	int mOriginalTextureHeight;
	int mOriginalWindowWidth;
	int mOriginalWindowHeight;
};

struct DXTextureFormat
{
	DXGI_FORMAT t[TEXTUREFORMAT_COUNT];

	DXTextureFormat()
	{
		t[TEXTUREFORMAT_R8] = DXGI_FORMAT_R8_UNORM;
		t[TEXTUREFORMAT_R8G8] = DXGI_FORMAT_R8G8_UNORM;
		t[TEXTUREFORMAT_R8G8B8A8] = DXGI_FORMAT_R8G8B8A8_UNORM;
		t[TEXTUREFORMAT_R16G16B16A16] = DXGI_FORMAT_R16G16B16A16_UNORM;
		t[TEXTUREFORMAT_R8G8B8A8SNORM] = DXGI_FORMAT_R8G8B8A8_SNORM;
		t[TEXTUREFORMAT_R8G8SNORM] = DXGI_FORMAT_R8G8_SNORM;
		t[TEXTUREFORMAT_R32] = DXGI_FORMAT_R32_FLOAT;
		t[TEXTUREFORMAT_R32G32] = DXGI_FORMAT_R32G32_FLOAT;
		t[TEXTUREFORMAT_R32G32B32] = DXGI_FORMAT_R32G32B32_FLOAT;
		t[TEXTUREFORMAT_R32G32B32A32] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	}
};
DXTextureFormat dxTextureFormats;

struct DXFillMode
{
	D3D11_FILL_MODE t[FILLMODE_COUNT];

	DXFillMode()
	{
		t[FILLMODE_WIREFRAME] = D3D11_FILL_WIREFRAME;
		t[FILLMODE_SOLID] = D3D11_FILL_SOLID;
	}
};
DXFillMode dxFillModes;

struct DXBlendMode
{
	D3D11_BLEND t[BLENDMODE_COUNT];

	DXBlendMode()
	{
		t[BLENDMODE_ZERO] = D3D11_BLEND_ZERO;
		t[BLENDMODE_ONE] = D3D11_BLEND_ONE;
		t[BLENDMODE_SRC_COLOR] = D3D11_BLEND_SRC_COLOR;
		t[BLENDMODE_INV_SRC_COLOR] = D3D11_BLEND_INV_SRC_COLOR;
		t[BLENDMODE_SRC_ALPHA] = D3D11_BLEND_SRC_ALPHA;
		t[BLENDMODE_INV_SRC_ALPHA] = D3D11_BLEND_INV_SRC_ALPHA;
		t[BLENDMODE_DEST_ALPHA] = D3D11_BLEND_DEST_ALPHA;
		t[BLENDMODE_INV_DEST_ALPHA] = D3D11_BLEND_INV_DEST_ALPHA;
		t[BLENDMODE_DEST_COLOR] = D3D11_BLEND_DEST_COLOR;
		t[BLENDMODE_INV_DEST_COLOR] = D3D11_BLEND_INV_DEST_COLOR;
		t[BLENDMODE_SRC_ALPHA_SAT] = D3D11_BLEND_SRC_ALPHA_SAT;
		t[BLENDMODE_BLEND_FACTOR] = D3D11_BLEND_BLEND_FACTOR;
		t[BLENDMODE_INV_BLEND_FACTOR] = D3D11_BLEND_INV_BLEND_FACTOR;
		t[BLENDMODE_SRC1_COLOR] = D3D11_BLEND_SRC1_COLOR;
		t[BLENDMODE_INV_SRC1_COLOR] = D3D11_BLEND_INV_SRC1_COLOR;
		t[BLENDMODE_SRC1_ALPHA] = D3D11_BLEND_SRC1_ALPHA;
		t[BLENDMODE_INV_SRC1_ALPHA] = D3D11_BLEND_INV_SRC1_ALPHA;
	}
};
DXBlendMode dxBlendMode;

struct DXBlendOP
{
	D3D11_BLEND_OP t[BLENDOP_COUNT];

	DXBlendOP()
	{
		t[BLENDOP_ADD] = D3D11_BLEND_OP_ADD;
		t[BLENDOP_SUBTRACT] = D3D11_BLEND_OP_SUBTRACT;
		t[BLENDOP_REV_SUBTRACT] = D3D11_BLEND_OP_REV_SUBTRACT;
		t[BLENDOP_MIN] = D3D11_BLEND_OP_MIN;
		t[BLENDOP_MAX] = D3D11_BLEND_OP_MAX;
	}
};
DXBlendOP dxBlendOP;

struct DXCullMode
{
	D3D11_CULL_MODE t[CULLMODE_COUNT];

	DXCullMode()
	{
		t[CULLMODE_BACK] = D3D11_CULL_BACK;
		t[CULLMODE_FRONT] = D3D11_CULL_FRONT;
		t[CULLMODE_NONE] = D3D11_CULL_NONE;
	}
};
DXCullMode dxCullModes;

struct DXTextureDimension
{
	D3D11_RTV_DIMENSION rtv[TEXTUREDIMENSION_COUNT];
	D3D11_SRV_DIMENSION srv[TEXTUREDIMENSION_COUNT];

	DXTextureDimension()
	{
		rtv[TEXTUREDIMENSION_1D] = D3D11_RTV_DIMENSION_TEXTURE1D;
		rtv[TEXTUREDIMENSION_2D] = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtv[TEXTUREDIMENSION_3D] = D3D11_RTV_DIMENSION_TEXTURE3D;

		srv[TEXTUREDIMENSION_1D] = D3D11_SRV_DIMENSION_TEXTURE1D;
		srv[TEXTUREDIMENSION_2D] = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv[TEXTUREDIMENSION_3D] = D3D11_SRV_DIMENSION_TEXTURE3D;
	}
};
DXTextureDimension dxDimensions;

class DXResources
{
public:
	std::vector<sCompiledVertexShader> mVertexShaders;
	std::vector<sCompiledPixelShader> mPixelShaders;

	std::vector<sVertexBuffer> mVertexBuffers;
	std::vector<ID3D11Buffer*> mIndexBuffers;
	std::vector<sConstantBuffer> mConstantBuffers;

	std::vector<sDXTexture> mTextures;
	std::vector<sRenderTarget> mRenderTargets;

	std::vector<ID3D11RasterizerState*> mRasterizerStates;
	std::vector<ID3D11BlendState*> mBlendStates;

	ID3D11RenderTargetView* mRenderTargetViewTempBuffer[128];
	D3D11_INPUT_ELEMENT_DESC mInputLayoutTempBuffer[128];
};

DXRenderer::DXRenderer()
{
	mContext = nullptr;
	mResources = nullptr;
}

DXRenderer::~DXRenderer()
{
	// Release resources
	for (sCompiledVertexShader res : mResources->mVertexShaders)
	{
		res.mData->Release();
		res.mShader->Release();
		res.mInputLayout->Release();
	}
	for (sCompiledPixelShader res : mResources->mPixelShaders)
	{
		res.mData->Release();
		res.mShader->Release();
	}
	for (sVertexBuffer res : mResources->mVertexBuffers)
	{
		res.mBuffer->Release();
	}
	for (ID3D11Buffer* res : mResources->mIndexBuffers)
	{
		res->Release();
	}
	for (sConstantBuffer res : mResources->mConstantBuffers)
	{
		res.mBuffer->Release();
	}
	for (sDXTexture res : mResources->mTextures)
	{
		res.mShaderResourceView->Release();
		if (res.mResource) res.mResource->Release();
	}
	for (sRenderTarget res : mResources->mRenderTargets)
	{
		res.mDepthStencilView->Release();
		res.mRenderTargetView->Release();
	}
	for (ID3D11RasterizerState* res : mResources->mRasterizerStates)
	{
		res->Release();
	}
	for (ID3D11BlendState* res : mResources->mBlendStates)
	{
		res->Release();
	}

	mContext->mImmediateContext->Flush();
	mContext->mImmediateContext->Release();
	mContext->mPD3DDevice->Release();

	delete mContext;
	delete mResources;
}

RenderWindow* DXRenderer::CreateRenderWindow(int aWidth, int aHeight, char* aTitle, eWindowMode aMode)
{
	assert(aWidth > 0);
	assert(aHeight > 0);
	assert(mContext == 0);

	DXRenderWindow* window = new DXRenderWindow(aWidth, aHeight, aTitle, aMode);
	window->mRenderer = this;

	mResources = new DXResources();
	mContext = new sDXContext();
	mContext->mWindow = window;

	if (!BuildContext(window, mContext) == S_OK)
	{
		// Could not build the context
		return 0;
	}

	return window;
}

HRESULT CompileShaderFromString(char* stringData, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompile(stringData, lstrlenA(stringData) + 1, szEntryPoint, NULL, NULL, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			printf(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

sResourceID DXRenderer::LoadTexture(int aWidth, int aHeight, char* aRawData, eTextureDimension dimension, eTextureFormat format, bool aRenderTargetBindable)
{
	assert(aWidth >= 0);
	assert(aHeight >= 0);

	sResourceID res = sResourceID();
	res.mResourceId = RESOURCE_UNDEFINED_ID;

	sDXTexture texture;
	texture.mWidth = aWidth;
	texture.mHeight = aHeight;
	texture.mDimension = dimension;
	texture.mFormat = format;
	texture.mRenderTargetBindable = aRenderTargetBindable;

	UINT bindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (aRenderTargetBindable)
	{
		bindFlags |= D3D11_BIND_RENDER_TARGET;
	}

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = aRawData;
	initData.SysMemPitch = static_cast<UINT>(aWidth * 4);
	initData.SysMemSlicePitch = static_cast<UINT>(0);
	D3D11_SUBRESOURCE_DATA* initDataPtr = &initData;
	if (aRawData == 0) initDataPtr = 0;

	ID3D11Resource* resource = nullptr;
	if (dimension == eTextureDimension::TEXTUREDIMENSION_1D)
	{
		// Create texture
		D3D11_TEXTURE1D_DESC desc;
		desc.Width = aWidth;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = dxTextureFormats.t[(int)(format)];
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = bindFlags;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		initData.SysMemPitch = static_cast<UINT>(4 * aWidth);

		ID3D11Texture1D* resTexture;
		HRESULT hr = mContext->mPD3DDevice->CreateTexture1D(&desc, initDataPtr, &resTexture);
		assert(SUCCEEDED(hr));
		resource = resTexture;
		texture.texture1D = resTexture;
	}
	if (dimension == eTextureDimension::TEXTUREDIMENSION_2D)
	{
		// Create texture
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = aWidth;
		desc.Height = aHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = dxTextureFormats.t[(int)(format)];
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = bindFlags;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		initData.SysMemPitch = static_cast<UINT>(4 * aWidth);

		ID3D11Texture2D* resTexture;
		HRESULT hr = mContext->mPD3DDevice->CreateTexture2D(&desc, initDataPtr, &resTexture);
		assert(SUCCEEDED(hr));
		resource = resTexture;
		texture.texture2D = resTexture;
	}
	if (dimension == eTextureDimension::TEXTUREDIMENSION_3D)
	{
		// Create texture
		D3D11_TEXTURE3D_DESC desc;
		desc.Width = aWidth;
		desc.Height = aHeight;
		desc.Depth = 1; // TODO
		desc.MipLevels = 1;
		desc.Format = dxTextureFormats.t[(int)(format)];
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = bindFlags;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		initData.SysMemPitch = static_cast<UINT>(aWidth * aHeight * 4);

		ID3D11Texture3D* resTexture;
		HRESULT hr = mContext->mPD3DDevice->CreateTexture3D(&desc, initDataPtr, &resTexture);
		assert(SUCCEEDED(hr));
		resource = resTexture;
		texture.texture3D = resTexture;
	}

	texture.mResource = resource;

	// Setup the description of the shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc; shaderResourceViewDesc.Format = dxTextureFormats.t[(int)(format)];
	shaderResourceViewDesc.ViewDimension = dxDimensions.srv[(int)(dimension)];
	shaderResourceViewDesc.Texture1D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture1D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture3D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture3D.MipLevels = 1;

	// Create the shader resource view
	HRESULT hr = mContext->mPD3DDevice->CreateShaderResourceView(resource, &shaderResourceViewDesc, &texture.mShaderResourceView);
	assert(SUCCEEDED(hr));

	// Add to resources
	int id = (int)(mResources->mTextures.size());
	res.mResourceId = id;
	mResources->mTextures.push_back(texture);

	return res;
}

sResourceID DXRenderer::CompileVertexShader(char* aShaderDataString)
{
	assert(aShaderDataString != nullptr);

	sResourceID res = sResourceID();
	res.mResourceId = RESOURCE_UNDEFINED_ID;

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromString(aShaderDataString, "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			"The VS file cannot be compiled. Please run this executable from the directory that contains the VS file.", "Error", MB_OK);
		return res;
	}

	// Create the vertex shader
	ID3D11VertexShader* newShaderPtr = nullptr;
	hr = mContext->mPD3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &newShaderPtr);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return res;
	}

	sCompiledVertexShader shader;
	shader.mShader = newShaderPtr;
	shader.mData = pVSBlob;

	// Reflect shader info
	ID3D11ShaderReflection* pVertexShaderReflection = NULL;
	if (FAILED(D3DReflect(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
	{
		res.mResourceId = -1;
		return res;
	}

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	pVertexShaderReflection->GetDesc(&shaderDesc);

	int elements = 0;
	int offset = 0;

	for (int i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);
		std::string name = std::string(paramDesc.SemanticName);

		D3D11_INPUT_ELEMENT_DESC d;
		bool found = false;

		if (name.compare("POSITION") == 0)
		{
			d = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, (UINT)(offset), D3D11_INPUT_PER_VERTEX_DATA, 0 };
			offset += 12;
			found = true;
		}
		if (name.compare("TEXCOORD") == 0)
		{
			d = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)(offset), D3D11_INPUT_PER_VERTEX_DATA, 0 };
			offset += 8;
			found = true;
		}
		if (name.compare("NORMAL") == 0)
		{
			d = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, (UINT)(offset), D3D11_INPUT_PER_VERTEX_DATA, 0 };
			offset += 12;
			found = true;
		}
		if (name.compare("TANGENT") == 0)
		{
			d = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, (UINT)(offset), D3D11_INPUT_PER_VERTEX_DATA, 0 };
			offset += 12;
			found = true;
		}
		if (name.compare("BITANGENT") == 0)
		{
			d = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, (UINT)(offset), D3D11_INPUT_PER_VERTEX_DATA, 0 };
			offset += 12;
			found = true;
		}

		// TEMP
		// DO NOT COMMIT
		if (!found)
		{
			d = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, (UINT)(offset), D3D11_INPUT_PER_VERTEX_DATA, 0 };
			offset += 12;
			found = true;
		}

		if (found)
		{
			mResources->mInputLayoutTempBuffer[elements] = d;

			elements++;
		}
		else
		{
			assert(false); // Could not find the meaning of this name.
		}

		continue;
	}

	shader.mElementCount = elements;

	// Create the input layout
	ID3D11InputLayout* tempInpurPtr = nullptr;
	hr = mContext->mPD3DDevice->CreateInputLayout(mResources->mInputLayoutTempBuffer, shader.mElementCount, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &tempInpurPtr);
	if (FAILED(hr))
		return res;

	shader.mInputLayout = tempInpurPtr;

	// Set the input layout
	mContext->mImmediateContext->IASetInputLayout(shader.mInputLayout);

	// Add to resources
	int id = (int)(mResources->mVertexShaders.size());
	res.mResourceId = id;
	mResources->mVertexShaders.push_back(shader);

	return res;
}

sResourceID DXRenderer::CompilePixelShader(char* aShaderDataString)
{
	assert(aShaderDataString != nullptr);

	sResourceID res = sResourceID();
	res.mResourceId = RESOURCE_UNDEFINED_ID;

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	HRESULT hr = CompileShaderFromString(aShaderDataString, "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			"The PS file cannot be compiled. Please run this executable from the directory that contains the PS file.", "Error", MB_OK);
		return res;
	}

	// Create the pixel shader
	ID3D11PixelShader* newShaderPtr = nullptr;
	hr = mContext->mPD3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &newShaderPtr);
	if (FAILED(hr))
		return res;

	sCompiledPixelShader shader;
	shader.mShader = newShaderPtr;
	shader.mData = pPSBlob;

	// Add to resources
	int id = (int)(mResources->mPixelShaders.size());
	res.mResourceId = id;
	mResources->mPixelShaders.push_back(shader);

	return res;
}

sResourceID DXRenderer::CompileGeometryShader(char* aShaderDataString)
{
	assert(aShaderDataString != nullptr);

	sResourceID res = sResourceID();
	res.mResourceId = RESOURCE_UNDEFINED_ID;

	return res;
}

sResourceID DXRenderer::CreateVertexBuffer(eBufferType aType, void* aData, int aStride, int aCount)
{
	assert(aData != nullptr);
	assert(aCount > 0);

	sResourceID res = sResourceID();
	res.mResourceId = RESOURCE_UNDEFINED_ID;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = aStride * aCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = aData;

	sVertexBuffer buffer;
	HRESULT hr = mContext->mPD3DDevice->CreateBuffer(&bd, &InitData, &buffer.mBuffer);
	assert(SUCCEEDED(hr));

	buffer.mOffset = 0;
	buffer.mStride = aStride;

	// Set primitive topology
	mContext->mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Add to resources
	int id = (int)(mResources->mVertexBuffers.size());
	res.mResourceId = id;
	mResources->mVertexBuffers.push_back(buffer);

	return res;
}

sResourceID DXRenderer::CreateIndexBuffer(eBufferType aType, int* aData, int aCount)
{
	assert(aData != nullptr);
	assert(aCount > 0);

	sResourceID res = sResourceID();
	res.mResourceId = RESOURCE_UNDEFINED_ID;

	// Describe index buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(int) * aCount;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = aData;

	// Create index buffer
	ID3D11Buffer* newBufferPtr;
	HRESULT hr = mContext->mPD3DDevice->CreateBuffer(&bd, &InitData, &newBufferPtr);
	assert(SUCCEEDED(hr));

	// Set index buffer
	mContext->mImmediateContext->IASetIndexBuffer(newBufferPtr, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology
	mContext->mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Add to resources
	int id = (int)(mResources->mIndexBuffers.size());
	res.mResourceId = id;
	mResources->mIndexBuffers.push_back(newBufferPtr);

	return res;
}

sResourceID DXRenderer::CreateConstantBuffer(eBufferType aType, void* aData, int aStructSize, int aCount)
{
	assert(aData != nullptr);

	sResourceID res = sResourceID();
	res.mResourceId = RESOURCE_UNDEFINED_ID;

	sConstantBuffer constantBuffer;

	int allocateBytes = aCount * aStructSize;

	// Make the data a multiple of 16
	if (allocateBytes % 16 != 0)
	{
		allocateBytes = allocateBytes + (16 - (allocateBytes % 16));
	}
	if (allocateBytes == 0)
	{
		allocateBytes = 16;
	}

	// Describe the buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = allocateBytes;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	// Create the constant buffer
	HRESULT hr = mContext->mPD3DDevice->CreateBuffer(&bd, nullptr, &constantBuffer.mBuffer);
	assert(SUCCEEDED(hr));

	// Add to resources
	int id = (int)(mResources->mConstantBuffers.size());
	res.mResourceId = id;
	mResources->mConstantBuffers.push_back(constantBuffer);

	UpdateConstantBuffer(res, aData, aStructSize, aCount);

	return res;
}

sResourceID DXRenderer::CreateRenderTarget(int aWidth, int aHeight, eTextureDimension dimension, eTextureFormat format, bool aScaleToWindow)
{
	sResourceID res;
	res.mResourceId = RESOURCE_UNDEFINED_ID;

	// Create render target texture
	sResourceID textureID = LoadTexture(aWidth, aHeight, 0, dimension, format, true);
	sDXTexture texture = mResources->mTextures[textureID.mResourceId];

	sRenderTarget renderTarget;
	renderTarget.mTextureID = textureID;
	renderTarget.mScaleToWindow = aScaleToWindow;
	renderTarget.mOriginalTextureWidth = aWidth;
	renderTarget.mOriginalTextureHeight = aHeight;
	renderTarget.mOriginalWindowWidth = mContext->mWindow->GetWidth();
	renderTarget.mOriginalWindowHeight = mContext->mWindow->GetHeight();

	D3D11_RENDER_TARGET_VIEW_DESC descRenderTarget;
	ZeroMemory(&descRenderTarget, sizeof(descRenderTarget));
	descRenderTarget.Format = dxTextureFormats.t[(int)(format)];
	descRenderTarget.ViewDimension = dxDimensions.rtv[(int)(dimension)];
	descRenderTarget.Texture2D.MipSlice = 0;

	HRESULT hr = mContext->mPD3DDevice->CreateRenderTargetView(texture.mResource, &descRenderTarget, &renderTarget.mRenderTargetView);
	assert(SUCCEEDED(hr));

	// Create depth stencil texture
	{
		ID3D11Texture2D* depthTexture = nullptr;

		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = aWidth;
		descDepth.Height = aHeight;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		HRESULT hr = mContext->mPD3DDevice->CreateTexture2D(&descDepth, nullptr, &depthTexture);
		assert(SUCCEEDED(hr));

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		hr = mContext->mPD3DDevice->CreateDepthStencilView(depthTexture, &descDSV, &renderTarget.mDepthStencilView);
		assert(SUCCEEDED(hr));
	}

	// Add render target to resources
	{
		int id = (int)(mResources->mRenderTargets.size());
		res.mResourceId = id;
		mResources->mRenderTargets.push_back(renderTarget);
	}

	return res;
}

sResourceID DXRenderer::CreateRasterizerState(eFillMode aFillMode, eCullMode aCullMode)
{
	sResourceID res = sResourceID();
	res.mResourceId = RESOURCE_UNDEFINED_ID;

	D3D11_RASTERIZER_DESC desc;

	desc.FillMode = dxFillModes.t[aFillMode];
	desc.CullMode = dxCullModes.t[aCullMode];
	desc.FrontCounterClockwise = true;
	desc.DepthBias = false;
	desc.DepthBiasClamp = 0;
	desc.SlopeScaledDepthBias = 0;
	desc.DepthClipEnable = false;
	desc.ScissorEnable = false;
	desc.MultisampleEnable = true;
	desc.AntialiasedLineEnable = true;

	ID3D11RasterizerState* rasterizerState;

	HRESULT hr = mContext->mPD3DDevice->CreateRasterizerState(&desc, &rasterizerState);
	assert(SUCCEEDED(hr));

	// Add to resources
	res.mResourceId = (int)(mResources->mRasterizerStates.size());
	mResources->mRasterizerStates.push_back(rasterizerState);

	return res;
}

sResourceID DXRenderer::CreateBlendState(sRenderTargetBlendDesc aBlendDesc)
{
	sResourceID res = sResourceID();
	res.mResourceId = RESOURCE_UNDEFINED_ID;

	D3D11_RENDER_TARGET_BLEND_DESC	tRendDesc;
	tRendDesc.BlendEnable = aBlendDesc.mBlendEnabled;
	tRendDesc.SrcBlend = dxBlendMode.t[aBlendDesc.mSrcBlend];
	tRendDesc.DestBlend = dxBlendMode.t[aBlendDesc.mDestBlend];
	tRendDesc.BlendOp = dxBlendOP.t[aBlendDesc.mBlendOP];
	tRendDesc.SrcBlendAlpha = dxBlendMode.t[aBlendDesc.mSrcBlendAlpha];
	tRendDesc.DestBlendAlpha = dxBlendMode.t[aBlendDesc.mDestBlendAlpha];
	tRendDesc.BlendOpAlpha = dxBlendOP.t[aBlendDesc.mBlendOPAlpha];
	tRendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	D3D11_BLEND_DESC desc;

	desc.AlphaToCoverageEnable = FALSE;
	desc.IndependentBlendEnable = FALSE;
	desc.RenderTarget[0] = tRendDesc;

	ID3D11BlendState* tBlendState;

	HRESULT hr = mContext->mPD3DDevice->CreateBlendState(&desc, &tBlendState);
	assert(SUCCEEDED(hr));

	// Add to resources
	res.mResourceId = (int)(mResources->mBlendStates.size());
	mResources->mBlendStates.push_back(tBlendState);

	return res;
}

void DXRenderer::UpdateConstantBuffer(sResourceID aConstantBufferID, void* aData, int aStructSize, int aCount)
{
	assert(aConstantBufferID.mResourceId != RESOURCE_UNDEFINED_ID);
	assert(aConstantBufferID.mResourceId < mResources->mConstantBuffers.size());

	if (aCount == 0) return;

	// Update constant buffer
	ID3D11Buffer* data = mResources->mConstantBuffers[aConstantBufferID.mResourceId].mBuffer;
	mContext->mImmediateContext->UpdateSubresource(data, 0, nullptr, aData, 0, 0);
}

void DXRenderer::UpdateTexture(sResourceID aTextureID, char* aRawData, int aWidth, int aHeight)
{
	if (aTextureID.mResourceId == RESOURCE_UNDEFINED_ID) return;

	sDXTexture data = mResources->mTextures[aTextureID.mResourceId];
	data.mResource->Release();
	data.mShaderResourceView->Release();
	data.mShaderResourceView = 0;
	data.mResource = 0;

	int newWidth = data.mWidth;
	int newHeight = data.mHeight;

	if (aWidth != -1 && aHeight != -1)
	{
		newWidth = aWidth;
		newHeight = aHeight;
	}

	sResourceID newRes = LoadTexture(newWidth, newHeight, aRawData, data.mDimension, data.mFormat, data.mRenderTargetBindable);

	// Rebind new texture to old texture
	mResources->mTextures[aTextureID.mResourceId] = mResources->mTextures[newRes.mResourceId];

	// Remove old data
	mResources->mTextures.erase(mResources->mTextures.begin() + newRes.mResourceId);
	mContext->mImmediateContext->Flush();
}

void DXRenderer::BindTexture(sResourceID aTextureID, int aTextureSlot)
{
	if (aTextureID.mResourceId == RESOURCE_UNDEFINED_ID)
	{
		// Unbind
		if (aTextureSlot != -1)
		{
			ID3D11ShaderResourceView* rTargets[1] = { NULL };
			mContext->mImmediateContext->PSSetShaderResources(aTextureSlot, 1, rTargets);
		}
		return;
	}
	assert(aTextureID.mResourceId < mResources->mTextures.size());

	if (aTextureSlot == -1)
	{
		return;
	}

	sDXTexture data = mResources->mTextures[aTextureID.mResourceId];
	mContext->mImmediateContext->PSSetShaderResources(aTextureSlot, 1, &data.mShaderResourceView);
}

void DXRenderer::BindVertexShader(sResourceID aVertexShaderID)
{
	assert(aVertexShaderID.mResourceId != RESOURCE_UNDEFINED_ID);
	assert(aVertexShaderID.mResourceId < mResources->mVertexShaders.size());

	sCompiledVertexShader data = mResources->mVertexShaders[aVertexShaderID.mResourceId];
	mContext->mImmediateContext->IASetInputLayout(data.mInputLayout);
	mContext->mImmediateContext->VSSetShader(data.mShader, nullptr, 0);
}

void DXRenderer::BindPixelShader(sResourceID aPixelShaderID)
{
	assert(aPixelShaderID.mResourceId != RESOURCE_UNDEFINED_ID);
	assert(aPixelShaderID.mResourceId < mResources->mPixelShaders.size());

	ID3D11PixelShader* data = mResources->mPixelShaders[aPixelShaderID.mResourceId].mShader;
	mContext->mImmediateContext->PSSetShader(data, nullptr, 0);
}

void DXRenderer::BindGeometryShader(sResourceID aGeometryShaderID)
{
}

void DXRenderer::BindVertexBuffer(sResourceID aVertexBufferID)
{
	assert(aVertexBufferID.mResourceId != RESOURCE_UNDEFINED_ID);
	assert(aVertexBufferID.mResourceId < mResources->mVertexBuffers.size());

	// Bind vertex buffer
	sVertexBuffer data = mResources->mVertexBuffers[aVertexBufferID.mResourceId];
	mContext->mImmediateContext->IASetVertexBuffers(0, 1, &data.mBuffer, &data.mStride, &data.mOffset);
}

void DXRenderer::BindIndexBuffer(sResourceID aIndexBufferID)
{
	assert(aIndexBufferID.mResourceId != RESOURCE_UNDEFINED_ID);
	assert(aIndexBufferID.mResourceId < mResources->mIndexBuffers.size());

	// Bind index buffer
	ID3D11Buffer* data = mResources->mIndexBuffers[aIndexBufferID.mResourceId];
	mContext->mImmediateContext->IASetIndexBuffer(data, DXGI_FORMAT_R32_UINT, 0);
}

void DXRenderer::BindConstantBuffer(eShaderType aShaderType, sResourceID aShaderID, sResourceID aConstantBufferID, int aBindSlot)
{
	assert(aConstantBufferID.mResourceId != RESOURCE_UNDEFINED_ID);
	assert(aConstantBufferID.mResourceId < mResources->mConstantBuffers.size());

	// Bind constant buffer
	sConstantBuffer data = mResources->mConstantBuffers[aConstantBufferID.mResourceId];

	if (aShaderType == eShaderType::VERTEX_SHADER)
	{
		mContext->mImmediateContext->VSSetConstantBuffers(aBindSlot, 1, &data.mBuffer);
	}

	if (aShaderType == eShaderType::PIXEL_SHADER)
	{
		mContext->mImmediateContext->PSSetConstantBuffers(aBindSlot, 1, &data.mBuffer);
	}
}

void DXRenderer::BindRenderTarget(sResourceID aRenderTargetID)
{
	assert(aRenderTargetID.mResourceId != RESOURCE_UNDEFINED_ID);
	assert(aRenderTargetID.mResourceId < mResources->mRenderTargets.size());

	// Obtain render target
	sRenderTarget renderTarget = mResources->mRenderTargets[aRenderTargetID.mResourceId];

	// Set shader resources to null
	ID3D11ShaderResourceView* const pSRV[1] = { NULL };
	mContext->mImmediateContext->PSSetShaderResources(0, 1, pSRV);

	mContext->mRenderTargetView = renderTarget.mRenderTargetView;
	mContext->mDepthStencilView = renderTarget.mDepthStencilView;

	// Bind render target
	ID3D11RenderTargetView* rTargets[4] = { NULL, NULL, NULL, NULL };
	mContext->mImmediateContext->OMSetRenderTargets(4, rTargets, mContext->mDepthStencilView);
	mContext->mImmediateContext->OMSetRenderTargets(1, &mContext->mRenderTargetView, mContext->mDepthStencilView);
}

void DXRenderer::BindRenderTargets(sResourceID* aRenderTargetIDs, int aCount)
{
	if (aCount <= 0) return;

	for (int i = 0; i < aCount; i++)
	{
		sResourceID aRenderTargetID = aRenderTargetIDs[i];
		assert(aRenderTargetID.mResourceId != RESOURCE_UNDEFINED_ID);
		assert(aRenderTargetID.mResourceId < mResources->mRenderTargets.size());

		// Obtain render target
		sRenderTarget data = mResources->mRenderTargets[aRenderTargetID.mResourceId];
		mResources->mRenderTargetViewTempBuffer[i] = data.mRenderTargetView;
	}

	// Set shader resources to null
	ID3D11ShaderResourceView* const pSRV[1] = { NULL };
	mContext->mImmediateContext->PSSetShaderResources(0, 1, pSRV);

	mContext->mRenderTargetView = mResources->mRenderTargetViewTempBuffer[0];
	mContext->mDepthStencilView = mResources->mRenderTargets[aRenderTargetIDs[0].mResourceId].mDepthStencilView;

	// Bind render targets
	mContext->mImmediateContext->OMSetRenderTargets(aCount, mResources->mRenderTargetViewTempBuffer, mContext->mDepthStencilView);
}

void DXRenderer::SetRasterizerState(sResourceID aRasterizer)
{
	assert(aRasterizer.mResourceId != RESOURCE_UNDEFINED_ID);
	assert(aRasterizer.mResourceId < mResources->mRasterizerStates.size());

	mContext->mImmediateContext->RSSetState(mResources->mRasterizerStates[aRasterizer.mResourceId]);
}

void DXRenderer::SetBlendState(sResourceID aBlendState)
{
	assert(aBlendState.mResourceId != RESOURCE_UNDEFINED_ID);
	assert(aBlendState.mResourceId < mResources->mBlendStates.size());

	mContext->mImmediateContext->OMSetBlendState(mResources->mBlendStates[aBlendState.mResourceId], NULL, 0xFFFFFFFF);
}

sResourceID DXRenderer::GetTextureFromRenderTarget(sResourceID aRenderTargetID)
{
	assert(aRenderTargetID.mResourceId != RESOURCE_UNDEFINED_ID);
	assert(aRenderTargetID.mResourceId < mResources->mRenderTargets.size());

	sRenderTarget data = mResources->mRenderTargets[aRenderTargetID.mResourceId];

	assert(data.mTextureID.mResourceId != RESOURCE_UNDEFINED_ID);
	assert(data.mTextureID.mResourceId < mResources->mTextures.size());

	return data.mTextureID;
}

int DXRenderer::ObtainShaderBindingSlot(eShaderType aShaderType, sResourceID aShaderID, char* aBindingName)
{
	assert(aShaderID.mResourceId != RESOURCE_UNDEFINED_ID);
	assert(aBindingName != 0);

	// Get shader data blob
	ID3DBlob* blob = nullptr;
	if (aShaderType == eShaderType::VERTEX_SHADER)
	{
		assert(aShaderID.mResourceId < mResources->mVertexShaders.size());
		blob = mResources->mVertexShaders[aShaderID.mResourceId].mData;
	}

	if (aShaderType == eShaderType::GEOMETRY_SHADER)
	{
		// TODO
		assert(false);
	}

	if (aShaderType == eShaderType::PIXEL_SHADER)
	{
		assert(aShaderID.mResourceId < mResources->mPixelShaders.size());
		blob = mResources->mPixelShaders[aShaderID.mResourceId].mData;
	}

	assert(blob != nullptr);

	// Get slot
	ID3D11ShaderReflection* reflectionInterface = nullptr;
	D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflectionInterface);

	D3D11_SHADER_INPUT_BIND_DESC bindDesc;
	HRESULT hr = reflectionInterface->GetResourceBindingDescByName(aBindingName, &bindDesc);

	if (FAILED(hr))
	{
		return -1;
	}

	return (int)(bindDesc.BindPoint);
}

char* DXRenderer::GetShaderExtension()
{
	return "hlsl";
}

char* DXRenderer::GetShaderPreFlags()
{
	return "";
}

char* DXRenderer::GetRendererName()
{
	return "DirectX";
}

D3D11_DEPTH_STENCIL_DESC BuildStencilState(bool mEnabled)
{
	D3D11_DEPTH_STENCIL_DESC dsDesc;

	dsDesc.DepthEnable = mEnabled;
	dsDesc.DepthWriteMask = mEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = mEnabled ? D3D11_COMPARISON_LESS : D3D11_COMPARISON_ALWAYS;

	// Stencil test parameters
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	return dsDesc;
}

void DXRenderer::SetDepthTestEnabled(bool mEnabled)
{
	if (mEnabled)
		mContext->mImmediateContext->OMSetDepthStencilState(mContext->mDepthStencilStateEnabled, 0);
	else
		mContext->mImmediateContext->OMSetDepthStencilState(mContext->mDepthStencilStateDisabled, 0);
}

void DXRenderer::ClearColorBuffer(Vec4 aColor)
{
	assert(mContext != nullptr);
	assert(mContext->mImmediateContext != nullptr);

	FLOAT data[4] = { aColor.getX(), aColor.getY(), aColor.getZ(), aColor.getW() };
	mContext->mImmediateContext->ClearRenderTargetView(mContext->mRenderTargetView, data);
}

void DXRenderer::ClearDepthBuffer(float aDepth)
{
	assert(mContext != nullptr);
	assert(mContext->mImmediateContext != nullptr);

	mContext->mImmediateContext->ClearDepthStencilView(mContext->mDepthStencilView, D3D11_CLEAR_DEPTH, aDepth, 0);
}

void DXRenderer::Draw(int aVertexCount, int aStartVertexLocation)
{
	assert(mContext != nullptr);
	assert(mContext->mImmediateContext != nullptr);
	assert(aVertexCount > 0);

	mContext->mImmediateContext->Draw(aVertexCount, aStartVertexLocation);
}

void DXRenderer::DrawIndexed(int aStart, int aCount)
{
	assert(mContext != nullptr);
	assert(mContext->mImmediateContext != nullptr);
	assert(aCount > 0);

	mContext->mImmediateContext->DrawIndexed(aCount, aStart, 0);
}

void DXRenderer::Swap()
{
	assert(mContext != nullptr);
	assert(mContext->mSwapChain != nullptr);

	mContext->mSwapChain->Present(0, 0);
}

void DXRenderer::UpdateFullscreen()
{
	if (mContext->mSwapChain)
	{
		mContext->mSwapChain->SetFullscreenState(mContext->mWindow->GetMode() == eWindowMode::FULLSCREEN, NULL);
	}
}

void DXRenderer::OnWindowResize(int aWidth, int aHeight)
{
	if (mContext->mSwapChain)
	{
		mContext->mImmediateContext->OMSetRenderTargets(0, 0, 0);

		mContext->mSwapChain->SetFullscreenState(mContext->mWindow->GetMode() == eWindowMode::FULLSCREEN, NULL);

		if (mContext->mBackRenderTargetView) mContext->mBackRenderTargetView->Release();
		if (mContext->mDepthStencil) mContext->mDepthStencil->Release();
		if (mContext->mBackDepthStencilView) mContext->mBackDepthStencilView->Release();

		HRESULT hr;
		hr = mContext->mSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

		// Resize back buffer
		ID3D11Texture2D* pBuffer;
		hr = mContext->mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
			(void**)&pBuffer);

		hr = mContext->mPD3DDevice->CreateRenderTargetView(pBuffer, NULL,
			&mContext->mBackRenderTargetView);
		pBuffer->Release();

		mContext->mImmediateContext->OMSetRenderTargets(1, &mContext->mBackRenderTargetView, NULL);

		// Resize depth buffer
		// Create depth stencil texture
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = aWidth;
		descDepth.Height = aHeight;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		hr = mContext->mPD3DDevice->CreateTexture2D(&descDepth, nullptr, &mContext->mDepthStencil);
		assert(SUCCEEDED(hr));

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		hr = mContext->mPD3DDevice->CreateDepthStencilView(mContext->mDepthStencil, &descDSV, &mContext->mBackDepthStencilView);
		assert(SUCCEEDED(hr));

		// Set up the viewport
		D3D11_VIEWPORT vp;
		vp.Width = (float)(aWidth);
		vp.Height = (float)(aHeight);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		mContext->mImmediateContext->RSSetViewports(1, &vp);

		mResources->mRenderTargets[0].mRenderTargetView = mContext->mBackRenderTargetView;
		mResources->mRenderTargets[0].mDepthStencilView = mContext->mBackDepthStencilView;
		BindRenderTarget();

		// Resize render targets
		int i = 0;
		for (sRenderTarget renderTarget : mResources->mRenderTargets)
		{
			if (renderTarget.mScaleToWindow)
			{
				sDXTexture texture = mResources->mTextures[renderTarget.mTextureID.mResourceId];
				int newWidth = (int)(((float)(renderTarget.mOriginalTextureWidth) / (float)(renderTarget.mOriginalWindowWidth)) * (float)(aWidth));
				int newHeight = (int)(((float)(renderTarget.mOriginalTextureHeight) / (float)(renderTarget.mOriginalWindowHeight)) * (float)(aHeight));

				if (newWidth != texture.mWidth || newHeight != texture.mHeight)
				{
					UpdateTexture(renderTarget.mTextureID, 0, newWidth, newHeight);
					texture = mResources->mTextures[renderTarget.mTextureID.mResourceId];

					renderTarget.mRenderTargetView->Release();
					renderTarget.mDepthStencilView->Release();

					// Recreate render target with new texture
					D3D11_RENDER_TARGET_VIEW_DESC descRenderTarget;
					ZeroMemory(&descRenderTarget, sizeof(descRenderTarget));
					descRenderTarget.Format = dxTextureFormats.t[(int)(texture.mFormat)];
					descRenderTarget.ViewDimension = dxDimensions.rtv[(int)(texture.mDimension)];
					descRenderTarget.Texture2D.MipSlice = 0;

					HRESULT hr = mContext->mPD3DDevice->CreateRenderTargetView(texture.mResource, &descRenderTarget, &renderTarget.mRenderTargetView);
					assert(SUCCEEDED(hr));

					// Create depth stencil texture
					{
						ID3D11Texture2D* depthTexture = nullptr;

						D3D11_TEXTURE2D_DESC descDepth;
						ZeroMemory(&descDepth, sizeof(descDepth));
						descDepth.Width = newWidth;
						descDepth.Height = newHeight;
						descDepth.MipLevels = 1;
						descDepth.ArraySize = 1;
						descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
						descDepth.SampleDesc.Count = 1;
						descDepth.SampleDesc.Quality = 0;
						descDepth.Usage = D3D11_USAGE_DEFAULT;
						descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
						descDepth.CPUAccessFlags = 0;
						descDepth.MiscFlags = 0;
						HRESULT hr = mContext->mPD3DDevice->CreateTexture2D(&descDepth, nullptr, &depthTexture);
						assert(SUCCEEDED(hr));

						// Create the depth stencil view
						D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
						ZeroMemory(&descDSV, sizeof(descDSV));
						descDSV.Format = descDepth.Format;
						descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
						descDSV.Texture2D.MipSlice = 0;
						hr = mContext->mPD3DDevice->CreateDepthStencilView(depthTexture, &descDSV, &renderTarget.mDepthStencilView);
						depthTexture->Release();
						assert(SUCCEEDED(hr));
					}
				}

				mResources->mRenderTargets[i].mRenderTargetView = renderTarget.mRenderTargetView;
				mResources->mRenderTargets[i].mDepthStencilView = renderTarget.mDepthStencilView;
			}
			i++;
		}
	}
}

int DXRenderer::BuildContext(DXRenderWindow* aWindow, sDXContext* aContext)
{
	assert(aWindow != nullptr);
	assert(aContext != nullptr);

	HRESULT hr = S_OK;

	HWND hwnd = *((HWND*)(aWindow->GetHwnd()));
	mContext->mHwnd = hwnd;

	UINT width = aWindow->GetWidth();
	UINT height = aWindow->GetHeight();

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		mContext->mDriverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, mContext->mDriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &mContext->mPD3DDevice, &mContext->mFeatureLevel, &mContext->mImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, mContext->mDriverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &mContext->mPD3DDevice, &mContext->mFeatureLevel, &mContext->mImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	assert(SUCCEEDED(hr));

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = mContext->mPD3DDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}

	assert(SUCCEEDED(hr));

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = mContext->mPD3DDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&mContext->mPD3DDevice1));
		if (SUCCEEDED(hr))
		{
			(void)mContext->mImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&mContext->mImmediateContext1));
		}
		assert(SUCCEEDED(hr));

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		hr = dxgiFactory2->CreateSwapChainForHwnd(mContext->mPD3DDevice, mContext->mHwnd, &sd, nullptr, nullptr, &mContext->mSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = mContext->mSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&mContext->mSwapChain));
		}
		assert(SUCCEEDED(hr));

		dxgiFactory2->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = mContext->mHwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		hr = dxgiFactory->CreateSwapChain(mContext->mPD3DDevice, &sd, &mContext->mSwapChain);
	}

	dxgiFactory->Release();

	assert(SUCCEEDED(hr));

	// Add backbuffer to resources at index 0
	sRenderTarget backBuffer;
	backBuffer.mScaleToWindow = false;
	mResources->mRenderTargets.insert(mResources->mRenderTargets.begin(), backBuffer);

	// Create the sample state
	ID3D11SamplerState* sampler;
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = mContext->mPD3DDevice->CreateSamplerState(&sampDesc, &sampler);
	assert(SUCCEEDED(hr));

	mContext->mImmediateContext->PSSetSamplers(0, 1, &sampler);

	// Create the depth states
	D3D11_DEPTH_STENCIL_DESC descEnabled = BuildStencilState(true);
	D3D11_DEPTH_STENCIL_DESC descDisabled = BuildStencilState(false);

	mContext->mPD3DDevice->CreateDepthStencilState(&descEnabled, &mContext->mDepthStencilStateEnabled);
	mContext->mPD3DDevice->CreateDepthStencilState(&descDisabled, &mContext->mDepthStencilStateDisabled);

	SetDepthTestEnabled(true);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	mContext->mImmediateContext->RSSetViewports(1, &vp);

	OnWindowResize(width, height);

	return S_OK;
}
