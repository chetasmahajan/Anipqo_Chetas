/*
 * Polygon.cpp
 *
 *  Created on: 04-Sep-2014
 *      Author: Chetas
 */

#include "Polygon.h"

Polygon_t::Polygon_t() {
	// TODO Auto-generated constructor stub

}

Polygon_t::~Polygon_t() {
	// TODO Auto-generated destructor stub
}

void Polygon_t::insertVertex(Point_t *vertex)
{
	this->vertices.Insert(vertex);
}

bool Polygon_t::isEqual(Polygon_t *newPoly)
{
	if(this->vertices.Size() != newPoly->vertices.Size())
		return false;
	ListIter_t<Point_t *> thisItr, newItr;
	thisItr.Attach(&this->vertices);
	newItr.Attach(&newPoly->vertices);
	while(!thisItr.IsEnd()) {
		Point_t *thisVert, *newVert;
		bool found = false;

		thisVert = thisItr.Next();
		newItr.Reset();
		while(!newItr.IsEnd()) {
			newVert = newItr.Next();
			if(newVert == thisVert)
				found = true;
		}
		if(!found)
			return false;
	}
	return true;
}

void Polygon_t::showPolygon(void)
{
	ListIter_t<Point_t *> vertexIter;

	vertexIter.Attach(&vertices);
	cout<<"Vertices are :\n";
	while(!vertexIter.IsEnd()) {
		vertexIter.Next()->PrintIntCoord();
	}
}

//Implemented algorithm "ray-casting to the right". http://stackoverflow.com/questions/11716268/point-in-polygon-algorithm
bool Polygon_t::contains(Point_t *vertex)
{
	bool ret = false;
	int size = vertices.Size();
	Point_t *first, *prev, *cur;
	ListIter_t<Point_t *> verticesIter;
	int prevx, prevy, curx, cury, verx, very;

	assert(vertex->numParams() == 2);
	assert(size > 0);

//	cout<<"In Polygon::contains.\n";
//	showPolygon();

	verticesIter.Attach(&vertices);
	first = cur = verticesIter.Next();
	if(cur->IsEqual(*vertex))
		return true;
	while(!verticesIter.IsEnd()) {
		prev = cur;
		cur = verticesIter.Next();
		if(cur->IsEqual(*vertex))
			return true;

//		cout<<"Current edge is p1 = ";
//		prev->PrintIntCoord();
//		cout<<" & p2 = ";
//		cur->PrintIntCoord();

		prevx = prev->GetIntCoord(0);
		prevy = prev->GetIntCoord(1);
		curx = cur->GetIntCoord(0);
		cury = cur->GetIntCoord(1);
		verx = vertex->GetIntCoord(0);
		very = vertex->GetIntCoord(1);

		if(prevx == curx && prevx == verx)
			if((prevy > very) != (cury > very))
				return true;
		if(prevy == cury && prevy == very)
			if((prevx > verx) != (curx > verx))
				return true;

		if((prevy > very) != (cury > very)) {
			float dist;
			dist = (float)(curx - prevx) * (very - prevy);
			dist = dist / (cury - prevy);
			dist = dist + prevx;
			if(verx <= dist)
				ret = !ret;
		}
	}
	prev = cur;
	cur = first;

//	cout<<"Current edge is p1 = ";
//	prev->PrintIntCoord();
//	cout<<" & p2 = ";
//	cur->PrintIntCoord();

	prevx = prev->GetIntCoord(0);
	prevy = prev->GetIntCoord(1);
	curx = cur->GetIntCoord(0);;
	cury = cur->GetIntCoord(1);
	verx = vertex->GetIntCoord(0);
	very = vertex->GetIntCoord(1);

	if(prevx == curx && prevx == verx)
		if((prevy > very) != (cury > very))
			return true;
	if(prevy == cury && prevy == very)
		if((prevx > verx) != (curx > verx))
			return true;

	if((prevy > very) != (cury > very)) {
		float dist;
		dist = (float)(curx - prevx) * (very - prevy);
		dist = dist / (cury - prevy);
		dist = dist + prevx;
		if(verx <= dist)
			ret = !ret;
	}
	return ret;
}

