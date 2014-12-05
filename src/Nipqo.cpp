#include <time.h>
//#include <resource.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cmath>
#include <iostream>
#include <string.h>
#include <fstream>

#include "TypeDefs.h"
#include "HashSet.h"
#include "Point.h"
#include "Rectangle.h"
#include "Graph.h"

//struct rusage rusageBeg, rusageEnd;		//Chetas

int *Utility_t::twoPowerN = NULL;
int Nipqo_t::methodNum = 2;
int Nipqo_t::optPhase = -1;

PlanInfo* PlanInfo::obj;

Boolean_t Nipqo_t::optimizeOnlySquares = FALSE;
Boolean_t Nipqo_t::optimizeIntermediateDimensions = TRUE;

int Nipqo_t::numParam = 0;
//IntCoord_t *Nipqo_t::relSize = NULL;
int Nipqo_t::resolution = 0;		//Chetas- Added resolution
float **Plan::selectivityConsts = NULL;
CostVal_t PD_THRESHOLD = 0;
PlanId_t newPlanID = 0;				//Chetas- Added for correct version.
string fixQuery;

RealCoord_t Nipqo_t::minSquareSize = 0.1;
// 0.03125; // = 1.0/32
// 0.125;   // = 1.0/8
Boolean_t Nipqo_t::exponentOn = FALSE;
RealCoord_t Nipqo_t::expWt = 5;
PlanId_t Nipqo_t::numPlans = 0;
int Nipqo_t::numOptimizers = 0;
HashSet_t Nipqo_t::vertexCache;
pOptimizer_t **Nipqo_t::pOpts = NULL;

Boolean Nipqo_t::useCostLimit = FALSE;
Boolean Nipqo_t::useConstCostLimit = FALSE;
CostVal_t Nipqo_t::constCostLimit = 1e10;

Boolean Nipqo_t::printTrace = FALSE;

static int vertexNotLocatedErrorsInSmallRect = 0;		//Chetas- Added for showing Equicost point not located errors.
static int vertexNotLocatedErrorsInBigRect = 0;		//Chetas- Added for showing Equicost point not located errors.
Point_t ***Nipqo_t::allVertices = NULL;				//Chetas- Added for computing errors.
bool Nipqo_t::isFirstOptimization;					//Chetas- Added for Correct version.
double Nipqo_t::timeForAnipqo = 0;
double Nipqo_t::timeToOptimizeAll = 0;
double Nipqo_t::timeToInferAll = 0;

void Utility_t::Initialize(void)
{
	assert(NULL == twoPowerN);
	twoPowerN = new IntCoord_t[Nipqo_t::NumParam()+1];
	assert(twoPowerN);

	twoPowerN[0] = 1;
	for(int i = 1; i <= Nipqo_t::NumParam(); i++)
		twoPowerN[i] = 2 * twoPowerN[i-1];
}

int Utility_t::TwoPowerN(int n)
{
	assert(0 <= n); assert(n <= Nipqo_t::NumParam());
	return twoPowerN[n];
}

Boolean_t Nipqo_t::Initialize(int argc, char *argv[])
{
	int i;

	assert((1 == methodNum) || (2 == methodNum));
	assert(-1 == optPhase);
	assert((FALSE == optimizeOnlySquares) || (TRUE == optimizeOnlySquares));
	assert((FALSE == optimizeIntermediateDimensions) || (TRUE == optimizeIntermediateDimensions));

	assert(0 == numParam);
//	assert(NULL == relSize);		//Chetas- Not needed.
	assert(0 == numOptimizers);
	assert(NULL == pOpts);

	if( argc != 9 ) {
		printf(
				"Usage: %s pqo singleDAG|multipleDAG costLimit|constCostLimit|noCostLimit printTrace|noPrintTrace verify|noVerify numParam costLimit RelSizeFile\n",
				argv[0]);
		exit(0);
	}

	if(strcmp(argv[2], "singleDAG") && strcmp(argv[2], "multipleDAG")) {
		printf(
				"Usage: %s pqo singleDAG|multipleDAG costLimit|noCostLimit printTrace|noPrintTrace verify|noVerify numParam costLimit RelSizeFile\n",
				argv[0]);
		exit(0);
	}

//	if(!strcmp(argv[2], "singleDAG")) OptimizerInterface_t::ResetUseMultDAGs();
//	else OptimizerInterface_t::SetUseMultDAGs();		//Chetas- Commented for DSL project.

	if(strcmp(argv[3], "costLimit") && strcmp(argv[3], "constCostLimit") && strcmp(argv[3], "noCostLimit")) {
		printf(
				"Usage: %s pqo singleDAG|multipleDAG costLimit|constCostLimit|noCostLimit printTrace|noPrintTrace verify|noVerify numParam costLimit RelSizeFile\n",
				argv[0]);
		exit(0);
	}

	if(!strcmp(argv[3], "noCostLimit")) useCostLimit = FALSE;
	else {
		useCostLimit = TRUE;
		if(!strcmp(argv[3], "constCostLimit")) useConstCostLimit = TRUE;
		else useConstCostLimit = FALSE;
	}

	if(strcmp(argv[4], "printTrace") && strcmp(argv[4], "noPrintTrace")) {
		printf(
				"Usage: %s pqo singleDAG|multipleDAG costLimit|constCostLimit|noCostLimit printTrace|noPrintTrace verify|noVerify numParam costLimit RelSizeFile\n",
				argv[0]);
		exit(0);
	}

	if(!strcmp(argv[4], "printTrace")) printTrace = TRUE;
	else printTrace = FALSE;

	if(strcmp(argv[5], "verify") && strcmp(argv[5], "noVerify")) {
		printf(
				"Usage: %s pqo singleDAG|multipleDAG costLimit|constCostLimit|noCostLimit printTrace|noPrintTrace verify|noVerify numParam costLimit RelSizeFile\n",
				argv[0]);
		exit(0);
	}

//	if(!strcmp(argv[5], "verify")) OptimizerInterface_t::SetVerifyMode();
//	else OptimizerInterface_t::ResetVerifyMode();		//Chetas- Commented for DSL project.

	numParam = atoi(argv[6]);
	assert(0 < numParam);

	//Chetas- Commented for selectivity consts.
//	relSize = new int[numParam];
//	assert(relSize);

	constCostLimit = atoi(argv[7]);
	assert(strcmp(argv[3], "constCostLimit") || (0 <= constCostLimit));

//	relSize = new int[numParam];
//	assert(relSize);

	resolution = atoi(argv[9]);		//Chetas- Added for maintaining resolution.

	string selecStr(argv[8]);
	string selConstFile = "./inputs/" + selecStr + ".csv";
//	string selConstFile = "../inputs/" + selecStr + ".csv";
	Plan::initializeSelectivityConsts(numParam, resolution, (char *)selConstFile.c_str());		//Chetas- Added for selectivity consts.

//	PD_THRESHOLD = atof(argv[10]);

	Plan::readQuery(argv[11]);

	numOptimizers = 1;
	for(i = 0; i < numParam; i++) numOptimizers *= 3;
	pOpts = new pOptimizer_t *[numOptimizers];
	for(i = 0; i < numOptimizers; i++) pOpts[i] = NULL;

	return TRUE;
}

pOptimizer_t::pOptimizer_t(RealCoord_t *constPredIn, int idIn)
: rect(NULL), id(idIn), falsePositivePointsDetected(0), CSOPsize(0)
{
	assert(0 <= idIn);

	constPred = new RealCoord_t[Nipqo_t::NumParam()];
	assert(constPred);

	numVaryingParam = 0;
	for(int i = 0; i < Nipqo_t::NumParam(); i++) {
		if(constPredIn) constPred[i] = constPredIn[i];
		else constPred[i] = -1;

		if(constPred[i] < 0) numVaryingParam++;
	}

	assert(numVaryingParam <= Nipqo_t::NumParam());
}

