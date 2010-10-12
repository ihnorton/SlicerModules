/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/


#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerApplicationGUI.h"

#include "vtkHybridRegisterLogic.h"

#include "vtkIndent.h"

#include "vtkKWTkUtilities.h"
#include "vtkTransformCollection.h"

#include "vtkMRMLTransformableNode.h"
#include "vtkMRMLLinearTransformNode.h"

vtkCxxRevisionMacro(vtkHybridRegisterLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkHybridRegisterLogic);

//---------------------------------------------------------------------------
vtkHybridRegisterLogic::vtkHybridRegisterLogic()
{
	this->CollectionInterval = 10000;
  // Timer Handling

  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback( vtkHybridRegisterLogic::DataCallback );

  //Set up internal objects
  this->TransformSet1 = vtkTransformCollection::New();
  this->TransformSet2 = vtkTransformCollection::New();

}

//---------------------------------------------------------------------------
vtkHybridRegisterLogic::~vtkHybridRegisterLogic()
{

  if (this->DataCallbackCommand) { this->DataCallbackCommand->Delete(); }
  if ( this->TransformSet1 ) { this->TransformSet1->Delete(); }
  if ( this->TransformSet2 ) { this->TransformSet2->Delete(); }
}

void vtkHybridRegisterLogic::ProcessTimerEvents()
{
	cout << "GOT TIMER EVENT" << endl;
	// Handle the callback at the end of time
	if (this->TimerFlag) {
		this->TimerFlag = 0;
		cout << "StopPointCollection" << endl;
		this->StopPointCollection();
	}
}

void vtkHybridRegisterLogic::StartPointCollection(vtkMRMLTransformableNode *node1, vtkMRMLTransformableNode *node2)
{
	cout << "Logic::StartPointCollection called" << endl;
	this->TransformNode1 =
	    vtkMRMLLinearTransformNode::SafeDownCast( node1 );
	this->TransformNode2 =
	    vtkMRMLLinearTransformNode::SafeDownCast( node2 );

//	vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
//		events->InsertNextValue ( vtkMRMLTransformableNode::TransformModifiedEvent );
//	vtkSetAndObserveMRMLNodeEventsMacro(this->TransformNode1, transformableNode, events);
//	vtkSetAndObserveMRMLNodeEventsMacro(this->TransformNode2, transformableNode, events);

	//TODO: Should vtkSetAndObserve macro be used or *->AddObserver ..?
	vtkIntArray* nodeEvents = vtkIntArray::New();
	nodeEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
	vtkSetAndObserveMRMLNodeEventsMacro( this->TransformNode1, this->TransformNode1, nodeEvents);
	vtkSetAndObserveMRMLNodeEventsMacro( this->TransformNode2, this->TransformNode2, nodeEvents);
	nodeEvents->Delete();

	// Start timer to wait CollectionInterval ms before killing the point collection.
	vtkKWTkUtilities::CreateTimerHandler( vtkKWApplication::GetMainInterp(), this->CollectionInterval, this, "ProcessTimerEvents");

}

void vtkHybridRegisterLogic::StopPointCollection()
{
	//TODO: is this the right/ok way to unobserve?
	//pattern from vtkSlicerClipModelsWidget::~vtkSlicerClipModelsWidget
	vtkSetAndObserveMRMLNodeMacro ( this->TransformNode1, NULL);
	vtkSetAndObserveMRMLNodeMacro ( this->TransformNode2, NULL);
	this->TimerFlag = 0;

	std::cerr << "Stopped Point Collection. Contents:" << endl;
	vtkIndent indent = vtkIndent::vtkIndent(0);
	this->TransformSet1->PrintSelf( std::cerr, indent );
}

int vtkHybridRegisterLogic::SetTransform(const char* nodeId, int transformNumber)
{
  vtkMRMLLinearTransformNode* node =
    vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nodeId));

  if (node && strcmp(node->GetNodeTagName(), "LinearTransform") == 0)
    {
			if ( transformNumber == 1 )
			{
				this->Transform1NodeId = nodeId;
			}
			else if ( transformNumber == 2 )
			{
				this->Transform2NodeId = nodeId;
			}
			return 1;
    }

  return 0;

}

int vtkHybridRegisterLogic::SetCollectionInterval(int interval)
{
	this->CollectionInterval = interval;
	return 0;
}

void vtkHybridRegisterLogic::ProcessMRMLEvents(vtkObject *caller,
                                                                  unsigned long event,
                                                                  void* callData)
{

	if ( event = vtkMRMLLinearTransformNode::TransformModifiedEvent )
	{
		vtkMRMLLinearTransformNode *node = vtkMRMLLinearTransformNode::SafeDownCast(caller);
		if (node == this->TransformNode1)
		{
			std::cerr << "Transform 1 modified" << endl;
			vtkMatrix4x4 *mat = vtkMatrix4x4::New();
			this->TransformNode1->GetMatrixTransformToWorld(mat);
			this->TransformSet1->AddItem(mat);
			mat->Delete();
		}
		if (node == this->TransformNode2)
		{
			std::cerr << "Transform 2 modified" << endl;
			vtkMatrix4x4 *mat = vtkMatrix4x4::New();
			this->TransformNode2->GetMatrixTransformToWorld(mat);
			this->TransformSet2->AddItem(mat);
			mat->Delete();
		}
	}


}





//---------------------------------------------------------------------------
void vtkHybridRegisterLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkHybridRegisterLogic:             " << this->GetClassName() << "\n";

}


//---------------------------------------------------------------------------
void vtkHybridRegisterLogic::DataCallback(vtkObject *caller, 
                                       unsigned long eid, void *clientData, void *callData)
{
  vtkHybridRegisterLogic *self = reinterpret_cast<vtkHybridRegisterLogic *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkHybridRegisterLogic DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkHybridRegisterLogic::UpdateAll()
{

}







