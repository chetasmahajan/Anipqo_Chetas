#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cmath>
#include <iostream>
#include <string.h>

#include "TypeDefs.h"
#include "HashSet.h"
#include "Point.h"
#include "Rectangle.h"

Boolean_t Rectangle_t::SanityCheck(void)
{
	int i, j;
	RealCoord_t *lowerRealCoord = lowCoord;
	assert(lowerRealCoord);
	RealCoord_t *higherRealCoord = highCoord;
	assert(higherRealCoord);
	IntCoord_t *lowerIntCoord = new IntCoord_t[Nipqo_t::NumParam()];
	assert(lowerIntCoord);
	IntCoord_t *higherIntCoord = new IntCoord_t[Nipqo_t::NumParam()];
	assert(higherIntCoord);


	for(i = 0; i < Nipqo_t::NumParam(); i++) {
		lowerIntCoord[i] = Nipqo_t::GetPredConst(lowerRealCoord[i], i);
		higherIntCoord[i] = Nipqo_t::GetPredConst(higherRealCoord[i], i);

		if(CONSTANT == rectSideStatus[i]) {
			assert(lowerRealCoord[i] == higherRealCoord[i]);
		}
		// what about UNDIVIDED = 1, DIVIDED = 2, DONT_DIVIDE = 3?
		else {
			assert(lowerRealCoord[i] < higherRealCoord[i]);
			assert(lowerIntCoord[i] < higherIntCoord[i]);
		}
	}

	for(i = 0; i < NumVertices(); i++) {
		for(j = 0; j < Nipqo_t::NumParam(); j++) {
			RealCoord_t rc = vertices[i]->GetRealCoord(j);
			IntCoord_t ic = vertices[i]->GetIntCoord(j);

			assert(Nipqo_t::GetPredConst(rc, j) == ic);
			// assert((rc == lowerRealCoord[j]) || (rc == higherRealCoord[j])); // Creates problem for method two
			assert((ic == lowerIntCoord[j]) || (ic == higherIntCoord[j]));
		}
	}

	delete lowerIntCoord; delete higherIntCoord;

	return TRUE;
}

void Rectangle_t::SetRectSideStatus(int partitioningAxisNum)
{
	IntCoord_t x1i = Nipqo_t::GetPredConst(
			lowCoord[partitioningAxisNum], partitioningAxisNum);
	IntCoord_t x2i = Nipqo_t::GetPredConst(
			highCoord[partitioningAxisNum], partitioningAxisNum);

	assert((x1i + 1) <= x2i);

	RealCoord_t xmr = (lowCoord[partitioningAxisNum]
	                            + highCoord[partitioningAxisNum])/2;
	IntCoord_t xmi = Nipqo_t::GetPredConst(xmr, partitioningAxisNum);
	// if(x1i >= (x2i - 2))
	// if((x2i  - 2) <= x1i)
	if((x2i  - 1) <= x1i) { // Changed tmp; revert back ?
		assert(xmi <= x2i);
		rectSideStatus[partitioningAxisNum] = DONT_DIVIDE;
	}
	else {
		assert(x1i <= xmi); assert(xmi <= x2i);
		rectSideStatus[partitioningAxisNum] = DIVIDED;
	}

	int i;
	for(i = 0; i < Nipqo_t::NumParam(); i++)
		if(UNDIVIDED == rectSideStatus[i]) break;

	if(Nipqo_t::NumParam() == i) {
		// All sides are divided; reset DIVIDED flags
		for(int j = 0; j < Nipqo_t::NumParam(); j++) {
			assert(UNDIVIDED != rectSideStatus[j]);
			if(DIVIDED == rectSideStatus[j]) rectSideStatus[j] = UNDIVIDED;
		}
	}
}

