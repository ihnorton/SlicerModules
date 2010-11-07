/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

// .NAME vtkHybridRegisterLogic - slicer logic class for Locator module 
// .SECTION Description
// This class manages the logic associated with tracking device for
// IGT. 


#ifndef __vtkHybridRegisterLogic_h
#define __vtkHybridRegisterLogic_h

#include "vtkHybridRegisterWin32Header.h"

#include "vtkPoints.h"
#include "vtkCollection.h"

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLSliceNode.h"

class vtkIGTLConnector;

class VTK_HybridRegister_EXPORT vtkHybridRegisterLogic : public vtkSlicerModuleLogic 
{
 public:
  //BTX
  enum {  // Events
    //LocatorUpdateEvent      = 50000,
    StatusUpdateEvent       = 50001,
  };
  //ETX

 public:
  
  static vtkHybridRegisterLogic *New();
  
  void StartPointCollection(vtkMRMLTransformableNode *node1, vtkMRMLTransformableNode *node2);
  void StopPointCollection();
  int RunRegistration();
  int SetCollectionInterval ( int );
  int SetTransform(const char*, int);
  int SetOutputTransformNodeID ( const char* );
  void ProcessTimerEvents();
  void ProcessMRMLEvents( vtkObject *, unsigned long, void *);

  void DebugDisplay();

  vtkTypeRevisionMacro(vtkHybridRegisterLogic,vtkObject);
  void PrintSelf(ostream&, vtkIndent);

 protected:
  
  vtkHybridRegisterLogic();
  ~vtkHybridRegisterLogic();

  void operator=(const vtkHybridRegisterLogic&);
  vtkHybridRegisterLogic(const vtkHybridRegisterLogic&);

  static void DataCallback(vtkObject*, unsigned long, void *, void *);
  void UpdateAll();

  vtkCallbackCommand *DataCallbackCommand;

 private:

  int CollectionInterval;
  int TimerFlag;
  const char* OutputTransformNodeID;
  const char* Transform1NodeId;
  const char* Transform2NodeId;
  vtkCollection* TransformSet1;
  vtkCollection* TransformSet2;
  vtkMRMLLinearTransformNode* TransformNode1;
  vtkMRMLLinearTransformNode* TransformNode2;

  vtkPoints* GetPointsFromTransformCollection(vtkCollection*);
  vtkPolyData* MakePolyDataWithCells(vtkPoints*);

  //DEBUG
  vtkPolyData* pd1;
  vtkPolyData* pd2;
  vtkMRMLModelNode* mod1;
  vtkMRMLModelNode* mod2;

};

#endif


  
