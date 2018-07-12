#include "OGLRenderer.h"

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glfw3.lib")
#pragma comment (lib, "glew32.lib")

#include "OGLRenderWindow.h"

#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GL_GLEXT_PROTOTYPES
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

GLenum Error;

#ifdef _DEBUG
#define ERROR_CHECK() if ((Error = glGetError()) != GL_NO_ERROR) assert(false);
#else
#define ERROR_CHECK() ((void)0)
#endif

struct sOGLContext
{
public:
    OGLRenderWindow* mWindow;
    GLFWwindow* mGLFWWindow;

    GLuint mDefaultPipeline = 0;
};

struct sInputLayout
{
    int mSize;
};

struct sRenderTarget
{
    sResourceID mTextureID;
    int mFrameBuffer;
    int mDepthBuffer;

    bool mScaleToWindow;
    int mOriginalTextureWidth;
    int mOriginalTextureHeight;
    int mOriginalWindowWidth;
    int mOriginalWindowHeight;
};

struct sRastirizerState
{
    eFillMode mFillMode;
    eCullMode mCullMode;
};

struct sOGLTexture
{
    int mTexture;

    int mWidth;
    int mHeight;
    eTextureDimension mDimension;
    eTextureFormat mFormat;
    bool mRenderTargetBindable;
};

int textureActives[] =
{
    GL_TEXTURE0,
    GL_TEXTURE1,
    GL_TEXTURE2,
    GL_TEXTURE3,
    GL_TEXTURE4,
    GL_TEXTURE5,
    GL_TEXTURE6,
    GL_TEXTURE7,
    GL_TEXTURE8,
};

int colorAttachments[] = 
{
    GL_COLOR_ATTACHMENT0_EXT,
    GL_COLOR_ATTACHMENT1_EXT,
    GL_COLOR_ATTACHMENT2_EXT,
    GL_COLOR_ATTACHMENT3_EXT,
    GL_COLOR_ATTACHMENT4_EXT,
    GL_COLOR_ATTACHMENT5_EXT,
};

struct OGLTextureFormat
{
    int t[TEXTUREFORMAT_COUNT];

    OGLTextureFormat()
    {
        t[TEXTUREFORMAT_R8] = GL_R8; // OGLGI_FORMAT_R8_UNORM; 
        t[TEXTUREFORMAT_R8G8] = GL_RG8; // OGLGI_FORMAT_R8G8_UNORM;
        t[TEXTUREFORMAT_R8G8B8A8] = GL_RGBA8; // OGLGI_FORMAT_R8G8B8A8_UNORM;
        t[TEXTUREFORMAT_R32] = GL_R32F; // OGLGI_FORMAT_R32_FLOAT;
        t[TEXTUREFORMAT_R32G32] = GL_RG32F; // OGLGI_FORMAT_R32G32_FLOAT;
        t[TEXTUREFORMAT_R32G32B32] = GL_RGB32F; // OGLGI_FORMAT_R32G32B32_FLOAT;
        t[TEXTUREFORMAT_R32G32B32A32] = GL_RGBA32F; // OGLGI_FORMAT_R32G32B32A32_FLOAT;
    }
};
OGLTextureFormat oglTextureFormats;

class OGLResources
{
public:
    std::vector<int> mVertexShaders;
    std::vector<int> mPixelShaders;

    std::vector<int> mVertexBuffers;
    std::vector<int> mIndexBuffers;
    std::vector<int> mConstantBuffers;

    std::vector<sRastirizerState> mRastirizerStates;
    std::vector<sRenderTargetBlendDesc> mBlendStates;
    std::vector<sOGLTexture> mTextures;

    std::vector<sRenderTarget> mRenderTargets;
};

OGLRenderer::OGLRenderer()
{
    assert(glfwInit());
}

OGLRenderer::~OGLRenderer()
{
    glfwTerminate();
}

iRenderWindow* OGLRenderer::CreateRenderWindow(int aWidth, int aHeight, char* aTitle, eWindowMode aMode)
{
    assert(aWidth > 0);
    assert(aHeight > 0);

    OGLRenderWindow* window = new OGLRenderWindow(aWidth, aHeight, aTitle, aMode);
    window->mRenderer = this;

    mResources = new OGLResources();
    mContext = new sOGLContext();

    assert(BuildContext(window, mContext));

    return window;
}

