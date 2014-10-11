// -*- C++ -*-

//=============================================================================
/**
 *  @file    pre.h
 *
 *  $Id: pre.h 80826 2008-03-04 14:51:23Z wotte $
 *
 *  @author Christopher Kohlhoff <chris@kohlhoff.com>
 *
 *  This file saves the original alignment rules and changes the alignment
 *  boundary to ACE's default.
 */
//=============================================================================

// No header guard

#if !defined (_MSC_VER)
# include <compiler/diagnostic_push.h>
#endif

#if defined (_MSC_VER)
# pragma warning (disable:4103)
# pragma warning (disable:4251)
# pragma pack (push, 8)
#elif defined (__BORLANDC__)
# pragma option push -a8 -b -Ve- -Vx- -w-rvl -w-rch -w-ccc -w-obs -w-aus -w-pia -w-inl -w-sig
# pragma nopushoptwarn
# pragma nopackwarning
#endif

#include <compiler/disable_unused_parameter.h>

#if defined __GNUC__ 
# if defined __APPLE__
#  pragma GCC diagnostic ignored "-Wdeprecated-register"
#  pragma GCC diagnostic ignored "-Wextern-c-compat"
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
# endif
#  pragma GCC diagnostic ignored "-Wunused-variable"
#endif

