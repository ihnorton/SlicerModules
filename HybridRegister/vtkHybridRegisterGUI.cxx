/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkHybridRegisterGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSlicesGUI.h"
#include "vtkMRMLTransformableNode.h"

#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWWidget.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWEvent.h"
#include "vtkKWPushButton.h"
#include "vtkKWEntryWithLabel.h"

#include "vtkCornerAnnotation.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkHybridRegisterGUI );
vtkCxxRevisionMacro ( vtkHybridRegisterGUI, "$Revision: 1.0 $");
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
vtkHybridRegisterGUI::vtkHybridRegisterGUI ( )
{

  //----------------------------------------------------------------
  // Logic values
  this->Logic = NULL;
  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkHybridRegisterGUI::DataCallback);
  
  //----------------------------------------------------------------
  // GUI widgets

  this->Transform1NodeSelectorMenu = vtkSlicerNodeSelectorWidget::New();
  this->Transform2NodeSelectorMenu = vtkSlicerNodeSelectorWidget::New();
  this->StartButton = vtkKWPushButton::New ( );
  this->StopButton = vtkKWPushButton::New ( );
  this->CollectionIntervalEntry = vtkKWEntryWithLabel::New();

  
  //----------------------------------------------------------------
  // Locator  (MRML)
  this->TimerFlag = 0;

}

//---------------------------------------------------------------------------
vtkHybridRegisterGUI::~vtkHybridRegisterGUI ( )
{

  //----------------------------------------------------------------
  // Remove Callbacks

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

  //----------------------------------------------------------------
  // Remove Observers

  this->RemoveGUIObservers();

  //----------------------------------------------------------------
  // Remove GUI widgets

  this->Transform1NodeSelectorMenu->Delete();
  this->Transform2NodeSelectorMenu->Delete();
  this->StartButton->Delete();
  this->StopButton->Delete();
  this->CollectionIntervalEntry->Delete();



  //----------------------------------------------------------------
  // Unregister Logic class

  this->SetModuleLogic ( NULL );

}


//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::Init()
{
}


//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::Enter()
{
  // Fill in
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  if (this->TimerFlag == 0)
    {
    this->TimerFlag = 1;
    this->TimerInterval = 100;  // 100 ms
    ProcessTimerEvents();
    }

}


//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::Exit ( )
{
  // Fill in
}


//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "HybridRegisterGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "Logic: " << this->GetLogic ( ) << "\n";
}


//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::RemoveGUIObservers ( )
{
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
 this->StartButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
 this->StopButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
 this->RemoveLogicObservers();

}


//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::AddGUIObservers ( )
{
  this->RemoveGUIObservers();

  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  //----------------------------------------------------------------
  // MRML

  vtkIntArray* events = vtkIntArray::New();
  //events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  //events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  
  if (this->GetMRMLScene() != NULL)
    {
    this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    }
  events->Delete();

  //----------------------------------------------------------------
  // GUI Observers

  this->StartButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->StopButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);


  this->AddLogicObservers();

}


//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::RemoveLogicObservers ( )
{
  if (this->GetLogic())
    {
    this->GetLogic()->RemoveObservers(vtkCommand::ModifiedEvent,
                                      (vtkCommand *)this->LogicCallbackCommand);
    }
}




//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::AddLogicObservers ( )
{
  this->RemoveLogicObservers();  

  if (this->GetLogic())
    {
    this->GetLogic()->AddObserver(vtkHybridRegisterLogic::StatusUpdateEvent,
                                  (vtkCommand *)this->LogicCallbackCommand);
    }
}

//---------------------------------------------------------------------------
//void vtkHybridRegisterGUI::SetModuleLogic ( vtkSlicerLogic *logic )
//{
//  this->ModuleLogic = vtkHybridRegisterGUI::SafeDownCast(logic);
//}

//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{
}


//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::ProcessGUIEvents(vtkObject *caller,
		unsigned long event, void *callData)
{

	if (this->StartButton == vtkKWPushButton::SafeDownCast(caller)
		&& event == vtkKWPushButton::InvokedEvent)
	{
		if ( this->Transform1NodeSelectorMenu->GetSelected() != NULL && this->Transform2NodeSelectorMenu->GetSelected() != NULL )
		{
			std::cerr << "StartButton is pressed." << std::endl;

			vtkMRMLTransformableNode *node1 = vtkMRMLTransformableNode::SafeDownCast( this->Transform1NodeSelectorMenu->GetSelected() );
			vtkMRMLTransformableNode *node2 = vtkMRMLTransformableNode::SafeDownCast( this->Transform2NodeSelectorMenu->GetSelected() );
			this->Logic->SetCollectionInterval( this->CollectionIntervalEntry->GetWidget()->GetValueAsInt() );
			this->Logic->StartPointCollection(node1,node2);
		}
		else
		{
			std::cerr << "ERROR: Must select both transforms!" << endl;
		}
	}
	else if (this->StopButton == vtkKWPushButton::SafeDownCast(caller)
	&& event == vtkKWPushButton::InvokedEvent)
	{
		std::cerr << "StopButton is pressed." << std::endl;
		this->Logic->StopPointCollection();
	}

} 




void vtkHybridRegisterGUI::DataCallback(vtkObject *caller, 
                                     unsigned long eid, void *clientData, void *callData)
{
  vtkHybridRegisterGUI *self = reinterpret_cast<vtkHybridRegisterGUI *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkHybridRegisterGUI DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::ProcessLogicEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{

  if (this->GetLogic() == vtkHybridRegisterLogic::SafeDownCast(caller))
    {
    if (event == vtkHybridRegisterLogic::StatusUpdateEvent)
      {
      //this->UpdateDeviceStatus();
      }
    }
}


//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
  // Fill in

  if (event == vtkMRMLScene::SceneCloseEvent)
    {
    }
}