void Rectangle_t::SetCentrePoint(void)
{
	assert(NULL == centrePoint);

	RealCoord_t *centre = new RealCoord_t[Nipqo_t::NumParam()];
	assert(centre);
#if 0
	static int ratalya = 0;
	cout << "Ratalya: " << ++ratalya << endl;
	if(399 == ratalya) {
		cout << "Stop here...";
	}
#endif
#if 1
	int numVar = 0;
	int varIdx = -1;
	for(int i = 0; i < Nipqo_t::NumParam(); i++) {
		if(CONSTANT != rectSideStatus[i]) {
			numVar++; varIdx = i;
		}
	}
#endif
	for(int i = 0; i < Nipqo_t::NumParam(); i++) {
		if(CONSTANT == rectSideStatus[i]) {
			centre[i] = lowCoord[i];
			assert(centre[i] == vertices[0]->GetRealCoord(i));
		}
		else {
			assert((DONT_DIVIDE == rectSideStatus[i])
					|| (DIVIDED == rectSideStatus[i])
					|| (UNDIVIDED == rectSideStatus[i]));

			int offset = 0;
			for(int j = 0; j < i; j++)
				if(CONSTANT != rectSideStatus[j]) offset++;
			int idx2 = Utility_t::TwoPowerN(offset);
			assert(idx2 < NumVertices());

			Point_t *vertex1 = vertices[0];
			assert(vertex1);
			Point_t *vertex2 = vertices[idx2];
			assert(vertex2);
			assert(vertex1->IsEqualExcept(*vertex2, i));
#if 0
			RealCoord_t x1r, x2r, xmr;

			if(1 == numVar) {
				assert(i == varIdx);

				CostVal_t minCost1 = vertex1->GetMinCost();
				CostVal_t minCost2 = vertex2->GetMinCost();

				CostVal_t maxCost1 = 0;
				ListIter_t<PlanId_t> planIter;
				planIter.Attach(&vertex2->MinCostPlans());
				while(!planIter.IsEnd()) {
					PlanId_t planId = planIter.Next();
					assert(0 < planId);

					CostVal_t cost = vertex1->GetPlanCost(planId);
					if(maxCost1 < cost) maxCost1 = cost;
				}

				CostVal_t maxCost2 = 0;
				planIter.Attach(&vertex1->MinCostPlans());
				while(!planIter.IsEnd()) {
					PlanId_t planId = planIter.Next();
					assert(0 < planId);

					CostVal_t cost = vertex2->GetPlanCost(planId);
					if(maxCost2 < cost) maxCost2 = cost;
				}

				CostVal_t diff1 = maxCost1 - minCost1;
				// assert(0 <= diff1);
				CostVal_t diff2 = maxCost2 - minCost2;
				// assert(0 <= diff2);

				x1r = lowCoord[i];
				assert(Nipqo_t::GetPredConst(x1r, i) == vertex1->GetIntCoord(i));
				x2r = highCoord[i];
				assert(Nipqo_t::GetPredConst(x2r, i) == vertex2->GetIntCoord(i));
				xmr = (x1r*diff1 + x2r*diff2)/(diff1+diff2);
			}
			else {
				x1r = lowCoord[i];
				assert(Nipqo_t::GetPredConst(x1r, i) == vertex1->GetIntCoord(i));
				x2r = highCoord[i];
				assert(Nipqo_t::GetPredConst(x2r, i) == vertex2->GetIntCoord(i));
				xmr = (x1r + x2r)/2;
			}
#else
			RealCoord_t x1r = lowCoord[i];
			// assert(x1r == vertex1->GetRealCoord(i)); // Changed to next line to accomodate metohd two
			assert(Nipqo_t::GetPredConst(x1r, i) == vertex1->GetIntCoord(i));
			RealCoord_t x2r = highCoord[i];
			// assert(x2r == vertex2->GetRealCoord(i)); // Changed to next line to accomodate metohd two
			assert(Nipqo_t::GetPredConst(x2r, i) == vertex2->GetIntCoord(i));
			RealCoord_t xmr = (x1r + x2r)/2;
#endif
			centre[i] = xmr;

			IntCoord_t x1i = vertex1->GetIntCoord(i);
			IntCoord_t x2i = vertex2->GetIntCoord(i);
			assert(x1i < x2i);

			IntCoord_t xmi = Nipqo_t::GetPredConst(xmr, i);
			if(x1i == xmi) { // Change?
#if 0
				if(1 < numVar)
#endif
					assert((x1i + 1) == x2i);
				xmi++;
				centre[i] = highCoord[i];
				// assert(centre[i] == vertex2->GetRealCoord(i)); // commented to accomodate method 2
				assert(Nipqo_t::GetPredConst(centre[i], i)
				== Nipqo_t::GetPredConst(vertex2->GetRealCoord(i), i));
			}
			else if(x2i == xmi) {
#if 0
				if(1 < numVar)
#endif
					assert((x1i + 1) == x2i);
				centre[i] = highCoord[i];
				// assert(centre[i] == vertex2->GetRealCoord(i)); // commented to accomodate method 2
				assert(Nipqo_t::GetPredConst(centre[i], i)
				== Nipqo_t::GetPredConst(vertex2->GetRealCoord(i), i));
			}

			// assert(x1i <= (x2i - 2)); assert(x1i < xmi); assert(xmi <= x2i);
			assert((x1i + 1) <= x2i); assert(x1i < xmi); assert(xmi <= x2i); // change first assert to 2 ?
		}
	}

	centrePoint =  (Nipqo_t::VertexCache()).Get(centre);
	//Chetas- Added for Correct version.*************
	/*if(NULL == centrePoint) {
		centrePoint = new Point_t(centre);
		assert(centrePoint);
		(Nipqo_t::VertexCache()).Put(centrePoint);
	}*/
	if(centrePoint != NULL) {
		bool isRectangleVertex = false;
		for(int i = 0; i < NumVertices(); i++) {
			if(centrePoint == vertices[i]) {
				isRectangleVertex = true;;
				break;
			}
		}
		if(!isRectangleVertex) {
			if(!pOpt->PartitioningVertexListContains(centrePoint)) {		//If this is not part of decomposition vertices, then only delete this.
				(Nipqo_t::VertexCache()).Remove(centre);
				delete centrePoint;
				centrePoint = NULL;
			}
			else {
				delete centre;
				return;
			}
		}
		else {
			isRectVertex = true;
			if(NumVaryingParam() == 2 && centrePoint->BoundaryNum() >= 1) {
				assert(centrePoint->BoundaryNum() == 1);
				cout<<"In setting the centre of Rectangle: ";
				centrePoint->PrintIntCoord();
				cout<<" $$$$$$$$$$$$$ Possible Error: False positive centre point of 2D rectangle got detected. $$$$$$$$$$$$$$$\n";

				//			pOpt->AddFalsePositivePartitioningVertex(centrePoint);
				centrePoint = NULL;
			}
			else if(!pOpt->PartitioningVertexListContains(centrePoint)) {		//If this is not part of decomposition vertices, then only delete this.
				(Nipqo_t::VertexCache()).Remove(centre);
				centrePoint = NULL;
			}
			else {
				delete centre;
				return;
			}
		}
	}
	assert(centrePoint == NULL);
	centrePoint = new Point_t(centre);
	assert(centrePoint);
	(Nipqo_t::VertexCache()).Put(centrePoint, !isRectVertex);
	//***********************************************

	delete centre;
}

RealCoord_t *Rectangle_t::CentrePointReal(void)
{
	RealCoord_t *centre = new RealCoord_t[Nipqo_t::NumParam()];
	assert(centre);
#if 0
	static int ratalya = 0;
	cout << "Ratalya: " << ++ratalya << endl;
	if(399 == ratalya) {
		cout << "Stop here...";
	}
#endif
	for(int i = 0; i < Nipqo_t::NumParam(); i++) {
		switch(rectSideStatus[i]) {
		case CONSTANT: {
			centre[i] = lowCoord[i];
			assert(centre[i] == vertices[0]->GetRealCoord(i));
			break;
		}

		case DONT_DIVIDE:
		case DIVIDED:
		case UNDIVIDED: {
			int offset = 0;
			for(int j = 0; j < i; j++)
				if(CONSTANT != rectSideStatus[j]) offset++;
			int idx2 = Utility_t::TwoPowerN(offset);
			assert(idx2 < NumVertices());

			Point_t *vertex1 = vertices[0];
			assert(vertex1);
			Point_t *vertex2 = vertices[idx2];
			assert(vertex2);
			assert(vertex1->IsEqualExcept(*vertex2, i));

			RealCoord_t x1r = lowCoord[i];
			assert(x1r == vertex1->GetRealCoord(i));
			RealCoord_t x2r = highCoord[i];
			assert(x2r == vertex2->GetRealCoord(i));
			RealCoord_t xmr = (x1r + x2r)/2;
			centre[i] = xmr;

			IntCoord_t x1i = vertex1->GetIntCoord(i);
			IntCoord_t x2i = vertex2->GetIntCoord(i);
			assert(x1i < x2i);

			IntCoord_t xmi = Nipqo_t::GetPredConst(xmr, i);
			if(x1i == xmi) { // Change?
				assert((x1i + 1) == x2i);
				xmi++;
				centre[i] = highCoord[i];
				assert(centre[i] == vertex2->GetRealCoord(i));
			}
			else if(x2i == xmi) {
				assert((x1i + 1) == x2i);
				centre[i] = highCoord[i];
				assert(centre[i] == vertex2->GetRealCoord(i));
			}

			// assert(x1i <= (x2i - 2)); assert(x1i < xmi); assert(xmi <= x2i);
			assert((x1i + 1) <= x2i); assert(x1i < xmi); assert(xmi <= x2i); // change first assert to 2 ?

			break;
		}

		default: assert(FALSE);
		}
	}

	return centre;
}

