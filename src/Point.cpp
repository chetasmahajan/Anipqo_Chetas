#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cmath>
#include <iostream>
#include <string.h>

#include "TypeDefs.h"
#include "other/List.h"
#include "MyList.h"
#include "Nipqo.h"
#include "Point.h"

#include "Plan.h"	//Chetas- Newly added.

template<>		//Chetas Line added to compile successfully
int AppendDelList_t<Point_t *>::IsEqual(Point_t *const &x, Point_t *const &y) const
		{ return x == y; }

// check that planCostList is sorted on planIds
Boolean_t Point_t::PlanCostListIsOrdered(void)
{
	PlanId_t prevPlandId = -1;
	for(int i = 0; i < planCostList.Size(); i++) {
		assert(0 < planCostList.Entry(i).PlanId());
		if(planCostList.Entry(i).PlanId() <= prevPlandId) return FALSE;
		prevPlandId = planCostList.Entry(i).PlanId();
	}

	return TRUE;
}

CostVal_t Point_t::GetMaxCost(Boolean_t *planBitMap)
{
	assert(planBitMap);

	int listPos = -1; CostVal_t maxCost; Boolean_t firstEntry = TRUE;
	for(int i = 1; i <= Nipqo_t::NumPlans(); i++) {
		if(FALSE == planBitMap[i]) continue;

		listPos++;
		while(planCostList.Entry(listPos).PlanId() < i) listPos++;
		assert(listPos < planCostList.Size());
		assert(planCostList.Entry(listPos).PlanId() == i);

		if(TRUE == firstEntry) {
			maxCost = planCostList.Entry(listPos).Cost();
			firstEntry = FALSE;
		}
		else if(maxCost < planCostList.Entry(listPos).Cost()) {
			maxCost = planCostList.Entry(listPos).Cost();
		}
	}

	assert(FALSE == firstEntry);

	return maxCost;
}

//Chetas- Commenting & adding for Correct version.
//CostVal_t Point_t::GetPlanCost(PlanId_t planId)
CostVal_t Point_t::GetPlanCost(PlanId_t planId, CostVal_t planCost, int *fpcErr)
{
	assert(0 < planId); assert(planId <= Nipqo_t::NumPlans());
	assert(TRUE == PlanCostListIsOrdered());

	Boolean_t found = FALSE;
	int j;
	for(j = 0; j < planCostList.Size(); j++) {
		if(planCostList.Entry(j).PlanId() == planId) {
			found = TRUE;
			/*if(planCost != 0 && planCostList.Entry(j).Cost() != planCost)
			{
				assert(planCostList.Entry(j).Cost() >= planCost);
//				planCostList.Entry(j).SetCost(planCost);
				planCostList.DeleteEntryNum(j);
				PlanCostPair_t newPair(planId, planCost);
				planCostList.Insert(newPair, j);
				if((-1 == minCost) || (planCost < minCost)) {
					minCost = planCost;
					optPlan = planId;
				}
			}*/
			break;
		}

		if(planId < planCostList.Entry(j).PlanId()) break;
	}

	if(TRUE == found) {
		assert(j < planCostList.Size());
		return planCostList.Entry(j).Cost();
	}

	// the scope of j extends outside the for loop?
	if(0 < j) assert(planCostList.Entry(j-1).PlanId() < planId);
	if(j < planCostList.Size()) assert(planId < planCostList.Entry(j).PlanId());

//	CostVal_t cost = OptimizerInterface_t::GenerateQueryAndEvaluatePlanCost(intCoord, planId);
	CostVal_t cost = planCost;
	if(cost == 0)
		cost = Plan::GenerateQueryAndEvaluatePlanCost(intCoord, planId, fpcErr);		//Chetas- Replaced with above line.
	assert(0 <= cost);

	PlanCostPair_t entry(planId, cost);
	planCostList.Insert(entry, j);

	if((-1 == minCost) || (cost < minCost)) {
		//Chetas- Commented for correction.
		//		if(AlmostEqual(cost, minCost) || (FALSE == optimized));
		//		minCost = cost;
		//		optPlan = planId;
		//Chetas- Added for logical correction.
		//Chetas- Commented & added for Correct version.
		/*if(AlmostEqual(cost, minCost) || (FALSE == optimized)) {
			minCost = cost;
			optPlan = planId;
		}*/
		minCost = cost;
		optPlan = planId;
	}

	return cost;
}

