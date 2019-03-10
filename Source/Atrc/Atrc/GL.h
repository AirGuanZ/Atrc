#pragma once

#include <QOpenGLFunctions_4_5_Core>

inline QOpenGLFunctions_4_5_Core atrcQtGLCtx;

#define AGZ_USE_OPENGL
#define AGZ_DISABLE_NATIVE_GL_CONTEXT

#define AGZ_GL_CTX atrcQtGLCtx.

#include <AGZUtils/Utils/GL.h>
