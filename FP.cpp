/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
Struct: FP

Description: the FP tree, with the head table
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "misc.h"
#include "FP.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
void  addItemset ()

Decription: add an itemset to the FP-tree
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FP::addItemset(vector<unsigned short> & itemset, int tid)
{
	if ( itemset.size() == 0 ) return;
	else addHelp(FPRoot, itemset.size() - 1, itemset, tid);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
void  addHelp ()

Decription: helper function for addItemset
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FP::addHelp(FPNode & myNode, unsigned short ending, 
		vector<unsigned short> & itemset, int tid)
{
	unsigned newItem = itemset[ending];
	
	//update the global data structure
	headCount[newItem]++;
	headTidSum[newItem] += tid;

	myNode.children[newItem].count++;
	myNode.children[newItem].tid_sum += tid;
	if ( myNode.children[newItem].count == 1 ) {
		myNode.children[newItem].parent = & myNode;
		myNode.children[newItem].item = newItem;
		IdxNodePtr newIdxNode = new IdxNode;
		newIdxNode->left = & headTable[newItem];
		newIdxNode->right = headTable[newItem].right;
		headTable[newItem].right = newIdxNode;
		if ( newIdxNode->right != 0 ) {
			newIdxNode->right->left = newIdxNode;
		}
		newIdxNode->locInFP = & myNode.children[newItem];
		myNode.children[newItem].locInIdx = newIdxNode;
	}

	if ( ending > 0 ) { //not the tail of this path yet
		addHelp(myNode.children[newItem], ending - 1, itemset, tid);
	}
	else { //tail!
		TidNode dummy;
		dummy.tid = tid;
		dummy.tailOfItemset = & myNode.children[newItem];
		tidList.push_back(dummy);
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
void  deleteItemset ()

Decription: delete an itemset
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FP::deleteItemset(vector<unsigned short> & itemset, int & tid)
{
	tid = tidList.front().tid;
	FPNodePtr nodePtr = tidList.front().tailOfItemset;
	tidList.pop_front();
	while ( nodePtr->parent != 0 ) { //as long as not reach the root
		unsigned short oldItem = nodePtr->item;
		itemset.push_back(oldItem);
		headCount[oldItem]--;
		headTidSum[oldItem] -= tid;
		nodePtr->count--;
		nodePtr->tid_sum -= tid;
		FPNodePtr parentPtr = nodePtr->parent;
		if ( nodePtr->count == 0 ) { //need to remove this node from FP tree
			IdxNodePtr idxPtr = nodePtr->locInIdx;
			idxPtr->left->right = idxPtr->right;
			if ( idxPtr->right != 0 ) idxPtr->right->left = idxPtr->left;
			delete idxPtr; //delete from index link list
			parentPtr->children.erase(oldItem); //delete from FP tree
		}
		nodePtr = parentPtr;
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
void  printMe ()

Decription: a test function to print the whole FP-tree
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FP::printMe(FPNode & myNode) {
	FPFamily::iterator pos;
	for ( pos = myNode.children.begin(); pos != myNode.children.end(); ++pos ) {
		cout << pos->first << " " << pos->second.count << " "
			<< pos->second.tid_sum << endl;
		printMe(pos->second);
	}
}