//Chetas- Commenting & adding for Correct version.
//PlanId_t Point_t::Optimize(void)
PlanId_t Point_t::Optimize(CostVal_t *planCost)
{
	int i;

	assert(FALSE == optimized);
	int oldNumPlans = Nipqo_t::NumPlans();
	CostVal_t optCost;

	if(FALSE == Nipqo_t::UseCostLimit()) {
//		optPlan = OptimizerInterface_t::OptimizePoint(intCoord, &optCost);
		optPlan = Plan::optimizePoint(intCoord, &optCost);		//Chetas- Replace with above line.
		optimizerOptPlan = optPlan;
		optimizerOptCost = optCost;
	}
	else {
		CostVal_t costLimit = 1e10;

		if(FALSE == Nipqo_t::UseConstCostLimit()) {
			for(i = 0; i < planCostList.Size(); i++) {
				CostVal_t planCost = planCostList.Entry(i).Cost();
				assert(planCost <= 1e10);
				if(planCost < costLimit) costLimit = planCost;
			}
		}
		else costLimit = Nipqo_t::ConstCostLimit();
		assert(0 <= costLimit);

//		optPlan = OptimizerInterface_t::OptimizePoint(intCoord, &optCost, &costLimit);
		optPlan = Plan::optimizePoint(intCoord, &optCost);		//Chetas- Replaced with above line.
	}

	assert(oldNumPlans <= Nipqo_t::NumPlans());
	assert(0 < optPlan);
	assert(optPlan <= Nipqo_t::NumPlans());
	assert(0 <= optCost);
	optimized = TRUE;

	assert(TRUE == PlanCostListIsOrdered());
	Boolean_t found = FALSE;
	for(i = 0; i < planCostList.Size(); i++) {
		if(planCostList.Entry(i).PlanId() == optPlan) {
			// assert(planCostList.Entry(i).Cost() == optCost);
			float tempCost = planCostList.Entry(i).Cost();
//			assert(AlmostEqual(tempCost, optCost));
//			if(!AlmostEqual(tempCost, optCost)) {
//				cout<<"tempCost = "<<tempCost<<", optCost = "<<optCost<<endl;
//				assert(0);
//			}
			found = TRUE;
			break;
		}

		if(optPlan < planCostList.Entry(i).PlanId()) break;
	}

	if(TRUE == found) {
		assert(i < planCostList.Size());
		assert(oldNumPlans == Nipqo_t::NumPlans());
		// assert(planCostList.Entry(i).Cost() == optCost);
//		assert(AlmostEqual(planCostList.Entry(i).Cost(), optCost));
		return optPlan;
	}

	assert(i <= planCostList.Size());
	if(0 < i) assert(planCostList.Entry(i-1).PlanId() < optPlan);
	if(i < planCostList.Size()) assert(optPlan < planCostList.Entry(i).PlanId());

	if((0 <= minCost) && (minCost < optCost)) {
		float pDiff = (optCost - minCost)/minCost;
//		assert(pDiff <= PD_THRESHOLD);
		cout<<"Got an suboptimal plan after optimizing vertex having optCost = "<<optCost<<". Hahaha. Optimal plan is already known having mincost = "<<minCost<<".//////////////////////////\n";
		optCost = minCost;
	}

	//Chetas- Adding for Correct version.
	*planCost = optCost;

	//Chetas- Commenting to not insert plan which is co-optimal.
/*	PlanCostPair_t pcp(optPlan, optCost);
	planCostList.Insert(pcp, i);
	assert(TRUE == PlanCostListIsOrdered());*/

#if 0
	/*****************************************************************/
	// Optimizer may return slightly inferior plan as an optimal plan.
	// So the assert may fail (well, failed once :-)
	// assert((minCost < 0) || (optCost <= minCost));
	if((0 <= minCost) && (minCost < optCost)) {
		float pDiff = (optCost - minCost)/minCost;
		assert(pDiff <= PD_THRESHOLD);

		for(int i = 0; i < planCostList.Size(); i++) {
			PlanId_t plan = planCostList.Entry(i).PlanId();
			CostVal_t cost = planCostList.Entry(i).Cost();

			assert(minCost <= cost);
			if(cost == minCost) {
				optPlan = plan;
				optCost = minCost;
			}
		}

		assert(minCost == optCost);
	}
	/*****************************************************************/
	else
#endif
	//Chetas- Commenting to not insert plan which is co-optimal.
/*	{
		minCost = optCost;
	}*/

	return optPlan;
}