IntCoord_t *Rectangle_t::CentrePoint(void)
{
	IntCoord_t *centre = new IntCoord_t[Nipqo_t::NumParam()];
	assert(centre);

	for(int i = 0; i < Nipqo_t::NumParam(); i++) {
		switch(rectSideStatus[i]) {
		case CONSTANT: {
			centre[i] = vertices[0]->GetIntCoord(i);
			assert(centre[i] == Nipqo_t::GetPredConst(lowCoord[i], i));
			break;
		}

		case DONT_DIVIDE:
		case DIVIDED:
		case UNDIVIDED: {
			int offset = 0;
			for(int j = 0; j < i; j++)
				if(CONSTANT != rectSideStatus[j]) offset++;
			int idx2 = Utility_t::TwoPowerN(offset);
			assert(idx2 < NumVertices());

			Point_t *vertex1 = vertices[0];
			assert(vertex1);
			Point_t *vertex2 = vertices[idx2];
			assert(vertex2);
			assert(vertex1->IsEqualExcept(*vertex2, i));

			RealCoord_t x1r = lowCoord[i];
			assert(x1r == vertex1->GetRealCoord(i));
			RealCoord_t x2r = highCoord[i];
			assert(x2r == vertex2->GetRealCoord(i));
			RealCoord_t xmr = (x1r + x2r)/2;

			IntCoord_t x1i = vertex1->GetIntCoord(i);
			IntCoord_t x2i = vertex2->GetIntCoord(i);
			centre[i] = Nipqo_t::GetPredConst(xmr, i);
			if(x1i == centre[i]) { // Change?
				assert((x1i + 1) == x2i);
				centre[i]++;
			}
			// assert(x1i <= (x2i - 2)); assert(x1i < centre[i]); assert(centre[i] <= x2i);
			assert((x1i + 1) <= x2i); assert(x1i < centre[i]); assert(centre[i] <= x2i); // change first assert to 2 ?

			break;
		}

		default: assert(FALSE);
		}
	}

	return centre;
}

void Rectangle_t::AssignVertices(void)
{
	int i;

	RealCoord_t *vertex = new RealCoord_t[Nipqo_t::NumParam()];
	assert(vertex);

	for(i = 0; i < Nipqo_t::NumParam(); i++) {
		vertex[i] = lowCoord[i];
	}

	int numVertices = 0;

	while(1) {
		Point_t *cachedVertex =
				(Nipqo_t::VertexCache()).Get(vertex);
		if(NULL == cachedVertex) {
			vertices[numVertices] = new Point_t(vertex);
			assert(vertices[numVertices]);
			(Nipqo_t::VertexCache()).Put(vertices[numVertices]);
		}
		else {
			vertices[numVertices] = cachedVertex;
		}

		assert(vertices[numVertices]);
		numVertices++;

		for(i = 0; i < Nipqo_t::NumParam(); i++) {
			if(CONSTANT == rectSideStatus[i]) {
				assert(lowCoord[i] == highCoord[i]);
				continue;
			}

			assert(lowCoord[i] != highCoord[i]);

			if(lowCoord[i] == vertex[i]) {
				vertex[i] = highCoord[i];
				break;
			}
			else {
				assert(vertex[i] == highCoord[i]);
				vertex[i] = lowCoord[i];
			}
		}

		assert(i <= Nipqo_t::NumParam());
		if(i == Nipqo_t::NumParam()) break;
	}

	delete vertex;
	assert(NumVertices() == numVertices);
}

PlanId_t Rectangle_t::OptimizeCentre(void)
{
	assert(1 == Nipqo_t::MethodNum());

	if(NULL == centrePoint) {
		SetCentrePoint();
		assert(centrePoint);
	}
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "CentrePointInt (Optimized): ";
		centrePoint->PrintRealCoord();
		cout << endl;
		centrePoint->PrintIntCoord();
		cout << endl;
	}
#endif
	centreOptimized = TRUE;

	if(centrePoint->IsOptimized()) {		//Chetas- Need to clean up code.
		assert(0 < centrePoint->OptPlan());
		PlanId_t tmp = centrePoint->OptPlan();
		if(pOpt->LocalPlans().Contains(tmp)) return 0;
		else return centrePoint->OptPlan();
	}

	int oldNumPlans = Nipqo_t::NumPlans();
	CostVal_t optCost;
	PlanId_t optPlan = centrePoint->Optimize(&optCost);		//Chetas- It is not being used just added new variable optCost. While using this function, this optPlan should be added to planCostList.
	if(pOpt->LocalPlans().Contains(optPlan)) return 0;
	else return optPlan;
}

void Rectangle_t::CreatePartitions(void)
{
	assert(0 == partitionList.Size());
	assert(TRUE == IsPartitionable());

	int partitionAxis = GetPartitioningAxis();
	assert(0 <= partitionAxis);
	assert(partitionAxis < Nipqo_t::NumParam());
	assert(UNDIVIDED == rectSideStatus[partitionAxis]);
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "Partitioning Rectangle: ";
		PrintVertices(TRUE);
	}
#endif
	Rectangle_t *leftRect =
			new Rectangle_t(this, partitionAxis, TRUE);
	assert(leftRect);
	assert(leftRect->SanityCheck());

	Rectangle_t *rightRect =
			new Rectangle_t(this, partitionAxis, FALSE);
	assert(rightRect);
	assert(rightRect->SanityCheck());

	partitionList.Insert(leftRect);
	partitionList.Insert(rightRect);
}

PlanId_t Rectangle_t::OptimizePartitionsMethodOne(
		MyList_t<PlanId_t> *ancestorActivePlanList)
{
	assert(1 == Nipqo_t::MethodNum());
	assert(0);
	return 0;
}

