// $Id: typedefs.h,v 1.3 2003/05/30 16:25:03 arvind Exp arvind $
// system-wide typedefs

#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

// #define AlmostEqual(x,y)    ((x==y)?TRUE:(x<y)?(((y-x)/x)<0.000001):(((x-y)/y)<0.000001))
#define AlmostEqualZ(x,y,t)  ((x==y)?TRUE:(x<y)?(((y-x)/x)<t):(((x-y)/y)<t))
#define AlmostEqual(x,y)    AlmostEqualZ(x,y, 0.001)
//#define AlmostEqual(x,y)    AlmostEqualZ(x,y, 0.2)		//Chetas- Changed above line

#define ANIPQO_DEBUG

typedef float		CostVal_t;	// cost value as returned by cost model
typedef float		Card_t;		// cardinality type
typedef long		DataSize_t;	// data size type

typedef int             Boolean;        // data size type
const int FALSE  = 0, TRUE  = 1;

typedef int             QueryID_t;

// #define TOTALCOST

#if 0
// PQO stuff: used in OptimizerInterface_t

typedef float           RealCoord_t;
typedef int             IntCoord_t;
typedef int             PlanId_t;
// typedef float           CostVal_t;
typedef float           RealParameterValue_t;
typedef int             IntegerParameterValue_t;

typedef int             Boolean_t;
// const int FALSE  = 0, TRUE  = 1;

typedef int             PredicateStatus_t;
const int VARIABLE = 0, LOW_CONST = 1,  HIGH_CONST = 2;

typedef int             RectSideStatus_t;
const int CONST = 0, UNDIVIDED = 1, DIVIDED = 2, DONT_DIVIDE = 3;
#endif

#endif	// __TYPEDEFS_H__