void Point_t::UpdateMinCostPlanList(float newThreshold) // -1
{
#ifdef ANIPQO_DEBUG
	if(Nipqo_t::PrintTrace())
		if(-1 != newThreshold) {
			cout << "UpdateMinCostPlanList: ";
			PrintIntCoord();
			cout << "diffThreshold = " << diffThreshold;
			cout << "; newThreshold = " << newThreshold << endl;
		}
#endif
	assert(2 == Nipqo_t::MethodNum());
	if(-1 != newThreshold) {
		//Chetas- Commented & added for Correct version.
/*		assert(PD_THRESHOLD == diffThreshold);
		assert(PD_THRESHOLD < newThreshold);*/
		assert(diffThreshold <= newThreshold);

		assert(0 < newThreshold);
		/****************************************************************************/
		// assert(newThreshold < 1); // Will this be false?
		// yes it may be false and hence commented to accomodate new method two
		/****************************************************************************/
		diffThreshold = newThreshold;
	}

	// assert(0 <= minCost);
	if(minCost < 0) {
		// assert(0); // shouldnt be called.
		assert(-1 == minCost);
		assert(-1 == auxMinCost);
		assert(planCostList.IsEmpty());
		assert(minCostPlans.IsEmpty());
		return;
	}

	minCostPlans.MakeEmpty();

	for(int i = 0; i < planCostList.Size(); i++) {
		PlanId_t plan = planCostList.Entry(i).PlanId();
		CostVal_t cost = planCostList.Entry(i).Cost();

		float pDiffC = (cost - minCost)/minCost;
		assert(0 <= pDiffC);

		if(pDiffC <= diffThreshold) {
			minCostPlans.Insert(plan);
		}
	}

	auxMinCost = minCost;
}

int Point_t::NumCommonMinCostPlans(Point_t *otherPoint)
{
	int numCommonPlans = 0;

	ListIter_t<PlanId_t> planIter;
	planIter.Attach(&minCostPlans);

	while(!planIter.IsEnd()) {
		PlanId_t plan = planIter.Next();

		if(otherPoint->minCostPlans.Contains(plan)) {
			numCommonPlans++;
		}
	}

	return numCommonPlans;
}

Boolean_t Point_t::IsParameterSpaceHyperRectangleVertex(void)
{
	for(int i = 0; i < Nipqo_t::NumParam(); i++) {
		if((0 != realCoord[i]) && (1 != realCoord[i])) return FALSE;
	}

	return TRUE;
}

int Point_t::NumCommonBoundaries(Point_t *otherPoint)
{
	int boundaryNum = 0;

	for(int i = 0; i < Nipqo_t::NumParam(); i++) {
		if(((0 == realCoord[i]) || (1 == realCoord[i]))
				&& (realCoord[i] == otherPoint->realCoord[i])) boundaryNum++;
	}

	assert(0 <= boundaryNum);
	assert(boundaryNum <= Nipqo_t::NumParam());

	return boundaryNum;
}