Boolean_t Rectangle_t::OptimizePartitionsMethodTwo(
		MyList_t<PlanId_t> *ancestorActivePlanList)
{
	assert(2 == Nipqo_t::MethodNum());
	assert(2 == Nipqo_t::OptPhase());

	ListIter_t<Rectangle_t *> rectIter;
	rectIter.Attach(&partitionList);

	while(!rectIter.IsEnd()) {
		Rectangle_t *rect = rectIter.Next();
		assert(rect);

		Boolean_t vertexLocated = rect->OptimizeMethodTwo(ancestorActivePlanList);
		assert((TRUE == vertexLocated) || (FALSE == vertexLocated));

		if(TRUE == vertexLocated) return TRUE;
	}

	return FALSE;
}

PlanId_t Rectangle_t::OptimizePartitions(
		MyList_t<PlanId_t> *ancestorActivePlanList)
{
	ListIter_t<Rectangle_t *> rectIter;
	rectIter.Attach(&partitionList);

	while(!rectIter.IsEnd()) {
		Rectangle_t *rect = rectIter.Next();
		assert(rect);

		PlanId_t newPlan = rect->Optimize(ancestorActivePlanList);
		assert(0 <= newPlan);

		// if(0 < newPlan) return newPlan;
#if 1
		assert((1 == Nipqo_t::MethodNum()) || (2 == Nipqo_t::MethodNum()));
		if(1 == Nipqo_t::MethodNum()) assert(-1 == Nipqo_t::OptPhase());
		else {
			assert((1 == Nipqo_t::OptPhase()) || (2 == Nipqo_t::OptPhase()));

			if(1 == Nipqo_t::OptPhase()) assert(0 == newPlan);
			else assert((FALSE == newPlan) || (TRUE == newPlan));
		}

		if(0 < newPlan) {
			if(2 == Nipqo_t::MethodNum()) assert(TRUE == newPlan);
			return newPlan;
		}
#else
		assert((1 == Nipqo_t::MethodNum()) || (2 == Nipqo_t::MethodNum()));

		if(1 == Nipqo_t::MethodNum()) {
			assert(-1 == Nipqo_t::OptPhase());
			if(0 < newPlan) return newPlan;
		}
		else {
			assert((1 == Nipqo_t::OptPhase()) || (2 == Nipqo_t::OptPhase()));

			if(1 == Nipqo_t::OptPhase()) {
				assert(0 == newPlan);
			}
			else {
				assert((FALSE == newPlan) || (TRUE == newPlan));

				if(TRUE == newPlan) return TRUE;
			}
		}
#endif
	}

	return 0;
}

PlanId_t Rectangle_t::Optimize(MyList_t<PlanId_t> *ancestorActivePlanList)
{
	if(1 == NumVertices()) {
		assert(1 == Nipqo_t::MethodNum());
		return Optimize1(ancestorActivePlanList);
	}

	assert((1 == Nipqo_t::MethodNum()) || (2 == Nipqo_t::MethodNum()));

	if(1 == Nipqo_t::MethodNum()) {
		assert(-1 == Nipqo_t::OptPhase());
		return Optimize1(ancestorActivePlanList);
	}

	assert((1 == Nipqo_t::OptPhase()) || (2 == Nipqo_t::OptPhase()));

	if(1 == Nipqo_t::OptPhase()) return Optimize1(ancestorActivePlanList);
	else return Optimize2(ancestorActivePlanList);
}

PlanId_t Rectangle_t::OptimizeMethodOne(MyList_t<PlanId_t> *ancestorActivePlanList)
// = NULL
{
	assert(1 == Nipqo_t::MethodNum());
	assert(0);
	return 0;
}

PlanId_t Rectangle_t::Optimize1(MyList_t<PlanId_t> *ancestorActivePlanList)
// = NULL
{
	assert(1 == Nipqo_t::MethodNum());

	if(Nipqo_t::OptimizeOnlySquares()) {
		if((FALSE == IsSquare()) && (TRUE == IsPartitionable())) {
			if(0 == partitionList.Size()) CreatePartitions();
			assert(0 < partitionList.Size());
			return OptimizePartitions(ancestorActivePlanList);
		}
	}
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "Optimizing Rectangle(n_v = " << NumVertices() << "): ";
		PrintVertices(TRUE);
	}
#endif
	assert(0 < NumVertices());

	if(1 == NumVertices()) {
		CostVal_t optCost;
		PlanId_t optPlan = vertices[0]->Optimize(&optCost);			//Chetas- It is not being used just added new variable optCost. While using this function, this optPlan should be added to planCostList.
		assert(0 < optPlan); assert(optPlan <= Nipqo_t::NumPlans());
		numDistinctPlans1 = numDistinctPlans2 = 1;
		return optPlan;
	}

	int i;

	assert(1 < NumVertices());
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace())
		if(ancestorActivePlanList) {
			cout << "Active Plans: ";
			for(int i = 0; i < ancestorActivePlanList->Size(); i++)
				cout << ancestorActivePlanList->Entry(i) << ", ";
			cout << endl;
		}
#endif
	Boolean_t firstPOptUpdate = TRUE;
	numPlansEvaluated = 0;
	minPlanCostList.MakeEmpty();

	Boolean_t minMaxCostUpdated = FALSE;
	Boolean_t minMaxCostSet = FALSE;

	for(; numPlansEvaluated < pOpt->LocalPlans().Size();
			numPlansEvaluated++) {
		int i;
		PlanId_t planId = pOpt->LocalPlans().Entry(numPlansEvaluated);
		assert(0 < planId); assert(planId <= Nipqo_t::NumPlans());

		if(ancestorActivePlanList) {
			for(i = 0; i < ancestorActivePlanList->Size(); i++) {
				if(ancestorActivePlanList->Entry(i) ==
						pOpt->LocalPlans().Entry(numPlansEvaluated)) break;
			}

			if(ancestorActivePlanList->Size() == i) continue;
		}

		Boolean_t firstVertex = TRUE;
		CostVal_t planMaxCost, planMinCost;
		for(i = 0; i < NumVertices(); i++) {
			CostVal_t cost = vertices[i]->GetPlanCost(planId);
			assert(0 <= cost);

			if((TRUE == firstVertex) || (cost < planMinCost))
				planMinCost = cost;
			if((TRUE == firstVertex) || (planMaxCost < cost))
				planMaxCost = cost;

			firstVertex = FALSE;
		}

		assert(0 <= planMaxCost);
		assert(0 <= planMinCost);

		if((FALSE == minMaxCostSet) || (planMaxCost < minMaxCost)) {
			minMaxCost = planMaxCost;
			minMaxCostUpdated = TRUE;
			minMaxCostSet = TRUE;

			PlanCostPair_t pcp(planId, planMinCost);
			minPlanCostList.Append(pcp);
		}
		else {
			assert(TRUE == minMaxCostSet);

			if(planMinCost <= minMaxCost) {
				PlanCostPair_t pcp(planId, planMinCost);
				minPlanCostList.Append(pcp);
			}
		}
	}

	assert(TRUE == minMaxCostSet);

	if(TRUE == minMaxCostUpdated)
		for(int i = 0; i < minPlanCostList.Size(); i++)
			if(minMaxCost < minPlanCostList.Entry(i).Cost())
				minPlanCostList.DeleteEntryNum(i);
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace())
		for(i = 0; i < NumVertices(); i++) {
			cout << "Vertex " << i << ", PlanCosts: ";
			vertices[i]->PrintPlanCosts();
			cout << endl;
		}
