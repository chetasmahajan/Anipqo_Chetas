/*
 * Graph.cpp
 *
 *  Created on: 04-Sep-2014
 *      Author: Chetas
 */

#include "Graph.h"

Graph::Graph() {
	// TODO Auto-generated constructor stub

}

Graph::Graph(pOptimizer_t *pOpt)
{
	ListIter_t<Point_t *> partitioningVerticesItr;

	partitioningVerticesItr.Attach(pOpt->getPartitioningVertices());
	while(!partitioningVerticesItr.IsEnd()) {
		Point_t *vertex = partitioningVerticesItr.Next();
		vertices.Insert(vertex);

		AppendList_t<Point_t *> neighbors;
		pOpt->FindNeghbors(vertex, neighbors);

		ListIter_t<Point_t *> neighborItr;
		neighborItr.Attach(&neighbors);
		while(!neighborItr.IsEnd()) {
			Point_t *neighbor = neighborItr.Next();
			Edge_t *newEdge = new Edge_t(vertex, neighbor);

			bool found = false;
			ListIter_t<Edge_t *> edgeItr;
			edgeItr.Attach(&edges);
			while(!edgeItr.IsEnd()) {
				Edge_t *edge = edgeItr.Next();
				if(edge->IsEqual(newEdge)) {
					found = true;
					break;
				}
			}
			if(!found)
				edges.Insert(newEdge);
			else
				delete newEdge;
		}
	}
}

Graph::~Graph() {
	// TODO Auto-generated destructor stub
}

void Graph::deleteGraph(void)
{
	ListIter_t<Edge_t *> edgeItr;
	edgeItr.Attach(&edges);
	while(!edgeItr.IsEnd()) {
		Edge_t *edge = edgeItr.Next();
		delete edge;
	}
}

void Graph::findAllBoundaryVertices(void)
{
	ListIter_t<Point_t *> vertexItr;
	vertexItr.Attach(&vertices);
	while(!vertexItr.IsEnd()) {
		Point_t *vertex = vertexItr.Next();
		if(vertex->BoundaryNum() > 0)
			boundaryVertices.Insert(vertex);
	}
}