void pOptimizer_t::FindNeghbors(Point_t *vertex, List_t<Point_t *> &neirgbors)
{
	assert(vertex);
	assert(partitioningVertices.Contains(vertex));
	assert(neirgbors.IsEmpty());

	ListIter_t<Point_t *> partVertListIter;
	partVertListIter.Attach(&partitioningVertices);

	while(!partVertListIter.IsEnd()) {
		Point_t *vertex1 = partVertListIter.Next();
		assert(vertex1);
		if(vertex1 == vertex) continue;
		assert(FALSE == vertex->IsEqual(*vertex1));

		int numCommonMinCostPlans = vertex->NumCommonMinCostPlans(vertex1);
		assert(0 <= numCommonMinCostPlans);
		int numCommonBoundaries = vertex->NumCommonBoundaries(vertex1);
		assert(0 <= numCommonBoundaries);
		assert(numCommonBoundaries <= NumVaryingParam());

		int addition = numCommonMinCostPlans + numCommonBoundaries;

		/****************************************************************************/
		assert(0 < Nipqo_t::NumPlans());
		if(1 == Nipqo_t::NumPlans()) addition++; // A crude short-cut :-( Need to fix.
		if(Nipqo_t::isFirstOptimization)		//Chetas- Added for Correct Version.
			addition++;
		/****************************************************************************/

		// if(NumVaryingParam() <= numCommonMinCostPlans)
		if(NumVaryingParam() <= addition) {
			neirgbors.Insert(vertex1);
		}

//		if(1 == Nipqo_t::NumPlans()) {								//Necessary at the first optimization.
//			neirgbors.Insert(vertex1);
//			continue;
//		}
//		if(numCommonBoundaries >= 1 && numCommonMinCostPlans >= 1)	//Line segment between two points lies on a boundary of a HyperRectangle.
//			neirgbors.Insert(vertex1);
//		else if(numCommonMinCostPlans >= 2)
//			neirgbors.Insert(vertex1);
	}
}

//Chetas- Added for Correct Version.
void pOptimizer_t::FindNeghborsOnBoundary(Point_t *vertex, List_t<Point_t *> &neirgbors)
{
	assert(vertex);
	assert(partitioningVertices.Contains(vertex));
	assert(neirgbors.IsEmpty());
	assert(vertex->BoundaryNum() == 1);

	int axis = -1;
	for(int i = 0; i < vertex->numParams(); i++) {
		if(vertex->GetRealCoord(i) == 0 || vertex->GetRealCoord(i) == 1) {
			axis = i;
			break;
		}
	}
	assert(axis > -1 && axis < vertex->numParams());

	ListIter_t<Point_t *> partVertListIter;
	partVertListIter.Attach(&partitioningVertices);

	while(!partVertListIter.IsEnd()) {
		Point_t *vertex1 = partVertListIter.Next();
		assert(vertex1);
		if(vertex1 == vertex) continue;
//		assert(FALSE == vertex->IsEqual(*vertex1));

		int numCommonMinCostPlans = vertex->NumCommonMinCostPlans(vertex1);
		assert(0 <= numCommonMinCostPlans);
		int numCommonBoundaries = vertex->NumCommonBoundaries(vertex1);
		assert(0 <= numCommonBoundaries);
		assert(numCommonBoundaries <= NumVaryingParam());

		int addition = numCommonMinCostPlans + numCommonBoundaries;

		// if(NumVaryingParam() <= numCommonMinCostPlans)
		if(NumVaryingParam() <= addition) {
			if(vertex1->GetRealCoord(axis) == vertex->GetRealCoord(axis))
				neirgbors.Insert(vertex1);
		}
	}
}

Point_t *pOptimizer_t::GetFromPartitioningList(Point_t *vertex)					//Chetas- Added for Correct Version.
{
	ListIter_t<Point_t *> itr;
	itr.Attach(&partitioningVertices);
	while(!itr.IsEnd()) {
		Point_t *vert = itr.Next();
		if(vertex == vert)
			return vert;
		if(vertex->IsEqual(*vert))
			return vert;
	}
	return NULL;
}

void pOptimizer_t::FindConflictingEdges(
		Point_t *vertex, PlanId_t plan, List_t<Edge_t *> &conflictingEdges)
{
	assert(vertex);
	assert(FALSE == vertex->IsVisited());
	assert(FALSE == vertex->IsToBeDeleted());
	vertex->SetVisited();

	/****************************************************************************/
	// vertex->SetToBeDeleted();
	if(vertex->BoundaryNum() < Nipqo_t::NumParam()) vertex->SetToBeDeleted();
	/****************************************************************************/
	// assert(0 < Nipqo_t::NumPlans());
	// if(1 != Nipqo_t::NumPlans()) vertex->SetToBeDeleted(); // A crude short-cut :-( Need to fix.
	/****************************************************************************/

	assert(0 < plan);
	assert(plan <= Nipqo_t::NumPlans());
	// assert(conflictingEdges.IsEmpty());

	AppendList_t<Point_t *> neirgbors;
	FindNeghbors(vertex, neirgbors);

	// ********************************************************
	// **** Need this assert for proper space decomposition *****
	// assert(NumVaryingParam() < neirgbors.Size());
	// ********************************************************

	// assert(neirgbors.Contains(vertex));

	ListIter_t<Point_t *> neighborsIter;
	neighborsIter.Attach(&neirgbors);

	while(!neighborsIter.IsEnd()) {
		Point_t *nVertex = neighborsIter.Next();
		assert(nVertex);

		if(nVertex->IsVisited()) continue;

		// nVertex->UpdateMinCostPlanList(); // needed?

		//Chetas- Added for Correct version.
		CostVal_t oldMaxCost = nVertex->GetMaxOptCost();
		CostVal_t oldMinCost = nVertex->OptCost();
		CostVal_t planCost = nVertex->GetPlanCost(plan);
		// nVertex->UpdateMinCostPlanList();
		assert(0 <= oldMinCost);
		assert(0 <= planCost);

		float pDiffC;
		//Chetas- Commented & added for Correct version.
		/*if(oldMinCost <= planCost) {
			pDiffC = (planCost - oldMinCost)/oldMinCost;
		}
		else {
			pDiffC = (oldMinCost - planCost)/planCost;
		}*/
		if(oldMinCost <= planCost) {
			pDiffC = (planCost - oldMinCost)/oldMinCost;
			if(pDiffC <= nVertex->GetDiffThreshold()) {
				vertex->SetVisited();
				continue;
			}
		}
		else {
			pDiffC = (oldMinCost - planCost)/planCost;
			if(pDiffC <= PD_THRESHOLD) {
				cout<<"Neighbor pt ";
				nVertex->PrintIntCoord();
				cout<<" is already equicost point. So no need to search equicost point on this conflict edge.\n\n";

				pDiffC = (oldMaxCost - planCost)/planCost;
				if(nVertex->GetDiffThreshold() < pDiffC)
					nVertex->ChangeDiffThreshold(pDiffC);
				vertex->SetVisited();
				continue;
			}
		}

		assert(0 <= pDiffC);

		//Chetas- Commented for Correct version.
/*		if(pDiffC <= PD_THRESHOLD) { // replace PD_THRESHOLD by nVertex->GetDiffThreshold ?
			vertex->SetVisited();
		}
		else if(planCost < oldMinCost) {*/
		if(planCost < oldMinCost) {
			FindConflictingEdges(nVertex, plan, conflictingEdges);
		}
		//*********************
		else {
			Edge_t *newEdge = new Edge_t(vertex, nVertex);
			assert(newEdge);

			ListIter_t<Edge_t *> edgeIter;
			edgeIter.Attach(&conflictingEdges);

			Boolean_t edgeFound = FALSE;
			while(!edgeIter.IsEnd()) {
				Edge_t *oldEdge = edgeIter.Next();
				assert(oldEdge);

				if(newEdge->IsEqual(oldEdge)) {
					assert(FALSE == edgeFound);
					edgeFound = TRUE;
				}
			}
#if 1
			assert(FALSE == edgeFound);
			conflictingEdges.Insert(newEdge);
#else
			if(FALSE == edgeFound) {
				conflictingEdges.Insert(newEdge);
			}
#endif
		}
	}
}

void pOptimizer_t::MarkVisitedAllPartitioningVertices(void)
{
	ListIter_t<Point_t *> partVertListIter;
	partVertListIter.Attach(&partitioningVertices);

	while(!partVertListIter.IsEnd()) {
		Point_t *vertex = partVertListIter.Next();
		assert(vertex);
		vertex->SetVisited();
	}
}

void pOptimizer_t::UnmarkVisitedAllPartitioningVertices(void)
{
	ListIter_t<Point_t *> partVertListIter;
	partVertListIter.Attach(&partitioningVertices);

	while(!partVertListIter.IsEnd()) {
		Point_t *vertex = partVertListIter.Next();
		assert(vertex);
		vertex->ResetVisited();
	}
}