#endif
	Boolean_t *optPlanBits = new Boolean_t[Nipqo_t::NumPlans()+1];
	assert(optPlanBits);

	for(i = 0;  i <= Nipqo_t::NumPlans(); i++)
		optPlanBits[i] = FALSE;
	for(i = 0; i < NumVertices(); i++)
		optPlanBits[vertices[i]->OptPlan()] = TRUE;

	numDistinctPlans1 = 0;
	for(i = 0;  i <= Nipqo_t::NumPlans(); i++)
		if(TRUE == optPlanBits[i]) numDistinctPlans1++;
	assert(0 < numDistinctPlans1);
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "numDistinctPlans: " << numDistinctPlans1 << ", optPlans: ";
		for(i = 0;  i <= Nipqo_t::NumPlans(); i++)
			if(TRUE == optPlanBits[i]) cout << i << " ";
		cout << endl;
	}
#endif
	if(numDistinctPlans1 <= NumVaryingParam()) {
		delete optPlanBits;
		return 0;
	}

	if(NULL == centrePoint) {
		SetCentrePoint();
		assert(centrePoint);
	}

	Boolean_t centreMaxCostSet = FALSE;
	for(i = 1; i <= Nipqo_t::NumPlans(); i++)
		if(TRUE == optPlanBits[i]) {
			CostVal_t cost = centrePoint->GetPlanCost(i);
			if((FALSE == centreMaxCostSet) || (centreMaxCost < cost))
				centreMaxCost = cost;
			centreMaxCostSet = TRUE;
		}

	Boolean_t centreMinCostSet = FALSE;
	for(i = 0; i < minPlanCostList.Size(); i++) {
		PlanId_t planId = minPlanCostList.Entry(i).PlanId();
		CostVal_t cost = centrePoint->GetPlanCost(planId);
		if((FALSE == centreMinCostSet) || (cost < centreMinCost))
			centreMinCost = cost;
		centreMinCostSet = TRUE;
	}
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "centreMinCost: " << centreMinCost;
		cout << ", centreMaxCost: " << centreMaxCost << endl;
	}
#endif
	assert(centreMinCost <= centreMaxCost);
	float pDiffC = (centreMaxCost - centreMinCost)/centreMinCost;
	assert(0 <= pDiffC);
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "CentrePt PlanCosts: ";
		centrePoint->PrintPlanCosts();
		cout << endl;

		cout << "CentrePointReal (Evaluated): ";
		centrePoint->PrintRealCoord();
		cout << endl;
		centrePoint->PrintIntCoord();
		cout << "pDiffC: " << pDiffC;
		cout << endl << endl;
	}
#endif
	delete optPlanBits;

	if(pDiffC <= PD_THRESHOLD) {
		assert((1 == Nipqo_t::MethodNum()) || (2 == Nipqo_t::MethodNum()));

		if(2 == Nipqo_t::MethodNum()) {		//Chetas- Unreachable code. Because of assert(1 == Nipqo_t::MethodNum()) at the start of this function.
			centrePoint->UpdateMinCostPlanList();
			assert(NumVaryingParam() < centrePoint->MinCostPlansListSize());
			pOpt->AddPartitioningVertex(centrePoint);

			// return 0;
			assert((1 == Nipqo_t::OptPhase()) || (2 == Nipqo_t::OptPhase()));
			if(1 == Nipqo_t::OptPhase()) return FALSE; // find rest of the vertices
			else return TRUE; // found the vertex we are searching for
		}

		assert(1 == Nipqo_t::MethodNum());
		assert(-1 == Nipqo_t::OptPhase());

		if(TRUE == centreOptimized) {
#ifdef ANIPQO_DEBUG
			if(Nipqo_t::PrintTrace())
				cout << "CentrePoint already optimized" << endl;
#endif
			return 0;
		}

#ifdef ANIPQO_DEBUG
		if(Nipqo_t::PrintTrace())
			cout << "CentrePoint is being optimized" << endl;
#endif
		return OptimizeCentre();
	}
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace())
		cout << endl;
#endif
	if(0 == partitionList.Size()) {
		if(IsPartitionable()) {
			if(0 == partitionList.Size()) CreatePartitions();
			assert(0 < partitionList.Size());
		}
		else {
			assert((1 == Nipqo_t::MethodNum()) || (2 == Nipqo_t::MethodNum()));

			if(2 == Nipqo_t::MethodNum()) {
				centrePoint->UpdateMinCostPlanList(pDiffC);
				assert(NumVaryingParam() < centrePoint->MinCostPlansListSize());
				pOpt->AddPartitioningVertex(centrePoint);

				// return 0;
				assert((1 == Nipqo_t::OptPhase()) || (2 == Nipqo_t::OptPhase()));
				if(1 == Nipqo_t::OptPhase()) return FALSE; // find rest of the vertices
				else return TRUE; // found the vertex we are searching for
			}

			assert(1 == Nipqo_t::MethodNum());
			assert(-1 == Nipqo_t::OptPhase());

			if(TRUE == centreOptimized) return 0;
			return OptimizeCentre();
		}
	}

	MyList_t<PlanId_t>  activePlanList;

	for(int i = 0; i < minPlanCostList.Size(); i++) {
		PlanId_t plan = minPlanCostList.Entry(i).PlanId();
		activePlanList.Append(plan);
	}

	// return OptimizePartitions(&minPlanCostList);
	return OptimizePartitions(&activePlanList);
}