void Polygon_t::initializeOptPlanList(void)
{
	ListIter_t<Point_t *> vertIter;
	int *list = NULL, size;

	vertIter.Attach(&vertices);
	Point_t *vert = vertIter.Next();
	size = vert->GetMinCostPlans(&list);
	assert(list);

	while(!vertIter.IsEnd()) {
		vert = vertIter.Next();
		for(int i = 0; i < size; i++) {
			PlanId_t plan = list[i];
			if(plan > 0) {
				if(vert->MinCostPlanListContains(plan))
					continue;
				else
					list[i] = 0;
			}
		}
	}

	for(int i = 0; i < size; i++) {
		PlanId_t plan = list[i];
		if(plan > 0)
			optPlans.Append(plan);
	}
	delete list;
}

void Polygon_t::initializeAllPlanList(int totalPlans)
{
	for(int i = 1; i <= totalPlans; i++) {
		ListIter_t<Point_t *> itr;
		itr.Attach(&vertices);
		while(!itr.IsEnd()) {
			Point_t *vert = itr.Next();
			if(vert->MinCostPlanListContains(i)) {
				allPlans.Append(i);
				break;
			}
		}
	}
	assert(allPlans.Size() > 0);
}

void Polygon_t::inferPlan(AppendDelList_t<Polygon_t *> *allRegions, Point_t *vertex, int *fpcErr)
{
	vertex->setInferedOptPlan(0);
	Polygon_t *regions[allRegions->Size()];
	int cnt = 0, regionsFound = 0;
	ListIter_t<Polygon_t *> itr;
	itr.Attach(allRegions);
	while(!itr.IsEnd()) {
		Polygon_t *region = itr.Next();
		if(region->contains(vertex)) {
			regions[cnt++] = region;
			regionsFound++;
		}
		else
			regions[cnt++] = NULL;
	}
	assert(regionsFound > 0);
	Polygon_t *region = NULL;
	CostVal_t minCost = INT_MAX;
	PlanId_t optPlan = 0;
	if(regionsFound == 1) {		//Point lies on only 1 region.
		for(int i = 0; i < allRegions->Size(); i++)
			if(regions[i] != NULL) {
				region = regions[i];
				break;
			}
		assert(region != NULL);
		if(region->optPlans.Size() > 0) {
			for(int j = 0; j < region->optPlans.Size(); j++) {
				PlanId_t plan = region->optPlans.Entry(j);
				CostVal_t cost = vertex->GetPlanCost(plan, 0, fpcErr);
				if(cost < minCost) {
					optPlan = plan;
					minCost = cost;
				}
			}
		}
		else {
			assert(region->allPlans.Size() > 0);
			for(int j = 0; j < region->allPlans.Size(); j++) {
				PlanId_t plan = region->allPlans.Entry(j);
				CostVal_t cost = vertex->GetPlanCost(plan, 0, fpcErr);
				if(cost < minCost) {
					optPlan = plan;
					minCost = cost;
				}
			}
		}
	}
	else {		//Point lies on multiple region
		AppendDelList_t<PlanId_t> plans;
		bool isOptPlanPointRegion = true;	//Plan optimal at this point has point region or not.
		int cntr = 0;
		Point_t *regVert = NULL;
		for(int i = 0; i < allRegions->Size(); i++) {
			if(regions[i] == NULL)
				continue;
			region = regions[i];
			regVert = region->isRegionVertex(vertex);
			if(isOptPlanPointRegion && regVert != NULL) {
				PlanId_t minCostplan = regVert->GetMinCostPlan();
				Point_t *n1 = NULL, *n2 = NULL;
				region->getNeighbors(regVert, &n1, &n2);
				assert(n1 != NULL && n2 != NULL);
				if(n1->MinCostPlanListContains(minCostplan) || n2->MinCostPlanListContains(minCostplan))
					isOptPlanPointRegion = false;
				else if(regionsFound == ++cntr)
					plans.Insert(minCostplan);
			}
			for(int j = 0; j < region->optPlans.Size(); j++)
				plans.Insert(region->optPlans.Entry(j));
		}

		if(plans.Size() == 0) {
			for(int i = 0; i < allRegions->Size(); i++) {
				if(regions[i] == NULL)
					continue;
				region = regions[i];
				regVert = region->isRegionVertex(vertex);
				if(regVert != NULL) {
					vertex->setInferedOptPlan(regVert->GetMinCostPlan());
					vertex->setInferedOptCost(regVert->GetMinCost());
					return;
				}
				for(int j = 0; j < region->allPlans.Size(); j++)
					plans.Insert(region->allPlans.Entry(j));
			}
		}
		ListIter_t<PlanId_t> itr;
		itr.Attach(&plans);
		while(!itr.IsEnd()) {
			PlanId_t plan = itr.Next();
			CostVal_t cost;
			cost = regVert != NULL ? regVert->GetPlanCost(plan, 0, fpcErr) : vertex->GetPlanCost(plan, 0, fpcErr);
			if(cost < minCost) {
				optPlan = plan;
				minCost = cost;
			}
		}
	}
	vertex->setInferedOptPlan(optPlan);
	vertex->setInferedOptCost(minCost);
}