void Point_t::CommonMinCostPlansList(Point_t *otherPoint, MyList_t<PlanId_t> &commonMinPlanList)
{
	assert(commonMinPlanList.IsEmpty());

	ListIter_t<PlanId_t> planIter;
	planIter.Attach(&minCostPlans);

	while(!planIter.IsEnd()) {
		PlanId_t plan = planIter.Next();

		if(otherPoint->minCostPlans.Contains(plan)) {
			commonMinPlanList.Append(plan);
		}
	}
}

Boolean_t Point_t::MinCostPlanListContains(PlanId_t plan)
{
	assert(0 < plan);
	assert(plan <= Nipqo_t::NumPlans());
	return minCostPlans.Contains(plan);
}

CostVal_t Point_t::GetMinCost(MyList_t<PlanId_t> *planList)
{
	CostVal_t minCost;

	if(planList) {
		assert(0 < planList->Size());

		for(int i = 0; i < planList->Size(); i++) {
			PlanId_t planId = planList->Entry(i);
			assert(0 < planId); assert(planId <= Nipqo_t::NumPlans());
			CostVal_t cost = GetPlanCost(planId);
			if((0 == i) || (cost < minCost)) minCost = cost;
		}

		return minCost;
	}

	assert(0 < planCostList.Size());
	for(int i = 0; i < planCostList.Size(); i++) {
		CostVal_t cost = planCostList.Entry(i).Cost();
		if((0 == i) || (cost < minCost)) minCost = cost;
	}

	return minCost;
}

//Chetas- Added for Correct Version.
CostVal_t Point_t::GetMinCostPlan(MyList_t<PlanId_t> *planList)
{
	CostVal_t minCost;
	PlanId_t minCostPlan;

	if(planList) {
		assert(0 < planList->Size());

		for(int i = 0; i < planList->Size(); i++) {
			PlanId_t planId = planList->Entry(i);
			assert(0 < planId); assert(planId <= Nipqo_t::NumPlans());
			CostVal_t cost = GetPlanCost(planId);
			if((0 == i) || (cost < minCost)) {
				minCost = cost;
				minCostPlan = planId;
			}
		}

		return minCostPlan;
	}

	assert(0 < planCostList.Size());
	for(int i = 0; i < planCostList.Size(); i++) {
		CostVal_t cost = planCostList.Entry(i).Cost();
		PlanId_t planId = planCostList.Entry(i).PlanId();
		if((0 == i) || (cost < minCost)) {
			minCost = cost;
			minCostPlan = planId;
		}
	}

	return minCostPlan;
}

CostVal_t Point_t::GetMinMaxCostDiffRatio(MyList_t<PlanId_t> *planList)
{
	CostVal_t minCost = GetMinCost(planList);
	CostVal_t maxCost = GetMaxCost(planList);
	assert(minCost <= maxCost);

	float pDiffC = (maxCost - minCost)/minCost;
	assert(0 <= pDiffC);
	return pDiffC;
}

CostVal_t Point_t::GetMaxCost(MyList_t<PlanId_t> *planList) // = NULL
{
	assert(planList);
	assert(0 < planList->Size());
	CostVal_t maxCost;

	for(int i = 0; i < planList->Size(); i++) {
		PlanId_t planId = planList->Entry(i);
		assert(0 < planId); assert(planId <= Nipqo_t::NumPlans());
		CostVal_t cost = GetPlanCost(planId);
		if((0 == i) || (maxCost < cost)) maxCost = cost;
	}

	return maxCost;
}

CostVal_t Point_t::GetMaxOptCost(void)
{
//	assert(0 < minCostPlans.Size());
	CostVal_t maxCost = 0;
	ListIter_t<PlanId_t> planIter;

	planIter.Attach(&minCostPlans);
	while(!planIter.IsEnd()) {
		PlanId_t planId = planIter.Next();
		assert(0 < planId); assert(planId <= Nipqo_t::NumPlans());
		CostVal_t cost = GetPlanCost(planId);
		if(maxCost < cost) maxCost = cost;
	}
	return maxCost;
}