sResourceID OGLRenderer::LoadTexture(int aWidth, int aHeight, char* aRawData, eTextureDimension dimension, eTextureFormat format, bool aRenderTargetBindable)
{
    sResourceID res = sResourceID();
    res.mResourceId = RESOURCE_UNDEFINED_ID;

    ERROR_CHECK();

    sOGLTexture texture;

    GLuint newTextureID;
    glGenTextures(1, &newTextureID);

    glBindTexture(GL_TEXTURE_2D, newTextureID);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    ERROR_CHECK();

    texture.mTexture = newTextureID;
    texture.mWidth = aWidth;
    texture.mHeight = aHeight;
    texture.mFormat = format;
    texture.mDimension = dimension;
    texture.mRenderTargetBindable = aRenderTargetBindable;

    // Add to resources
    int id = (int)(mResources->mTextures.size());
    res.mResourceId = id;
    mResources->mTextures.push_back(texture);

    UpdateTexture(res, aRawData, aWidth, aHeight);

    return res;
}

sResourceID OGLRenderer::CompileVertexShader(char* aShaderDataString)
{
    sResourceID res = sResourceID();
    res.mResourceId = RESOURCE_UNDEFINED_ID;

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Create the shaders
    GLuint vertexProgramID = glCreateProgram();
    glProgramParameteri(vertexProgramID, GL_PROGRAM_SEPARABLE, GL_TRUE);

    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);

    // Compile Vertex Shader
    GLint vertexLength = (GLint)(strlen(aShaderDataString));
    char* vertexSourcePointer = aShaderDataString;
    glShaderSource(vertexShaderID, 1, &vertexSourcePointer, &vertexLength);
    glCompileShader(vertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (Result == GL_FALSE)
    {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(vertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
        assert(false);
    }
    else
    {
        glAttachShader(vertexProgramID, vertexShaderID);
        glLinkProgram(vertexProgramID);
        glDetachShader(vertexProgramID, vertexShaderID);
        glDeleteShader(vertexShaderID);

        GLint isLinked = 0;
        glGetProgramiv(vertexProgramID, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(vertexProgramID, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(vertexProgramID, maxLength, &maxLength, &infoLog[0]);
            std::cout << infoLog.data() << std::endl;
            assert(false);
        }

        glUseProgramStages(mContext->mDefaultPipeline, GL_VERTEX_SHADER_BIT, vertexProgramID);

        // Add to resources
        int id = (int)(mResources->mVertexShaders.size());
        res.mResourceId = id;
        mResources->mVertexShaders.push_back((int)(vertexProgramID));
    }

    ERROR_CHECK();

    return res;
}

sResourceID OGLRenderer::CompilePixelShader(char* aShaderDataString)
{
    sResourceID res = sResourceID();
    res.mResourceId = RESOURCE_UNDEFINED_ID;

    // Create the shaders
    GLuint pixelProgramID = glCreateProgram();
    glProgramParameteri(pixelProgramID, GL_PROGRAM_SEPARABLE, GL_TRUE);

    GLuint pixelShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Compile Fragment Shader
    GLint fragmentLength = (GLint)(strlen(aShaderDataString));
    char* fragmentSourcePointer = aShaderDataString;

    glShaderSource(pixelShaderID, 1, &fragmentSourcePointer, &fragmentLength);
    glCompileShader(pixelShaderID);

    // Check Fragment Shader
    GLint isCompiled = GL_FALSE;
    int InfoLogLength;

    glGetShaderiv(pixelShaderID, GL_COMPILE_STATUS, &isCompiled);
    glGetShaderiv(pixelShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (isCompiled == GL_FALSE)
    {
        std::vector<char> fragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(pixelShaderID, InfoLogLength, NULL, &fragmentShaderErrorMessage[0]);
        printf("%s\n", &fragmentShaderErrorMessage[0]);
        assert(false);
    }
    else
    {
        glAttachShader(pixelProgramID, pixelShaderID);
        glLinkProgram(pixelProgramID);
        glDetachShader(pixelProgramID, pixelShaderID);
        glDeleteShader(pixelShaderID);

        GLint isLinked = 0;
        glGetProgramiv(pixelProgramID, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(pixelProgramID, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(pixelProgramID, maxLength, &maxLength, &infoLog[0]);
            std::cout << infoLog.data() << std::endl;
            assert(false);
        }

        glUseProgramStages(mContext->mDefaultPipeline, GL_FRAGMENT_SHADER_BIT, pixelProgramID);

        // Add to resources
        int id = (int)(mResources->mPixelShaders.size());
        res.mResourceId = id;
        mResources->mPixelShaders.push_back((int)(pixelProgramID));
    }

    ERROR_CHECK();

    return res;
}

// TODO
sResourceID OGLRenderer::CompileGeometryShader(char* aShaderDataString)
{
    sResourceID res = sResourceID();
    res.mResourceId = RESOURCE_UNDEFINED_ID;

    assert(false);

    return res;
}

sResourceID OGLRenderer::CreateVertexBuffer(eBufferType aType, void* aData, int aStride, int aCount)
{
    sResourceID res = sResourceID();
    res.mResourceId = RESOURCE_UNDEFINED_ID;

    GLuint newbuffer;

    ERROR_CHECK();

    glGenBuffers(1, &newbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, newbuffer);
    glBufferData(GL_ARRAY_BUFFER, aCount* sizeof(sVertex), aData, GL_STATIC_DRAW);

    ERROR_CHECK();

    // Add to resources
    int id = (int)(mResources->mVertexBuffers.size());
    res.mResourceId = id;
    mResources->mVertexBuffers.push_back((int)(newbuffer));

    return res;
}

sResourceID OGLRenderer::CreateIndexBuffer(eBufferType aType, int* aData, int aCount)
{
    sResourceID res = sResourceID();
    res.mResourceId = RESOURCE_UNDEFINED_ID;

    ERROR_CHECK();

    GLuint newbuffer;

    glGenBuffers(1, &newbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, aCount * sizeof(int), aData, GL_STATIC_DRAW);

    // Add to resources
    int id = (int)(mResources->mIndexBuffers.size());
    res.mResourceId = id;
    mResources->mIndexBuffers.push_back((int)(newbuffer));

    ERROR_CHECK();

    return res;
}

sResourceID OGLRenderer::CreateConstantBuffer(eBufferType aType, void* aData, int aStructSize, int aCount)
{
    ERROR_CHECK();

    sResourceID res = sResourceID();
    res.mResourceId = RESOURCE_UNDEFINED_ID;

    GLuint newbuffer;
    glGenBuffers(1, &newbuffer);

    // Add to resources
    int id = (int)(mResources->mConstantBuffers.size());
    res.mResourceId = id;
    mResources->mConstantBuffers.push_back(newbuffer);

    UpdateConstantBuffer(res, aData, aStructSize, aCount);

    ERROR_CHECK();

    return res;
}

// TODO
sResourceID OGLRenderer::CreateRenderTarget(int aWidth, int aHeight, eTextureDimension dimension, eTextureFormat format, bool aScaleToWindow)
{
    sResourceID res = sResourceID();
    res.mResourceId = RESOURCE_UNDEFINED_ID;

    sRenderTarget renderTarget;

    renderTarget.mScaleToWindow = aScaleToWindow;
    renderTarget.mOriginalTextureWidth = aWidth;
    renderTarget.mOriginalTextureHeight = aHeight;
    renderTarget.mOriginalWindowWidth = mContext->mWindow->GetWidth();
    renderTarget.mOriginalWindowHeight = mContext->mWindow->GetHeight();

    ERROR_CHECK();

    // Create FrameBuffer
    GLuint newFrameBuffer = 0;
    glGenFramebuffersEXT(1, &newFrameBuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, newFrameBuffer);

    renderTarget.mFrameBuffer = newFrameBuffer;

    ERROR_CHECK();

    // Create Texture
    sResourceID textureID = LoadTexture(aWidth, aHeight, 0, dimension, format, true);
    int texture = mResources->mTextures[textureID.mResourceId].mTexture;

    renderTarget.mTextureID = textureID;

    ERROR_CHECK();

    // Overwrite texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    ERROR_CHECK();

    GLuint depthBuffer;
    glGenRenderbuffersEXT(1, &depthBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, aWidth, aHeight);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthBuffer);

    renderTarget.mDepthBuffer = depthBuffer;

    ERROR_CHECK();

    // Add to resources
    int id = (int)(mResources->mRenderTargets.size());
    res.mResourceId = id;
    mResources->mRenderTargets.push_back(renderTarget);

    return res;
}

// TODO
sResourceID OGLRenderer::CreateRasterizerState(eFillMode aFillMode, eCullMode aCullMode)
{
    sResourceID res;

    sRastirizerState rastirizer;
    rastirizer.mFillMode = aFillMode;
    rastirizer.mCullMode = aCullMode;

    // Add to resources
    int id = (int)(mResources->mRastirizerStates.size());
    res.mResourceId = id;
    mResources->mRastirizerStates.push_back(rastirizer);

    return res;
}

// TODO
sResourceID OGLRenderer::CreateBlendState(sRenderTargetBlendDesc aBlendDesc)
{
    sResourceID res;

    // Add to resources
    int id = (int)(mResources->mBlendStates.size());
    res.mResourceId = id;
    mResources->mBlendStates.push_back(aBlendDesc);

    return res;
}

void OGLRenderer::UpdateConstantBuffer(sResourceID aConstantBufferID, void* aData, int aStructSize, int aCount)
{
    ERROR_CHECK();
    int allocateBytes = aCount * aStructSize;
    if (allocateBytes % 16 != 0)
    {
        allocateBytes = allocateBytes + (16 - (allocateBytes % 16));
    }

    int data = mResources->mConstantBuffers[aConstantBufferID.mResourceId];
    glBindBuffer(GL_UNIFORM_BUFFER, data);
    glBufferData(GL_UNIFORM_BUFFER, allocateBytes, aData, GL_DYNAMIC_DRAW);

    ERROR_CHECK();
}

void OGLRenderer::UpdateTexture(sResourceID aTexture, char* aRawData, int aWidth, int aHeight)
{
    ERROR_CHECK();

    sOGLTexture texture = mResources->mTextures[aTexture.mResourceId];
    glBindTexture(GL_TEXTURE_2D, texture.mTexture);

    ERROR_CHECK();

    int newWidth = texture.mWidth;
    int newHeight = texture.mHeight;

    if (aWidth != -1 && aHeight != -1)
    {
        newWidth = aWidth;
        newHeight = aHeight;
    }

    if (newWidth == 0 || newHeight == 0) return;

    eTextureFormat format = texture.mFormat;
    texture.mWidth = newWidth;
    texture.mHeight = newHeight;

    ERROR_CHECK();

    if (format == TEXTUREFORMAT_R32G32B32A32)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, newWidth, newHeight, 0, GL_RGBA, GL_FLOAT, aRawData);
    }
    else if (format == TEXTUREFORMAT_R32)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, newWidth, newHeight, 0, GL_RED, GL_FLOAT, aRawData);
    }
    else if (format == TEXTUREFORMAT_R8G8B8A8)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, 4, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, aRawData);
    }
    else if (format == TEXTUREFORMAT_R8G8B8A8SNORM)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_SNORM, newWidth, newHeight, 0, GL_RGBA, GL_FLOAT, aRawData);
    }
    else
    {
        assert(false);
    }

    ERROR_CHECK();
}