void pOptimizer_t::OptimizeForMethodTwo(void)
{
#if 0
	Nipqo_t::SetPhaseOne();
	PlanId_t optPlan = rect->Optimize();
	assert(0 == optPlan); // For method 2; determines partitioning vertices only;
	// determines partitioning vertices only
	// no optimization

	if(NumVaryingParam() < Nipqo_t::NumParam()) return;
#endif
	Nipqo_t::SetPhaseTwo();
	Boolean_t keepLooping = TRUE;

	ListIter_t<Point_t *> partVertListIter;
	partVertListIter.Attach(&partitioningVertices);
#if 0
	int numVerticesToBePostponedOptOf = 2; // for NN3R3P
	int verticesToBePostponedOptOf[] = {2, 3};
	// int numVerticesToBePostponedOptOf = 3; // for NN4R4P
	// int verticesToBePostponedOptOf[] = {3, 10, 11};

	for(int i = 0; i < numVerticesToBePostponedOptOf; i++) {
		int vertexNum = verticesToBePostponedOptOf[i];
		vertexNum -= i;
		assert(0 < vertexNum);
		assert(vertexNum <= Utility_t::TwoPowerN(numVaryingParam));

		partVertListIter.Reset();
		Point_t *vertex = NULL;
		for(int j = 0; j < vertexNum; j++) {
			assert(!partVertListIter.IsEnd());
			vertex = partVertListIter.Next();
			assert(vertex);
		}

		assert(vertex);
		partitioningVertices.Delete(vertex);
		partitioningVertices.Insert(vertex);
	}
#endif

	while(keepLooping) {
		keepLooping = FALSE;
		partVertListIter.Reset();
#if 0
		while(!partVertListIter.IsEnd()) {
			Point_t *vertex = partVertListIter.Next();
			assert(vertex);
			PlanId_t optPlan = vertex->Optimize();
		}

		for(int planId = 1; planId <= Nipqo_t::NumPlans(); planId++)
		{
			partVertListIter.Reset();
			while(!partVertListIter.IsEnd()) {
				Point_t *vertex = partVertListIter.Next();
				assert(vertex);
				CostVal_t cost = vertex->GetPlanCost(planId);
			}
		}

		PrintPartitioningVertices();
		return;
#endif
#ifdef ANIPQO_DEBUG
		if(Nipqo_t::PrintTrace())
			PrintPartitioningVertices();
#endif
		while(!partVertListIter.IsEnd()) {
			Point_t *vertex = partVertListIter.Next();
			assert(vertex);
			vertex->UpdateMinCostPlanList();

			if(vertex->IsOptimized()) continue;
#ifdef ANIPQO_DEBUG
			if(Nipqo_t::PrintTrace()) {
				cout << "Optimizing vertex ";
				vertex->PrintIntCoord(1);
			}
#endif
			int numOldPlans = Nipqo_t::NumPlans();
			CostVal_t oldMinCost = vertex->OptCost();
			//Chetas- Added for Correct version.
			CostVal_t newMinCost;
			//Chetas- Commented & added for Correct version.
//			PlanId_t optPlan = vertex->Optimize();
			PlanId_t optPlan = vertex->Optimize(&newMinCost);
			CostVal_t beforeOptCost = vertex->getFromPlanCostList(optPlan);			//Chetas- Added for SQL server's issue.
			if(beforeOptCost != 0) {												//Chetas- If new plan was already there in list, then take that cost.
				newMinCost = beforeOptCost;
				if(oldMinCost < newMinCost)
					newMinCost = oldMinCost;
				cout<<"$$$$$$$$$ Error: FPC Error. $$$$$$$$$$$\n";
			}
			assert(numOldPlans <= Nipqo_t::NumPlans());
			assert(0 < optPlan); assert(optPlan <= Nipqo_t::NumPlans());

			if(numOldPlans < Nipqo_t::NumPlans()) {
				// Nothing doing here?
				assert((numOldPlans + 1) == Nipqo_t::NumPlans());
				assert(optPlan == Nipqo_t::NumPlans());
#ifdef ANIPQO_DEBUG
				if(Nipqo_t::PrintTrace())
					cout << "New plan found, plan num: " << optPlan << endl;
#endif
			}

			if(vertex->MinCostPlanListContains(optPlan)) {
#ifdef ANIPQO_DEBUG
				if(Nipqo_t::PrintTrace())
					cout << "New Optimal plan "<<optPlan<<" is already in CSOP. So, no further processing as is a vertex of the final cost polytope.\n\n";
#endif
				continue; // Is this right?
			}

			//Chetas- Added for Correct version.
//			CostVal_t newMinCost = vertex->OptCost();
			assert(newMinCost <= oldMinCost);

			float pDiffC = (oldMinCost - newMinCost)/newMinCost;
			assert(0 <= pDiffC);

			if(pDiffC <= PD_THRESHOLD) { // replace PD_THRESHOLD by nVertex->GetDiffThreshold ?
#ifdef ANIPQO_DEBUG
				if(Nipqo_t::PrintTrace())
					cout << "New Optimal plan "<<optPlan<<" of optCost "<<newMinCost<<" is co-optimal with oldMinCost "<<oldMinCost<<" & hence not added to CSOP; no further processing of the vertex.\n\n";
#endif
				//Chetas- Added for Aggressive version.*********
/*				CostVal_t tmpCost = vertex->GetMaxOptCost();
				float newDiffC = (tmpCost - newMinCost)/newMinCost;
				if(newDiffC > vertex->GetDiffThreshold()) {
					pDiffC = newDiffC;
					assert(0 <= pDiffC);
					vertex->UpdateMinCostPlanList(pDiffC);
				}
				else
					vertex->UpdateMinCostPlanList();*/
				//*******************
//				vertex->UpdateMinCostPlanList();	//Chetas- Commented for Correct version.
				continue; // Is this right?
			}
			//Chetas- Added for Correct version.*************
			else {
				vertex->GetPlanCost(optPlan, newMinCost);		//Chetas. If new plan is not co-optimal, then this plan should be added to CSOP.
				cout<<"New optimal plan "<<optPlan<<" is not co-optimal. Hence added to CSOP.\n\n";
				vertex->resetDiffThreshold();		//Chetas- Added for Correct version.
				if(!POSP.Contains(optPlan))			//Chetas- Added for computing error
					POSP.Append(optPlan);			//Chetas- Added for computing error
				newPlanID = optPlan;
			}
			//***********************************************

			keepLooping = TRUE;
			UnmarkVisitedAllPartitioningVertices();

			AppendList_t<Edge_t *> conflictingEdges;
			FindConflictingEdges(vertex, optPlan, conflictingEdges);

			ListIter_t<Edge_t *> edgeIter;
			edgeIter.Attach(&conflictingEdges);

			while(!edgeIter.IsEnd()) {
				Edge_t *edge = edgeIter.Next();
				assert(edge);
				Optimize(edge, optPlan);
			}

			//Chetas- Freeing memory
			edgeIter.Reset();
			while(!edgeIter.IsEnd())
				delete edgeIter.Next();

			//Chetas- Added for Correct Version.***************
			if(falsePositivePartitioningVertices.Size() > 0) {
				ListIter_t<Point_t *> itr;
				itr.Attach(&falsePositivePartitioningVertices);
				while(!itr.IsEnd()) {
					Point_t *FPVertex = itr.Next();
					Point_t *partVert = GetFromPartitioningList(FPVertex);
					if(partVert != NULL && partVert != FPVertex) {
						MyList_t<PlanId_t> list;
						partVert->UpdateMinCostPlanList();
						FPVertex->CommonMinCostPlansList(partVert, list);
						if(list.Size() == FPVertex->MinCostPlansListSize()) {
							FPVertex->SetToBeDeleted();
							cout<<"Point ";
							FPVertex->PrintIntCoord();
							cout<<" with same plans is already there. So, deleting false positive point.\n";
						}
						else {
							cout<<"Point ";
							FPVertex->PrintIntCoord();
							cout<<" is already there. But plans are not same. FP point has "<<FPVertex->MinCostPlansListSize()
									<<" & partVert has "<<list.Size()<<"\n";
							ListIter_t<PlanId_t> itr;
							itr.Attach(&FPVertex->MinCostPlans());
							while(!itr.IsEnd()) {
								PlanId_t planId = itr.Next();
								if(!partVert->MinCostPlanListContains(planId))
									partVert->GetPlanCost(planId);
							}
							FPVertex->SetToBeDeleted();
//							assert(0);
						}
						continue;
					}
					AppendDelList_t<Point_t *> neighbors;
					FindNeghborsOnBoundary(FPVertex, neighbors);
//					assert(neighbors.Size() >= 3);
					Point_t *newPt = FPVertex->adjustPartitioningVertex();
					assert(newPt);
					FPVertex->SetToBeDeleted();
					AddPartitioningVertex(newPt);
					cout<<"False positive vertex ";
					FPVertex->PrintIntCoord();
					cout<<" is now adjusted with vertex ";
					newPt->PrintIntCoord(TRUE);
				}
				falsePositivePartitioningVertices.MakeEmpty();
			}
			//*************************************************

			DeleteToBeDeletedVertices();
			UpdateMinPlanListsOfAllPartitioningVertices();
			UnmarkVisitedAllPartitioningVertices();
			(Nipqo_t::VertexCache()).freeHashset(&partitioningVertices);		//Chetas- Freeing memory.
			Nipqo_t::isFirstOptimization = false;
			break;
		}
	}

//	getrusage(RUSAGE_SELF, &rusageEnd);

	Boolean_t *planFlags = new Boolean_t[Nipqo_t::NumPlans()+1];
	assert(planFlags);

	planFlags[0] = FALSE;
	int fDash = Nipqo_t::NumPlans();

	for(int i = 1; i <= Nipqo_t::NumPlans(); i++) {
		planFlags[i] = TRUE;

		partVertListIter.Reset();
		while(!partVertListIter.IsEnd()) {
			Point_t *vertex = partVertListIter.Next();
			assert(vertex);

			assert(TRUE == planFlags[i]);

			if((vertex->IsParameterSpaceHyperRectangleVertex())
					&& (vertex->MinCostPlanListContains(i))) {
				planFlags[i] = FALSE;
				fDash--;
				break;
			}
		}
	}

	cout << endl;
	cout << "Final partitioning vertices [Num = "
			<< partitioningVertices.Size() << "]" << endl;
	PrintPartitioningVertices();
	cout << endl;
	cout << "Final POSP [Num = "
			<< Nipqo_t::NumPlans() << ", fDash = "
			<< fDash << "]" << endl;

	for(int i = 1; i <= Nipqo_t::NumPlans(); i++) {
		if(planFlags[i]) cout << "1";
		else cout << "0";
	}
	cout << endl;
//	cout << "NumOptCalls: " << OptimizerInterface_t::NumOptCalls()
//	<< " NumEvalCalls: " <<  OptimizerInterface_t::NumEvalCalls() << endl;
	cout<<"NumOptCalls: "<<Plan::numOptCalls()<<".\nNumEvalCalls: "<<Plan::numEvalCalls()<<endl;		//Chetas- Replaced with above line.

	/* Chetas- Added to show plans. */
//	cout<<"Optimal plans are:\n";
//	for(int i = 1; i <= Nipqo_t::NumPlans(); i++) {
//		Plan *plan = Plan::getPlanById(i);
//		plan->showPlan();
//	}
	graph = new Graph(this);
//	graph->showGraph();
	allRegions = new AppendDelList_t<Polygon_t *>();
	graph->extractRegions(allRegions);

	cout<<"\n"<<allRegions->Size()<<" regions found.\n";

//	ListIter_t<Polygon_t *> regionIter;
//	regionIter.Attach(allRegions);
//	int cnt = 1;
//	while(!regionIter.IsEnd()) {
//		cout<<"\nRegion "<<cnt++<<" : \n";
//		regionIter.Next()->showPolygon();
//	}

//	ListIter_t<Point_t *> templist;
//	templist.Attach(&graph->vertices);
//	while(!templist.IsEnd()) {
//		Point_t *pt = templist.Next();
//		ListIter_t<Polygon_t *> regIter;
//		regIter.Attach(allRegions);
//		int cnt = 1;
////		cout<<"In Nipqo: \nCurrent point is ";
////		pt->PrintIntCoord(TRUE);
//		while(!regIter.IsEnd()) {
//			Polygon_t *reg = regIter.Next();
//			if(reg->contains(pt)) {
//				cout<<"Point ";
//				pt->PrintIntCoord();
//				cout<<" is in region "<<cnt<<endl;
//			}
//			else {
//				cout<<"Point ";
//				pt->PrintIntCoord();
//				cout<<" is not in region "<<cnt<<endl;
//			}
//			cnt++;
//		}
//	}

	ListIter_t<Polygon_t *> regIter;
	regIter.Attach(allRegions);
	int cnt = 1;
	while(!regIter.IsEnd()) {
		Polygon_t *region = regIter.Next();
		region->initializeOptPlanList();
		if(region->optPlans.Size() == 0)
			region->initializeAllPlanList(Nipqo_t::NumPlans());
		if(region->optPlans.Size() > 0) {
			cout<<"Region "<<cnt++<<" has common plans = ";
			assert(region->optPlans.Size() <= 2);
			for (int i = 0; i < region->optPlans.Size(); i++)
				cout<<region->optPlans.Entry(i)<<", ";
			cout<<endl;
			region->showPolygon();
			cout<<endl;
		}
		else {
			cout<<"Region "<<cnt++<<" has no any common plans. So all plans are = ";
			for (int i = 0; i < region->allPlans.Size(); i++)
				cout<<region->allPlans.Entry(i)<<", ";
			cout<<endl;
			region->showPolygon();
			cout<<endl;
		}
		cout<<endl;
	}
	regIter.Reset();
	bool toRepeat = false;
	while(!regIter.IsEnd()) {
		Polygon_t *reg = regIter.Next();
		if(reg->optPlans.Size() == 2) {
			toRepeat = true;
			PlanId_t p1 = reg->optPlans.Entry(0), p2 = reg->optPlans.Entry(1);
			ListIter_t<Polygon_t *> itr2;
			itr2.Attach(allRegions);
			while(!itr2.IsEnd()) {
				Polygon_t *reg2 = itr2.Next();
				if(reg2 != reg && reg2->optPlans.Size() == 1) {
					if(reg2->optPlans.Contains(p1)) {
						reg->optPlans.DeleteEntry(p2);
						break;
					}
					else if(reg2->optPlans.Contains(p2)) {
						reg->optPlans.DeleteEntry(p1);
						break;
					}
				}
			}
			if(reg->optPlans.Size() == 2)
				reg->optPlans.DeleteEntry(p1);
		}
	}
	if(toRepeat) {
		cout<<endl<<endl<<endl<<"Repeating after cleaning regions.\n\n\n\n";
		int cnt = 1;
		regIter.Reset();
		while(!regIter.IsEnd()) {
			Polygon_t *region = regIter.Next();
			if(region->optPlans.Size() > 0) {
				cout<<"Region "<<cnt++<<" has common plans = ";
				assert(region->optPlans.Size() <= 2);
				for (int i = 0; i < region->optPlans.Size(); i++)
					cout<<region->optPlans.Entry(i)<<", ";
				cout<<endl;
				region->showPolygon();
				cout<<endl;
			}
			else
				cnt++;
			cout<<endl;
		}
	}

	if(vertexNotLocatedErrorsInSmallRect) {
		cout<<vertexNotLocatedErrorsInSmallRect<<" times equicost point couldn't find in small rectangle.\n";
		cout<<vertexNotLocatedErrorsInBigRect<<" times equicost point couldn't find in big rectangle.\n";
	}
	if(falsePositivePointsDetected)
		cout<<falsePositivePointsDetected<<" number of false positive centers detected.\n";

	bool allPlans[Nipqo_t::NumPlans()];
	for(int i = 0; i < Nipqo_t::NumPlans(); i++)
		allPlans[i] = false;
	ListIter_t<Point_t *> partItr;
	partItr.Attach(&partitioningVertices);
	while(!partItr.IsEnd()) {
		Point_t *vert = partItr.Next();
		for(int i = 0; i < Nipqo_t::NumPlans(); i++) {
			if(!allPlans[i] && vert->MinCostPlanListContains(i+1))
				allPlans[i] = true;
		}
	}
	for(int i = 0; i < Nipqo_t::NumPlans(); i++)
		if(allPlans[i])
			CSOPsize++;

	cout<<CSOPsize<<" plans are in CSOP.\n";
	cout<<"AniPQO finished successfully.\n";

	//Chetas- Get used ram size.***
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);
	cout<<"RAM currently used in GBs = "<<(float) (status.ullTotalPhys - status.ullAvailPhys) / (1024 * 1024 * 1024);
	//********

	delete planFlags;
}

