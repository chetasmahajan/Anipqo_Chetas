/*
 * Plan.h
 *
 *  Created on: 07-Aug-2014
 *      Author: Chetas
 */

#ifndef PLAN_H_
#define PLAN_H_

#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <windows.h>
#include <sqlext.h>
#include <fstream>
#include <streambuf>
#include <assert.h>

#include "Node.h"

using namespace std;
static string emptyStr("");
extern string fixQuery;
static string consts("");

class Plan;

class PlanInfo {
public:
	int planCount;
	vector<Plan*> *allPlans;
	int optCalls;
	int evalCalls;

private:
	static PlanInfo *obj;

protected:
	PlanInfo(void)
	{
		planCount = 0;
		evalCalls = 0;
		optCalls = 0;
		allPlans = new vector<Plan*>();
	}
	~PlanInfo()
	{
		for(std::vector<Plan*>::iterator it = allPlans->begin(); it != allPlans->end(); it++) {
			Plan *plan = *it;
			delete plan;
		}
		delete allPlans;
	}

public:
	static PlanInfo* getObj(void)
	{
		if(obj == NULL)
			obj = new PlanInfo();
		return obj;
	}
	static void deleteObj(void) {
		if(obj != NULL) {
			delete obj;
			obj = NULL;
		}
	}
};

class DBConn {
public:
	static SQLHENV henv;
	static SQLHDBC connNorm, connPlanAll, connPlanXML;
	static SQLHSTMT stmtNorm, stmtPlanAll, stmtPlanXML;
	static SQLRETURN ret;
//	DBConn();
//	virtual ~DBConn();
	static void createObjs(void);
	static void destroyObjs(void);
	static Plan* getPlan(string query, string foreignPlan = emptyStr);
	static string getXmlPlan(string query);
};