PlanId_t Point_t::GetMaxOptCostPlan(MyList_t<PlanId_t> *planList, PlanId_t exceptPlan)	// = NULL
{
	CostVal_t maxCost = 0;
	PlanId_t maxCostPlan = 0;
	ListIter_t<PlanId_t> planIter;

	if(planList != NULL) {
		for(int i = 0; i < planList->Size(); i++) {
			PlanId_t planId = planList->Entry(i);
			assert(0 < planId); assert(planId <= Nipqo_t::NumPlans());
			if(planId == exceptPlan)
				continue;
			CostVal_t cost = GetPlanCost(planId);
			if(maxCost < cost) {
				maxCost = cost;
				maxCostPlan = planId;
			}
		}
	}
	else {
		planIter.Attach(&minCostPlans);
		while(!planIter.IsEnd()) {
			PlanId_t planId = planIter.Next();
			assert(0 < planId); assert(planId <= Nipqo_t::NumPlans());
			if(planId == exceptPlan)
				continue;
			CostVal_t cost = GetPlanCost(planId);
			if(maxCost < cost) {
				maxCost = cost;
				maxCostPlan = planId;
			}
		}
	}
	return maxCostPlan;
}

int Point_t::numParams(void) {		//Chetas- Added for Aggressive version.
	return Nipqo_t::NumParam();
}

//Chetas- Added for Correct version.
Point_t *Point_t::adjustPartitioningVertex(void)
{
	Point_t *minPoint = NULL;
	int axis = -1, varyingAxis;
	for(int i = 0; i < numParams(); i++) {
		if(GetRealCoord(i) == 0 || GetRealCoord(i) == 1) {
			axis = i;
			break;
		}
	}
	assert(axis > -1 && axis < numParams());
	varyingAxis = (axis + 1) % 2;

	AppendDelList_t<Point_t *> list;
	if(GetRealCoord(axis) == 0) {
		int cnt = -1;
		for(int i = 0; i < 3; i++) {
			Point_t *pt;
			int array[numParams()];
			array[axis] = GetIntCoord(axis) + 1;
			array[varyingAxis] = GetIntCoord(varyingAxis) + cnt++;
			pt = new Point_t(array);
			if(pt->BoundaryNum() >= 1) {
				delete pt;
				continue;
			}
			else
				list.Insert(pt);
		}
	}
	else {
		int cnt = -1;
		for(int i = 0; i < 3; i++) {
			Point_t *pt;
			int array[numParams()];
			array[axis] = GetIntCoord(axis) - 1;
			array[varyingAxis] = GetIntCoord(varyingAxis) + cnt++;
			pt = new Point_t(array);
			if(pt->BoundaryNum() >= 1) {
				delete pt;
				continue;
			}
			else
				list.Insert(pt);
		}
	}
	assert(list.Size() >= 1);
	int *plans, size;
	size = GetMinCostPlans(&plans);
	assert(plans);
	MyList_t<PlanId_t> planList;
	for(int i = 0; i < size; i++)
		planList.Append(plans[i]);
	ListIter_t<Point_t *> itr;
	itr.Attach(&list);
	float minDiff = 1000000;
	while(!itr.IsEnd()) {
		Point_t *pt = itr.Next();
		CostVal_t maxCostTmp = pt->GetMaxCost(&planList);
		CostVal_t minCostTmp = pt->GetMinCost();

		float tmpDiff = (maxCostTmp - minCostTmp)/minCostTmp;
		assert(0 <= tmpDiff);
		if(tmpDiff < minDiff) {
			minDiff = tmpDiff;
			minPoint = pt;
		}
	}
	minPoint->ChangeDiffThreshold(minDiff);
	return minPoint;
}