void pOptimizer_t::UpdateMinPlanListsOfAllPartitioningVertices(void)
{
	ListIter_t<Point_t *> partVertListIter;
	partVertListIter.Attach(&partitioningVertices);

	while(!partVertListIter.IsEnd()) {
		Point_t *vertex = partVertListIter.Next();
		assert(vertex);
		vertex->UpdateMinCostPlanList();
	}
}

void pOptimizer_t::DeleteToBeDeletedVertices(void)
{
	ListIter_t<Point_t *> partVertListIter;
	partVertListIter.Attach(&partitioningVertices);

	Boolean_t atLeastOneVertexDeleted = FALSE;

	while(TRUE) {
		Boolean_t found = FALSE;
		Point_t *vertex = NULL;
		partVertListIter.Reset();

		while(!partVertListIter.IsEnd()) {
			vertex = partVertListIter.Next();
			assert(vertex);
			if(vertex->IsToBeDeleted()) {
//				vertex->UpdateMinCostPlanList();				//Chetas- Added for incorrect deletions
//				if(vertex->MinCostPlansListSize() == 1) {		//Chetas- Added for incorrect deletions
					found = TRUE; // Cant delete it here;
					break;        // Otherwise iterator goes haywire :-(
//				}												//Chetas- Added for incorrect deletions
			}
		}

		if(FALSE == found) {
			/****************************************************************************/
			// assert(TRUE == atLeastOneVertexDeleted); // commented to accomodate new method two
			/****************************************************************************/

			return;
		}

		assert(vertex);
		partitioningVertices.Delete(vertex);
		atLeastOneVertexDeleted = TRUE;
	}
}

