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
#include "vtkPolyData.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkVertexGlyphFilter.h"
#include "vtkLandmarkTransform.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkMatrix4x4.h"
#include "vtkIterativeClosestPointTransform.h"
#include "vtkTransformPolyDataFilter.h"

#include "vtkKWTkUtilities.h"
#include "vtkTransformCollection.h"

#include "vtkMRMLTransformableNode.h"
#include "vtkMRMLLinearTransformNode.h"

//DEBUG
#include "vtkGlyph3D.h"
#include "vtkSphereSource.h"
#include "vtkMRMLModelDisplayNode.h"

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
  //DEBUG
  if ( this->mod1 ) { this->mod1->Delete(); }
  if ( this->mod2 ) { this->mod2->Delete(); }
  //END DEBUG

  if (this->DataCallbackCommand) { this->DataCallbackCommand->Delete(); }
  if ( this->TransformSet1 ) { this->TransformSet1->Delete(); }
  if ( this->TransformSet2 ) { this->TransformSet2->Delete(); }

}

//---------------------------------------------------------------------------
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

//---------------------------------------------------------------------------
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

//---------------------------------------------------------------------------
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
	this->TransformSet2->PrintSelf( std::cerr, indent );

	vtkPoints* pts1 = vtkPoints::New();
	vtkPoints* pts2 = vtkPoints::New();
	pts1 = this->GetPointsFromTransformCollection(this->TransformSet1);
	pts2 = this->GetPointsFromTransformCollection(this->TransformSet2);
	this->pd1 = this->MakePolyDataWithCells(pts1);
	this->pd2 = this->MakePolyDataWithCells(pts2);
	pts1->Delete();
	pts1->Delete();

}

//---------------------------------------------------------------------------
int vtkHybridRegisterLogic::SetOutputTransformNodeID ( const char* id )
{
	this->OutputTransformNodeID = id;
}


//---------------------------------------------------------------------------
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

//---------------------------------------------------------------------------
int vtkHybridRegisterLogic::SetCollectionInterval(int interval)
{
	this->CollectionInterval = interval;
	return 0;
}

vtkPoints* vtkHybridRegisterLogic::GetPointsFromTransformCollection(vtkCollection* t)
{
	double x,y,z;
	vtkPoints* p = vtkPoints::New();
	for (int i = 0; i < t->GetNumberOfItems(); i++ )
		{
			vtkMatrix4x4* mat = vtkMatrix4x4::SafeDownCast ( t->GetItemAsObject(i) );
			x = mat->Element[0][3];
			y = mat->Element[1][3];
			z = mat->Element[2][3];
			p->InsertNextPoint(x,y,z);
			mat->Delete();
		}
	return p;
}

vtkPolyData* vtkHybridRegisterLogic::MakePolyDataWithCells(vtkPoints* p)
{
	vtkPolyData* temppd = vtkPolyData::New();
	vtkPolyData* output = vtkPolyData::New();
	vtkVertexGlyphFilter* gf = vtkVertexGlyphFilter::New();
	temppd->SetPoints(p);
	gf->SetInputConnection(temppd->GetProducerPort());
	gf->Update();
	output->ShallowCopy(gf->GetOutput());
	return output;
}

