#ifndef __POINT_H__
#define __POINT_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cmath>
#include <iostream>
#include <string.h>
#include "TypeDefs.h"
#include "Nipqo.h"
#include "other/List.h"
#include "MyList.h"

using namespace std;
class Point_t;

//const CostVal_t PD_THRESHOLD = 0.1;
extern CostVal_t PD_THRESHOLD;

class Rectangle_t;
class pOptimizer_t;

class Point_t {
	RealCoord_t *realCoord;
	IntCoord_t *intCoord;
	MyList_t<PlanCostPair_t>planCostList;
	Boolean_t optimized;
	PlanId_t optPlan;
	PlanId_t inferedOptPlan;		//Chetas- Added for Correct version.
	CostVal_t inferedOptCost;		//Chetas- Added for Correct version.
	PlanId_t optimizerOptPlan;		//Chetas- Added for Correct version.
	CostVal_t optimizerOptCost;		//Chetas- Added for Correct version.
	CostVal_t minCost; // Can remove this; get cost of optPlan from planCostList
	CostVal_t auxMinCost; // Can remove this; get cost of optPlan from planCostList
	AppendDelList_t<PlanId_t>minCostPlans; // used in method2

	float diffThreshold;

	int boundaryNum;
	Boolean_t visited; // used by method two
	Boolean_t toBeDeleted; // used by method two

	Point_t *next; // Used in point hash-table

	Boolean_t PlanCostListIsOrdered(void);

public:
	bool isincluded;				//Chetas- Added for Correct version.
	Point_t(void)  // Not used?
: optimized(FALSE), optPlan(0), minCost(-1), auxMinCost(-1),
  planCostList(0), diffThreshold(PD_THRESHOLD), boundaryNum(-1),
  visited(FALSE), toBeDeleted(FALSE), next(NULL) {

		realCoord = new RealCoord_t[Nipqo_t::NumParam()];
		assert(realCoord);
		intCoord = new IntCoord_t[Nipqo_t::NumParam()];
		assert(intCoord);

		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
			realCoord[i] = -1;
			intCoord[i] = -1;
		}
	}

	Point_t(Point_t *inp)
	: optimized(FALSE), optPlan(0), minCost(-1), auxMinCost(-1),
	  planCostList(0), diffThreshold(PD_THRESHOLD), boundaryNum(-1),
	  visited(FALSE), toBeDeleted(FALSE), next(NULL) {

		realCoord = new RealCoord_t[Nipqo_t::NumParam()];
		assert(realCoord);
		intCoord = new IntCoord_t[Nipqo_t::NumParam()];
		assert(intCoord);

		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
			realCoord[i] = inp->realCoord[i];
			intCoord[i] = inp->intCoord[i];
		}
	}

	Point_t(RealCoord_t *coordArray)
	: optimized(FALSE), optPlan(0), minCost(-1), auxMinCost(-1),
	  planCostList(0), diffThreshold(PD_THRESHOLD), boundaryNum(-1),
	  visited(FALSE), toBeDeleted(FALSE), next(NULL) {

		realCoord = new RealCoord_t[Nipqo_t::NumParam()];
		assert(realCoord);
		intCoord = new IntCoord_t[Nipqo_t::NumParam()];
		assert(intCoord);

		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
			realCoord[i] = coordArray[i];
			intCoord[i] = Nipqo_t::GetPredConst(realCoord[i], i);
		}
	}

	Point_t(IntCoord_t *coordArray) // Not used?
	: optimized(FALSE), optPlan(0), minCost(-1), auxMinCost(-1),
	  planCostList(0), diffThreshold(PD_THRESHOLD), boundaryNum(-1),
	  visited(FALSE), toBeDeleted(FALSE), next(NULL) {

		realCoord = new RealCoord_t[Nipqo_t::NumParam()];
		assert(realCoord);
		intCoord = new IntCoord_t[Nipqo_t::NumParam()];
		assert(intCoord);

		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
			realCoord[i] = Nipqo_t::GetPredFloat(coordArray[i], i);
			intCoord[i] = coordArray[i];
		}
	}

	RealCoord_t GetRealCoord(int coordNum)
	{
		assert(0 <= coordNum); assert(coordNum < Nipqo_t::NumParam());
		return realCoord[coordNum];
	}

	IntCoord_t GetIntCoord(int coordNum)
	{
		assert(0 <= coordNum); assert(coordNum < Nipqo_t::NumParam());
		return intCoord[coordNum];
	}

	// RealCoord_t *GetRealCoordArray(void) { return realCoord; } // Not used
	IntCoord_t *GetIntCoordArray(void) { return intCoord; }
