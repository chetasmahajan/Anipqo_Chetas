#ifndef __NIPQO_H__
#define __NIPQO_H__

#include <cmath>

#include "MyList.h"
#include "other/List.h"
//#include "other/OptimizerInterface.h"
#include "Plan.h"	//Chetas- Added instead of other/OptimizerInterface.h

class HashSet_t;
class pOptimizer_t;
class Rectangle_t;
class Point_t;
class Edge_t;
class Polygon_t;
class Graph;

class Utility_t {
	static int *twoPowerN; // where and how to deallocate?
public:
	static void Initialize(void);
	static int TwoPowerN(int n);
};

class Nipqo_t {
	static int *twoPowerN; 	// where and how to deallocate?
	static int methodNum;	//Chetas- Don't know use of methodNum.
	static int optPhase;	//Chetas- meaning?
	static Boolean_t optimizeOnlySquares;	//Chetas- What is the purpose?
	static Boolean_t optimizeIntermediateDimensions;	//Chetas- What is the purpose?

	static int numParam;
	//static IntCoord_t *relSize;		//Chetas- Commented for selectivity consts.

	static RealCoord_t minSquareSize; // =  0.1;	//Chetas- meaning?
	// 0.03125; // = 1.0/32
	// 0.125;   // = 1.0/8
	static Boolean_t exponentOn; // = TRUE;
	static RealCoord_t expWt; // = 5;

	static int numPlans;

	static int numOptimizers;		//Chetas- What is the purpose?
	static pOptimizer_t **pOpts;	//Chetas- What is the purpose?

	static HashSet_t vertexCache;

	static Boolean useCostLimit;
	static Boolean useConstCostLimit;
	static CostVal_t constCostLimit;

	static Boolean printTrace;

public:
	static Point_t ***allVertices;						//Chetas- Added for computing errors.
	static int resolution;								//Chetas- Added resolution
	static void PQO(int argc, char *argv[]);			//Chetas- Added for calling this function.
	static void computeError(pOptimizer_t *pOpt, FILE *mtfp, FILE *inferfp, FILE *errfp);		//Chetas- Added for Correct version.
	static void writeRegions(pOptimizer_t *pOpt, FILE *fp);		//Chetas- Added for Correct version.
	static void optimizeAll(void);						//Chetas- Added for Correct version.
	static double timeForAnipqo;						//Chetas- Added to measure time.
	static double timeToOptimizeAll;					//Chetas- Added to measure time.
	static double timeToInferAll;					//Chetas- Added to measure time.
	static streambuf *redirectCoutTo(string file);		//Chetas- Added for storing output in a file.
	static void resetCout(streambuf *coutbuf);			//Chetas- Added for storing output in a file.
	static bool isFirstOptimization;					//Chetas- Added for Correct version.
	static void inferBestPlan(pOptimizer_t *pOpt, Point_t *vertex, int *fpcErr, PlanId_t optPlan);	//Chetas- Added for Correct version.

	static int MethodNum(void) { return methodNum; }

	static int OptPhase(void) { return optPhase; }
//	static int SetPhaseOne(void) { optPhase = 1;; }		//Chetas- changed returntype.
//	static int SetPhaseTwo(void) { optPhase = 2;; }		//Chetas- changed returntype.
	static void SetPhaseOne(void) { optPhase = 1; }
	static void SetPhaseTwo(void) { optPhase = 2; }

	static Boolean_t OptimizeOnlySquares(void) { return optimizeOnlySquares; }
	static Boolean_t OptimizeIntermediateDimensions(void) { return optimizeIntermediateDimensions; }

	static Boolean_t Initialize(int argc, char *argv[]);

	static int NumParam(void) { return numParam; }
	static void SetNumParam(int np) { assert(0 < np); numParam = np; }

	static RealCoord_t MinSquareSize(void) { return minSquareSize; }
	static void SetMinSquareSize(RealCoord_t msz) { assert(0 < msz); minSquareSize = msz; }		//Chetas- This function is not used. But this might be setting resolution.

	static RealCoord_t ExpWt(void) { return expWt; }
	static void SetExpWt(RealCoord_t ew) { assert(0 < ew); expWt = ew; }

	static Boolean_t IsExpWtOn(void) { return exponentOn; }
	static void SetExpWtFlag(void) { exponentOn = TRUE; }
	static void ResetExpWtFlag(void) { exponentOn = FALSE; }

	// static void ResetUseCostLimit(void) { useCostLimit = FALSE; }
	// static void SetUseCostLimit(void) { useCostLimit = TRUE; }
	static Boolean UseCostLimit(void) { return useCostLimit; }

