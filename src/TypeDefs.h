#ifndef __PQOTYPEDEFS_H__
#define __PQOTYPEDEFS_H__

#include <assert.h>
#include "other/typedefs.h"

typedef float           RealCoord_t;
typedef int             IntCoord_t;
typedef int             PlanId_t;
typedef float           CostVal_t;
typedef float           RealParameterValue_t;

typedef int             IntegerParameterValue_t;
typedef int             Boolean_t;
// const int FALSE  = 0, TRUE  = 1;

typedef int             PredicateStatus_t;
const int VARIABLE = 0, LOW_CONST = 1,  HIGH_CONST = 2;

typedef int             RectSideStatus_t;
//const int CONST = 0, UNDIVIDED = 1, DIVIDED = 2, DONT_DIVIDE = 3;		//Chetas- Commented and added for changing just datatype.
const RectSideStatus_t CONSTANT = 0, UNDIVIDED = 1, DIVIDED = 2, DONT_DIVIDE = 3;

class PlanCostPair_t {
    PlanId_t planId;
    CostVal_t cost;

public:
    PlanCostPair_t(void)
    : planId(0), cost(-1) { }

    PlanCostPair_t(PlanId_t p, CostVal_t t)
    : planId(p), cost(t) {
	assert(0 < planId);
	assert(0 <= cost);
    }

    PlanId_t PlanId(void) { return planId; }
    CostVal_t Cost(void) { return cost; }

    void SetPlanId(PlanId_t pi)
    {
    	assert(0 < pi);
    	planId = pi;
    }
    void SetCost(CostVal_t cst)
    {
    	assert(0 <= cst);
    	cost = cst;
    }

    friend Boolean_t operator==(PlanCostPair_t& a, PlanCostPair_t&b)
    {
	if(a.planId != b.planId) return FALSE;
	return TRUE;
    }
};

#endif // __PQOTYPEDEFS_H__
