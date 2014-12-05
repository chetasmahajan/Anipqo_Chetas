/*
 * Plan.cpp
 *
 *  Created on: 07-Aug-2014
 *      Author: Chetas
 */

#include "Plan.h"
#define DSN "TPCH"
//#define DSN "TPCH_allIndices"
#define USER "sa"
#define PASS "sa123"

Plan::Plan() {
	// TODO Auto-generated constructor stub
	planNo = 0;
	cost = 0;
}

Plan::~Plan() {
	// TODO Auto-generated destructor stub
	for(std::vector<Node*>::iterator it = nodes.begin(); it != nodes.end(); it++)
		delete *it;
}

SQLRETURN DBConn::ret;
SQLHENV DBConn::henv;
SQLHDBC DBConn::connNorm, DBConn::connPlanAll, DBConn::connPlanXML;
SQLHSTMT DBConn::stmtNorm, DBConn::stmtPlanAll, DBConn::stmtPlanXML;

//DBConn::DBConn() {
//}
//
//DBConn::~DBConn() {
//}

void DBConn::createObjs(void){
	// Allocate environment handle
	ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

		// Allocate all connection handles
		if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
			// Allocate normal query execution connection handle.
			ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &connNorm);

			// Set login timeout to 5 seconds
			if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(connNorm, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source
				ret = SQLConnect(connNorm, (SQLCHAR*) DSN, SQL_NTS, (SQLCHAR*) USER, SQL_NTS, (SQLCHAR*) PASS, SQL_NTS);

				// Allocate statement handle
				if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
					cout<<"Normal Connection couldn't established.\n";
				else {
					// Allocate statement handle for normal queries.
					ret = SQLAllocHandle(SQL_HANDLE_STMT, connNorm, &stmtNorm);
				}
			}

			// Allocate connection handle for getting plan details.
			ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &connPlanAll);

			// Set login timeout to 5 seconds
			if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(connPlanAll, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source
				ret = SQLConnect(connPlanAll, (SQLCHAR*) DSN, SQL_NTS, (SQLCHAR*) USER, SQL_NTS, (SQLCHAR*) PASS, SQL_NTS);

				// Allocate statement handle
				if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
					cout<<"Connection for getting plans couldn't established.\n";
				else {
					// Allocate statement handle for getting plan details.
					ret = SQLAllocHandle(SQL_HANDLE_STMT, connPlanAll, &stmtPlanAll);
					SQLExecDirect(stmtPlanAll, (SQLCHAR *) "set showplan_all on", SQL_NTS);
				}
			}

			// Allocate connection handle for getting xml plan for FPC.
			ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &connPlanXML);

			// Set login timeout to 5 seconds
			if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(connPlanXML, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source
				ret = SQLConnect(connPlanXML, (SQLCHAR*) DSN, SQL_NTS, (SQLCHAR*) USER, SQL_NTS, (SQLCHAR*) PASS, SQL_NTS);

				// Allocate statement handle
				if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
					cout<<"Connection for getting xml plans couldn't established.\n";
				else {
					// Prepare statement for getting xml plan.
					ret = SQLAllocHandle(SQL_HANDLE_STMT, connPlanXML, &stmtPlanXML);
					SQLExecDirect(stmtPlanXML, (SQLCHAR *) "set showplan_xml on", SQL_NTS);
				}
			}
		}
	}
}
void DBConn::destroyObjs(void)
{
	SQLFreeHandle(SQL_HANDLE_STMT, stmtNorm);
	SQLFreeHandle(SQL_HANDLE_STMT, stmtPlanAll);
	SQLFreeHandle(SQL_HANDLE_STMT, stmtPlanXML);

	SQLDisconnect(connNorm);
	SQLDisconnect(connPlanAll);
	SQLDisconnect(connPlanXML);

	SQLFreeHandle(SQL_HANDLE_DBC, connNorm);
	SQLFreeHandle(SQL_HANDLE_DBC, connPlanAll);
	SQLFreeHandle(SQL_HANDLE_DBC, connPlanXML);

	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}
