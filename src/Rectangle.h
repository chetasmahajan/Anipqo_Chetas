#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__

#include "HashSet.h"
#include "Point.h"

class pOptimizer_t;

extern PlanId_t newPlanID;	//Chetas Added for correct version.

class Rectangle_t {
	pOptimizer_t *pOpt;
	int numVaryingParam;
	RealCoord_t *lowCoord;
	RealCoord_t *highCoord;
	Point_t **vertices;
	RectSideStatus_t *rectSideStatus;
	AppendList_t<Rectangle_t *> partitionList;
	MyList_t<PlanCostPair_t> minPlanCostList;
	CostVal_t minMaxCost;
	int numDistinctPlans1, numDistinctPlans2; // numDistinctPlans2 not used.	Chetas- need to remove numDistinctPlans2.
	CostVal_t pDiff; // Not used	Chetas- need to remove pDiff.

	Point_t *centrePoint;
	bool isRectVertex;					//Chetas- Added for Correct Version.
	Boolean_t centreOptimized;
	CostVal_t centreMinCost, centreMaxCost;

	int pOptIdOptimizedLast;
	PlanId_t numPlansEvaluated;
	// not actual PlanId but an index in pOpt::localPlans

	int pOptIdOptimizedLastForCentrePt; // Not used
	PlanId_t numPlansEvalautedForCentrePt; // Not used
	// not actual PlanId but an index in pOpt::localPlans
	//
	int boundaryNum; // # of boundary hyperplane the rect is touching to

	void SetRectSideStatus(int partitioningAxisNum);

	void SetNumVaryingParam(void) {
		numVaryingParam = 0;

		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
			if(lowCoord[i] != highCoord[i]) numVaryingParam++;
		}
	}

	// int NumVertices(void) { return pOpt->NumRectVertices(); }
	int NumVertices(void) { return Utility_t::TwoPowerN(numVaryingParam); }

	Boolean_t IsSquare(void) // Disgarding some DONT_DIVIDEs
	{
		assert(rectSideStatus);
		for(int i = 0; i < Nipqo_t::NumParam(); i++)
			if(DIVIDED == rectSideStatus[i]) return FALSE;
		return TRUE;
	}

	RealCoord_t *CentrePointReal(void);
	IntCoord_t *CentrePoint(void);

	void AssignVertices(void);

	void SetCentrePoint(void);
	PlanId_t OptimizeCentre(void);

	void CreatePartitions(void);
	PlanId_t OptimizePartitions(MyList_t<PlanId_t> *ancestorActivePlanList);

	int BoundaryNum(void)
	{
		assert(-1 <= boundaryNum);
		assert(boundaryNum <= Nipqo_t::NumParam()); // Relace Nipqo_t::NumParam() by pOpt->NumVaryingParam() ???

		if(-1 < boundaryNum) return boundaryNum;

		boundaryNum = 0;
		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
			if(lowCoord[i] == highCoord[i]) continue;
			if((0 == lowCoord[i]) || (1 == highCoord[i])) boundaryNum++;
		}

		assert(0 <= boundaryNum);
		assert(boundaryNum <= Nipqo_t::NumParam()); // Relace Nipqo_t::NumParam() by pOpt->NumVaryingParam() ???

		return boundaryNum;
	}

	// Boolean_t AdjustCentrePoint(MyList_t<PlanId_t> &planList, float &pDiffC);
	Boolean_t AdjustCentrePoint(MyList_t<PlanId_t> &planList,
			float &pDiffC, float centreMinCost, float centreMaxCost);

	Boolean_t SanityCheck(void);

public:
	Rectangle_t(Rectangle_t *parentPartition, int partitioningAxisNum,
			// Point_t **partVertices,
			Boolean_t leftPartition)