	// static void ResetUseConstCostLimit(void) { useConstCostLimit = FALSE; }
	// static void SetUseConstCostLimit(void) { useConstCostLimit = TRUE; }
	static Boolean UseConstCostLimit(void) { return useConstCostLimit; }

	// static void ResetPrintTrace(void) { printTrace = FALSE; }
	// static void SetPrintTrace(void) { printTrace = TRUE; }
	static Boolean PrintTrace(void) { return printTrace; }

	static void SetConstCostLimit(CostVal_t costLimit_a)
	{ assert(0 <= costLimit_a); constCostLimit = FALSE; }

	static CostVal_t ConstCostLimit(void) { return constCostLimit; }

	static int NumPlans(void)
	{
//		return OptimizerInterface_t::NumPlans();
		return Plan::numPlans();	//Chetas- Replaced with above.
	}

	// static void IncrNumPlans(void) { numPlans++; }

//	static IntCoord_t RelSize(int idx) { assert(idx < numParam); return relSize[idx]; }		//Not needed.

	static IntCoord_t GetPredConst(RealCoord_t sel, int predNo)
	{
		assert(0 <= sel); assert(sel <= 1);
		assert(0 <= predNo); assert(predNo < numParam);

		// int predConst = relSize[predNo] * Exponent(sel[predNo]);
		// int predConst = floor(relSize[predNo] * Exponent(sel) + 0.5) + 1;
		// int predConst = floor(relSize[predNo] * Exponent(sel) + 0.5) + 2;
		// int predConst = (int)(floor(relSize[predNo] * sel + 0.5) + 2);
//		int predConst = (int)(floor((relSize[predNo]-2) * sel + 2.5));		//Chetas- Commented for selectivity consts.
		// cout << endl << "GetPredConst[" << predNo << "]: "
		// 	<< sel << " => " << predConst << endl;

		if(sel == 0)
			return 0;
		int predIdx = ceil(resolution * sel) - 1;
		//Chetas- Commented for selectivity consts.**********************************
//		if(relSize[predNo] < predConst) {
//			// assert(predConst == (relSize[predNo] + 1));
//			// predConst--;
//			if(predConst == (relSize[predNo] + 1)) predConst--;
//			else if(predConst == (relSize[predNo] + 2)) predConst -= 2;
//			else assert(0);
//		}
//
//		assert(1 <= predConst);
//		assert(predConst <= relSize[predNo]);
//
//		return predConst;
		//********************************************************************
		return predIdx;
	}

	static RealCoord_t GetPredFloat(IntCoord_t intSel, int predNo)		//Chetas- Function is unreachable.
	{

		assert(0 <= intSel && intSel < resolution);
//		assert(intSel <= relSize[predNo]);		//Chetas- Not needed.
		assert(0 <= predNo); assert(predNo < numParam);

		RealCoord_t low = 0, high = 1;

		IntCoord_t lowInt = GetPredConst(low, predNo);
		if(intSel == lowInt) return low;

		IntCoord_t highInt = GetPredConst(high, predNo);
		if(intSel == highInt) return high;

		while(TRUE) {
			RealCoord_t centre = (low + high)/2;
			IntCoord_t centreInt = GetPredConst(centre, predNo);

			if(intSel == centreInt) {
				assert(0 < centre);
				assert(centre < 1);
				return centre;
			}

			if(intSel < centreInt) {
				high = centre;
				highInt = centreInt;
				continue;
			}

			if(centreInt < intSel) {
				low = centre;
				lowInt = centreInt;
				continue;
			}

			assert(FALSE);
		}
		cout<<"\nFloat predicate not found.\n";
		assert(0);
		return FALSE;		//Chetas- Added for returning false.

/*		assert(intSel >= 0 && intSel < resolution);
		assert(predNo < numParam);
		if(intSel == 0)
			return 0;
		double tmpd = ((double)(intSel + 1)) / (double)resolution;
		char num[10];
		sprintf(num, "%0.4lf", tmpd);
//		float rounded = ((int)(tmpd * 10000 + 0.5) / 10000.0);
//		return rounded;
		float tmpf = 0;
		tmpf = atof(num);
		return tmpf;*/
	}

	static int GetOptNum(RealCoord_t *constPred = NULL) {
		if(NULL == constPred) {
			return 0;
		}

		int optNum = 0;
		for(int i = 0; i < Nipqo_t::NumParam(); i++) {
			assert((-1 == constPred[i]) || (0 == constPred[i]) || (1 == constPred[i]));
			optNum = optNum * 3 + ((int)constPred[i] + 1);
		}

		assert(optNum < numOptimizers);
		return optNum;
	}