void OGLRenderer::BindTexture(sResourceID aTextureID, int aTextureSlot)
{
    ERROR_CHECK();
    if (aTextureID.mResourceId == RESOURCE_UNDEFINED_ID)
    {
        // Unbind
        if (aTextureSlot != -1)
        {
            ERROR_CHECK();
            glUniform1i(aTextureSlot, aTextureSlot);
            glGetError(); // Ignore error
            ERROR_CHECK();
            glActiveTexture(textureActives[aTextureSlot]);
            glBindTexture(GL_TEXTURE_2D, 0);
            ERROR_CHECK();
        }
        return;
    }
    assert(aTextureID.mResourceId < mResources->mTextures.size());

    if (aTextureSlot == -1)
    {
        return;
    }

    if (aTextureID.mResourceId < mResources->mTextures.size())
    {
        ERROR_CHECK();
        glUniform1i(aTextureSlot, aTextureSlot);
        glGetError(); // Ignore error
        ERROR_CHECK();
        glActiveTexture(textureActives[aTextureSlot]);
        int data = mResources->mTextures[aTextureID.mResourceId].mTexture;
        glBindTexture(GL_TEXTURE_2D, data);
        ERROR_CHECK();
    }
    ERROR_CHECK();
}

void OGLRenderer::BindVertexShader(sResourceID aVertexShaderID)
{
    ERROR_CHECK();
    if (aVertexShaderID.mResourceId < mResources->mVertexShaders.size())
    {
        int data = mResources->mVertexShaders[aVertexShaderID.mResourceId];
        glUseProgramStages(mContext->mDefaultPipeline, GL_VERTEX_SHADER_BIT, data);
        glUseProgram(data);
    }
    else
    {
        assert(false);
    }
    ERROR_CHECK();
}

