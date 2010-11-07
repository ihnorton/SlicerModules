/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkHybridRegisterGUI_h
#define __vtkHybridRegisterGUI_h

#ifdef WIN32
#include "vtkHybridRegisterWin32Header.h"
#endif

#include "vtkSlicerModuleGUI.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"

#include "vtkHybridRegisterLogic.h"

class vtkKWPushButton;
class vtkKWEntryWithLabel;
class vtkSlicerNodeSelectorWidget;

class VTK_HybridRegister_EXPORT vtkHybridRegisterGUI : public vtkSlicerModuleGUI
{
 public:

  vtkTypeRevisionMacro ( vtkHybridRegisterGUI, vtkSlicerModuleGUI );

  //----------------------------------------------------------------
  // Set/Get Methods
  //----------------------------------------------------------------

  vtkGetObjectMacro ( Logic, vtkHybridRegisterLogic );
  void SetModuleLogic ( vtkSlicerLogic *logic )
  { 
    this->SetLogic ( vtkObjectPointer (&this->Logic), logic );
  }

 protected:
  //----------------------------------------------------------------
  // Constructor / Destructor (proctected/private) 
  //----------------------------------------------------------------

  vtkHybridRegisterGUI ( );
  virtual ~vtkHybridRegisterGUI ( );

 private:
  vtkHybridRegisterGUI ( const vtkHybridRegisterGUI& ); // Not implemented.
  void operator = ( const vtkHybridRegisterGUI& ); //Not implemented.

 public:
  //----------------------------------------------------------------
  // New method, Initialization etc.
  //----------------------------------------------------------------

  static vtkHybridRegisterGUI* New ();
  void Init();
  virtual void Enter ( );
  virtual void Exit ( );
  void PrintSelf (ostream& os, vtkIndent indent );

  //----------------------------------------------------------------
  // Observer Management
  //----------------------------------------------------------------

  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );
  void AddLogicObservers ( );
  void RemoveLogicObservers ( );

  //----------------------------------------------------------------
  // Event Handlers
  //----------------------------------------------------------------

  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  void ProcessTimerEvents();
  void HandleMouseEvent(vtkSlicerInteractorStyle *style);
  static void DataCallback(vtkObject *caller, 
                           unsigned long eid, void *clientData, void *callData);
  
  //----------------------------------------------------------------
  // Build Frames
  //----------------------------------------------------------------

  virtual void BuildGUI ( );
  void BuildGUIForHelpFrame();
  void BuildGUIForAcquireControlFrame();
  void BuildGUIForRegisterControlFrame();


  //----------------------------------------------------------------
  // Update routines
  //----------------------------------------------------------------

  void UpdateAll();


 protected:
  
  //----------------------------------------------------------------
  // Timer
  //----------------------------------------------------------------
  
  int TimerFlag;
  int TimerInterval;

  //----------------------------------------------------------------
  // GUI widgets
  //----------------------------------------------------------------

  vtkSlicerNodeSelectorWidget* Transform1NodeSelectorMenu;
  vtkSlicerNodeSelectorWidget* Transform2NodeSelectorMenu;
  vtkSlicerNodeSelectorWidget* OutputTransformNodeSelectorMenu;
  vtkKWPushButton* StartButton;
  vtkKWPushButton* StopButton;
  vtkKWPushButton* RegisterButton;
  vtkKWPushButton* DebugButton;
  vtkKWEntryWithLabel* CollectionIntervalEntry;

  //----------------------------------------------------------------
  // Logic Values
  //----------------------------------------------------------------

  vtkHybridRegisterLogic *Logic;
  vtkCallbackCommand *DataCallbackCommand;
  int                        CloseScene;

};



#endif
