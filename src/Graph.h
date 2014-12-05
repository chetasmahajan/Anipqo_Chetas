/*
 * Graph.h
 *
 *  Created on: 04-Sep-2014
 *      Author: Chetas
 */

#ifndef GRAPH_H_
#define GRAPH_H_
#include<iostream>
#include<stdlib.h>
#include<assert.h>
#include<climits>
#include<math.h>
#include"Polygon.h"
#include"Point.h"
#include"MyList.h"
#include"other/List.h"
#include"other/typedefs.h"

class EdgeAngle {
public:
	Edge_t *edge;
	double angle;

	EdgeAngle(Point_t *p1, Point_t *p2) {
		int a1, a2, b1, b2, crossProduct;
		b1 = p2->GetIntCoord(0) - p1->GetIntCoord(0);		//Make p1 to be origin. So, b1, b2 should set accordingly.
		b2 = p2->GetIntCoord(1) - p1->GetIntCoord(1);
		a1 = 1;
		a2 = 0;
		crossProduct = b2 * a1 - b1 * a2;

		double norm1, norm2, dotProduct, cosOfAngle;
		dotProduct = a1 * b1 + a2 * b2;
		norm1 = sqrt((double)(a1 * a1 + a2 * a2));
		norm2 = sqrt((double)(b1 * b1 + b2 * b2));
		cosOfAngle = dotProduct / (norm1 * norm2);

		angle = acos(cosOfAngle);
		if(crossProduct < 0)
			angle = 2 * 3.14159265 - angle;

		edge = new Edge_t(p1, p2);
	}

	~EdgeAngle(void) {
		delete edge;
	}

};

class Wedge {
public:
	Point_t *p1, *p2, *p3;
	bool isUsed;

	Wedge(Point_t *p1, Point_t *p2, Point_t *p3) {
		this->p1 = p1;
		this->p2 = p2;
		this->p3 = p3;
		isUsed = false;
	}

	~Wedge(void) {
	}

	void showWedge(void) {
		p1->PrintIntCoord();
		p2->PrintIntCoord();
		p3->PrintIntCoord();
	}
};

class Graph {
public:
	AppendDelList_t<Edge_t *> edges;
	AppendList_t<Point_t *> vertices;
	AppendList_t<Point_t *> boundaryVertices;

	Graph();
	virtual ~Graph();

	Graph(pOptimizer_t *pOpt);
	void extractRegions(AppendDelList_t<Polygon_t *> *regions);
	void findAllBoundaryVertices(void);
//	Graph* removeEdge(Edge_t *edge);
//	void findShortestPath(Point_t *src, Point_t *dest);
//	static Point_t *getMin(AppendDelList_t<Point_t *> *list);
//	AppendList_t<Point_t *> *getNeighbors(AppendDelList_t<Point_t *> *list, Point_t *vertex);
	void showGraph(void);
	void deleteGraph(void);

};

#endif /* GRAPH_H_ */