//---------------------------------------------------------------------------
void vtkHybridRegisterLogic::DebugDisplay()
{
	if ( not (this->pd1) and (this->pd2) ) { return; };
	//TODO: (remove) Perturb the points so we know it actually did something.
	vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
	vtkTransform* transform = vtkTransform::New();
	transform->Translate(-10,5,12);
	filter->SetInput(this->pd1);
	filter->SetTransform(transform);
	filter->Update();
	pd1 = filter->GetOutput();
	pd1->Update();

	mod1 = vtkMRMLModelNode::New();
	mod2 = vtkMRMLModelNode::New();
	vtkMRMLModelDisplayNode* mdn1 = vtkMRMLModelDisplayNode::New();
	vtkMRMLModelDisplayNode* mdn2 = vtkMRMLModelDisplayNode::New();
	this->GetMRMLScene()->AddNodeNoNotify(mdn1);
	this->GetMRMLScene()->AddNodeNoNotify(mdn2);
	vtkSphereSource* sphere = vtkSphereSource::New();
	sphere->SetRadius(1);
	sphere->SetThetaResolution(10);
	sphere->SetPhiResolution(10);
	sphere->Update();
	if (this->pd1)
	{
		vtkGlyph3D* g1 = vtkGlyph3D::New();
		g1->SetSource ( sphere->GetOutput() );
		g1->SetInput ( this->pd1 );
		g1->Update();

		mdn1->SetScene( this->MRMLScene );

		mod1->SetScene( this->MRMLScene );
		mod1->SetAndObservePolyData( g1->GetOutput() );
		mod1->SetAndObserveDisplayNodeID ( mdn1->GetID() );
		mod1->Modified();
		this->MRMLScene->AddNode(mod1);
	}
	if (this->pd2)
	{
		vtkGlyph3D* g2 = vtkGlyph3D::New();
		g2->SetSource ( sphere->GetOutput() );
		g2->SetInput ( this->pd2 );
		g2->Update();

		mod2->SetScene( this->MRMLScene);
		mod2->SetAndObservePolyData( g2->GetOutput() );
		mod2->SetAndObserveDisplayNodeID ( mdn2->GetID() );
		mod2->Modified();
		this->GetMRMLScene()->AddNode(mod2);
	}
}

//---------------------------------------------------------------------------
int vtkHybridRegisterLogic::RunRegistration()
{
	vtkMatrix4x4* resultmatrix = vtkMatrix4x4::New();
	vtkIndent indent = vtkIndent::vtkIndent(2);

	std::cerr << "RunRegistration called" << endl;

	//pd1->PrintSelf(std::cerr, indent);
	//pd2->PrintSelf(std::cerr, indent);

	std::cerr << "Running ICP now..." << endl;
	//pd1->PrintSelf(std::cerr,indent);
	//pd2->PrintSelf(std::cerr,indent);
	vtkIterativeClosestPointTransform* icp = vtkIterativeClosestPointTransform::New();
	icp->GetLandmarkTransform()->SetModeToRigidBody();
	icp->SetMaximumNumberOfIterations(200);
	cout<<"Setting source.." << endl;
	icp->SetSource(pd2);
	cout << "Setting target.." << endl;
	icp->SetTarget(pd1);
	icp->Modified();
	icp->Update();
	std::cerr << "Ran ICP: " << std::endl;
	//icp->PrintSelf(std::cerr, indent);
	std::cerr << "completed registration" << endl;

	icp->GetMatrix()->PrintSelf(std::cerr, indent);
	//icp->GetLandmarkTransform()->PrintSelf(std::cerr,indent);

	pd1->Update();
	pd2->Update();

	resultmatrix = icp->GetMatrix();
	std::cerr << "printing output xform node" << endl;
	resultmatrix->PrintSelf(std::cerr,indent);
	cout << "output transform id:" << this->OutputTransformNodeID << endl;
	vtkMRMLLinearTransformNode* node = vtkMRMLLinearTransformNode::SafeDownCast( this->MRMLScene->GetNodeByID(this->OutputTransformNodeID) );
	if ( node )
	{
		std::cerr << "trying to apply xfrom" << endl;
		node->GetMatrixTransformToParent()->DeepCopy( resultmatrix );
	}
	std::cerr << "done applying" << endl;
	return 0;
}


//---------------------------------------------------------------------------
void vtkHybridRegisterLogic::ProcessMRMLEvents(vtkObject *caller,
                                                                  unsigned long event,
                                                                  void* callData)
{

	if ( (event = vtkMRMLLinearTransformNode::TransformModifiedEvent) )
	{
		vtkMRMLLinearTransformNode *node = vtkMRMLLinearTransformNode::SafeDownCast(caller);
		if (node == this->TransformNode1)
		{
			//std::cerr << "Transform 1 modified" << endl;
			vtkMatrix4x4 *mat = vtkMatrix4x4::New();
			this->TransformNode1->GetMatrixTransformToWorld(mat);
			this->TransformSet1->AddItem(mat);


			mat->Delete();
		}
		if (node == this->TransformNode2)
		{
			//std::cerr << "Transform 2 modified" << endl;
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