Point_t *Polygon_t::isRegionVertex(Point_t *vertex)
{
	ListIter_t<Point_t *> itr;
	itr.Attach(&vertices);
	while(!itr.IsEnd()) {
		Point_t *regVert = itr.Next();
		if(regVert->IsEqual(*vertex))
			return regVert;
	}
	return NULL;
}

void Polygon_t::getNeighbors(Point_t *vertex, Point_t **prevNeighbor, Point_t **nextNeighbor)
{
	ListIter_t<Point_t *> itr;
	Point_t *first, *cur, *prev;
	bool found = false;

	*prevNeighbor = *nextNeighbor = NULL;
	itr.Attach(&vertices);
	first = cur = itr.Next();
	if(cur == vertex) {
		*nextNeighbor = itr.Next();
		while(!itr.IsEnd())
			cur = itr.Next();
		*prevNeighbor = cur;
		return;
	}
	while(!itr.IsEnd()) {
		prev = cur;
		cur = itr.Next();
		if(cur == vertex) {
			found = true;
			*prevNeighbor = prev;
		}
		else if(found) {
			*nextNeighbor = cur;
			return;
		}
	}
	assert(found && *prevNeighbor != NULL);
	*nextNeighbor = first;
}

/*Polygon_t *Polygon_t::merge(Polygon_t *reg1, Polygon_t *reg2) {
	Polygon_t *newReg = new Polygon_t();
	ListIter_t<Point_t *> itr1, itr2;
	itr1.Attach(&reg1->vertices);
	itr2.Attach(&reg2->vertices);
	Point_t *p1, *p2, *pt;
	p1 = p2 = pt = NULL;
	while(!itr1.IsEnd()) {
		Point_t *vert = itr1.Next();
		vert->isincluded = false;
		if(reg2->vertices.Contains(vert)) {
			if(p1 == NULL)
				p1 = vert;
			else
				p2 = vert;
		}
	}
	assert(p1 != NULL && p2 != NULL);
	while(!itr2.IsEnd())
		itr2.Next()->isincluded = false;
	bool keepLooping = true;
	bool isFirstFound = false;
	bool toJump = false;
	bool toImmJump = false;
	while(keepLooping) {
		keepLooping = false;
		itr1.Reset();
		itr2.Reset();
		while(!itr1.IsEnd()) {
			Point_t *vert = itr1.Next();
			if(vert == p1 || vert == p2) {
				if(toImmJump) {
					assert(pt == NULL);
					pt = p1->isincluded ? p1 : p2;
				}
				else if(!isFirstFound) {
					toJump = toImmJump = isFirstFound = true;
					vert->isincluded = true;
				}
				else {
					toImmJump = false;
					vert->isincluded = true;
				}
			}
			else
				vert->isincluded = true;
		}
		while(!itr2.IsEnd()) {
			Point_t *vert = itr2.Next();

		}
	}
}*/
