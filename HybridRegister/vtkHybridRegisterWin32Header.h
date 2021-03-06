/*==========================================================================

Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $HeadURL: $
Date:      $Date: $
Version:   $Revision: $

==========================================================================*/

#ifndef __vtkHybridRegisterWin32Header_h
#define __vtkHybridRegisterWin32Header_h

#include <vtkHybridRegisterConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(HybridRegister_EXPORTS)
#define VTK_HybridRegister_EXPORT __declspec( dllexport ) 
#else
#define VTK_HybridRegister_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_HybridRegister_EXPORT 
#endif
#endif