void pOptimizer_t::PrintPartitioningVertices(void)
{
	cout << "Partitioning Vertices: " << endl;

	ListIter_t<Point_t *> partVertListIter;
	partVertListIter.Attach(&partitioningVertices);

	while(!partVertListIter.IsEnd()) {
		Point_t *vertex = partVertListIter.Next();
		assert(vertex);
		vertex->PrintIntCoord();
		cout << "Plan Costs: " ;
		vertex->PrintPlanCosts();
		cout << "Min Plan Costs: " ;
		vertex->PrintMinCostPlans();
		if(vertex->IsOptimized()) cout << " Optimized. ";
		cout<<"Threshold = "<<vertex->GetDiffThreshold();
		cout << endl;
	}

	cout << endl;
}

void pOptimizer_t::Optimize(Edge_t *edge, PlanId_t plan)
{
#ifdef ANIPQO_DEBUG
	static int invocationNum = 0;
	if(Nipqo_t::PrintTrace())
		cout << "pOptimizer_t::Optimize(Edge_t, PlanId_t) invocation # = " << ++invocationNum << endl;
#endif
	assert(edge);
	assert(0 < plan);
	assert(plan <= Nipqo_t::NumPlans());

	Point_t *vertex1 = edge->EndPoint1();
	Point_t *vertex2 = edge->EndPoint2();
	assert(vertex1); assert(vertex2);

	MyList_t<PlanId_t>  planList;
	vertex1->CommonMinCostPlansList(vertex2, planList);
	// assert(NumVaryingParam() <= planList.Size()); // moved to Optimize(vertex, planList)
	assert(FALSE == planList.Contains(plan));

	//Chetas- Added for Correct version. When equicost point is one of the vertices of the conflict edge, then don't search it...***********
//	int varyingParam = 0;
//	for(int i = 0; i < Nipqo_t::NumParam(); i++)
//		if(vertex1->GetIntCoord(i) != vertex2->GetIntCoord(i))
//			varyingParam++;
//
//	if(varyingParam > 1) {
//		CostVal_t maxCost1, maxCost2, planCost1, planCost2;
//		float pDiffC1, pDiffC2, diffThres1, diffThres2;
//
//		maxCost1 = vertex1->GetMaxCost(&planList);
//		maxCost2 = vertex2->GetMaxCost(&planList);
//		planCost1 = vertex1->GetPlanCost(plan);
//		planCost2 = vertex2->GetPlanCost(plan);
//		pDiffC1 = maxCost1 > planCost1 ? (maxCost1 - planCost1)/planCost1 : (planCost1 - maxCost1)/maxCost1;
//		pDiffC2 = maxCost2 > planCost2 ? (maxCost2 - planCost2)/planCost2 : (planCost2 - maxCost2)/maxCost2;
//		diffThres1 = vertex1->GetDiffThreshold();
//		diffThres2 = vertex2->GetDiffThreshold();
//
//		if((pDiffC1 <= diffThres1 && diffThres1 <= PD_THRESHOLD * PD_THRESHOLD)
//				|| (pDiffC2 <= diffThres2 && diffThres1 <= PD_THRESHOLD * PD_THRESHOLD)) {
//			cout<<"Equicost point is one of vertices of the conflict edge. pDiffc1 = "<<pDiffC1<<", diffThresh1 = "<<diffThres1<<", pDiffc2 = "<<pDiffC2<<", diffThresh2 = "<<diffThres2<<".";
//			vertex1->PrintIntCoord();
//			vertex2->PrintIntCoord();
//			cout<<endl<<endl;
//			if(FALSE == localPlans.Contains(plan))
//				localPlans.Append(plan); // No need of ordered insertions.
//			return;
//		}
//	}
	//*************************************

	planList.Append(plan);

	// assert(FALSE == localPlans.Contains(plan));
	if(FALSE == localPlans.Contains(plan))
		localPlans.Append(plan); // No need of ordered insertions.

	int oldNumPartVertices = partitioningVertices.Size();
#if 0
	CostVal_t minCost1 = vertex1->AuxMinCost();
	CostVal_t minCost2 = vertex2->AuxMinCost();
	CostVal_t planCost1 = vertex1->GetPlanCost(plan);
	CostVal_t planCost2 = vertex2->GetPlanCost(plan);

	CostVal_t diff1 = minCost1 - planCost1;
	CostVal_t diff2 = planCost2 - minCost2;
	assert(0 < diff1);
	assert(0 < diff2);

	float interpolationRatio = diff1/(diff1 + diff2);
	Point_t *vertex = GetInterpolatedPoint(vertex1, vertex2, interpolationRatio);
	assert(vertex);

	Optimize(vertex, planList);
#else
	Rectangle_t *rect = new Rectangle_t(this, vertex1, vertex2);
	assert(rect);
	assert(rect->NumVaryingParam() < planList.Size());		//Chetas- Why this assertion is not a just if condition?
//#ifdef ANIPQO_DEBUG
//	if(Nipqo_t::PrintTrace())
//		rect->PrintVertices(TRUE);
//#endif
	assert(rect->NumVaryingParam() < rect->NumOptPlans(planList));
	Boolean_t vertexLocated;
	vertexLocated = rect->OptimizeMethodTwo(&planList);
	assert((TRUE == vertexLocated) || (FALSE == vertexLocated));

	//Chetas- Added for Correct version. ******
	if(vertexLocated == FALSE && (rect->NumVaryingParam() + 1) < rect->NumOptPlans(planList)) {
		vertexNotLocatedErrorsInSmallRect++;
		cout<<"$$$$$$$$$ Error: Couldnt locate the equicost vertex because conflict edge has some more plans than required. $$$$$$$$$$$$\n"<<endl;

		CostVal_t newPlanCost = vertex1->GetPlanCost(plan);
		if(newPlanCost > vertex1->GetMinCost()) {		//If new plan's cost is definitely lesser than oldMinCostPlan. If it is not then this is the neighbor point.
			PlanId_t maxCostPlan1 = vertex1->GetMaxOptCostPlan(&planList, plan);
			planList.DeleteEntry(maxCostPlan1);
		}
		else {
			PlanId_t maxCostPlan2 = vertex2->GetMaxOptCostPlan(&planList, plan);
			planList.DeleteEntry(maxCostPlan2);
		}
		vertexLocated = rect->OptimizeMethodTwo(&planList);
		assert((TRUE == vertexLocated) || (FALSE == vertexLocated));
	}
	delete rect;	//Chetas- Freeing memory;
	//*****************

#endif
	int newNumPartVertices = partitioningVertices.Size();
	// assert(oldNumPartVertices <= newNumPartVertices);
	assert((oldNumPartVertices == newNumPartVertices)
			|| ((oldNumPartVertices + 1) == newNumPartVertices));

	if(FALSE == vertexLocated) {
		vertexNotLocatedErrorsInSmallRect++;
		// if(oldNumPartVertices == newNumPartVertices)
#ifdef ANIPQO_DEBUG
		if(Nipqo_t::PrintTrace())
			cout << "$$$$$$$$$$ Error: Couldnt locate the new partitioning vertex. $$$$$$$$$$$$\n" << endl;
#endif
#if 1
#ifdef ANIPQO_DEBUG
		if(Nipqo_t::PrintTrace())
			cout << "Attempting BIG rectangle" << endl;
#endif
		Rectangle_t *rect = new Rectangle_t(this, vertex1, vertex2, TRUE);
		assert(rect);
		assert(rect->NumVaryingParam() < planList.Size());
#ifdef ANIPQO_DEBUG
		if(Nipqo_t::PrintTrace())
			rect->PrintVertices(TRUE);
#endif
		/*************************************************************/
		// assert(rect->NumVaryingParam() < rect->NumOptPlans(planList));
		if(rect->NumOptPlans(planList) <= rect->NumVaryingParam()) {
			vertexNotLocatedErrorsInBigRect++;
#ifdef ANIPQO_DEBUG
			if(Nipqo_t::PrintTrace())
				cout << "$$$$$$$$$$$$ Error: BIG rectangle failed as rect->NumOptPlans(planList) <= rect->NumVaryingParam(). $$$$$$$$$$$$\n" << endl;
#endif
			return;
		}
		/*************************************************************/

		vertexLocated = rect->OptimizeMethodTwo(&planList);
		assert((TRUE == vertexLocated) || (FALSE == vertexLocated));

		newNumPartVertices = partitioningVertices.Size();
#if 1
		assert((oldNumPartVertices == newNumPartVertices)
				|| ((oldNumPartVertices + 1) == newNumPartVertices));
		if(FALSE == vertexLocated)
			vertexNotLocatedErrorsInBigRect++;
#ifdef ANIPQO_DEBUG
		if(Nipqo_t::PrintTrace())
			if(FALSE == vertexLocated) {
				cout << "$$$$$$$$$$$$ Error: Even BIG rectangle couldnt locate the new partitioning vertex. $$$$$$$$$$$$\n" << endl;
			}
#endif
#else
		if(oldNumPartVertices == newNumPartVertices) {
			cout << "Even BIG rectangle couldnt locate the new partitioning vertex" << endl;
		}
		else assert((oldNumPartVertices + 1) == newNumPartVertices);
#endif
#endif
		delete rect;
	}
#if 0
	else
		assert((oldNumPartVertices + 1) == newNumPartVertices);
#endif
}

