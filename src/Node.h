/*
 * Node.h
 *
 *  Created on: 07-Aug-2014
 *      Author: Chetas
 */

#ifndef NODE_H_
#define NODE_H_

#include <iostream>
#include <string>
#include <vector>

using namespace std;
class Node {
	int id, parentId;
	float cost, card;
	string name;
	vector<string> argNames;
public:
	Node();
	virtual ~Node();

	int getId()
	{
		return id;
	}
	void setId(int id)
	{
		this->id = id;
	}
	int getParentId()
	{
		return parentId;
	}
	void setParentId(int parentId)
	{
		this->parentId = parentId;
	}
	double getCost()
	{
		return cost;
	}
	void setCost(double cost)
	{
		this->cost = cost;
	}
	double getCard()
	{
		return card;
	}
	void setCard(double card)
	{
		this->card = card;
	}
	string getName()
	{
		return name;
	}
	void setName(string name)
	{
		if (name.empty())
			this->name = "Select";
		else
			this->name = name;
	}
	vector<string> getArgName()
	{
		return argNames;
	}
	void addArgName(string argName)
	{
		this->argNames.push_back(argName);
	}
	void showNode()
	{
//		cout<<"NodeId = "<<id<<"\tParentId = "<<parentId<<"\tName = "<<name<<"\tArgument = "<<argName<<"\tCost = "<<cost<<endl;
		cout<<"NodeId = "<<id<<"\tParentId = "<<parentId<<"\tName = "<<name<<"\tCost = "<<cost<<endl;
	}
	static bool areArgsEqual(vector<string> args1, vector<string> args2) {
		if(args1.size() != args2.size())
			return false;
		for(vector<string>::iterator i = args1.begin(); i != args1.end(); i++) {
			string arg1 = *i;
			string arg2 = "";
			bool found = false;
			for(vector<string>::iterator j = args2.begin(); j != args2.end(); j++) {
				arg2 = *j;
				if(arg1 == arg2) {
					found = true;
					break;
				}
			}
			if(!found)
				return false;
		}
		return true;
	}
};

#endif /* NODE_H_ */
