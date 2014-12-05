/*
 * Polygon.h
 *
 *  Created on: 04-Sep-2014
 *      Author: Chetas
 */

#ifndef POLYGON_H_
#define POLYGON_H_
#include<iostream>
#include<stdlib.h>
#include<assert.h>
#include<climits>
#include"Point.h"
#include"Nipqo.h"
#include"MyList.h"
#include"other/List.h"
#include"other/typedefs.h"

class Polygon_t {
public:
	AppendList_t<Point_t *> vertices;
	MyList_t<PlanId_t> optPlans;
	MyList_t<PlanId_t> allPlans;

	Polygon_t();
	virtual ~Polygon_t();

	void insertVertex(Point_t *vertex);
	bool isEqual(Polygon_t *newPoly);
	void showPolygon(void);
	bool contains(Point_t *vertex);
	void initializeOptPlanList(void);
	void initializeAllPlanList(int totalPlans);
	static void inferPlan(AppendDelList_t<Polygon_t *> *allRegions, Point_t *vertex, int *fpcErr = NULL);
	Point_t *isRegionVertex(Point_t *vertex);
	void getNeighbors(Point_t *vertex, Point_t **n1, Point_t **n2);
	static Polygon_t *merge(Polygon_t *reg1, Polygon_t *reg2);
};

#endif /* POLYGON_H_ */