void OGLRenderer::BindPixelShader(sResourceID aPixelShaderID)
{
    ERROR_CHECK();
    if (aPixelShaderID.mResourceId < mResources->mPixelShaders.size())
    {
        int data = mResources->mPixelShaders[aPixelShaderID.mResourceId];
        glUseProgramStages(mContext->mDefaultPipeline, GL_FRAGMENT_SHADER_BIT, data);
        glUseProgram(data);

        glBindFragDataLocation(data, 0, "PS_Normal");
        glBindFragDataLocation(data, 1, "PS_DiffuseAlbedo");
        glBindFragDataLocation(data, 2, "PS_Position");
    }
    else
    {
        assert(false);
    }
    ERROR_CHECK();
}

// TODO
void OGLRenderer::BindGeometryShader(sResourceID aGeometryShaderID)
{
}

// TODO: NOT HARDCODED
void OGLRenderer::BindVertexBuffer(sResourceID aVertexBufferID)
{
    int data = mResources->mVertexBuffers[aVertexBufferID.mResourceId];
    ERROR_CHECK();
    glBindBuffer(GL_ARRAY_BUFFER, data);
    ERROR_CHECK();

    // Layout
    // vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(sVertex), (void*)(0));
    // tex coords
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(sVertex), (void*)(3 * sizeof(float)));
    // normal
    glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(sVertex), (void*)(5 * sizeof(float)));
    // tangent
    glVertexAttribPointer(3, 3, GL_FLOAT, false, sizeof(sVertex), (void*)(8 * sizeof(float)));
    // bitangent
    glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(sVertex), (void*)(11 * sizeof(float)));
    ERROR_CHECK();
}