#if 0
	void SetRealCoord(int coordNum, RealCoord_t cin)
	{
		assert(0 <= coordNum); assert(coordNum < Nipqo_t::NumParam());
		assert(0 <= cin); assert(cin <= 1);
		realCoord[coordNum] = cin;
		intCoord[coordNum] = Nipqo_t::GetPredConst(realCoord[coordNum], coordNum);
	}

	void SetIntCoord(int coordNum, IntCoord_t cin)
	{
		assert(0 <= coordNum); assert(coordNum < Nipqo_t::NumParam());
		intCoord[coordNum] = cin;
	}
#endif
	PlanId_t OptPlan(void) { return optPlan; }
	CostVal_t OptCost(void) { if(-1 == minCost) return 10000000.0; return minCost; }
	CostVal_t AuxMinCost(void) { return auxMinCost; }

	CostVal_t GetMaxCost(Boolean_t *planBitMap);

	//Chetas- Commenting & adding for Correct version.
//	CostVal_t GetPlanCost(PlanId_t planId);
	CostVal_t GetPlanCost(PlanId_t planId, CostVal_t planCost = 0, int *fpcErr = NULL);

	Boolean_t IsOptimized(void) { return optimized; }

	//Chetas- Commenting & adding for Correct version.
//	PlanId_t Optimize(void);
	PlanId_t Optimize(CostVal_t *planCost);

	Boolean_t IsEqual(RealCoord_t *rc) {
		for(int i = 0; i < Nipqo_t::NumParam(); i++)
			if(intCoord[i] != Nipqo_t::GetPredConst(rc[i], i)) return FALSE;

		return TRUE;
	}

	Boolean_t IsEqual(IntCoord_t *rc) {
		for(int i = 0; i < Nipqo_t::NumParam(); i++)
			if(intCoord[i] != rc[i]) return FALSE;

		return TRUE;
	}

	Boolean_t IsEqualExcept(Point_t& point2, int differentInAxis = -1) {
		assert(-1 <= differentInAxis);
		assert(differentInAxis < Nipqo_t::NumParam());

		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
			if(i == differentInAxis) {
				if(intCoord[i] == point2.intCoord[i]) return FALSE;
			}
			else {
				if(intCoord[i] != point2.intCoord[i]) return FALSE;
			}
		}

		return TRUE;
	}

	Boolean_t IsEqual(Point_t& point2) { return IsEqualExcept(point2, -1); }

	int PlanCostListSize(void) { return planCostList.Size(); }
	int MinCostPlansListSize(void) { return minCostPlans.Size(); }

	AppendDelList_t<PlanId_t> &MinCostPlans(void)
    		{ return minCostPlans; }

	void UpdateMinCostPlanList(float newThreshold = -1);
	int NumCommonMinCostPlans(Point_t *otherPoint);
	void CommonMinCostPlansList(Point_t *otherPoint, MyList_t<PlanId_t> &commonMinPlanList);
	Boolean_t MinCostPlanListContains(PlanId_t plan);

	Boolean_t IsParameterSpaceHyperRectangleVertex(void);
	int NumCommonBoundaries(Point_t *otherPoint);

	void SetVisited(void) { visited = TRUE; }
	void ResetVisited(void) { visited = FALSE; }
	Boolean_t IsVisited(void) { return visited; }

	void SetToBeDeleted(void) { toBeDeleted = TRUE; }
	void ResetToBeDeleted(void) { toBeDeleted = FALSE; }
	Boolean_t IsToBeDeleted(void) { return toBeDeleted; }

	CostVal_t GetMinCost(MyList_t<PlanId_t> *planList = NULL);
	CostVal_t GetMinCostPlan(MyList_t<PlanId_t> *planList = NULL);
	CostVal_t GetMaxCost(MyList_t<PlanId_t> *planList = NULL);
	CostVal_t GetMinMaxCostDiffRatio(MyList_t<PlanId_t> *planList);
	CostVal_t GetMaxOptCost(void);
	PlanId_t GetMaxOptCostPlan(MyList_t<PlanId_t> *planList = NULL, PlanId_t exceptPlan = 0);
	Point_t *adjustPartitioningVertex(void);		//Chetas- Added for Correct Version.

	int BoundaryNum(void)
	{
		assert(-1 <= boundaryNum);
		assert(boundaryNum <= Nipqo_t::NumParam());

		if(-1 < boundaryNum) return boundaryNum;

		boundaryNum = 0;
		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
			// What if realCoord is to be dispensed with?
					assert((0 <= realCoord[i]) || (realCoord[i] <= 1));
					if((0 == realCoord[i]) || (1 == realCoord[i])) boundaryNum++;
		}

		assert(0 <= boundaryNum);
		assert(boundaryNum <= Nipqo_t::NumParam()); // Relace Nipqo_t::NumParam() by pOpt->NumVaryingParam() ???

		return boundaryNum;
	}

	void ChangeDiffThreshold(float newThreshold) {
		// What if newThreshold < diffThreshold?
		// May need to change only if diffThreshold < newThreshold
#ifdef ANIPQO_DEBUG
		if(Nipqo_t::PrintTrace()) {
			cout << "ChangeDiffThreshold: ";
			PrintIntCoord();
			cout << "diffThreshold = " << diffThreshold;
			cout << "; newThreshold = " << newThreshold << endl<<endl;
		}
#endif
//		 assert(diffThreshold <= newThreshold);
		//Chetas- Commented and added to change threshold.
//		if(diffThreshold <= newThreshold)
//			return;
		if(diffThreshold >= newThreshold) {
			cout<<"New threshold is not greater. So, not changed.\n\n";
			return;
		}

		diffThreshold = newThreshold;
	}

	float GetDiffThreshold(void) { return diffThreshold; }

	void resetDiffThreshold(void) { diffThreshold = PD_THRESHOLD; }		//Chetas- Added for Correct version.

	void PrintRealCoord(Boolean_t insertNewLine = FALSE) {
		cout << "[";
		for(int i = 0; i < Nipqo_t::NumParam(); i++) cout << realCoord[i] << " ";
		cout << "] ";
		if(insertNewLine) cout << endl;
	}

	void PrintIntCoord(Boolean_t insertNewLine = FALSE) {
		cout << "[";
		for(int i = 0; i < Nipqo_t::NumParam(); i++) cout << intCoord[i] << " ";
		cout << "] ";
		if(insertNewLine) cout << endl;
	}

	void PrintPlanCosts(Boolean_t insertNewLine = FALSE) {
		for(int i = 0; i < planCostList.Size(); i++) {
			cout << "[P:" << planCostList.Entry(i).PlanId() << ", C:";
			cout << planCostList.Entry(i).Cost() << "] ";
		}

		if(insertNewLine) cout << endl;
	}

	void PrintMinCostPlans(Boolean_t insertNewLine = FALSE)
	{
		ListIter_t<PlanId_t> planIter;
		planIter.Attach(&minCostPlans);

		while(!planIter.IsEnd()) {
			PlanId_t plan = planIter.Next();
			cout << "[P:" << plan << ", C:";
			cout << GetPlanCost(plan) << "] ";
		}

		if(insertNewLine) cout << endl;
	}

	void SetNext(Point_t *np) { next =  np; }
	Point_t *GetNext(void) { return next; }
	int numParams(void);					//Chetas- Added for Aggressive version.
	int GetMinCostPlans(int **list) {		//Chetas- Added for Correct version.
		ListIter_t<PlanId_t> itr;
		itr.Attach(&minCostPlans);
		*list = new int[minCostPlans.Size()];
		int cnt = 0;
		while(!itr.IsEnd()) {
			(*list)[cnt++] = itr.Next();
		}
		return cnt;
	}
	CostVal_t getFromPlanCostList(PlanId_t planId)
	{
		for(int i = 0; i < planCostList.Size(); i++) {
			PlanId_t plan = planCostList.Entry(i).PlanId();
			if(plan > planId)
				break;
			if(plan == planId)
				return planCostList.Entry(i).Cost();
		}
		return 0;
	}
	//Chetas- Added for computing errors. ********************
	PlanId_t getInferedOptPlan(void) { return inferedOptPlan; }
	CostVal_t getInferedOptCost(void) { return inferedOptCost; }
	void setInferedOptPlan(PlanId_t plan) { inferedOptPlan = plan; }
	void setInferedOptCost(CostVal_t cost) { inferedOptCost = cost; }
	PlanId_t getOptimizerOptPlan(void) { return optimizerOptPlan; }
	CostVal_t getOptimizerOptCost(void) { return optimizerOptCost; }
	void resetPlansHistory(void)
	{
		minCost = -1;
		optPlan = 0;
		planCostList.MakeEmpty();
		minCostPlans.MakeEmpty();
	}
//	void setOptimizerOptPlan(PlanId_t plan) { optimizerOptPlan = plan; }
//	void setOptimizerOptCost(CostVal_t cost) { optimizerOptCost = cost; }
	//*********************************************************

	~Point_t(void) {
		delete realCoord;
		delete intCoord;
	}
};

class Edge_t {
	Point_t *point1;
	Point_t *point2;

public:
	Edge_t(Point_t *p1, Point_t *p2)
: point1(p1), point2(p2)
{
		assert(point1);
		assert(point2);
		assert(point1 != point2);
		assert(FALSE == point1->IsEqual(*point2)); // creating a copy of *point2?
}

	Point_t *EndPoint1(void) { return point1; }
	Point_t *EndPoint2(void) { return point2; }

	Boolean_t IsEqual(Edge_t *edge)
	{
		assert(edge);
		if(((point1 == edge->point1) && (point2 == edge->point2))
				|| ((point2 == edge->point1) && (point1 == edge->point2))) {
			return TRUE;
		}

		return FALSE;
	}
};

#endif // __POINT_H__