	static HashSet_t& VertexCache(void) { return vertexCache; }

	static pOptimizer_t *Optimize(RealCoord_t *constPred = NULL);
};

class pOptimizer_t {
	int id;
	MyList_t<PlanId_t> localPlans;
	RealCoord_t *constPred;
	Rectangle_t *rect;

	AppendDelList_t<Point_t *> partitioningVertices;

	AppendDelList_t<Point_t *> falsePositivePartitioningVertices;		//Chetas- Added for Correct Version.

	void OptimizeForMethodOne(void);
	void OptimizeForMethodTwo(void);
	void FindConflictingEdges(Point_t *vertex, PlanId_t plan,
			List_t<Edge_t *> &conflictingEdges);
//	void FindNeghbors(Point_t *vertex, List_t<Point_t *> &neirgbors);

	Point_t *GetInterpolatedPoint(Point_t *point1, Point_t *point2, float ratio);

	void Optimize(Edge_t *edge, PlanId_t plan);
	void Optimize(Point_t *point, MyList_t<PlanId_t> &planList);

	void MarkVisitedAllPartitioningVertices(void);
	void UnmarkVisitedAllPartitioningVertices(void);
	void UpdateMinPlanListsOfAllPartitioningVertices(void);

	int NumVaryingParam() { return numVaryingParam; }
	void DeleteToBeDeletedVertices(void);
	void PrintPartitioningVertices(void);

	void AssignVertices(void);

public:
	//Chetas- Added for Aggressive version.***********
	Graph *graph;								//Chetas- Added for Correct version.
	AppendDelList_t<Polygon_t *> *allRegions;	//Chetas- Added for Correct version.
	int CSOPsize;								//Chetas- Added for Correct version.
	int numVaryingParam;
	int falsePositivePointsDetected;									//Chetas- Added for Correct Version.
	AppendDelList_t<Point_t *> *getPartitioningVertices(void) { return &partitioningVertices; }
	void FindNeghbors(Point_t *vertex, List_t<Point_t *> &neirgbors);
	void FindNeghborsOnBoundary(Point_t *vertex, List_t<Point_t *> &neirgbors);	//Chetas- Added for Correct Version.
	Point_t *GetFromPartitioningList(Point_t *vertex);								//Chetas- Added for Correct Version.
	MyList_t<PlanId_t> POSP;														//Chetas- Added for Correct Version.
	//***************************

	pOptimizer_t(RealCoord_t *constPredIn, int id);

	int Id(void) { return id; }

	int NumLocalPlans(void) { return localPlans.Size(); }

	int LocalPlanNum(int globalPlanNum) {
		for(int i = 0; i < localPlans.Size(); i++) {
			if(globalPlanNum == localPlans.Entry(i)) return i;
			// if(globalPlanNum < localPlans.Entry(i)) return -1;
		}

		return -1;
	}

	MyList_t<PlanId_t>& LocalPlans(void) { return localPlans; }
	RealCoord_t *ConstPred(void) { return constPred; }

	RealCoord_t GetConstPred(int predNum)
	{ assert(predNum < Nipqo_t::NumParam()); return constPred[predNum]; }

	void SetConstPred(int predNum, RealCoord_t value)
	{ assert(predNum < Nipqo_t::NumParam()); constPred[predNum] = value; }

	void SetConstPredArray(RealCoord_t *valueArray)
	{
		for(int i = 0; i < Nipqo_t::NumParam(); i++) constPred[i] = valueArray[i];
	}

	void AddPartitioningVertex(Point_t *vertex)
	{ partitioningVertices.Insert(vertex); }

	Boolean_t PartitioningVertexListContains(Point_t *vertex)
	{ return partitioningVertices.Contains(vertex); }

	//Chetas- Added for Correct Version. *************************
	void AddFalsePositivePartitioningVertex(Point_t *vertex)
	{ falsePositivePartitioningVertices.Insert(vertex); }

	Boolean_t FalsePositivePartitioningVertexListContains(Point_t *vertex)
	{ return falsePositivePartitioningVertices.Contains(vertex); }

	void DeleteFalsePositivePartitioningVertex(Point_t *vertex) {
		assert(FalsePositivePartitioningVertexListContains(vertex) == TRUE);
		falsePositivePartitioningVertices.Delete(vertex);
	}
	//*************************************************************

	void MergePartitioningVertices(pOptimizer_t *facetOpt);

	int NumRectVertices(void) { return Utility_t::TwoPowerN(numVaryingParam); }

	void Optimize(void);
};

#endif // __NIPQO_H__