void OGLRenderer::BindIndexBuffer(sResourceID aIndexBufferID)
{
    int data = mResources->mIndexBuffers[aIndexBufferID.mResourceId];
    ERROR_CHECK();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data);
    ERROR_CHECK();
}

void OGLRenderer::BindConstantBuffer(eShaderType aShaderType, sResourceID aShaderID, sResourceID aConstantBufferID, int aBindSlot)
{
    if (aConstantBufferID.mResourceId == -1) return;

    ERROR_CHECK();

    int offset = 0;
    int shaderID = -1;
    if (aShaderType == eShaderType::VERTEX_SHADER)
    {
        BindVertexShader(aShaderID);
        shaderID = mResources->mVertexShaders[aShaderID.mResourceId];


        // WORKAROUND
        offset += 8;
    }

    if (aShaderType == eShaderType::PIXEL_SHADER)
    {
        BindPixelShader(aShaderID);
        shaderID = mResources->mPixelShaders[aShaderID.mResourceId];

    }

    ERROR_CHECK();
    int data = mResources->mConstantBuffers[aConstantBufferID.mResourceId];
    glBindBuffer(GL_UNIFORM_BUFFER, data);
    glUniformBlockBinding(shaderID, aBindSlot, aBindSlot + offset);
    glBindBufferBase(GL_UNIFORM_BUFFER, aBindSlot + offset, data);
    ERROR_CHECK();
}

