// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#pragma once

#if defined(ADCONTROLS_LIBRARY)
#  define ADCONTROLSSHARED_EXPORT __declspec(dllexport)
#else
#  define ADCONTROLSSHARED_EXPORT __declspec(dllimport)
#endif

