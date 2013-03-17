/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
Header: CET.h
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef CET_H
#define CET_H

#include "misc.h"
#include "FP.h"

struct TreeNode;
typedef map<unsigned short, TreeNode> Family;
typedef hash_multimap<int, pair<int,vector<unsigned short> > > HashClosed; //1st int=tidSum, 2nd int=support

struct TreeNode {
	static long numberOfCETNodes;

	Family myChildren;
	int mySupport;
	int myTidSum;
	int childrenSupport;
	bool isClosed;
	bool isInfrequent;
	bool isUnpromising;

	TreeNode()
	{
		numberOfCETNodes ++;

		mySupport = 0;
		myTidSum = 0;
		childrenSupport = 0;
		isClosed = false;
		isInfrequent = false;
		isUnpromising = false;
	}

	TreeNode(const TreeNode & rhs)
	{
		numberOfCETNodes ++;

		mySupport = rhs.mySupport;
		myTidSum = rhs.myTidSum;
		childrenSupport = rhs.childrenSupport;
		isClosed = rhs.isClosed;
		isInfrequent = rhs.isInfrequent;
		isUnpromising = rhs.isUnpromising;
	}

	~TreeNode() 
	{
		numberOfCETNodes --;
	}
};

struct CET {
	vector<unsigned short> currentPrefix;
	HashClosed closedItemsets;
	TreeNode CETRoot;

	void initialize(FP & FPTree);
	int initializeHelp(TreeNode & node,
					   map<FPNodePtr,pair<int,int> > & occurrence, //pair<sup,tid>
					   unsigned short begin,
					   unsigned short end,
					   	vector<bool> & isFrequent);
	void addition(const int tid, 
			const vector<unsigned short> & itemset,
			const FP & FPTree);
	int addHelp(const int tid,
			   const unsigned short previousPrefix,
			   const map<FPNodePtr,pair<int,int> > & previousOccurrence,
			   const vector<unsigned short> & parentItemset,
			   const vector<bool> & parentIsNew,
			   const unsigned short startPosition,
			   const FP & FPTree,
			   TreeNode & node);
	void deletion(const int tid,
			const vector<unsigned short> & itemset,
			const FP & FPTree);
	void deleteHelp(const int tid,
			   const vector<unsigned short> & parentItemset,
			   const vector<bool> & parentIsNew,
			   const unsigned short startPosition,
			   const FP & FPTree,
			   TreeNode & node);

	void getOccurrence(const unsigned short previousPrefix, 
					   const map<FPNodePtr,pair<int,int> > & previousOccurrence, 
					   const FP & FPTree,
					   map<FPNodePtr,pair<int,int> > & myOccurrence);
				
	void cleanNode(TreeNode & node);

	void printMe(TreeNode & node, unsigned short level);
	void printHash ();
	void checkMe();
};

#endif //CET_H