Boolean_t Rectangle_t::AdjustCentrePoint(MyList_t<PlanId_t> &planList,
		float &pDiffC, float centreMinCost, float centreMaxCost)
{
	assert(centrePoint);
	Point_t *origCentrePoint = centrePoint;
	assert(0 <= pDiffC);

	for(int i = 0; i < NumVertices(); i++) {
		CostVal_t centreMaxCostTmp = vertices[i]->GetMaxCost(&planList);
		CostVal_t centreMinCostTmp = vertices[i]->GetMinCost();
		/****************************************************************************/
		// assert(centreMinCostTmp < centreMaxCostTmp); // changed to accomodate new method two
		assert(centreMinCostTmp <= centreMaxCostTmp);
		/****************************************************************************/
#ifdef ANIPQO_DEBUG
		if(Nipqo_t::PrintTrace()) {
			cout << "{In AdjustCentrePoint} centreMinCostTmp: " << centreMinCostTmp;
			cout << ", centreMaxCostTmp: " << centreMaxCostTmp << endl;
		}
#endif
		// float tmpDiff = vertices[i]->GetMinMaxCostDiffRatio(&planList);
		// assert(0 <= tmpDiff);
		float tmpDiff = (centreMaxCostTmp - centreMinCostTmp)/centreMinCostTmp;
		assert(0 <= tmpDiff);
		// assert(centrePoint->GetMinMaxCostDiffRatio(*planList) == tmpDiff);

		if(tmpDiff < pDiffC) {
			centrePoint = vertices[i];
			centreMinCost = centreMinCostTmp;
			centreMaxCost = centreMaxCostTmp;
			pDiffC = tmpDiff;
		}
	}
	cout<<"New adjusted centerpoint = ";
	centrePoint->PrintIntCoord(TRUE);

	//Chetas- Added for Correct Version. ********************************
	if(pOpt->PartitioningVertexListContains(centrePoint) && NumVaryingParam() == 2 && centrePoint->BoundaryNum() >= 1) {
		assert(centrePoint->BoundaryNum() == 1);
		Point_t *duplicate = new Point_t(centrePoint);
		//Chetas- added for fixing bug for QT8_100_uni lambda = 0.005...***********
		assert(newPlanID > 0);
		CostVal_t newPlanCost = centrePoint->GetPlanCost(newPlanID);
		assert(newPlanCost > 0);
		duplicate->GetPlanCost(newPlanID, newPlanCost);
		//*************************************************************************
		centrePoint = duplicate;
		centrePoint->GetMinCost(&planList);
		centrePoint->ChangeDiffThreshold(pDiffC);
		centrePoint->UpdateMinCostPlanList();
//		cout<<"\n$$$$$$$$$$$$$ Error: False positive centre point of 2D rectangle got detected. $$$$$$$$$$$$$$$\n";
//		pOpt->AddFalsePositivePartitioningVertex(centrePoint);
	}
	//*******************************************************************

	if(origCentrePoint == centrePoint) return FALSE;
	return TRUE;
}

Boolean_t Rectangle_t::OptimizeMethodTwo(MyList_t<PlanId_t> *planList)
// = NULL
{
	assert(2 == Nipqo_t::MethodNum());
	assert(2 == Nipqo_t::OptPhase());
#if 0
	static int ratalya = 0;
	cout << "Ratalya: " << ++ratalya << endl;
	if(595 == ratalya) {
		cout << "Stop here...";
	}
#endif
	if(Nipqo_t::OptimizeOnlySquares()) {
		if((FALSE == IsSquare()) && (TRUE == IsPartitionable())) {
			if(0 == partitionList.Size()) CreatePartitions();
			assert(0 < partitionList.Size());
			return OptimizePartitionsMethodTwo(planList);
		}
	}

#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "Optimizing Rectangle(n_v = " << NumVertices() << "): ";
		PrintVertices(TRUE);
	}
#endif
	assert(1 < NumVertices());
	assert(planList);

	int i;
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "Active Plans: ";
		for(int i = 0; i < planList->Size(); i++)
			cout << planList->Entry(i) << ", ";
		cout << endl;
	}
#endif
	int numDistinctPlans = NumOptPlans(*planList, FALSE);
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace())
		for(i = 0; i < NumVertices(); i++) {
			cout << "Vertex " << i << ", PlanCosts: ";
			vertices[i]->PrintPlanCosts();
			cout << endl;
		}
#endif
	// if(numDistinctPlans <= NumVaryingParam())
	if(numDistinctPlans < planList->Size()) {
		cout<<"Skipping this rectangle..................\n\n";
		return FALSE;
	}

	if(NULL == centrePoint) {
		SetCentrePoint();
		assert(centrePoint);
	}

	CostVal_t centreMaxCost = centrePoint->GetMaxCost(planList);
	CostVal_t centreMinCost = centrePoint->GetMinCost();
	/****************************************************************************/
	// assert(centreMinCost < centreMaxCost); // changed to accomodate new method two
	assert(centreMinCost <= centreMaxCost);
	/****************************************************************************/
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "centreMinCost: " << centreMinCost;
		cout << ", centreMaxCost: " << centreMaxCost << endl;
	}
#endif
	float pDiffC = (centreMaxCost - centreMinCost)/centreMinCost;
	assert(0 <= pDiffC);
	// assert(centrePoint->GetMinMaxCostDiffRatio(*planList) == pDiffC);
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "CentrePt PlanCosts: ";
		centrePoint->PrintPlanCosts();
		cout << endl;

		cout << "CentrePointReal (Evaluated): ";
		centrePoint->PrintRealCoord();
		cout << endl;
		centrePoint->PrintIntCoord();
		cout << "pDiffC: " << pDiffC;
//#endif		//Chetas- Commented for incorrect position.
		cout << endl <<endl;
	}			//Chetas- Added proper enclosing bracket.