void pOptimizer_t::Optimize(Point_t *point, MyList_t<PlanId_t> &planList)
{
	float rectSide = Nipqo_t::MinSquareSize();
	Rectangle_t *rect = new Rectangle_t(this, point, rectSide);
	assert(rect);
	assert(rect->NumVaryingParam() < planList.Size());
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace())
		rect->PrintVertices(TRUE);
#endif
	// while(rect->NumOptPlans(planList) <= rect->NumVaryingParam()) {
	while(rect->NumOptPlans(planList) < planList.Size()) {
		if(FALSE == rect->IsExpandable()) {
			assert(0);
		}

		rectSide *= 2;
		if(1 < rectSide) {
			assert(0);
		}

		delete rect;
		rect = new Rectangle_t(this, point, rectSide);
		assert(rect);
	}

	// rect->Optimize(&rect->MinPlanCostList());
	rect->OptimizeMethodTwo(&planList);
}

Point_t *pOptimizer_t::GetInterpolatedPoint(Point_t *point1, Point_t *point2, float ratio)
{
	assert(point1);
	assert(point2);
	assert(point1 != point2);

	assert(0 < ratio);
	assert(ratio < 1);

	RealCoord_t *interpolation = new RealCoord_t[Nipqo_t::NumParam()];
	assert(interpolation);

	for(int i = 0; i < Nipqo_t::NumParam(); i++) {
		// interpolation[i] = ratio * point1->GetRealCoord(i)
		// 		    + (1 - ratio) * point2->GetRealCoord(i);
		interpolation[i] = point1->GetRealCoord(i) +
				+ (point2->GetRealCoord(i) - point1->GetRealCoord(i)) * ratio;
		assert(((point1->GetRealCoord(i) == point2->GetRealCoord(i))
				&& (point1->GetRealCoord(i) == interpolation[i]))
				|| ((point1->GetRealCoord(i) < point2->GetRealCoord(i))
						&& (point1->GetRealCoord(i) < interpolation[i])
						&& (interpolation[i] < point2->GetRealCoord(i)))
						|| ((point2->GetRealCoord(i) < point1->GetRealCoord(i))
								&& (point2->GetRealCoord(i) < interpolation[i])
								&& (interpolation[i] < point1->GetRealCoord(i))));
	}

	Point_t *interpolationPoint =  (Nipqo_t::VertexCache()).Get(interpolation);
	if(NULL == interpolationPoint) {
		interpolationPoint = new Point_t(interpolation);
		assert(interpolationPoint);
		(Nipqo_t::VertexCache()).Put(interpolationPoint);
	}

	delete interpolation;
	return interpolationPoint;
}

void pOptimizer_t::OptimizeForMethodOne(void)
{
	while(TRUE) {
		int oldNumPlans = Nipqo_t::NumPlans();

		PlanId_t optPlan = rect->OptimizeMethodOne();
		if(0 == optPlan) {
			assert(Nipqo_t::NumPlans() == oldNumPlans);
			break;
		}

		assert(0 < optPlan); assert(optPlan <= Nipqo_t::NumPlans());
		assert(FALSE == localPlans.Contains(optPlan));
		localPlans.Append(optPlan); // No need of ordered insertions.
	}
}

void pOptimizer_t::MergePartitioningVertices(pOptimizer_t *facetOpt)
{
	assert(facetOpt);

	ListIter_t<Point_t *> partVertListIter;
	partVertListIter.Attach(&facetOpt->partitioningVertices);

	while(!partVertListIter.IsEnd()) {
		Point_t *vertex = partVertListIter.Next();
		assert(vertex);

		if(FALSE == partitioningVertices.Contains(vertex))
			partitioningVertices.Insert(vertex);
	}
}

void pOptimizer_t::Optimize(void)
{
	assert(NULL == rect);

	if(2 == Nipqo_t::MethodNum()) {
		assert(NULL == rect);
		assert(Nipqo_t::NumParam() == numVaryingParam);
		assert(0 == NumLocalPlans());

		assert(0 == partitioningVertices.Size());
		AssignVertices();

		OptimizeForMethodTwo();
		return;
	}

	assert(1 == Nipqo_t::MethodNum());

	if(0 == numVaryingParam) {
		rect = new Rectangle_t(this);
		PlanId_t planId = rect->OptimizeMethodOne();
		assert(0 < planId); assert(planId <= Nipqo_t::NumPlans());
		assert(0 == localPlans.Size());
		localPlans.Append(planId);
		assert(1 == localPlans.Size());
		assert(partitioningVertices.IsEmpty());
		rect->InsertVerticesIntoList(partitioningVertices); // Check if this is really necessary.
		UpdateMinPlanListsOfAllPartitioningVertices();
		return;
	}

	int i;

	RealCoord_t *facetConstPred = new RealCoord_t[Nipqo_t::NumParam()];
	for(i = 0; i < Nipqo_t::NumParam(); i++) facetConstPred[i] = constPred[i];

	assert(0 == NumLocalPlans());

	for(i = 0; i < Nipqo_t::NumParam(); i++) {
		if(-1 == facetConstPred[i]) {
			facetConstPred[i] = 0;
			pOptimizer_t *facetOpt = Nipqo_t::Optimize(facetConstPred);
			assert(facetOpt);
			localPlans.Merge(facetOpt->localPlans, Nipqo_t::NumPlans());
			MergePartitioningVertices(facetOpt);

			facetConstPred[i] = 1;
			facetOpt = Nipqo_t::Optimize(facetConstPred);
			assert(facetOpt);
			localPlans.Merge(facetOpt->localPlans, Nipqo_t::NumPlans());
			MergePartitioningVertices(facetOpt);

			facetConstPred[i] = -1;
		}
	}

	delete facetConstPred;
	assert(0 < NumLocalPlans());

	assert(0 < numVaryingParam);
	assert(numVaryingParam <= Nipqo_t::NumParam());

	if((FALSE == Nipqo_t::OptimizeIntermediateDimensions())
			&& (numVaryingParam < Nipqo_t::NumParam())) {
		return;
	}

	rect = new Rectangle_t(this);
	// assert(partitioningVertices.IsEmpty());
	// rect->InsertVerticesIntoList(partitioningVertices); // Check if this is really necessary.

	if(2 == Nipqo_t::MethodNum()) {  // Not used
		assert(0);
		OptimizeForMethodTwo();
		return;
	}

	assert(1 == Nipqo_t::MethodNum());
	OptimizeForMethodOne();
}

void pOptimizer_t::AssignVertices(void)
{
	int i;
	assert(0 == partitioningVertices.Size());

	RealCoord_t *vertex = new RealCoord_t[Nipqo_t::NumParam()];
	assert(vertex);

	for(i = 0; i < Nipqo_t::NumParam(); i++) {
		vertex[i] = 0;
	}

	int numVertices = 0;

	while(1) {
		Point_t *parameterSpaceRectVertex = (Nipqo_t::VertexCache()).Get(vertex);
		assert(NULL == parameterSpaceRectVertex);

		parameterSpaceRectVertex = new Point_t(vertex);
		assert(parameterSpaceRectVertex);
		(Nipqo_t::VertexCache()).Put(parameterSpaceRectVertex);
		partitioningVertices.Insert(parameterSpaceRectVertex);

		numVertices++;

		for(i = 0; i < Nipqo_t::NumParam(); i++) {
			if(0 == vertex[i]) {
				vertex[i] = 1;
				break;
			}
			else {
				assert(vertex[i] == 1);
				vertex[i] = 0;
			}
		}

		assert(i <= Nipqo_t::NumParam());
		if(i == Nipqo_t::NumParam()) break;
	}

	delete vertex;
	assert(Utility_t::TwoPowerN(numVaryingParam) == numVertices);
	assert(partitioningVertices.Size() == numVertices);
}