: pOpt(parentPartition->pOpt), minMaxCost(-1), numDistinctPlans1(-1),
  numDistinctPlans2(-1), pDiff(-1), centreOptimized(FALSE),
  pOptIdOptimizedLast(-1), numPlansEvaluated(0), centrePoint(NULL),
  pOptIdOptimizedLastForCentrePt(-1), numPlansEvalautedForCentrePt(0),
  centreMinCost(-1), centreMaxCost(-1), boundaryNum(-1), isRectVertex(false) {

		assert(parentPartition);
		// assert(partVertices);
		assert(0 <= partitioningAxisNum);
		assert(partitioningAxisNum < Nipqo_t::NumParam());

		lowCoord = new RealCoord_t[Nipqo_t::NumParam()];
		assert(lowCoord);
		highCoord = new RealCoord_t[Nipqo_t::NumParam()];
		assert(highCoord);
		rectSideStatus = new RectSideStatus_t[Nipqo_t::NumParam()];
		assert(rectSideStatus);

		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
			lowCoord[i] = parentPartition->lowCoord[i];
			highCoord[i] = parentPartition->highCoord[i];
			rectSideStatus[i] = parentPartition->rectSideStatus[i];
		}

		RealCoord_t divisionCoord =
				(parentPartition->lowCoord[partitioningAxisNum]
				                           + parentPartition->highCoord[partitioningAxisNum])/2;

		if(leftPartition) highCoord[partitioningAxisNum] = divisionCoord;
		else lowCoord[partitioningAxisNum] = divisionCoord;

		SetRectSideStatus(partitioningAxisNum);
		SetNumVaryingParam();

		vertices = new Point_t *[NumVertices()];
		assert(vertices);
		AssignVertices();

		// cout << "Created Rectangle: ";
		// PrintVertices();
		// cout << endl;
	}

	Rectangle_t(pOptimizer_t *po)
	: pOpt(po), minMaxCost(-1), numDistinctPlans1(-1),
	  numDistinctPlans2(-1), pDiff(-1), centreOptimized(FALSE),
	  pOptIdOptimizedLast(-1), numPlansEvaluated(0), centrePoint(NULL),
	  pOptIdOptimizedLastForCentrePt(-1), numPlansEvalautedForCentrePt(0),
	  centreMinCost(-1), centreMaxCost(-1), boundaryNum(-1), isRectVertex(false) {
		assert(po);

		lowCoord = new RealCoord_t[Nipqo_t::NumParam()];
		assert(lowCoord);
		highCoord = new RealCoord_t[Nipqo_t::NumParam()];
		assert(highCoord);
		rectSideStatus = new RectSideStatus_t[Nipqo_t::NumParam()];
		assert(rectSideStatus);

		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
			if(po->ConstPred()[i] < 0) {
				lowCoord[i] = 0;
				highCoord[i] = 1;
				rectSideStatus[i] = UNDIVIDED;
			}
			else {
				lowCoord[i] = highCoord[i] = po->ConstPred()[i];
				rectSideStatus[i] = CONSTANT;
			}
		}

		SetNumVaryingParam();

		vertices = new Point_t *[NumVertices()];
		assert(vertices);
		AssignVertices();

		// cout << "Created Rectangle: ";
		// PrintVertices();
		// cout << endl;
	}

	Rectangle_t(pOptimizer_t *po, Point_t *centrePoint, float side)
	: pOpt(po), minMaxCost(-1), numDistinctPlans1(-1),
	  numDistinctPlans2(-1), pDiff(-1), centreOptimized(FALSE),
	  pOptIdOptimizedLast(-1), numPlansEvaluated(0), centrePoint(NULL),
	  pOptIdOptimizedLastForCentrePt(-1), numPlansEvalautedForCentrePt(0),
	  centreMinCost(-1), centreMaxCost(-1), boundaryNum(-1), isRectVertex(false) {
		assert(po);
		assert(centrePoint);
		assert(0 < side);
		assert(side <= 1);

		lowCoord = new RealCoord_t[Nipqo_t::NumParam()];
		assert(lowCoord);
		highCoord = new RealCoord_t[Nipqo_t::NumParam()];
		assert(highCoord);
		rectSideStatus = new RectSideStatus_t[Nipqo_t::NumParam()];
		assert(rectSideStatus);

		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
#if 1
			if((0 == centrePoint->GetRealCoord(i))
					|| (1 == centrePoint->GetRealCoord(i))) {
				lowCoord[i] = highCoord[i] = centrePoint->GetRealCoord(i);
				rectSideStatus[i] = CONSTANT;
			}
			else {
				lowCoord[i] = centrePoint->GetRealCoord(i) - side/2;
				if(lowCoord[i] < 0) lowCoord[i] = 0;

				highCoord[i] = centrePoint->GetRealCoord(i) + side/2;
				if(1 < highCoord[i]) highCoord[i] =1;
#if 1
				IntCoord_t x1i = Nipqo_t::GetPredConst(lowCoord[i], i);
				IntCoord_t x2i = Nipqo_t::GetPredConst(highCoord[i], i);
				assert((x1i + 1) <= x2i);

				if((x1i + 1) == x2i) rectSideStatus[i] = DONT_DIVIDE;
				else rectSideStatus[i] = UNDIVIDED;
#else
				rectSideStatus[i] = UNDIVIDED;
#endif
			}
#else
			if(po->ConstPred()[i] < 0) {
				lowCoord[i] = centrePoint->GetRealCoord(i) - side/2;
				if(lowCoord[i] < 0) lowCoord[i] = 0;

				highCoord[i] = centrePoint->GetRealCoord(i) + side/2;
				if(1 < highCoord[i]) highCoord[i] =1;

				rectSideStatus[i] = UNDIVIDED;
			}
			else {
				assert(centrePoint->GetRealCoord(i) == po->ConstPred()[i]);
				lowCoord[i] = highCoord[i] = po->ConstPred()[i];
				rectSideStatus[i] = CONST;
			}
#endif
		}

		SetNumVaryingParam();

		vertices = new Point_t *[NumVertices()];
		assert(vertices);
		AssignVertices();

		// cout << "Created Rectangle: ";
		// PrintVertices();
		// cout << endl;
	}

	Rectangle_t(pOptimizer_t *po,
			Point_t *externalPoint1, Point_t *externalPoint2,
			Boolean_t maxRect = FALSE)
	: pOpt(po), minMaxCost(-1), numDistinctPlans1(-1),
	  numDistinctPlans2(-1), pDiff(-1), centreOptimized(FALSE),
	  pOptIdOptimizedLast(-1), numPlansEvaluated(0), centrePoint(NULL),
	  pOptIdOptimizedLastForCentrePt(-1), numPlansEvalautedForCentrePt(0),
	  centreMinCost(-1), centreMaxCost(-1), boundaryNum(-1), isRectVertex(false) {
		assert(po);
		assert(externalPoint1);
		assert(externalPoint2);

		lowCoord = new RealCoord_t[Nipqo_t::NumParam()];
		assert(lowCoord);
		highCoord = new RealCoord_t[Nipqo_t::NumParam()];
		assert(highCoord);
		rectSideStatus = new RectSideStatus_t[Nipqo_t::NumParam()];
		assert(rectSideStatus);

		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
			if(((0 == externalPoint1->GetRealCoord(i))
					&& (0 == externalPoint2->GetRealCoord(i)))
					|| ((1 == externalPoint1->GetRealCoord(i))
							&& (1 == externalPoint2->GetRealCoord(i)))) {
				lowCoord[i] = highCoord[i] = externalPoint1->GetRealCoord(i);
				rectSideStatus[i] = CONSTANT;
			}
			else if(TRUE == maxRect) {
				lowCoord[i] = 0;
				highCoord[i] = 1;
				rectSideStatus[i] = UNDIVIDED;
			}
			else if(externalPoint1->GetIntCoord(i)
					== externalPoint2->GetIntCoord(i)) {
				lowCoord[i] = highCoord[i] = externalPoint1->GetRealCoord(i);
				rectSideStatus[i] = CONSTANT;
			}
			else {
#if 0
				IntCoord_t lowIntCoord = externalPoint1->GetIntCoord(i);
				IntCoord_t highIntCoord = externalPoint2->GetIntCoord(i);
				assert(lowIntCoord != highIntCoord);

				if(highIntCoord < lowIntCoord) {
					IntCoord_t tmp = lowIntCoord;
					lowIntCoord = highIntCoord;
					highIntCoord = tmp;
				}

				lowIntCoord++; highIntCoord--;
				assert(lowIntCoord < highIntCoord);

				lowCoord[i] = Nipqo_t::GetPredFloat(lowIntCoord, i);
				highCoord[i] = Nipqo_t::GetPredFloat(highIntCoord, i);
#else
				lowCoord[i] = externalPoint1->GetRealCoord(i);
				highCoord[i] = externalPoint2->GetRealCoord(i);
				assert(lowCoord[i] != highCoord[i]);
				if(highCoord[i] < lowCoord[i]) {
					RealCoord_t tmp = lowCoord[i];
					lowCoord[i] = highCoord[i];
					highCoord[i] = tmp;
				}
#endif
				assert(0 <= lowCoord[i]); assert(lowCoord[i] <= 1);
				assert(0 <= highCoord[i]); assert(highCoord[i] <= 1);
#if 1
				IntCoord_t x1i = Nipqo_t::GetPredConst(lowCoord[i], i);
				IntCoord_t x2i = Nipqo_t::GetPredConst(highCoord[i], i);
				assert((x1i + 1) <= x2i);

				if((x1i + 1) == x2i) rectSideStatus[i] = DONT_DIVIDE;
				else rectSideStatus[i] = UNDIVIDED;
#else
				rectSideStatus[i] = UNDIVIDED;
#endif
			}
		}

		SetNumVaryingParam();

		vertices = new Point_t *[NumVertices()];
		assert(vertices);
		AssignVertices();

		// cout << "Created Rectangle: ";
		// PrintVertices();
		// cout << endl;
	}

	RealCoord_t GetLowCoord(int coordNum)
	{
		assert(0 <= coordNum); assert(coordNum < Nipqo_t::NumParam());
		return lowCoord[coordNum];
	}

	RealCoord_t GetHighCoord(int coordNum)
	{
		assert(0 <= coordNum); assert(coordNum < Nipqo_t::NumParam());
		return highCoord[coordNum];
	}

	// RealCoord_t *ConstPreds(void) { return pOpt->ConstPred(); };
	MyList_t<PlanCostPair_t>& MinPlanCostList(void) { return minPlanCostList; }

	Boolean_t IsPartitioned(void) { return (FALSE == partitionList.IsEmpty()); }

	Boolean_t IsPartitionable(void)
	{
		for(int i = 0; i < Nipqo_t::NumParam(); i++)
			if((UNDIVIDED == rectSideStatus[i])
					|| (DIVIDED == rectSideStatus[i])) return TRUE;

		return FALSE;
	}

	int GetPartitioningAxis(void) {
		int i;
		for(i = 0; i < Nipqo_t::NumParam(); i++)
			if(UNDIVIDED == rectSideStatus[i]) return i;	//Chetas Why DIVIDED is not used?

		for(i = 0; i < Nipqo_t::NumParam(); i++)
			assert((CONSTANT == rectSideStatus[i])
					|| (DONT_DIVIDE == rectSideStatus[i]));

		return -1;
	}

	PlanId_t Optimize(MyList_t<PlanId_t> *ancestorActivePlanList = NULL);
	PlanId_t Optimize1(MyList_t<PlanId_t> *ancestorActivePlanList = NULL);
	PlanId_t Optimize2(MyList_t<PlanId_t> *ancestorActivePlanList = NULL);
	PlanId_t OptimizeMethodOne(MyList_t<PlanId_t> *ancestorActivePlanList = NULL);
	Boolean_t OptimizeMethodTwo(MyList_t<PlanId_t> *ancestorActivePlanList = NULL);
	PlanId_t OptimizePartitionsMethodOne(
			MyList_t<PlanId_t> *ancestorActivePlanList);
	Boolean_t OptimizePartitionsMethodTwo(
			MyList_t<PlanId_t> *ancestorActivePlanList);

	int NumOptPlans(MyList_t<PlanId_t> &planList, Boolean_t toShow = TRUE);
	int NumVaryingParam() { return numVaryingParam; }

	Boolean_t IsExpandable(void)
	{
		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
			if(CONSTANT == rectSideStatus[i]) continue;

			assert(0 <= lowCoord[i]); assert(lowCoord[i] < 1);
			assert(0 < highCoord[i]); assert(highCoord[i] <= 1);

			if(0 < lowCoord[i]) return TRUE;
			if(highCoord[i] < 1) return TRUE;
		}

		return FALSE;
	}

	void InsertVerticesIntoList(List_t<Point_t *> &vertexList)
	{
		for(int i = 0; i < NumVertices(); i++) {
			assert(vertices[i]);
			// vertexList.Insert(vertices[i]);
			if(FALSE == vertexList.Contains(vertices[i]))
				vertexList.Insert(vertices[i]);
		}
	}

	void PrintSides(int newLine = FALSE) {
		for(int i = 0; i < Nipqo_t::NumParam(); i++)  {
			cout << "[";
			cout << lowCoord[i] << ", ";
			cout << highCoord[i] << "] ";
		}
		if(TRUE == newLine) cout << endl;
	}

	void PrintVertices(int newLine = FALSE) {
//#if 0		//Chetas- Changed because of hardcoded things.
#if 1
		PrintSides(TRUE);
#else
		PrintSides(FALSE);
		int lowCoordInt1 = Nipqo_t::GetPredConst(lowCoord[0], 0);
		int lowCoordInt2 = Nipqo_t::GetPredConst(lowCoord[1], 1);
		int highCoordInt1 = Nipqo_t::GetPredConst(highCoord[0], 0);
		int highCoordInt2 = Nipqo_t::GetPredConst(highCoord[1], 1);

		if((lowCoordInt1 <= 11) && (11 <= highCoordInt1)
				&& (lowCoordInt2 <= 564) && (564 <= highCoordInt2)) cout << "*";

		cout << endl;
#endif
		for(int i = 0; i < NumVertices(); i++) vertices[i]->PrintIntCoord();
		cout << endl;
		for(int i = 0; i < NumVertices(); i++) vertices[i]->PrintRealCoord();
		if(TRUE == newLine) cout << endl;
	}


	~Rectangle_t(void) {
		ListIter_t<Rectangle_t *> itr;
		itr.Attach(&partitionList);
		while(!itr.IsEnd())
			delete itr.Next();
		partitionList.MakeEmpty();
		delete lowCoord;
		delete highCoord;
		delete vertices;
		delete rectSideStatus;
	}
};

#endif // __RECTANGLE_H__