class Plan {
	vector<Node*> nodes;
	int planNo;
	double cost;
	string xml;
public:
	Plan();
	virtual ~Plan();
	static float **selectivityConsts;		//Chetas- Added for selectivity consts.

//	static void initializePlans()
//	{
//		planCount = 0;
//		allPlans = new vector<Plan*>();
//		optCalls = 0;
//		evalCalls = 0;
//	}
	int getPlanNo()
	{
		return planNo;
	}
	void setPlanNo(int foreignPlan = 0, int *fpcErr = NULL)
	{
		//Commenting because MAXDOP 1 option used. Lets see what happens.
		if(foreignPlan != 0) {
			if(fpcErr != NULL) {
				*fpcErr = 0;
				/*vector<Plan*> *allPlans = PlanInfo::getObj()->allPlans;
				for(vector<Plan*>::iterator it = allPlans->begin(); it != allPlans->end(); it++) {
					Plan *plan = *it;
					if(this->equals(plan, fpcErr)) {
						assert(*fpcErr == 0);
						break;
					}
				}*/
				Plan *plan = getPlanById(foreignPlan);
				assert(plan);
				if(this->equals(plan, fpcErr))
					assert(*fpcErr == 0);
			}
			this->planNo = foreignPlan;
			return;
		}
		//Uncommented this. sort & compute_scalar order changed.
		vector<Plan*> *allPlans = PlanInfo::getObj()->allPlans;
		for(vector<Plan*>::iterator it = allPlans->begin(); it != allPlans->end(); it++) {
			Plan *plan = *it;
			if(this->equals(plan)) {
				this->planNo = plan->getPlanNo();
				return;
			}
		}
		this->planNo = ++PlanInfo::getObj()->planCount;
		allPlans->push_back(this);
	}
	void copyPlanNo(Plan plan)
	{
		this->planNo = plan.getPlanNo();
	}
	double getCost()
	{
		return this->cost;
	}
	void setCost(double cost)
	{
		this->cost = cost;
	}
	void addNode(Node *node)
	{
		this->nodes.push_back(node);	//Modify this code.
	}
	string getXml()
	{
		return this->xml;
	}
	void setXml(string xml)
	{
		this->xml = xml;
	}
	void showPlan()
	{
		Node *node;
		cout<<"Plan No = "<<planNo<<endl;
		for(std::vector<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
			node = *it;
			node->showNode();
		}
	}
	static int numPlans(void)
	{
		return PlanInfo::getObj()->allPlans->size();
	}
	static string getXmlPlan(string query)
	{
		return DBConn::getXmlPlan(query);
	}
	static void readQuery(char *file)
	{
		std::ifstream t(file);
		std::string query((std::istreambuf_iterator<char>(t)),
		                 std::istreambuf_iterator<char>());
		fixQuery = query;
	}
	static void initializeSelectivityConsts(int numParam, int resolution, char *file)
	{
		selectivityConsts = new float*[numParam];
		for(int i = 0; i < numParam; i++)
			selectivityConsts[i] = new float[resolution];

		std::ifstream t(file);
		std::string temp((std::istreambuf_iterator<char>(t)),
				std::istreambuf_iterator<char>());
		consts = temp;
		unsigned int k = 0;
		char buf[consts.size() + 1];
		int offset;
		offset = 0;
		copy(consts.begin(), consts.end(), buf);
		buf[consts.size()] = '\0';
		for(int i = 0; i < resolution; i++) {
			for(int j = 0; j < numParam; j++) {
				float *curPar = selectivityConsts[j];
				offset = k;
				while(k < consts.size()) {
					if(buf[k] == ',' || buf[k] == '\n')
						break;
					k++;
				}
				buf[k++] = '\0';
				curPar[i] = atof(buf + offset);
			}
		}
	}
	static void deleteSelectivityConsts(int numParam) {
		for(int i = 0; i < numParam; i++)
			delete selectivityConsts[i];
		delete selectivityConsts;
		selectivityConsts = NULL;
	}
	static bool replace(std::string& str, const std::string& from, const std::string& to)
	{
	    size_t start_pos = str.find(from);
	    if(start_pos == std::string::npos)
	        return false;
	    str.replace(start_pos, from.length(), to);
	    return true;
	}
	static string generateQuery(int *intCoord)
	{
		assert(!fixQuery.empty());
		string query(fixQuery), from(":varies");
		int i = 0;
		while(query.find(from) != string::npos) {
			char buf[32];
			float *curPar = selectivityConsts[i];
			sprintf(buf, "%f", curPar[intCoord[i]]);
			string to(buf);
			to = "<= " + to;
			replace(query, from, to);
			i++;
		}
		return query;
	}
	static int optimizePoint(int *intCoord, float *cost, int foreignPlan = 0, int *fpcErr = NULL)
	{
		int planNo = 0;
		int oldNumPlans = 0;
		string foreignPlanStr(""), query = generateQuery(intCoord);
		bool toDelete = false;
		if(foreignPlan) {
			Plan *obj = getPlanById(foreignPlan);
			foreignPlanStr = obj->getXml();
			PlanInfo::getObj()->evalCalls++;
			toDelete = true;
		}
		else {
			PlanInfo::getObj()->optCalls++;
			oldNumPlans = numPlans();
		}
		Plan *plan = optimize(query, foreignPlan, foreignPlanStr, fpcErr);
		*cost = (float) plan->getCost();
		planNo = plan->getPlanNo();
		if(oldNumPlans > 0 && planNo <= oldNumPlans)
			toDelete = true;
		if(toDelete)
			delete plan;
		return planNo;
	}
	static Plan* optimize(string query, int foreignPlan, string foreignPlanStr = emptyStr, int *fpcErr = NULL)
	{
		Plan *plan = DBConn::getPlan(query, foreignPlanStr);
		int oldNumPlan = numPlans();
		plan->setPlanNo(foreignPlan, fpcErr);
		if(!foreignPlanStr.empty())
			plan->setXml(foreignPlanStr);
		else {
			if(plan->getPlanNo() <= oldNumPlan) {
				Plan *old = getPlanById(plan->getPlanNo());
				plan->setXml(old->getXml());
			}
			else
				plan->setXml(getXmlPlan(query));
		}
		return plan;
	}
	static float GenerateQueryAndEvaluatePlanCost(int *intCoord, int foreignPlan, int *fpcErr = NULL)
	{
		float cost;
		int planId;
		if(fpcErr != NULL) {
			planId = optimizePoint(intCoord, &cost, foreignPlan, fpcErr);
//			if(*fpcErr)
//				cout<<"different plan found while doing FPC.....................\n";
		}
		else {
			int fpcErrLoc = 0;
			planId = optimizePoint(intCoord, &cost, foreignPlan, &fpcErrLoc);
			if(fpcErrLoc) {
				switch(fpcErrLoc) {
				case 1: cout<<"different plan found while doing FPC..........Arguments different...........\n";
				break;
				case 2: cout<<"different plan found while doing FPC..........Order different...........\n";
				break;
				case 3: cout<<"different plan found while doing FPC..........some operators got deleted...........\n";
				break;
				case 4: cout<<"different plan found while doing FPC..........some operators got added...........\n";
				break;
				}
			}
		}
		assert(planId == foreignPlan);
		return cost;
	}
	bool equals(Plan *plan, int *fpcErr = NULL)
	{
		if(plan->getPlanNo() == this->getPlanNo())
			return true;
		if(plan->nodes.size() == this->nodes.size()) {
			if(fpcErr != NULL)
				*fpcErr = 0;		//Initialize with no error.
			Node *planNode, *thisNode;
			std::vector<Node*>::iterator planItr = plan->nodes.begin();
			std::vector<Node*>::iterator thisItr = this->nodes.begin();
			while(thisItr != this->nodes.end()) {
				planNode = *planItr;
				thisNode = *thisItr;
//				if(planNode->getId() != thisNode->getId()
//						|| planNode->getParentId() != thisNode->getParentId()
//						|| planNode->getName().compare(thisNode->getName())
				//						|| planNode->getArgName().compare(thisNode->getArgName()))
				if(planNode->getName().compare(thisNode->getName())
						|| !Node::areArgsEqual(planNode->getArgName(), thisNode->getArgName())) {
					if(fpcErr != NULL && planNode->getName().compare(thisNode->getName()))
						*fpcErr = 2;		//Different order may be.
					else if(fpcErr != NULL)
						*fpcErr = 1;		//Different arguments.
					return false;
				}
				planItr++;
				thisItr++;
			}
			return true;
		}
		else {
			if(fpcErr != NULL && plan->nodes.size() > this->nodes.size())
				*fpcErr = 3;		//Operator got removed.
			else if(fpcErr != NULL) {
				assert(plan->nodes.size() < this->nodes.size());
				*fpcErr = 4;		//More operators got added.
			}
		}
		return false;
	}
	static Plan* getPlanById(int planId)
	{
		vector<Plan*> *allPlans = PlanInfo::getObj()->allPlans;
		for(vector<Plan*>::iterator it = allPlans->begin(); it != allPlans->end(); it++) {
			Plan *plan = *it;
			if(plan->getPlanNo() == planId)
				return plan;
		}
		return NULL;
	}
	static int numOptCalls(void)
	{
		return PlanInfo::getObj()->optCalls;
	}
	static int numEvalCalls(void)
	{
		return PlanInfo::getObj()->evalCalls;
	}
	static void resetOptCalls(void)
	{
		PlanInfo::getObj()->optCalls = 0;
	}
	static void resetEvalCalls(void)
	{
		PlanInfo::getObj()->evalCalls = 0;
	}
	static void refine(string *arg)
	{
		if(arg->substr(0, 5) == "WHERE") {
			int pos1 = arg->find("<=");
			if(pos1 != -1) {
				int pos2 = arg->find(")", pos1);
				string str1 = arg->substr(0, pos1);
				string str2 = arg->substr(pos2 + 1);
				string varies(":varies");
				*arg = str1 + varies + str2;
			}
		}
		else if(arg->substr(0, 6) == "DEFINE") {
			*arg = "DEFINE:";
		}
		else {
			cout<<"\nArgument is not WHERE or DEFINE. It is "<<arg<<"\n";
			assert(0);
		}
	}
};

#endif /* PLAN_H_ */