// TODO
void OGLRenderer::BindRenderTarget(sResourceID aRenderTargetID)
{
    ERROR_CHECK();
    
    if (aRenderTargetID.mResourceId == 0)
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        ERROR_CHECK();
		
        return;
    }

    sRenderTarget renderTarget = mResources->mRenderTargets[aRenderTargetID.mResourceId];
    int texture = mResources->mTextures[renderTarget.mTextureID.mResourceId].mTexture;

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderTarget.mFrameBuffer);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture, 0);

    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, DrawBuffers);

    ERROR_CHECK();
}

GLenum DrawBuffersBuffer[128];
// TODO
void OGLRenderer::BindRenderTargets(sResourceID* aRenderTargetIDs, int aCount)
{
    ERROR_CHECK();

    for (int i = 0; i < aCount; i++)
    {
        ERROR_CHECK();
        sResourceID aRenderTargetID = aRenderTargetIDs[i];
        sRenderTarget renderTarget = mResources->mRenderTargets[aRenderTargetID.mResourceId];
        sOGLTexture texture = mResources->mTextures[renderTarget.mTextureID.mResourceId];
        if (i == 0)
        {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderTarget.mFrameBuffer); // todo
        }

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, colorAttachments[i], GL_TEXTURE_2D, texture.mTexture, 0);
        DrawBuffersBuffer[i] = colorAttachments[i];
        ERROR_CHECK();
    }

    ERROR_CHECK();
    glDrawBuffers(aCount, DrawBuffersBuffer);
    ERROR_CHECK();
}

// TODO
void OGLRenderer::SetRasterizerState(sResourceID aRasterizer)
{
    ERROR_CHECK();

    sRastirizerState rastirizer = mResources->mRastirizerStates[aRasterizer.mResourceId];

    if (rastirizer.mCullMode == eCullMode::CULLMODE_BACK)
    {
        glCullFace(GL_BACK);
    }
    else if (rastirizer.mCullMode == eCullMode::CULLMODE_FRONT)
    {
        glCullFace(GL_FRONT);
    }
}

// TODO
void OGLRenderer::SetBlendState(sResourceID aBlendState)
{
    ERROR_CHECK();

    sRenderTargetBlendDesc blend = mResources->mBlendStates[aBlendState.mResourceId];
    if (blend.mBlendEnabled)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    ERROR_CHECK();
}

sResourceID OGLRenderer::GetTextureFromRenderTarget(sResourceID aRenderTargetID)
{
    sResourceID res = sResourceID();
    res.mResourceId = RESOURCE_UNDEFINED_ID;

    sRenderTarget renderTarget = mResources->mRenderTargets[aRenderTargetID.mResourceId];

    return renderTarget.mTextureID;
}

int OGLRenderer::ObtainShaderBindingSlot(eShaderType aShaderType, sResourceID aShaderID, char* aBindingName)
{
    ERROR_CHECK();
    int shaderID = -1;
    if (aShaderType == eShaderType::VERTEX_SHADER)
    {
        assert(aShaderID.mResourceId < mResources->mVertexShaders.size());
        shaderID = mResources->mVertexShaders[aShaderID.mResourceId];
        BindVertexShader(aShaderID);
    }

    if (aShaderType == eShaderType::GEOMETRY_SHADER)
    {
        // TODO
        assert(false);
    }

    if (aShaderType == eShaderType::PIXEL_SHADER)
    {
        assert(aShaderID.mResourceId < mResources->mPixelShaders.size());
        shaderID = mResources->mPixelShaders[aShaderID.mResourceId];
        BindPixelShader(aShaderID);
    }

    int uniformBlockPosition = glGetUniformBlockIndex(shaderID, aBindingName);
    if (uniformBlockPosition != -1)
    {
        ERROR_CHECK();
        return uniformBlockPosition;
    }

    int uniformPosition = (int)(glGetUniformLocation(shaderID, aBindingName));
    if (uniformPosition != -1)
    {
        ERROR_CHECK();
        return uniformPosition;
    }
    ERROR_CHECK();
    return uniformPosition;
}

char* OGLRenderer::GetShaderExtension()
{
    return "glsl";
}