Plan* DBConn::getPlan(string query, string foreignPlan)
{
	Plan *plan = NULL;
	plan = new Plan();
	assert(plan);
	ret = SQLExecDirect(stmtNorm, (SQLCHAR *) "DBCC FREEPROCCACHE", SQL_NTS);

	if(!SQL_SUCCEEDED(ret))
		cout<<"DBCC FREEPROCCACHE failed to be executed.";

	if(!foreignPlan.empty()) {
		query += " option (use plan N'" + foreignPlan + "')";
	}
	//Chetas- Added to avoid of getting different plan at the time of FPC. This is specifically to get rid of Bitmap create operator in execution plan.
	else
		query += " option (MAXDOP 1)";
	//*************

	ret = SQLExecDirect(stmtPlanAll, (SQLCHAR *) query.c_str(), SQL_NTS);
	if(SQL_SUCCEEDED(ret)) {
		while(SQL_SUCCEEDED(SQLFetch(stmtPlanAll))) {
			char physicalOp[128], logicalOp[128], arguments[512];
			SQLINTEGER indicator;
			int id, parentId;
			float cost, card;
			Node *node = NULL;
			node = new Node();
			assert(node);
			ret = SQLGetData(stmtPlanAll, 3, SQL_C_DEFAULT, &id, sizeof(&id), (SQLINTEGER *) &indicator);
			if(SQL_SUCCEEDED(ret)) {
				if(indicator == SQL_NULL_DATA)
					id = 0;
				node->setId(id);
			}
			else
				goto Error;
			ret = SQLGetData(stmtPlanAll, 4, SQL_C_DEFAULT, &parentId, sizeof(&parentId), (SQLINTEGER *) &indicator);
			if(SQL_SUCCEEDED(ret)) {
				if(indicator == SQL_NULL_DATA)
					parentId = 0;
				node->setParentId(parentId);
			}
			else
				goto Error;
			ret = SQLGetData(stmtPlanAll, 5, SQL_C_CHAR, physicalOp, sizeof(physicalOp), (SQLINTEGER *) &indicator);
			if(SQL_SUCCEEDED(ret)) {
				if(indicator == SQL_NULL_DATA)
					strcpy(physicalOp, "");
				string str(physicalOp);
				node->setName(str);
			}
			else
				goto Error;
			if(!strcmp(physicalOp, "Hash Match")) {
				ret = SQLGetData(stmtPlanAll, 6, SQL_C_CHAR, logicalOp, sizeof(logicalOp), (SQLINTEGER *) &indicator);
				if(SQL_SUCCEEDED(ret)) {
					if(indicator == SQL_NULL_DATA)
						strcpy(logicalOp, "LogicalOp");
					if(!strcmp(logicalOp, "Aggregate") || !strcmp(logicalOp, "Partial Aggregate")) {
						string str(logicalOp), hashmatch("Hash Match - ");
						str = hashmatch + str;
						node->setName(str);
					}
				}
				else
					goto Error;
			}
			ret = SQLGetData(stmtPlanAll, 7, SQL_C_CHAR, arguments, sizeof(arguments), (SQLINTEGER *) &indicator);
			if(SQL_SUCCEEDED(ret)) {
				if(indicator == SQL_NULL_DATA)
					strcpy(arguments, "NULL");
				string str(arguments);
				if(str.compare("NULL")) {
/*						&& (node->getName().find("Scan") != string::npos
								|| node->getName().find("Seek") != string::npos
								|| node->getName().find("Index Spool") != string::npos
								|| node->getName().find("Table Spool") != string::npos
								|| node->getName().find("Lookup") != string::npos)) {*/
					/*string relName;
					string openBracket("[");
					string closeBracket("]");
					int posClose, posOpen = -1;
					for(int i = 0; i < 3; i++) {
						posOpen = str.find(openBracket, posOpen + 1);
					}
					posClose = str.find(closeBracket, posOpen + 1);
					relName = str.substr(posOpen + 1, posClose - posOpen - 1);
					str = relName;
					if(str.substr(0, 6) == "OBJECT") {
						string objName;
						string comma(",");
						int pos = str.find(comma);
						objName = str.substr(0, pos);
						str = objName;
					}
					else
						str = "Object Not Found";*/
					string arg;
					int numArgs = 0;
					unsigned pos = str.find(":", 0);
					string colon(":");
					while(pos != string::npos) {
						numArgs++;
						pos = str.find(":", pos + 1);
					}
					pos = 0;
					for(int i = 1; i < numArgs; i++) {
						int commaPos = 0;
						int spacePos = 0;
						int colPos = str.find(":", 0);
						colPos = str.find(":", colPos + 1);
						string temp = str.substr(0, colPos);
						commaPos = temp.find_last_of(",");
						arg = temp.substr(0, commaPos);
						if(arg.substr(0, 5) == "WHERE" || arg.substr(0, 6) == "DEFINE")
							Plan::refine(&arg);
						node->addArgName(arg);
						spacePos = temp.find_last_of(" ");
						str = str.substr(spacePos + 1);
					}
					if(str.substr(0, 5) == "WHERE" || str.substr(0, 6) == "DEFINE")
						Plan::refine(&str);
					node->addArgName(str);
				}
			}
			else
				goto Error;
			ret = SQLGetData(stmtPlanAll, 9, SQL_C_FLOAT, &card, sizeof(card), (SQLINTEGER *) &indicator);
			if(SQL_SUCCEEDED(ret)) {
				if(indicator == SQL_NULL_DATA)
					card = 0;
				node->setCard(card);
			}
			else
				goto Error;
			ret = SQLGetData(stmtPlanAll, 13, SQL_C_FLOAT, &cost, sizeof(&cost), (SQLINTEGER *) &indicator);
			if(SQL_SUCCEEDED(ret)) {
				if(indicator == SQL_NULL_DATA)
					cost = 0;
				node->setCost(cost);
			}
			else
				goto Error;
			plan->addNode(node);
			if(plan->getCost() == 0)
				plan->setCost(node->getCost());
		}
		SQLFreeHandle(SQL_HANDLE_STMT, stmtPlanAll);
		SQLAllocHandle(SQL_HANDLE_STMT, connPlanAll, &stmtPlanAll);
		return plan;
	}

	Error:
	SQLFreeHandle(SQL_HANDLE_STMT, stmtPlanAll);
	SQLAllocHandle(SQL_HANDLE_STMT, connPlanAll, &stmtPlanAll);
	delete plan;
	return NULL;
}
string DBConn::getXmlPlan(string query)
{
	ret = SQLExecDirect(stmtNorm, (SQLCHAR *) "DBCC FREEPROCCACHE", SQL_NTS);
	if(!SQL_SUCCEEDED(ret))
		cout<<"DBCC FREEPROCCACHE failed to be executed.";
	query += " option (MAXDOP 1)";	//Chetas- Added to avoid of getting different plan at the time of FPC. This is specifically to get rid of Bitmap create operator in execution plan.
	ret = SQLExecDirect(stmtPlanXML, (SQLCHAR *) query.c_str(), SQL_NTS);
	if(SQL_SUCCEEDED(ret)) {
		if(SQL_SUCCEEDED(SQLFetch(stmtPlanXML))) {
			char xmlStr[102400];
			SQLINTEGER indicator;
			ret = SQLGetData(stmtPlanXML, 1, SQL_C_CHAR, xmlStr, sizeof(xmlStr), (SQLINTEGER *) &indicator);
			if(SQL_SUCCEEDED(ret)) {
				if(indicator == SQL_NULL_DATA)
					strcpy(xmlStr, "");
				SQLFreeHandle(SQL_HANDLE_STMT, stmtPlanXML);
				SQLAllocHandle(SQL_HANDLE_STMT, connPlanXML, &stmtPlanXML);
				string str(xmlStr);
				return str;
			}
		}
	}

	SQLFreeHandle(SQL_HANDLE_STMT, stmtPlanXML);
	SQLAllocHandle(SQL_HANDLE_STMT, connPlanXML, &stmtPlanXML);
	string str("");
	return str;
}