//This extracts all regions in graph using optimal algo of Jiang & Bunke.
void Graph::extractRegions(AppendDelList_t<Polygon_t *> *regions)
{
	int totalEdges = edges.Size(), cnt = 0, totalWedges = 2 * totalEdges;
	EdgeAngle *allEdgeAngles[totalWedges];
	Wedge *allWedges[totalWedges];
	ListIter_t<Edge_t *> edgeItr;
	edgeItr.Attach(&edges);
	while(!edgeItr.IsEnd()) {		//Create all EdgeAngles.
		Edge_t *edge = edgeItr.Next();
		allEdgeAngles[cnt++] = new EdgeAngle(edge->EndPoint1(), edge->EndPoint2());
		allEdgeAngles[cnt++] = new EdgeAngle(edge->EndPoint2(), edge->EndPoint1());
/*//		cout<<"Edge to remove is p1 = ";
//		edge->EndPoint1()->PrintIntCoord();
//		cout<<" & p2 = ";
//		edge->EndPoint2()->PrintIntCoord(TRUE);
//		cout<<endl;
		Graph *graph = removeEdge(edge);
//		graph->showGraph();
		Point_t *src, *dest, *temp;
		src = edge->EndPoint1();
		dest = edge->EndPoint2();

		graph->findShortestPath(src, dest);
		Polygon_t *region = new Polygon_t();

		temp = dest;
		while(temp != NULL) {
			region->insertVertex(temp);
			temp = temp->prev;
		}

		ListIter_t<Polygon_t *> regionItr;
		bool found = false;
		regionItr.Attach(regions);
		while(!regionItr.IsEnd()) {
			if(region->isEqual(regionItr.Next())) {
				found = true;
				break;
			}
		}
		if(!found)
			regions->Insert(region);
		delete graph;*/
	}
	for(int pass = 0; pass < totalWedges - 1; pass++) {			//Sort all edgeAngles with v1 as primary key & angle as secondary
		for (int i = 0; i < totalWedges - 1 - pass; i++) {		//key. v1's pointer is used as key as pointer is also int.
			EdgeAngle *temp = NULL;
			Point_t *p1 = allEdgeAngles[i]->edge->EndPoint1(), *p2 = allEdgeAngles[i + 1]->edge->EndPoint1();
			if(p1 > p2) {
				temp = allEdgeAngles[i + 1];
				allEdgeAngles[i + 1] = allEdgeAngles[i];
				allEdgeAngles[i] = temp;
			}
			else if(p1 == p2 && (allEdgeAngles[i]->angle > allEdgeAngles[i + 1]->angle)) {
				temp = allEdgeAngles[i + 1];
				allEdgeAngles[i + 1] = allEdgeAngles[i];
				allEdgeAngles[i] = temp;
			}
		}
	}

	//Create all Wedges from above EdgeAngles.
	cnt = 0;
	EdgeAngle *first, *prev, *cur;
	first = prev = allEdgeAngles[0];
	for(int i = 1; i < totalWedges; i++) {
		cur = allEdgeAngles[i];
		if(first->edge->EndPoint1() == cur->edge->EndPoint1()) {		//Check if in a same group or not.
			allWedges[cnt++] = new Wedge(cur->edge->EndPoint2(), cur->edge->EndPoint1(), prev->edge->EndPoint2());
			prev = cur;
			continue;
		}
		allWedges[cnt++] = new Wedge(first->edge->EndPoint2(), first->edge->EndPoint1(), prev->edge->EndPoint2());
		first = prev = cur;
	}
	allWedges[cnt++] = new Wedge(first->edge->EndPoint2(), first->edge->EndPoint1(), prev->edge->EndPoint2());
	assert(cnt == totalWedges);

	//Free all EdgeAngles.
	for(int i = 0; i < totalWedges; i++)
		delete allEdgeAngles[i];

	//Sort all Wedges.
	for(int pass = 0; pass < totalWedges - 1; pass++) {			//Sort all wedges with v1 as primary key & v2 as secondary
		for (int i = 0; i < totalWedges - 1 - pass; i++) {		//key. v1's & v2's pointer are used as keys as pointer is also int.
			Wedge *temp = NULL;
			Point_t *p1v1 = allWedges[i]->p1, *p1v2 = allWedges[i]->p2,
			*p2v1 = allWedges[i + 1]->p1, *p2v2 = allWedges[i + 1]->p2;
			if(p1v1 > p2v1) {
				temp = allWedges[i + 1];
				allWedges[i + 1] = allWedges[i];
				allWedges[i] = temp;
			}
			else if(p1v1 == p2v1 && p1v2 > p2v2) {
				temp = allWedges[i + 1];
				allWedges[i + 1] = allWedges[i];
				allWedges[i] = temp;
			}
		}
	}

//	cout<<"All wedges are:\n";
//	for(int i = 0; i < totalWedges; i++) {
//		cout<<i+1<<") ";
//		allWedges[i]->showWedge();
//		cout<<endl;
//	}
//	cout<<endl;

	findAllBoundaryVertices();		//Find which are the boundary vertices.

	Wedge *firstWedge, *curWedge, *prevWedge;
	firstWedge = curWedge = prevWedge = NULL;
	Polygon_t *region;
	bool isFullRegionDeleted = false;
	while(1) {
//		cout<<"Searching for unused wedge:\n";
		for(int i = 0; i < totalWedges; i++) {
//			cout<<i+1<<") ";
//			allWedges[i]->showWedge();
//			!allWedges[i]->isUsed ? cout<<": Not Used" : cout<<": Used";
//			cout<<endl;
			if(!allWedges[i]->isUsed) {
				firstWedge = allWedges[i];
				firstWedge->isUsed = true;
				region = new Polygon_t();
				region->insertVertex(firstWedge->p2);
				region->insertVertex(firstWedge->p3);
				break;
			}
		}
		if(firstWedge == NULL)
			break;
//		cout<<"Wedge selected: ";
//		firstWedge->showWedge();
//		cout<<endl;
		prevWedge = firstWedge;
		while(1) {		//Search for all wedges of current region.
			int start = 0, end = totalWedges - 1, mid = floor((start + end) / 2);
			while(start <= end) {	//Do binary search for next edge.
				if(allWedges[mid]->p1 == prevWedge->p2 && allWedges[mid]->p2 == prevWedge->p3) {
					curWedge = allWedges[mid];
					assert(curWedge->isUsed == false);
					break;
				}
				else if(allWedges[mid]->p1 < prevWedge->p2 ||
						(allWedges[mid]->p1 == prevWedge->p2 && allWedges[mid]->p2 < prevWedge->p3)) {
					start = mid + 1;
					mid = floor((start + end) / 2);
					continue;
				}
				else if(allWedges[mid]->p1 > prevWedge->p2 ||
						(allWedges[mid]->p1 == prevWedge->p2 && allWedges[mid]->p2 > prevWedge->p3)) {
					end = mid - 1;
					mid = floor((start + end) / 2);
					continue;
				}
				else {
					cout<<"\nNot a plane graph.\n";
					assert(0);
				}
			}
			assert(curWedge != NULL);
			curWedge->isUsed = true;
			prevWedge = curWedge;
//			cout<<"Found wedge : ";
//			curWedge->showWedge();
//			cout<<endl;
			if(curWedge->p2 == firstWedge->p1 && curWedge->p3 == firstWedge->p2) {		//Check for contiguous wedge.
//				cout<<"Contiguous wedge found.\n\n";
				curWedge = NULL;
				break;
			}
			else {
				region->insertVertex(curWedge->p3);
				curWedge = NULL;
			}
		}
		firstWedge = NULL;
//		cout<<"Region ";
//		region->showPolygon();
//		cout<<endl;
		if(!isFullRegionDeleted && region->vertices.Size() == boundaryVertices.Size()) {
//			cout<<"Full region deleted.\n";
			bool toDelete = true;
			ListIter_t<Point_t *> listItr;
			listItr.Attach(&boundaryVertices);
			while(!listItr.IsEnd()) {
				Point_t *vertex = listItr.Next();
				if(!region->vertices.Contains(vertex)) {
					toDelete = false;
					break;
				}
			}
			if(toDelete) {
				isFullRegionDeleted = true;
				delete region;
			}
			else
				regions->Insert(region);
		}
		else
			regions->Insert(region);
		region = NULL;
	}

	//Free all Wedges.
	for(int i = 0; i < totalWedges; i++)
		delete allWedges[i];
}