pOptimizer_t *Nipqo_t::Optimize(RealCoord_t *constPred) // = NULL
{
	int optNum = GetOptNum(constPred);

	clock_t tic = clock(), toc;
	isFirstOptimization = true;
	if(NULL == pOpts[optNum]) {
		pOpts[optNum] = new pOptimizer_t(constPred, optNum);
		assert(pOpts[optNum]);
		pOpts[optNum]->Optimize(); // Put inside pOptimizer_t constructor
	}
	toc = clock();
	timeForAnipqo = (double)(toc - tic);
	cout << "\nTime taken by Anipqo to finish : " << timeForAnipqo/CLOCKS_PER_SEC << " secs.\n";

	return pOpts[optNum];
}

#if 0
int main(int argc, char *argv[])
{
	if( argc != 3 ) {
		printf("Usage: %s numParam RelSizeFile\n", argv[0]);
		exit(0);
	}

	if(FALSE == Nipqo_t::Initialize(argc, argv)) exit(0);
	Utility_t::Initialize();
	Nipqo_t::Optimize();
}
#endif

void Nipqo_t::PQO(int argc, char *argv[])
{
	if(FALSE == Nipqo_t::Initialize(argc, argv)) {
		printf("Nipqo failed\n");
		exit(0);
	}

//	assert(Nipqo_t::NumParam() == Parameter_t::NumParam());		//Chetas not needed.
	Utility_t::Initialize();

//	getrusage(RUSAGE_SELF, &rusageBeg);

//	OptimizerInterface_t::Initialize();
	DBConn::createObjs();		//Chetas- Replaced with above line.

	//Chetas- Added for computing errors. **********************
//	float allThresholds[] = {0.01, 0.05, 0.1, 0.2};		//Chetas- No Index
	float allThresholds[] = {0.005};		//Chetas- No Index
//	float allThresholds[] = {0.1, 0.2};					//Chetas- Index
	string mode(argv[12]);

	if(mode == "debug" || mode == "Debug") {
	/*	string file(argv[8]);
		file = "../" + file + ".txt";
		ofstream fout(file.c_str());
		streambuf *coutbuf = cout.rdbuf();
		cout.rdbuf(fout.rdbuf());*/

		PD_THRESHOLD = atof(argv[10]);
		vertexCache = HashSet_t();
		pOptimizer_t *pOpt = Nipqo_t::Optimize();
		vertexCache.freeHashset();

		pOpt->graph->deleteGraph();
		delete pOpt->graph;
		ListIter_t<Polygon_t *> regionItr;
		regionItr.Attach(pOpt->allRegions);
		while(!regionItr.IsEnd())
			delete regionItr.Next();
		delete pOpt->allRegions;

		delete pOpt;						//Chetas- Freeing memory
		/*cout.rdbuf(coutbuf);
		cout<<"AniPQO finished successfully.\n";*/
	}
	else if(mode == "release" || mode == "Release"){
		string prefix("../outputs/NoIndices/");			//Chetas- No Index
//		string prefix("../outputs/AllIndices/");		//Chetas- Index
		string file(argv[8]);
		file = prefix + file + "_";
		streambuf *coutbuf;

		cout<<"Optimizing all vertices.\n";
		optimizeAll();
		cout<<"Plan diagram has "<<NumPlans()<<" plans.\n";
		cout<<"Done.\nStarting Anipqo.\n";
		int size = sizeof(allThresholds) / sizeof(allThresholds[0]);
		for(int i = 0; i < size; i++) {
			PD_THRESHOLD = allThresholds[i];
			char *buf;

			switch(i) {					//Chetas- No Index
			case 0:	buf = "05";
//			case 0:	buf = "1";
//			break;
//			case 1:	buf = "5";
//			break;
//			case 2:	buf = "10";
//			break;
//			case 3:	buf = "20";
			}
//			switch(i) {					//Chetas- Index
//			case 0:	buf = "10";
//			break;
//			case 1:	buf = "20";
//			}
			string threshold(buf);
			string outputFile = file + threshold + ".txt";
			string matlabFile = file + threshold + ".m";
			string errorFile = file + threshold + "errlog.csv";
			string inferFile = file + threshold + "inferlog.csv";

			ofstream fout(outputFile.c_str());
			streambuf *coutbuf = cout.rdbuf();
			cout.rdbuf(fout.rdbuf());

			vertexCache = HashSet_t();
			Plan::resetOptCalls();
			Plan::resetEvalCalls();
			pOptimizer_t *pOpt = Nipqo_t::Optimize();
			FILE *matlabFP = fopen((char *)matlabFile.c_str(), "w");
			assert(matlabFP);
			FILE *errorFP = fopen((char *)errorFile.c_str(), "w");
			assert(errorFP);
			FILE *inferFP = fopen((char *)inferFile.c_str(), "w");
			assert(inferFP);
			fprintf(matlabFP, "hold all;\n");
			writeRegions(pOpt, matlabFP);
			computeError(pOpt, matlabFP, inferFP, errorFP);
			fprintf(matlabFP, "hold off;\n");
			fclose(matlabFP);
			fclose(errorFP);
			vertexCache.freeHashset();

			pOpt->graph->deleteGraph();
			delete pOpt->graph;
			ListIter_t<Polygon_t *> regionItr;
			regionItr.Attach(pOpt->allRegions);
			while(!regionItr.IsEnd())
				delete regionItr.Next();
			delete pOpt->allRegions;

			delete pOpt;						//Chetas- Freeing memory
			pOpts[0] = NULL;
			cout.rdbuf(coutbuf);
			cout<<"AniPQO finished successfully for threshold = "<<PD_THRESHOLD<<"\n";
		}

		for(int i = 0; i < resolution; i++)
			for(int j = 0; j < resolution; j++)
				delete allVertices[i][j];
		for(int i = 0; i < resolution; i++)
			delete allVertices[i];
		delete allVertices;
	}
	else
		assert(0);

	DBConn::destroyObjs();				//Chetas- Freeing memory
	Plan::deleteSelectivityConsts(2);	//Chetas- Freeing memory
	PlanInfo::deleteObj();				//Chetas- Freeing memory
	delete pOpts;						//Chetas- Freeing memory
	//**********************************************************

//	OptimizerInterface_t::Destroy();

	//Chetas
//	int utimetaken =
//			(rusageEnd.ru_utime.tv_sec-rusageBeg.ru_utime.tv_sec)*1000 +
//			(rusageEnd.ru_utime.tv_usec-rusageBeg.ru_utime.tv_usec)/1000;
//	if (utimetaken < 10) utimetaken = 10; // < 10 msec must be noise!
//	int stimetaken =
//			(rusageEnd.ru_stime.tv_sec-rusageBeg.ru_stime.tv_sec)*1000 +
//			(rusageEnd.ru_stime.tv_usec-rusageBeg.ru_stime.tv_usec)/1000;
//
//	cout << "USRT " << utimetaken << endl;
//	cout << "SYST " << stimetaken << endl;
}

void Nipqo_t::optimizeAll(void) {
	allVertices = new Point_t**[resolution];
	for(int i = 0; i < resolution; i++)
		allVertices[i] = new Point_t*[resolution];

	clock_t tic = clock(), toc;
	for(int i = 0; i < Nipqo_t::resolution; i++) {
		for(int j = 0; j < Nipqo_t::resolution; j++) {
			int temp[2];
			temp[0] = i;
			temp[1] = j;
			Point_t *vertex = new Point_t(temp);
			assert(vertex != NULL);
			allVertices[i][j] = vertex;

			//optimize vertex.
			CostVal_t optCost = 0;
			PlanId_t optPlan = 0;
			optPlan = vertex->Optimize(&optCost);
			assert(optPlan > 0);
//			vertex->GetPlanCost(optPlan, optCost);
		}
		cout<<i+1<<", ";
	}
	cout<<endl;
	toc = clock();
	timeToOptimizeAll = (double)(toc - tic);
	cout << "\nTime taken to optimize all points: " << timeToOptimizeAll/CLOCKS_PER_SEC << " secs.\n";
}