//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::ProcessTimerEvents()
{
  if (this->TimerFlag)
    {
    // update timer
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                                         this->TimerInterval,
                                         this, "ProcessTimerEvents");        
    }
}


//---------------------------------------------------------------------------
void vtkHybridRegisterGUI::BuildGUI ( )
{

  // ---
  // MODULE GUI FRAME 
  // create a page
  this->UIPanel->AddPage ( "HybridRegister", "HybridRegister", NULL );

  BuildGUIForHelpFrame();
  BuildGUIForAcquireControlFrame();

}


void vtkHybridRegisterGUI::BuildGUIForHelpFrame ()
{
  // Define your help text here.
  const char *help = 
    "See "
    "<a>http://www.slicer.org/slicerWiki/index.php/Modules:HybridRegister</a> for details.";
  const char *about =
    "This work is supported by NCIGT, NA-MIC.";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "HybridRegister" );
  this->BuildHelpAndAboutFrame (page, help, about);
}


//---------------------------------------------------------------------------Selection
void vtkHybridRegisterGUI::BuildGUIForAcquireControlFrame()
{

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("HybridRegister");
  
  vtkSlicerModuleCollapsibleFrame *moduleFrame = vtkSlicerModuleCollapsibleFrame::New();

  moduleFrame->SetParent(page);
  moduleFrame->Create();
  moduleFrame->SetLabelText("Controller");
  //transformSelectFrame->CollapseFrame();
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               moduleFrame->GetWidgetName(), page->GetWidgetName());

  this->Transform1NodeSelectorMenu->SetParent( moduleFrame->GetFrame() );
  this->Transform1NodeSelectorMenu->Create();
  this->Transform1NodeSelectorMenu->SetWidth(30);
  this->Transform1NodeSelectorMenu->SetNewNodeEnabled(0);
  this->Transform1NodeSelectorMenu->SetNodeClass("vtkMRMLLinearTransformNode", NULL, NULL, NULL);
  this->Transform1NodeSelectorMenu->NoneEnabledOn();
  this->Transform1NodeSelectorMenu->SetShowHidden(1);
  this->Transform1NodeSelectorMenu->Create();
  this->Transform1NodeSelectorMenu->SetMRMLScene(this->Logic->GetMRMLScene());
  this->Transform1NodeSelectorMenu->UpdateMenu();
  this->Transform1NodeSelectorMenu->SetBorderWidth(0);
  this->Transform1NodeSelectorMenu->SetBalloonHelpString("Select a transform from the Scene");
  app->Script("pack %s %s -side top -anchor w -fill x -padx 2 -pady 2",
              moduleFrame->GetWidgetName() , this->Transform1NodeSelectorMenu->GetWidgetName());


  this->Transform2NodeSelectorMenu->SetParent(moduleFrame->GetFrame());
  this->Transform2NodeSelectorMenu->Create();
  this->Transform2NodeSelectorMenu->SetWidth(30);
  this->Transform2NodeSelectorMenu->SetNewNodeEnabled(0);
  this->Transform2NodeSelectorMenu->SetNodeClass("vtkMRMLLinearTransformNode", NULL, NULL, NULL);
  this->Transform2NodeSelectorMenu->NoneEnabledOn();
  this->Transform2NodeSelectorMenu->SetShowHidden(1);
  this->Transform2NodeSelectorMenu->Create();
  this->Transform2NodeSelectorMenu->SetMRMLScene(this->Logic->GetMRMLScene());
  this->Transform2NodeSelectorMenu->UpdateMenu();
  this->Transform2NodeSelectorMenu->SetBorderWidth(0);
  this->Transform2NodeSelectorMenu->SetBalloonHelpString("Select a transform from the Scene");
  app->Script("pack %s %s -side top -anchor w -fill x -padx 2 -pady 2",
              moduleFrame->GetWidgetName() , this->Transform2NodeSelectorMenu->GetWidgetName());

  // -----------------------------------------
  this->CollectionIntervalEntry->SetParent( moduleFrame->GetFrame() );
  this->CollectionIntervalEntry->Create();
  this->CollectionIntervalEntry->SetWidth( 8 );
  this->CollectionIntervalEntry->SetLabelPositionToLeft();
  this->CollectionIntervalEntry->SetLabelText( "Time (ms):");
  this->CollectionIntervalEntry->Create();
  this->CollectionIntervalEntry->SetBorderWidth(0);
  app->Script("pack %s %s -side top -anchor w -fill x -padx 2 -pady 2",
                moduleFrame->GetWidgetName() , this->CollectionIntervalEntry->GetWidgetName());

  // -----------------------------------------

  this->StartButton->SetParent ( moduleFrame->GetFrame() );
  this->StartButton->Create ( );
  this->StartButton->SetText ("Start");
  this->StartButton->SetWidth (12);

  this->StopButton->SetParent ( moduleFrame->GetFrame() );
  this->StopButton->Create ( );
  this->StopButton->SetText ("Stop");
  this->StopButton->SetWidth (12);

  this->Script("pack %s %s -side left -padx 2 -pady 2", 
               this->StartButton->GetWidgetName(),
               this->StopButton->GetWidgetName());

  moduleFrame->Delete();

}



//----------------------------------------------------------------------------
void vtkHybridRegisterGUI::UpdateAll()
{
}

