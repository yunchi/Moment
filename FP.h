/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
Header: FP.h
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef FP_H
#define FP_H

struct IdxNode; //a node in the indexing link list
struct FPNode; //a node in the FP tree
typedef IdxNode* IdxNodePtr; 
typedef FPNode* FPNodePtr; 
typedef map<unsigned short, FPNode> FPFamily; //<node label, FPNode>


struct TidNode
{
	int tid;
	FPNodePtr tailOfItemset;

	TidNode ()
	{
		tid = 0;
		tailOfItemset = 0;
	}

	TidNode (int newTid, FPNodePtr newTail)
	{
		tid = newTid;
		tailOfItemset = newTail;
	}
};

struct FPNode
{
	static long numberOfFPNodes;

	FPNodePtr parent;
	unsigned short item;
	int count;
	int tid_sum;
	FPFamily children;
	IdxNodePtr locInIdx;

	FPNode () //default constructor
	{
		numberOfFPNodes ++;

		parent = 0;
		item = 0;
		count = 0;
		tid_sum = 0;
		locInIdx = 0;
	}

	FPNode (const FPNode & rhs) //copy constructor, defined because need to update number of nodes
	{
		numberOfFPNodes ++;

		parent = rhs.parent;
		item = rhs.item;
		count = rhs.count;
		tid_sum = rhs.tid_sum;
		locInIdx = rhs.locInIdx;
	}

	~FPNode () //default destructor
	{
		numberOfFPNodes --;
	}

};

struct IdxNode
{
	IdxNodePtr left;
	IdxNodePtr right;
	FPNodePtr locInFP;
	IdxNode () //default constructor
	{
		left = 0;
		right = 0;
		locInFP = 0;
	}
};

struct FP
{
	vector<IdxNode> headTable;
	FPNode FPRoot;
	deque<TidNode> tidList;
	vector<int> headCount;
	vector<int> headTidSum;

	FP (unsigned short size) //there is no default constructor
	{
		IdxNode dummy;
		headTable.assign(size, dummy);
		headCount.assign(size,0);
		headTidSum.assign(size,0);
	}

	~FP() //destructor, clear the headTable
	{
		//IdxNodePtr dummy1, dummy2;
		//for ( unsigned short i = 0; i < headTable.size(); i++ ) {
		//	dummy1 = headTable[i].right;
		//	while ( dummy1 != 0 ) {
		//		dummy2 = dummy1;
		//		dummy1 = dummy1->right;
		//		delete dummy2;
		//	}
		//}
	}

	void addItemset(vector<unsigned short> & itemset, int tid);
	void addHelp(FPNode & myNode, unsigned short ending, 
		vector<unsigned short> & itemset, int tid);
	void deleteItemset(vector<unsigned short> & itemset, int & tid);

	void printMe(FPNode & myNode);

};

#endif //FP_H