#endif			//Chetas- Added proper statement.
	if(pDiffC <= PD_THRESHOLD) {
//
//	}			//Chetas- Commented for incorrect position.
		assert(FALSE == pOpt->PartitioningVertexListContains(centrePoint));
		centrePoint->UpdateMinCostPlanList();
		// assert(NumVaryingParam() < centrePoint->MinCostPlansListSize());

		//Chetas- Commented & changed for Correct version.****************
//		assert(planList->Size() == centrePoint->MinCostPlansListSize());

//		pOpt->AddPartitioningVertex(centrePoint);
		if(!pOpt->PartitioningVertexListContains(centrePoint)) {
			assert(planList->Size() == centrePoint->MinCostPlansListSize());
			pOpt->AddPartitioningVertex(centrePoint);

			if(NumVaryingParam() == 2 && centrePoint->BoundaryNum() >= 1) {
				assert(centrePoint->BoundaryNum() == 1);
				cout<<"\n$$$$$$$$$$$$$ Error: False positive centre point of 2D rectangle got detected. $$$$$$$$$$$$$$$\n";
				pOpt->AddFalsePositivePartitioningVertex(centrePoint);
				pOpt->falsePositivePointsDetected++;
			}
		}
/*		else if(NumVaryingParam() == 2 && centrePoint->BoundaryNum() >= 1) {
			assert(centrePoint->BoundaryNum() == 1);
			Point_t *duplicate = new Point_t(centrePoint);
			centrePoint = duplicate;
			centrePoint->GetMinCost(planList);
			centrePoint->ChangeDiffThreshold(pDiffC);
			centrePoint->UpdateMinCostPlanList();
			cout<<"\n$$$$$$$$$$$$$ Error: False positive centre point of 2D rectangle got detected. $$$$$$$$$$$$$$$\n";
			pOpt->AddFalsePositivePartitioningVertex(centrePoint);
			pOpt->falsePositivePointsDetected++;
		}*/
		else
			assert(planList->Size() <= centrePoint->MinCostPlansListSize());
		//***************************************************************

		// return 0;
		assert((1 == Nipqo_t::OptPhase()) || (2 == Nipqo_t::OptPhase()));
		if(1 == Nipqo_t::OptPhase()) return FALSE; // find rest of the vertices
		else return TRUE; // found the vertex we are searching for
	}
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace())
		cout << endl;
#endif

	if(0 == partitionList.Size()) { // Shouldnt this be IsPartitionable() ?
		if(IsPartitionable()) {
			if(0 == partitionList.Size()) CreatePartitions();
			assert(0 < partitionList.Size());
		}
		else {
			Boolean_t centrePointIsRectVertex = FALSE;
			for(i = 0; i < NumVertices(); i++) {
				if(centrePoint == vertices[i]) {
					centrePointIsRectVertex = TRUE;
					break;
				}
			}

			//Chetas- Commented & added for Correct version. *********************
//			if(centrePointIsRectVertex) {
			if(centrePointIsRectVertex || isRectVertex) {
				isRectVertex = false;
			//********************************************************************
#if 1
				AdjustCentrePoint(*planList, pDiffC, centreMinCost, centreMaxCost);
#else
				for(i = 0; i < NumVertices(); i++) {
					float tmpDiff = vertices[i]->GetMinMaxCostDiffRatio(*planList);
					if(tmpDiff < pDiffC) {
						centrePoint = vertices[i];
						pDiffC = tmpDiff;
					}
				}
#endif
			}

#if 1
			if(pOpt->PartitioningVertexListContains(centrePoint)) {
				centrePoint->ChangeDiffThreshold(pDiffC);

				//Chetas- Added for Correct version. *****************
				cout<<"Center was already in partitioning vertices list.\n";
				if(centrePoint->IsToBeDeleted()) {
					cout<<"This vertex stopped from getting deleted.\n\n";
					centrePoint->ResetToBeDeleted();
					assert(!centrePoint->IsToBeDeleted());
				}
				//***********************************
			}
			else
#endif
			{
				// assert(FALSE == pOpt->PartitioningVertexListContains(centrePoint));
				if(PD_THRESHOLD < pDiffC) {
					centrePoint->UpdateMinCostPlanList(pDiffC);
				}
				else {
					centrePoint->UpdateMinCostPlanList();
				}
				cout<<endl;
				// assert(NumVaryingParam() < centrePoint->MinCostPlansListSize());
				if(planList->Size() <= centrePoint->MinCostPlansListSize()){}
				else {
					cout<<Plan::numOptCalls()<<endl<<Plan::numEvalCalls();
					assert(0);
				}
				pOpt->AddPartitioningVertex(centrePoint);

				//Chetas- Added for Correct Version. ********************************
				if(NumVaryingParam() == 2 && centrePoint->BoundaryNum() >= 1) {
					assert(centrePoint->BoundaryNum() == 1);
					cout<<"\n$$$$$$$$$$$$$ Error: False positive centre point of 2D rectangle got detected. $$$$$$$$$$$$$$$\n";
					pOpt->AddFalsePositivePartitioningVertex(centrePoint);
					pOpt->falsePositivePointsDetected++;
				}
				//*******************************************************************
			}

			return TRUE; // found the vertex we are searching for
		}
	}

	return OptimizePartitionsMethodTwo(planList);
	//Chetas- Code to be added to do something if no partition returns equicost point.
}

PlanId_t Rectangle_t::Optimize2(MyList_t<PlanId_t> *planList)
// = NULL
{
	assert(2 == Nipqo_t::MethodNum());

	if(Nipqo_t::OptimizeOnlySquares()) {
		if((FALSE == IsSquare()) && (TRUE == IsPartitionable())) {
			if(0 == partitionList.Size()) CreatePartitions();
			assert(0 < partitionList.Size());
			return OptimizePartitions(planList);
		}
	}

#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "Optimizing Rectangle(n_v = " << NumVertices() << "): ";
		PrintVertices(TRUE);
	}
#endif
	assert(0 < NumVertices());
	assert(1 < NumVertices());
	assert(planList);

	int i;
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "Active Plans: ";
		for(int i = 0; i < planList->Size(); i++)
			cout << planList->Entry(i) << ", ";
		cout << endl;
	}
#endif
	int numDistinctPlans = NumOptPlans(*planList, FALSE);
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace())
		for(i = 0; i < NumVertices(); i++) {
			cout << "Vertex " << i << ", PlanCosts: ";
			vertices[i]->PrintPlanCosts();
			cout << endl;
		}
#endif
	// if(numDistinctPlans <= NumVaryingParam())
	if(numDistinctPlans < planList->Size()) {
		return 0;
	}

	if(NULL == centrePoint) {
		SetCentrePoint();
		assert(centrePoint);
	}

	CostVal_t centreMaxCost = centrePoint->GetMaxCost(planList);
	CostVal_t centreMinCost = centrePoint->GetMinCost(planList);
	/****************************************************************************/
	// assert(centreMinCost < centreMaxCost); // changed to accomodate new method two
	assert(centreMinCost <= centreMaxCost);
	/****************************************************************************/
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "centreMinCost: " << centreMinCost;
		cout << ", centreMaxCost: " << centreMaxCost << endl;
	}