/*Graph* Graph::removeEdge(Edge_t *edge)
{
	Graph *graph = new Graph();
	ListIter_t<Point_t *> verticesItr;
	verticesItr.Attach(&vertices);
	while(!verticesItr.IsEnd())
		graph->vertices.Insert(verticesItr.Next());
	ListIter_t<Edge_t *> edgeItr;
	edgeItr.Attach(&edges);
	while(!edgeItr.IsEnd()) {
		Edge_t *nextEdge = edgeItr.Next();
		if(edge->IsEqual(nextEdge))
			continue;
		graph->edges.Insert(nextEdge);
	}
	return graph;
}*/

/*Point_t *Graph::getMin(AppendDelList_t<Point_t *> *list)
{
	ListIter_t<Point_t *> Itr;
	Point_t *minVertex, *vertex;

	Itr.Attach(list);
	while(!Itr.IsEnd()) {
		vertex = Itr.Next();
		if(minVertex == NULL || vertex->dist < minVertex->dist)
			minVertex = vertex;
	}
	return minVertex;
}*/

/*AppendList_t<Point_t *> *Graph::getNeighbors(AppendDelList_t<Point_t *> *list, Point_t *vertex)
{
	AppendList_t<Point_t *> *neighbors = new AppendList_t<Point_t *>();
	ListIter_t<Edge_t *> edgeItr;
	ListIter_t<Point_t *> verticeItr;

	edgeItr.Attach(&this->edges);
	verticeItr.Attach(list);
	while(!verticeItr.IsEnd()) {
		Point_t *newVertex = verticeItr.Next();
		edgeItr.Reset();
		while(!edgeItr.IsEnd()) {
			Edge_t *edge = edgeItr.Next();
			Point_t *p1 = edge->EndPoint1(), *p2 = edge->EndPoint2();
			if((p1 == vertex && p2 == newVertex) || (p2 == vertex && p1 == newVertex)) {
				neighbors->Insert(newVertex);
				break;
			}
		}
	}
	return neighbors;
}*/

/*void Graph::findShortestPath(Point_t *src, Point_t *dest)
{
	AppendDelList_t<Point_t *> queue;
	ListIter_t<Point_t *> verticesItr;
	bool keepLooping = true;

	src->dist = 0;
	src->prev = NULL;
	verticesItr.Attach(&this->vertices);
	while(!verticesItr.IsEnd()) {
		Point_t *vertex = verticesItr.Next();
		if(vertex != src) {
			vertex->dist = INT_MAX;
			vertex->prev = NULL;
		}
		queue.Insert(vertex);
	}

	while(!queue.IsEmpty()) {
		Point_t *minVertex = getMin(&queue);
		if(minVertex == dest)
			return;
		queue.Delete(minVertex);

		AppendList_t<Point_t *> *neighbors = getNeighbors(&queue, minVertex);
		ListIter_t<Point_t *> neighborsItr;
		neighborsItr.Attach(neighbors);
		while(!neighborsItr.IsEnd()) {
			Point_t *newNeighbor = neighborsItr.Next();
			if(minVertex->dist + 1 < newNeighbor->dist) {
				newNeighbor->dist = minVertex->dist + 1;
				newNeighbor->prev = minVertex;
			}
		}
		delete neighbors;
	}
}*/

void Graph::showGraph(void)
{
	ListIter_t<Point_t *> vertexIter;
	ListIter_t<Edge_t *> edgeIter;

	vertexIter.Attach(&vertices);
	edgeIter.Attach(&edges);
	cout<<"Vertices are :\n";
	while(!vertexIter.IsEnd()) {
		vertexIter.Next()->PrintIntCoord(TRUE);
	}
	cout<<"Edges are :\n";
	while(!edgeIter.IsEnd()) {
		Edge_t *edge = edgeIter.Next();
		cout<<"p1 = ";
		edge->EndPoint1()->PrintIntCoord();
		cout<<" & p2 = ";
		edge->EndPoint2()->PrintIntCoord(TRUE);
	}
}