void Nipqo_t::writeRegions(pOptimizer_t *pOpt, FILE *fp)
{
	ListIter_t<Polygon_t *> regItr;
	regItr.Attach(pOpt->allRegions);
	while(!regItr.IsEnd()) {
		Polygon_t *reg = regItr.Next();
		ListIter_t<Point_t *> ptItr;
		ptItr.Attach(&reg->vertices);
		Point_t *first, *cur;
		cur = first = ptItr.Next();
		fprintf(fp, "a = [%d, %d", cur->GetIntCoord(0), cur->GetIntCoord(1));
		while(!ptItr.IsEnd()) {
			cur = ptItr.Next();
			fprintf(fp, "; %d, %d", cur->GetIntCoord(0), cur->GetIntCoord(1));
		}
		fprintf(fp, "; %d, %d];\n", first->GetIntCoord(0), first->GetIntCoord(1));
		fprintf(fp, "plot(a(:,1), a(:,2));\n");
	}
}

void Nipqo_t::inferBestPlan(pOptimizer_t *pOpt, Point_t *vertex, int *fpcErr, PlanId_t optPlan)
{
	vertex->setInferedOptPlan(0);
	vertex->setInferedOptCost(0);

	/*if(pOpt->POSP.Contains(optPlan)) {
		CostVal_t planCost = vertex->GetPlanCost(optPlan, 0, fpcErr);
		vertex->setInferedOptPlan(optPlan);
		vertex->setInferedOptCost(planCost);
		return;
	}
	Point_t *partVert = pOpt->GetFromPartitioningList(vertex);
	if(partVert != NULL) {
		vertex->setInferedOptPlan(partVert->GetMinCostPlan());
		vertex->setInferedOptCost(partVert->GetMinCost());
		*fpcErr = false;
		return;
	}*/
	CostVal_t minCost = 1000000;
	PlanId_t minplan = 0;
	for(int i = 0; i < pOpt->POSP.Size(); i++) {
		int locErr;
		locErr = 0;
		PlanId_t planId = pOpt->POSP.Entry(i);
		CostVal_t planCost = vertex->GetPlanCost(planId, 0, &locErr);
		if(planCost < minCost) {
			minplan = planId;
			minCost = planCost;
			*fpcErr = locErr;
		}
	}
	vertex->setInferedOptPlan(minplan);
	vertex->setInferedOptCost(minCost);
}

void Nipqo_t::computeError(pOptimizer_t *pOpt, FILE *mtfp, FILE *inferfp, FILE *errfp) {
	int localityErrCnt = 0, localityErrCnt10 = 0, localityErrCnt20 = 0, localityErrCnt30 = 0, localityFPCErrCnt = 0;
	int gridSize = resolution * resolution;
	float totalError = 0, maxError = 0, totaldiff = 0;

	cout<<"Inferring plans at all points.\n";
	clock_t tic = clock(), toc;
	for(int i = 0; i < resolution; i++) {
		for(int j = 0; j < resolution; j++) {
			Point_t *vertex = allVertices[i][j];

			CostVal_t optCost = vertex->getOptimizerOptCost();
			PlanId_t optPlan = vertex->getOptimizerOptPlan();

			//infer plan
			int fpcErr = 0;
//			Polygon_t::inferPlan(pOpt->allRegions, vertex, &fpcErr);
			inferBestPlan(pOpt, vertex, &fpcErr, optPlan);
			PlanId_t inferedPlan = vertex->getInferedOptPlan();
			CostVal_t inferredCost = vertex->getInferedOptCost();
			assert(inferedPlan > 0);
//			cout<<"Plan inferred at point ";
//			vertex->PrintIntCoord();
//			cout<<"is "<<inferedPlan<<" with inferred cost "<<inferredCost<<" & optplan is "<<optPlan<<" with optCost "<<optCost;

			float dffPercent = (inferredCost - optCost) / optCost;
			if(optCost <= inferredCost) {
				totaldiff += dffPercent;
			}
			fprintf(inferfp, "%d,%d,%s,%d,%f,%d,%f,%f,%d\n", vertex->GetIntCoord(0), vertex->GetIntCoord(1), "Err", optPlan
					, optCost, inferedPlan, inferredCost, dffPercent, fpcErr);
			//Compute error
			if(optCost <= inferredCost && PD_THRESHOLD < dffPercent) {// && optPlan != inferedPlan) {
//				cout<<" with diffPercent "<<dffPercent<<endl;
//				if(dffPercent > PD_THRESHOLD) {
				fprintf(errfp, "%d,%d,%s,%d,%f,%d,%f,%f,%d\n", vertex->GetIntCoord(0), vertex->GetIntCoord(1), "Err", optPlan
						, optCost, inferedPlan, inferredCost, dffPercent, fpcErr);
				totalError += dffPercent;
				if(maxError < dffPercent)
					maxError = dffPercent;
//				}
				if(PD_THRESHOLD + 0.3 < dffPercent) {
					localityErrCnt30++;
					localityErrCnt++;
					if(optPlan != inferedPlan)
						fprintf(mtfp,"scatter(%d, %d, '*', 'black');\n", vertex->GetIntCoord(0), vertex->GetIntCoord(1));
					else {
						fprintf(mtfp,"scatter(%d, %d, '*', 'yellow');\n", vertex->GetIntCoord(0), vertex->GetIntCoord(1));
						localityFPCErrCnt++;
					}
					continue;
				}
				else if(PD_THRESHOLD + 0.2 < dffPercent) {
					localityErrCnt20++;
					localityErrCnt++;
					if(optPlan != inferedPlan)
						fprintf(mtfp,"scatter(%d, %d, '*', 'red');\n", vertex->GetIntCoord(0), vertex->GetIntCoord(1));
					else {
						fprintf(mtfp,"scatter(%d, %d, '*', 'yellow');\n", vertex->GetIntCoord(0), vertex->GetIntCoord(1));
						localityFPCErrCnt++;
					}
					continue;
				}
				else if(PD_THRESHOLD + 0.1 < dffPercent) {
					localityErrCnt10++;
					localityErrCnt++;
					if(optPlan != inferedPlan)
						fprintf(mtfp,"scatter(%d, %d, '*', 'blue');\n", vertex->GetIntCoord(0), vertex->GetIntCoord(1));
					else {
						fprintf(mtfp,"scatter(%d, %d, '*', 'yellow');\n", vertex->GetIntCoord(0), vertex->GetIntCoord(1));
						localityFPCErrCnt++;
					}
					continue;
				}
				else {//if(PD_THRESHOLD < dffPercent) {
					if(optPlan != inferedPlan)
						fprintf(mtfp,"scatter(%d, %d, '*', 'green');\n", vertex->GetIntCoord(0), vertex->GetIntCoord(1));
					else {
						fprintf(mtfp,"scatter(%d, %d, '*', 'yellow');\n", vertex->GetIntCoord(0), vertex->GetIntCoord(1));
						localityFPCErrCnt++;
					}
					localityErrCnt++;
				}
			}
//			else
//				cout<<endl;
//			vertex->setInferedOptCost(0);
//			vertex->setInferedOptPlan(0);
//			vertex->resetPlansHistory();
		}
	}

	cout<<"Total Plan Locality error is "<<localityErrCnt<<" & in percent is "<<((float)localityErrCnt)/gridSize<<endl;
	cout<<"Total plus 10 Plan Locality error is "<<localityErrCnt10<<" & in percent is "<<((float)localityErrCnt10)/gridSize<<endl;
	cout<<"Total plus 20 Plan Locality error is "<<localityErrCnt20<<" & in percent is "<<((float)localityErrCnt20)/gridSize<<endl;
	cout<<"Total plus 30 Plan Locality error is "<<localityErrCnt30<<" & in percent is "<<((float)localityErrCnt30)/gridSize<<endl;
	cout<<"Total Plan Locality FPC error is "<<localityFPCErrCnt<<" & in percent is "<<((float)localityFPCErrCnt)/gridSize<<" Need to be subtracted."<<endl<<endl<<endl;
	cout<<"Max percent difference in cost is "<<maxError<<endl;
	cout<<"Average percent difference in error points cost is "<<totalError / localityErrCnt<<endl;
	cout<<"Average percent difference in all points cost is "<<totaldiff / gridSize<<endl;
	toc = clock();
	timeToInferAll = (double)(toc - tic);
	cout << "\nTime taken to optimize all points: " << timeToInferAll/CLOCKS_PER_SEC << " secs.\n";
}

streambuf *Nipqo_t::redirectCoutTo(string file)
{
	ofstream fout(file.c_str());
	streambuf *coutbuf = cout.rdbuf();
	cout.rdbuf(fout.rdbuf());
	return coutbuf;
}

void Nipqo_t::resetCout(streambuf *coutbuf)
{
	cout.rdbuf(coutbuf);
}