#endif
	float pDiffC = (centreMaxCost - centreMinCost)/centreMinCost;
	assert(0 <= pDiffC);
	// assert(centrePoint->GetMinMaxCostDiffRatio(*planList) == pDiffC);
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "CentrePt PlanCosts: ";
		centrePoint->PrintPlanCosts();
		cout << endl;

		cout << "CentrePointReal (Evaluated): ";
		centrePoint->PrintRealCoord();
		cout << endl;
		centrePoint->PrintIntCoord();
		cout << "pDiffC: " << pDiffC;
		cout << endl << endl;
	}
#endif
	if(pDiffC <= PD_THRESHOLD) {
		if(2 == Nipqo_t::MethodNum()) {
			assert(FALSE == pOpt->PartitioningVertexListContains(centrePoint));

			centrePoint->UpdateMinCostPlanList();
			// assert(NumVaryingParam() < centrePoint->MinCostPlansListSize());
			assert(planList->Size() == centrePoint->MinCostPlansListSize());
			pOpt->AddPartitioningVertex(centrePoint);

			// return 0;
			assert((1 == Nipqo_t::OptPhase()) || (2 == Nipqo_t::OptPhase()));
			if(1 == Nipqo_t::OptPhase()) return FALSE; // find rest of the vertices
			else return TRUE; // found the vertex we are searching for
		}

		assert(0);	//Chetas- code unreachable.
		assert(1 == Nipqo_t::MethodNum());
		assert(-1 == Nipqo_t::OptPhase());

		if(TRUE == centreOptimized) {
#ifdef ANIPQO_DEBUG
			if(Nipqo_t::PrintTrace())
				cout << "CentrePoint already optimized" << endl;
#endif
			return 0;
		}
#ifdef ANIPQO_DEBUG
		if(Nipqo_t::PrintTrace())
			cout << "CentrePoint is being optimized" << endl;
#endif
		return OptimizeCentre();
	}
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace())
		cout << endl;
#endif
	if(0 == partitionList.Size()) {
		if(IsPartitionable()) {
			if(0 == partitionList.Size()) CreatePartitions();
			assert(0 < partitionList.Size());
		}
		else {
			if(2 == Nipqo_t::MethodNum()) {
				Boolean_t centrePointIsRectVertex = FALSE;
				for(i = 0; i < NumVertices(); i++) {
					if(centrePoint == vertices[i]) {
						centrePointIsRectVertex = TRUE;
						break;
					}
				}

				if(centrePointIsRectVertex) {
#if 1
					assert(0);	//Chetas- code unreachable.
					// AdjustCentrePoint(*planList, pDiffC);
#else
					for(i = 0; i < NumVertices(); i++) {
						float tmpDiff = vertices[i]->GetMinMaxCostDiffRatio(*planList);
						if(tmpDiff < pDiffC) {
							centrePoint = vertices[i];
							pDiffC = tmpDiff;
						}
					}
#endif
				}

#if 1
				if(pOpt->PartitioningVertexListContains(centrePoint)) {
					centrePoint->ChangeDiffThreshold(pDiffC);
				}
				else
#endif
				{
					// assert(FALSE == pOpt->PartitioningVertexListContains(centrePoint));
					centrePoint->UpdateMinCostPlanList(pDiffC);
					// assert(NumVaryingParam() < centrePoint->MinCostPlansListSize());
					assert(planList->Size() == centrePoint->MinCostPlansListSize());
					pOpt->AddPartitioningVertex(centrePoint);
				}
				// return 0;
				assert((1 == Nipqo_t::OptPhase()) || (2 == Nipqo_t::OptPhase()));
				if(1 == Nipqo_t::OptPhase()) return FALSE; // find rest of the vertices
				else return TRUE; // found the vertex we are searching for
			}

			assert(0);		//Chetas- code unreachable.
			assert(1 == Nipqo_t::MethodNum());
			assert(-1 == Nipqo_t::OptPhase());

			if(TRUE == centreOptimized) return 0;
			return OptimizeCentre();
		}
	}

	return OptimizePartitions(planList);
}

int Rectangle_t::NumOptPlans(MyList_t<PlanId_t> &planList, Boolean_t toShow)
{
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace() && toShow == TRUE) {
		cout << "In Rectangle_t::NumOptPlans (n_v = " << NumVertices() << "): ";
		PrintVertices(TRUE);
	}
#endif
	assert(0 < NumVaryingParam());
	assert(1 < NumVertices());
	assert(pOpt->Id() != pOptIdOptimizedLast);

	int i;
	int numDistinctPlans = 0;
	Boolean_t *optPlanBits = new Boolean_t[planList.Size()];
	assert(optPlanBits);
	for(i = 0; i < planList.Size(); i++) {
		optPlanBits[i] = FALSE;
	}
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace() && toShow == TRUE) {
		cout << "Active Plans: ";
		for(i = 0; i < planList.Size(); i++)
			cout << planList.Entry(i) << ", ";
		cout << endl;
	}
#endif
	for(i = 0; i < NumVertices(); i++) {
		Point_t *vertex = vertices[i];
		assert(vertex);

		CostVal_t minCost;

		for(int planIndex = 0; planIndex < planList.Size(); planIndex++) {
			PlanId_t planId = planList.Entry(planIndex);
			assert(0 < planId); assert(planId <= Nipqo_t::NumPlans());
			CostVal_t cost = vertex->GetPlanCost(planId);

			if((0 == planIndex) || (cost < minCost)) minCost = cost;
		}

		for(int planIndex = 0; planIndex < planList.Size(); planIndex++) {
			PlanId_t planId = planList.Entry(planIndex);
			assert(0 < planId); assert(planId <= Nipqo_t::NumPlans());
			CostVal_t cost = vertex->GetPlanCost(planId);
			assert(minCost <= cost);
			float pDiffC = (cost - minCost)/minCost;

			// if(pDiffC <= PD_THRESHOLD)
			if((pDiffC <= vertex->GetDiffThreshold())
					&& (FALSE == optPlanBits[planIndex])) {
				numDistinctPlans++;
				optPlanBits[planIndex] = TRUE;
			}
		}
	}
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace()) {
		cout << "numDistinctPlans: " << numDistinctPlans << ", optPlans: ";
		for(i = 0;  i < planList.Size(); i++)
			if(TRUE == optPlanBits[i]) cout << planList.Entry(i) << " ";
		cout << endl;
	}
#endif
	delete optPlanBits;

	assert(0 < numDistinctPlans); assert(numDistinctPlans <= planList.Size());
	return numDistinctPlans;
}