char* OGLRenderer::GetShaderPreFlags()
{
    return "#version 410 core\n";
}

char* OGLRenderer::GetRendererName()
{
    return "OpenGL";
}

void OGLRenderer::SetDepthTestEnabled(bool mEnabled)
{
    ERROR_CHECK();
    if (mEnabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    ERROR_CHECK();
}

void OGLRenderer::ClearColorBuffer(Vec4 aColor)
{
    ERROR_CHECK();
    glClearColor(aColor.getX(), aColor.getY(), aColor.getZ(), aColor.getW());
    glClear(GL_COLOR_BUFFER_BIT);
    ERROR_CHECK();
}

void OGLRenderer::ClearDepthBuffer(float aDepth)
{
    ERROR_CHECK();
    glClearDepth(aDepth);
    glClear(GL_DEPTH_BUFFER_BIT);
    ERROR_CHECK();
}

void OGLRenderer::Draw(int aVertexCount, int aStartVertexLocation)
{
    ERROR_CHECK();
    glUseProgram(0);

    ERROR_CHECK();
    glDrawArrays(GL_TRIANGLES, aStartVertexLocation, aVertexCount);
    ERROR_CHECK();
}

void OGLRenderer::DrawIndexed(int aStart, int aCount)
{
    ERROR_CHECK();
    glUseProgram(0);

    ERROR_CHECK();
    glDrawElements(GL_TRIANGLES, aCount, GL_UNSIGNED_INT, (void*)(aStart * sizeof(unsigned int)));
    ERROR_CHECK();
}

void OGLRenderer::Swap()
{
    ERROR_CHECK();
    glfwSwapBuffers(mContext->mGLFWWindow);
    ERROR_CHECK();
}

void OGLRenderer::OnResize(int aWidth, int aHeight)
{
    glViewport(0, 0, aWidth, aHeight);

    BindRenderTarget();

    // Resize render targets
    int i = 0;
    for (sRenderTarget renderTarget : mResources->mRenderTargets)
    {
        if (renderTarget.mScaleToWindow)
        {
            sOGLTexture texture = mResources->mTextures[renderTarget.mTextureID.mResourceId];
            int newWidth = (int)(((float)(renderTarget.mOriginalTextureWidth) / (float)(renderTarget.mOriginalWindowWidth)) * (float)(aWidth));
            int newHeight = (int)(((float)(renderTarget.mOriginalTextureHeight) / (float)(renderTarget.mOriginalWindowHeight)) * (float)(aHeight));
            {
                ERROR_CHECK();
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderTarget.mFrameBuffer);

                ERROR_CHECK();
                // resize color buffer
                UpdateTexture(renderTarget.mTextureID, 0, newWidth, newHeight);

                ERROR_CHECK();

                // resize depth buffer
                glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderTarget.mDepthBuffer);
                ERROR_CHECK();
                glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, newWidth, newHeight);
                ERROR_CHECK();
            }
        }
        i++;
    }
}

int OGLRenderer::BuildContext(OGLRenderWindow* aWindow, sOGLContext* aContext)
{
    mContext->mWindow = aWindow;

    HWND hwnd = glfwGetWin32Window(aWindow->GetGLFWWindow());
    HDC hdc = GetDC(hwnd);
    HGLRC context = wglCreateContext(hdc);
    wglMakeCurrent(hdc, context);
    
    ERROR_CHECK();

    SetWindow(aWindow);

    ERROR_CHECK();

    assert(glewInit() == GLEW_OK);

    ERROR_CHECK();

    printf("OpenGL version: %s;\n", glGetString(GL_VERSION));

    glGenProgramPipelines(1, &mContext->mDefaultPipeline);
    glBindProgramPipeline(mContext->mDefaultPipeline);

    ERROR_CHECK();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ERROR_CHECK();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    ERROR_CHECK();

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    ERROR_CHECK();

    mResources->mRenderTargets.push_back(sRenderTarget());

    return true;
}

void OGLRenderer::SetWindow(class OGLRenderWindow* aWindow)
{
    if (mContext)
    {
        mContext->mGLFWWindow = aWindow->GetGLFWWindow();
    }
}
