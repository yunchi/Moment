/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
Struct: CET

Description: the CET
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "misc.h"
#include "FP.h"
#include "CET.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
bool isSubset ()

Decription: a helper function to check if list2 is a sublist of list1 
assuming data are sorted and list2 is non-empty
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool isSubset (const vector<unsigned short>& list1, const vector<unsigned short>& list2 )
{
	if ( list2.size() > list1.size() ) return false;
	else if ( list2.size() == list1.size() ) return ( list2 == list1 );
	unsigned int idx1 = 0, idx2 = 0;
	while ( idx1 < list1.size() && idx2 < list2.size() ) {
		if ( list1[idx1] == list2[idx2] ) {
			idx1++;
			idx2++;
		}
		else if ( list1[idx1] < list2[idx2] ) {
			idx1++;
			while ( idx1 < list1.size() && list1[idx1] < list2[idx2] ) idx1++;
		}
		else
			return false;
	}

	if ( idx2 < list2.size() ) return false;
	else return true;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
bool isPrefix ()

Decription: a helper function to check if list1 is a prefix of list2 
assuming data are sorted and list2 is non-empty
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool isPrefix (const vector<unsigned short>& list1, const vector<unsigned short>& list2 )
{
	if ( list1.size() > list2.size() ) return false;
	else if ( list1.size() == list2.size() ) return ( list1 == list2 );
	int idx1 = 0, idx2 = 0;
	while ( idx1 < list1.size() && idx2 < list2.size() ) {
		if ( list1[idx1] == list2[idx2] ) {
			idx1++;
			idx2++;
		}
		else
			return false;
	}

	return true;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
void  initialize ()

Decription: initialize the CET from the FP-tree
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CET::initialize(FP & FPTree)
{
	vector<bool> isFrequent(FPTree.headTable.size(), false);
	unsigned short begin = 0;
	unsigned short end = 0;

	//check if each item is frequent
	for ( unsigned short i = 0; i < FPTree.headTable.size(); i++ ) {
		CETRoot.myChildren[i].mySupport = FPTree.headCount[i]; //initialize
		CETRoot.myChildren[i].myTidSum = FPTree.headTidSum[i];
		if ( CETRoot.myChildren[i].mySupport < SUPPORT ) {
			CETRoot.myChildren[i].isInfrequent = true;
		}
		else {
			isFrequent[i] = true;
			end = i; //here end records the largest frequent item
		}
	}

	for ( unsigned short i = 0; i < isFrequent.size() && i <= end; i++ ) {
		if ( isFrequent[i] ) {
			map<FPNodePtr, pair<int,int> > occurrence;
			IdxNodePtr posIdx = FPTree.headTable[i].right;
			while ( posIdx != 0 ) {
				occurrence.insert(make_pair(posIdx->locInFP, 
					make_pair(posIdx->locInFP->count,posIdx->locInFP->tid_sum)));
				posIdx = posIdx->right;
			}

			bool stillClosed = true;

			currentPrefix.push_back(i);

			//second, check if it is promising
			pair<HashClosed::const_iterator, HashClosed::const_iterator> p =
				closedItemsets.equal_range(CETRoot.myChildren[i].myTidSum);

			for ( HashClosed::const_iterator pos = p.first; pos != p.second; ++pos ) {
				if ( pos->second.first == CETRoot.myChildren[i].mySupport 
					&& isSubset(pos->second.second, currentPrefix ) ) {
						CETRoot.myChildren[i].isUnpromising = true;
						stillClosed = false;
						break;
					}
			}

			if ( !stillClosed ) {
				currentPrefix.pop_back();
				continue; //next i, please
			}

			CETRoot.myChildren[i].childrenSupport = 
				initializeHelp(CETRoot.myChildren[i],
				occurrence, i, end, isFrequent);

			//check if it is closed
			if ( CETRoot.myChildren[i].childrenSupport < CETRoot.myChildren[i].mySupport ) {
				CETRoot.myChildren[i].isClosed = true;
				closedItemsets.insert(make_pair(CETRoot.myChildren[i].myTidSum, 
					make_pair(CETRoot.myChildren[i].mySupport,currentPrefix)));
			}

			currentPrefix.pop_back();
		}
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
int initializeHelp ()

Decription: a helper function for initialize() 
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int CET::initializeHelp(TreeNode & node,
						map<FPNodePtr,pair<int,int> > & occurrence,
						unsigned short begin,
						unsigned short end,
						vector<bool> & isFrequent)
{
	if ( begin >= end ) return 0; //end must > begin

	for ( unsigned short i = begin + 1; i <= end; i++ ) {
		if ( isFrequent[i] ) node.myChildren[i].isInfrequent = true; //initialize
	}

	unsigned short newend = begin;
	int bestSupport = 0;

	//step 1, compute the support and tid_sum for all children
	for ( map<FPNodePtr,pair<int,int> >::iterator pos = occurrence.begin(); 
		pos != occurrence.end(); ++pos) 
	{
		FPNodePtr  posFP = pos->first; //find the location in FP
		int tempCount = pos->second.first; //support
		int tempTidSum = pos->second.second; //tid sum
		posFP = posFP->parent;
		while ( posFP->parent != 0 && posFP->item <= end ) { //while not the root
			if ( isFrequent[posFP->item] ) {
				node.myChildren[posFP->item].mySupport += tempCount;
				node.myChildren[posFP->item].myTidSum += tempTidSum;
			}
			posFP = posFP->parent;
		}
	}

	//step 2, find the newly infrequent children
	for ( unsigned short i = begin + 1; i <= end; i++ ) {
		if ( isFrequent[i] ) { 
			if ( node.myChildren[i].mySupport > bestSupport )
				bestSupport = node.myChildren[i].mySupport;
			if ( node.myChildren[i].mySupport < SUPPORT ) {
				isFrequent[i] = false; //temporarily make it false
			}
			else {
				node.myChildren[i].isInfrequent = false;
				newend = i;
			}
		}
	}

	end = newend;

	if ( begin == end ) { //if no frequent children, stop here and return
		//recover the original indicator vector for frequent items of the above level
		for ( Family::iterator pos1 = node.myChildren.begin();
			pos1 != node.myChildren.end(); ++pos1 )
		{
			if ( pos1->second.isInfrequent == true ) {
				isFrequent[pos1->first] = true;
			}
		}
		return bestSupport;
	}

	//step 3, for the children remained frequent, create their occurrence list
	vector<map<FPNodePtr,pair<int,int> > > newOccurrence(end-begin);

	for ( map<FPNodePtr,pair<int,int> >::iterator pos = occurrence.begin(); 
		pos != occurrence.end(); ++pos) 
	{
		FPNodePtr  posFP = pos->first; //find the location in FP
		int tempCount = pos->second.first; //support
		int tempTidSum = pos->second.second; //tid sum
		posFP = posFP->parent;
		while ( posFP->parent != 0 && posFP->item <= end ) { //while not the root
			if ( isFrequent[posFP->item] ) {
				map<FPNodePtr,pair<int,int> >::iterator pos2 = 
					newOccurrence[posFP->item - begin - 1].find(posFP);
				if ( pos2 != newOccurrence[posFP->item - begin - 1].end() ) {
					pos2->second.first += tempCount;
					pos2->second.second += tempTidSum;
				}
				else {
					newOccurrence[posFP->item - begin - 1].insert(
						make_pair(posFP, make_pair(tempCount, tempTidSum)));
				}
			}
			posFP = posFP->parent;
		}
	}

	//step 4, for the children remained frequent, recursively explore
	for ( unsigned short i = begin + 1; i <= end; i++ ) {
		if ( isFrequent[i] ) { 

			//first, mark the node to be frequent
			node.myChildren[i].isInfrequent = false; //redundant

			bool stillClosed = true;

			currentPrefix.push_back(i);

			//second, check if it is promising
			pair<HashClosed::const_iterator, HashClosed::const_iterator> p =
				closedItemsets.equal_range(node.myChildren[i].myTidSum);

			for ( HashClosed::const_iterator pos = p.first; pos != p.second; ++pos ) {
				if ( pos->second.first == node.myChildren[i].mySupport 
					&& isSubset(pos->second.second, currentPrefix ) ) {
						node.myChildren[i].isUnpromising = true;
						stillClosed = false;
						break;
					}
			}

			if ( !stillClosed ) {
				currentPrefix.pop_back();
				continue; //next i, please
			}

			node.myChildren[i].childrenSupport = 
				initializeHelp(node.myChildren[i],
				newOccurrence[i-begin-1], i, end, isFrequent);

			//check if it is closed
			if ( node.myChildren[i].childrenSupport < node.myChildren[i].mySupport ) {
				node.myChildren[i].isClosed = true;
				closedItemsets.insert(make_pair(node.myChildren[i].myTidSum, 
					make_pair(node.myChildren[i].mySupport,currentPrefix)));
			}

			currentPrefix.pop_back();
		}
	}

	//step 5, recover the original indicator vector for frequent items of the above level
	for ( Family::iterator pos1 = node.myChildren.begin();
		pos1 != node.myChildren.end(); ++pos1 )
	{
		if ( pos1->second.isInfrequent == true ) {
			isFrequent[pos1->first] = true;
		}
	}

	return bestSupport;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
void getOccurrence ()

Decription: grab the occurrence list to the current node
Note: assumed that previousPrefix < currentPrefix.size()
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CET::getOccurrence(const unsigned short previousPrefix, 
						const map<FPNodePtr,pair<int,int> > & previousOccurrence, 
						const FP & FPTree,
						map<FPNodePtr,pair<int,int> > & myOccurrence)
{
	//case 1, called by CETRoot, when we actually don't use the returned value
	if ( previousPrefix == 0 && currentPrefix.size() == 0 ) {
		return;
	}
	//case 2, called by the children of CETRoot. We use FPTree directly
	else if ( previousPrefix == 0 && currentPrefix.size() == 1 ) { //the root node
		IdxNodePtr posIdx = FPTree.headTable[currentPrefix[0]].right;
		while ( posIdx != 0 ) {
			myOccurrence.insert(make_pair(posIdx->locInFP, 
				make_pair(posIdx->locInFP->count,posIdx->locInFP->tid_sum)));
			posIdx = posIdx->right;
		}
	}
	//case 3, called by descendants of CETRoot, but never called before
	else if ( previousPrefix == 0 && currentPrefix.size() > 1 ) {
		map<FPNodePtr,pair<int,int> > tempOccurrence;
		IdxNodePtr posIdx = FPTree.headTable[currentPrefix[0]].right;
		while ( posIdx != 0 ) {
			tempOccurrence.insert(make_pair(posIdx->locInFP, 
				make_pair(posIdx->locInFP->count,posIdx->locInFP->tid_sum)));
			posIdx = posIdx->right;
		}
		getOccurrence(1, tempOccurrence, FPTree, myOccurrence);
	}
	//case 4, they are the same. This should not happen at all.
	else if ( previousPrefix == currentPrefix.size() ) {
		myOccurrence = previousOccurrence; 
	}
	//case 5, the general case, i.e., previousPrefix >= 1, so previousOccurrence non-empty 
	else {
		for ( map<FPNodePtr,pair<int,int> >::const_iterator pos = previousOccurrence.begin(); 
			pos != previousOccurrence.end(); ++pos) 
		{
			FPNodePtr  posFP = pos->first; //find the location in FP
			int tempCount = pos->second.first; //support
			int tempTidSum = pos->second.second; //tid sum
			bool isFound = false;
			unsigned short start = previousPrefix;
			posFP = posFP->parent;
			while ( posFP->parent != 0 && !isFound ) {
				if ( posFP->item > currentPrefix[start] ) { //failed
					break;
				}
				else if ( posFP->item == currentPrefix[start] ) {
					start++;
					if ( start == currentPrefix.size() ) {
						isFound = true;
						break;
					}
				}
				posFP = posFP->parent;
			} //end of while

			if ( isFound ) {
				map<FPNodePtr,pair<int,int> >::iterator pos2 = 
					myOccurrence.find(posFP);
				if ( pos2 != myOccurrence.end() ) {
					pos2->second.first += tempCount;
					pos2->second.second += tempTidSum;
				}
				else {
					myOccurrence.insert(make_pair(posFP, make_pair(tempCount, tempTidSum)));
				}
			} //end of if

		} //end of for
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
void addition ()

Decription: add an itemset to the CET 
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CET::addition(const int tid, 
				   const vector<unsigned short> & itemset, 
				   const FP & FPTree)
{
	map<FPNodePtr,pair<int,int> > previousOccurrence;
	vector<bool> parentIsNew(itemset.size(), false);

	////debug
	//cout << "To add itemset: ";
	//for ( int i = 0; i < itemset.size(); i++ )
	//	cout << itemset[i] << " ";
	//cout << endl;

	addHelp(tid, 0, previousOccurrence, itemset, parentIsNew,
		0, FPTree, CETRoot);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
int addHelp ()

Decription: a helper function for addition() 
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int CET::addHelp(const int tid,
				 const unsigned short previousPrefix,
				 const map<FPNodePtr,pair<int,int> > & previousOccurrence,
				 const vector<unsigned short> & parentItemset,
				 const vector<bool> & parentIsNew,
				 const unsigned short startPosition,
				 const FP & FPTree,
				 TreeNode & node)
{
	int maxNewSupport = 0;

	////debug
	//cout << "currentPrefix: " << endl;
	//for ( int i = 0; i < currentPrefix.size(); i++ )
	//	cout << currentPrefix[i] << " ";
	//cout << endl;
	//cout << "parentItemset: " << endl;
	//for ( int i = 0; i < parentItemset.size(); i++ )
	//	cout << parentItemset[i] << " ";
	//cout << endl;
	//cout << "parentIsNew: " << endl;
	//for ( int i = 0; i < parentIsNew.size(); i++ )
	//	cout << parentIsNew[i] << " ";
	//cout << endl;


	unsigned short myPrefix;
	map<FPNodePtr, pair<int,int> > myOccurrence;
	vector<unsigned short> myItemset;
	vector<bool> myIsNew;

	map<unsigned short, unsigned short> inverse;
	vector<map<FPNodePtr, pair<int,int> > > occurrenceFamily;

	bool needLoadOccurrence = false;
	bool isOccurrenceLoaded = false;
	bool existNewFrequent = false;

	unsigned short newStart = 0;
	unsigned short maxEnd = 0;

	vector<unsigned short> tempItemset;
	vector<bool> tempIsNew;

	//step 1, insert new items propogated down
	for ( unsigned short s = startPosition; s < parentItemset.size(); s++ ) {
		////debug
		//cout << "the item is: " << parentItemset[s] << endl;
		if ( parentIsNew[s] ) { //a new item propogated down
			node.myChildren[parentItemset[s]].isInfrequent = true; //initialize a new child
			tempItemset.push_back(parentItemset[s]);
			tempIsNew.push_back(true);
			needLoadOccurrence = true;
			inverse.insert(make_pair(parentItemset[s], newStart));
			newStart++;
			maxEnd = parentItemset[s]; //assuming that parentItemset is in order
		}
		else { //the updated item was one member of the family
			node.myChildren[parentItemset[s]].mySupport++;
			node.myChildren[parentItemset[s]].myTidSum += tid;
			if ( maxNewSupport < node.myChildren[parentItemset[s]].mySupport )
				maxNewSupport = node.myChildren[parentItemset[s]].mySupport;
			if ( node.myChildren[parentItemset[s]].mySupport < SUPPORT ) { 
				continue;//no need to pass down
			}
			else {
				tempItemset.push_back(parentItemset[s]);
				if ( node.myChildren[parentItemset[s]].isInfrequent ) { 
					//a newly frequent node
					node.myChildren[parentItemset[s]].mySupport = 0;
					node.myChildren[parentItemset[s]].myTidSum = 0;			
					tempIsNew.push_back(true);
					needLoadOccurrence = true;
					inverse.insert(make_pair(parentItemset[s], newStart));
					newStart++;
					maxEnd = parentItemset[s]; //assuming that parentItemset is in order
				}
				else {
					tempIsNew.push_back(false);
				}
			}
		}
	} //end for

	//step 2, build the occurrence lists
	if ( needLoadOccurrence ) {
		occurrenceFamily.resize(newStart);
		isOccurrenceLoaded = true;
		myPrefix = currentPrefix.size(); //most up-to-date

		if ( currentPrefix.size() == 0 ) { //if still at the root
			for ( map<unsigned short, unsigned short>::iterator pos = inverse.begin();
				pos != inverse.end(); ++pos )
			{
				IdxNodePtr posIdx = FPTree.headTable[pos->first].right;
				while ( posIdx != 0 ) {
					occurrenceFamily[pos->second].insert(make_pair(posIdx->locInFP, 
						make_pair(posIdx->locInFP->count,posIdx->locInFP->tid_sum)));
					node.myChildren[pos->first].mySupport += posIdx->locInFP->count;
					node.myChildren[pos->first].myTidSum += posIdx->locInFP->tid_sum;
					posIdx = posIdx->right;
				}
			}
		}
		else {
			getOccurrence(previousPrefix, previousOccurrence, FPTree, myOccurrence);
			for ( map<FPNodePtr, pair<int,int> >::iterator pos = myOccurrence.begin();
				pos != myOccurrence.end(); ++pos ) 
			{
				FPNodePtr  posFP = pos->first; //find the location in FP
				int tempCount = pos->second.first; //support
				int tempTidSum = pos->second.second; //tid sum
				posFP = posFP->parent;
				while ( posFP->parent != 0 && posFP->item <= maxEnd ) { //while not the root
					map<unsigned short, unsigned short>::iterator pos2 =
						inverse.find( posFP->item );
					if ( pos2 != inverse.end() ) {
						node.myChildren[posFP->item].mySupport += tempCount;
						node.myChildren[posFP->item].myTidSum += tempTidSum;

						map<FPNodePtr,pair<int,int> >::iterator pos3 = 
							occurrenceFamily[pos2->second].find(posFP);
						if ( pos3 != occurrenceFamily[pos2->second].end() ) {
							pos3->second.first += tempCount;
							pos3->second.second += tempTidSum;
						}
						else {
							occurrenceFamily[pos2->second].insert(make_pair(posFP, 
								make_pair(tempCount,tempTidSum)));
						}
					}
					posFP = posFP->parent;
				}
			}
		}// end else
	}

	vector<bool>::iterator pos8 = tempIsNew.begin();
	for ( vector<unsigned short>::iterator pos7 = tempItemset.begin();
		pos7 != tempItemset.end(); ++pos7, ++pos8) 
	{
		if ( maxNewSupport < node.myChildren[*pos7].mySupport )
			maxNewSupport = node.myChildren[*pos7].mySupport;

		if ( node.myChildren[*pos7].mySupport >= SUPPORT ) {
			node.myChildren[*pos7].isInfrequent = false;
			myItemset.push_back(*pos7);
			myIsNew.push_back(*pos8);
			if ( *pos8 == true ) existNewFrequent = true;
		}

	}

	////debug
	//cout << "myItemset: " << endl;
	//for ( int i = 0; i < myItemset.size(); i++ )
	//	cout << myItemset[i] << " ";
	//cout << endl;
	//cout << "myIsNew: " << endl;
	//for ( int i = 0; i < myIsNew.size(); i++ )
	//	cout << myIsNew[i] << " ";
	//cout << endl;


	//step 3, recursively update each child
	unsigned short myStartPosition = 0;
	for ( Family::iterator pos1 = node.myChildren.begin(); 
		pos1 != node.myChildren.end() && myStartPosition < myItemset.size(); ++pos1 )
	{
		if ( pos1->first > myItemset.back() ) break; //done! (not possible)

		//case 1, an old node, not in the myItemset, 
		//need to proprogate the update down ONE LEVEL
		else if ( pos1->first < myItemset[myStartPosition] ) {
			if ( pos1->second.isInfrequent || pos1->second.isUnpromising ||
				!existNewFrequent ) 
			{
				continue;
			}
			set<unsigned short> myExtendItemset;
			maxEnd = 0;
			for ( int j = myStartPosition; j < myItemset.size(); j++ ) {
				if ( myIsNew[j] ) {
					myExtendItemset.insert(myItemset[j]);
					pos1->second.myChildren[myItemset[j]].isInfrequent = true;
					maxEnd = myItemset[j];
				}
			}

			if ( myExtendItemset.size() != 0 ) {
				currentPrefix.push_back(pos1->first);
				map<FPNodePtr, pair<int,int> > childOccurrence;
				getOccurrence(myPrefix, myOccurrence, FPTree, childOccurrence);
				//if comes here, there must be newly frequent item in this level
				//so myOccurrence must have been populated

				for ( map<FPNodePtr, pair<int,int> >::iterator pos5 = childOccurrence.begin();
					pos5 != childOccurrence.end(); ++pos5 ) 
				{
					FPNodePtr  posFP = pos5->first; //find the location in FP
					int tempCount = pos5->second.first; //support
					int tempTidSum = pos5->second.second; //tid sum
					posFP = posFP->parent;
					while ( posFP->parent != 0 && posFP->item <= maxEnd ) { //while not the root
						set<unsigned short>::iterator pos6 =
							myExtendItemset.find( posFP->item );
						if ( pos6 != myExtendItemset.end() ) {
							pos1->second.myChildren[*pos6].mySupport += tempCount;
							pos1->second.myChildren[*pos6].myTidSum += tempTidSum;
						}
						posFP = posFP->parent;
					}
				}

				for ( set<unsigned short>::iterator pos6 = myExtendItemset.begin();
					pos6 != myExtendItemset.end(); ++pos6 ) 
				{
					if ( pos1->second.childrenSupport < 
						pos1->second.myChildren[*pos6].mySupport )
					{
						pos1->second.childrenSupport = 
							pos1->second.myChildren[*pos6].mySupport;
					}
				}
				currentPrefix.pop_back();
			}
		}

		//case 2, an old node, in the itemset, need to add items down
		//fact: pos1->first == myItemset[myStartPosition]
		else if ( !myIsNew[myStartPosition] ) {
			if ( pos1->second.isInfrequent ) { //need not to pass down, redundant
				continue;
			}
			else if ( pos1->second.isUnpromising ) { //if originally unpromising,

				//add the item to the end of currentPrefix
				currentPrefix.push_back(pos1->first);

				bool stillClosed = true;

				//check left-pruning
				pair<HashClosed::const_iterator, HashClosed::const_iterator> p = 
					closedItemsets.equal_range(pos1->second.myTidSum);

				for ( HashClosed::const_iterator pos = p.first; pos != p.second; ++pos ) {
					if ( pos->second.first == pos1->second.mySupport 
						&& isSubset(pos->second.second, currentPrefix ) ) {
							pos1->second.isUnpromising = true;
							stillClosed = false;
							break;
						}
				}

				if ( stillClosed ) {
					pos1->second.isUnpromising = false;
					Family::iterator pos2 = pos1;
					vector<bool> tempBool(FPTree.headTable.size(), false);
					maxEnd = 0;
					while ( pos2 != node.myChildren.end() ) {
						if ( pos2->second.mySupport >= SUPPORT ) {
							tempBool[pos2->first] = true;
							maxEnd = pos2->first;
						}
						++pos2;
					}

					map<FPNodePtr, pair<int,int> > childOccurrence;
					getOccurrence(
						(isOccurrenceLoaded? myPrefix : previousPrefix),
						(isOccurrenceLoaded? myOccurrence : previousOccurrence),
						FPTree, childOccurrence);

					pos1->second.childrenSupport = 
						initializeHelp(pos1->second, childOccurrence, 
						pos1->first, maxEnd, tempBool);

					numberOfExploreCall ++;

					//second, check if I am closed
					if ( pos1->second.childrenSupport < pos1->second.mySupport ) {
						pos1->second.isClosed = true;
						closedItemsets.insert(make_pair(pos1->second.myTidSum, 
							make_pair(pos1->second.mySupport,currentPrefix)));
					}
				}

				//remove the item from the end of currentPrefix
				currentPrefix.pop_back();
			}
			else { //if originally not unPromising and not inFrequent
				int tempSupport = 0;

				currentPrefix.push_back(pos1->first);
				if ( myStartPosition+1 < myItemset.size() ) {
					//if I am not the last item in the itemset to be updated
					tempSupport = addHelp(tid, 
						(isOccurrenceLoaded? myPrefix : previousPrefix),
						(isOccurrenceLoaded? myOccurrence : previousOccurrence),
						myItemset,
						myIsNew,
						myStartPosition+1,
						FPTree,
						pos1->second);
					if ( tempSupport > pos1->second.childrenSupport )
						pos1->second.childrenSupport = tempSupport;
				}


				//if originally closed, still closed, but need to update the hash table
				if ( pos1->second.isClosed ) {
					pair<HashClosed::iterator, HashClosed::iterator> p = 
						closedItemsets.equal_range(pos1->second.myTidSum-tid); //old value

					for ( HashClosed::iterator pos = p.first; pos != p.second; ++pos ) {
						if ( pos->second.first == (pos1->second.mySupport-1)
							&& pos->second.second == currentPrefix  ) {
								closedItemsets.erase(pos);
								break;
							}
					}
					closedItemsets.insert(make_pair(pos1->second.myTidSum, 
						make_pair(pos1->second.mySupport,currentPrefix)));
				}

				else if ( pos1->second.childrenSupport < pos1->second.mySupport ) { 
					//a new closed itemset
					closedItemsets.insert(make_pair(pos1->second.myTidSum, 
						make_pair(pos1->second.mySupport,currentPrefix)));
					pos1->second.isClosed = true;
				}

				currentPrefix.pop_back();
			}
			myStartPosition++;
		}

		//case 3, a newly frequent item
		//two types: passed down from parent, or newly frequent in this level
		else {
			//add the item to the end of currentPrefix
			currentPrefix.push_back(pos1->first);

			bool stillClosed = true;

			//check left-pruning
			pair<HashClosed::const_iterator, HashClosed::const_iterator> p = 
				closedItemsets.equal_range(pos1->second.myTidSum);

			for ( HashClosed::const_iterator pos = p.first; pos != p.second; ++pos ) {
				if ( pos->second.first == pos1->second.mySupport 
					&& isSubset(pos->second.second, currentPrefix ) ) {
						pos1->second.isUnpromising = true;
						stillClosed = false;
						break;
					}
			}

			if ( stillClosed ) {
				pos1->second.isUnpromising = false;
				Family::iterator pos2 = pos1;
				vector<bool> tempBool(FPTree.headTable.size(), false);
				maxEnd = 0;
				while ( pos2 != node.myChildren.end() ) {
					if ( pos2->second.mySupport >= SUPPORT ) {
						tempBool[pos2->first] = true;
						maxEnd = pos2->first;
					}
					++pos2;
				}

				map<FPNodePtr, pair<int,int> > childOccurrence;
				getOccurrence( myPrefix + 1,
					occurrenceFamily[inverse[pos1->first]],
					FPTree, childOccurrence);

				pos1->second.childrenSupport = 
					initializeHelp(pos1->second, childOccurrence,
					pos1->first, maxEnd, tempBool);

				numberOfExploreCall ++;

				//second, check if I am closed
				if ( pos1->second.childrenSupport < pos1->second.mySupport ) {
					pos1->second.isClosed = true;
					closedItemsets.insert(make_pair(pos1->second.myTidSum, 
						make_pair(pos1->second.mySupport,currentPrefix)));
				}
			}

			//remove the item from the end of currentPrefix
			currentPrefix.pop_back();
			myStartPosition++;
		}

	}//end for (step 3)

	return maxNewSupport;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
void  cleanNode ()

Decription: a helper function for recursively removing closed itemsets
in a subtree from the Hashtable
Note: assuming currentPrefix contains the item at "node"
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CET::cleanNode(TreeNode & node)
{
	//step 1, if I am closed, remove me from the hash table
	if ( node.isClosed ) {
		pair<HashClosed::iterator, HashClosed::iterator> p = 
			closedItemsets.equal_range(node.myTidSum);
		for ( HashClosed::iterator pos = p.first; pos != p.second; ++pos ) {
			if ( pos->second.first == (node.mySupport)
				&& pos->second.second == currentPrefix ) {
					closedItemsets.erase(pos);
					break;
				}
		}
		node.isClosed = false;
	}

	//step 2, if I have children, then recursively clean my children
	if ( node.myChildren.size() != 0 ) {
		for ( Family::iterator pos1 = node.myChildren.begin(); 
			pos1 != node.myChildren.end(); ++pos1 ) 
		{
			currentPrefix.push_back(pos1->first);
			cleanNode(pos1->second);
			currentPrefix.pop_back();
		}
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
void deletion ()

Decription: delete an itemset from the CET 
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CET::deletion(const int tid,
				   const vector<unsigned short> & itemset,
				   const FP & FPTree)
{
	vector<bool> parentIsNew(itemset.size(), false);
	deleteHelp(tid, itemset, parentIsNew, 0, FPTree, CETRoot);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
void deleteHelp ()

Decription: a helper function for deletion() 
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CET::deleteHelp(const int tid,
					 const vector<unsigned short> & parentItemset,
					 const vector<bool> & parentIsNew,
					 const unsigned short startPosition,
					 const FP & FPTree,
					 TreeNode & node)
{

	vector<unsigned short> myItemset;
	vector<bool> myIsNew;
	vector<bool> myNeedErase; //check if this node should be erased
							  //according to if it is infrequent at parent's level

	bool needCount = false;
	bool existNewInFrequent = false;
	set<unsigned short> itemsToErase;

	////debug
	//cout << "current prefix: ";
	//for ( unsigned short s = 0; s < currentPrefix.size(); s++ )
	//	cout << currentPrefix[s];
	//cout << endl;
	//cout << "items: ";
	//for ( unsigned short s = 0; s < parentItemset.size(); s++ )
	//	cout << parentItemset[s];
	//cout << endl;
	//cout << "isNew: ";
	//for ( unsigned short s = 0; s < parentIsNew.size(); s++ )
	//	cout << parentIsNew[s];
	//cout << endl;

	//step 1, remove the parentItemset components that were infrequent in the above level
	//and copy the itemset to pass down to myItemset.
	//myIsNew records if an item becomes infrequent at THIS level
	for ( unsigned short s = startPosition; s < parentItemset.size(); s++ ) {
		if ( node.myChildren[parentItemset[s]].mySupport == node.childrenSupport )
			needCount = true; //need to recount the support of node's children

		if ( parentIsNew[s] ) {//just became infrequent in PARENT's level
			if ( node.myChildren[parentItemset[s]].mySupport >= SUPPORT ) {
				myItemset.push_back(parentItemset[s]);
				myIsNew.push_back(true);
				myNeedErase.push_back(true);
				itemsToErase.insert(parentItemset[s]);
				existNewInFrequent = true;
				currentPrefix.push_back(parentItemset[s]);
				cleanNode(node.myChildren[parentItemset[s]]);
				currentPrefix.pop_back();
			}
			else { 
				node.myChildren.erase(parentItemset[s]);
			}
			//actually, there is no difference to erase here or to erase in step 3
		}
		else {//update the support and tidsum of corresponding items
			node.myChildren[parentItemset[s]].mySupport --;
			node.myChildren[parentItemset[s]].myTidSum -= tid;
			if ( !node.myChildren[parentItemset[s]].isInfrequent ) //even if unpromising
			{
				myItemset.push_back(parentItemset[s]);
				myIsNew.push_back( node.myChildren[parentItemset[s]].mySupport < SUPPORT );
				myNeedErase.push_back(false);
				if ( node.myChildren[parentItemset[s]].mySupport < SUPPORT ) {
					node.myChildren[parentItemset[s]].isInfrequent = true; //temporary
					//later, recursive checking will handle it
					existNewInFrequent = true;
				}
			}
		}
	}

	//step 2, recursively update the children of each family
	unsigned short myStartPosition = 0;
	for ( Family::iterator pos1 = node.myChildren.begin(); 
		pos1 != node.myChildren.end() && myStartPosition < myItemset.size(); ++pos1 )
	{
		if ( pos1->first > myItemset.back() ) break; //done! (not possible)

		//case 1, an old node, not in the myItemset, 
		//need to proprogate the update down ONE LEVEL
		else if ( pos1->first < myItemset[myStartPosition] ) {
			if ( pos1->second.isInfrequent || pos1->second.isUnpromising ||
				!existNewInFrequent ) 
			{
				continue;
			}
			for ( int j = myStartPosition; j < myItemset.size(); j++ ) {
				if ( myIsNew[j] ) {
					pos1->second.myChildren.erase(myItemset[j]);
					//this children could not be frequent before: it is not
					//in the deleted itemset
				}
			}
		}

		//case 2, an old node, in the itemset, still frequent after deletion
		//need to delete items down
		//fact: pos1->first == myItemset[myStartPosition]
		else if ( !myIsNew[myStartPosition] ) {
			if ( pos1->second.isUnpromising ) {//if originally unpromising				
				myStartPosition++; //still unpromising
				continue;
			}

			else if ( pos1->second.isClosed ) {//if originally closed
				//add the item to the end of currentPrefix
				currentPrefix.push_back(pos1->first);

				//first, remove old value from the hash table, and set me to be non-closed
				pair<HashClosed::iterator, HashClosed::iterator> p = 
					closedItemsets.equal_range(pos1->second.myTidSum+tid); //old value

				for ( HashClosed::iterator pos = p.first; pos != p.second; ++pos ) {
					if ( pos->second.first == (pos1->second.mySupport+1)
						&& pos->second.second == currentPrefix  ) {
							closedItemsets.erase(pos);
							break;
						}
				}

				pos1->second.isClosed = false;

				//second, check left pruning
				bool stillClosed = true;

				p = closedItemsets.equal_range(pos1->second.myTidSum);

				for ( HashClosed::const_iterator pos = p.first; pos != p.second; ++pos ) {
					if ( pos->second.first == pos1->second.mySupport 
						&& isSubset(pos->second.second, currentPrefix)
						&& !isPrefix(currentPrefix, pos->second.second) ) {
							//notice: avoid my descendents, i.e., my support and tidsum
							//have been updated, but not my children
							pos1->second.isUnpromising = true;
							stillClosed = false;
							break;
						}
				}

				if ( !stillClosed ) { //if becomes unpromising, prune
					cleanNode(pos1->second);
					pos1->second.myChildren.erase(pos1->second.myChildren.begin(),
						pos1->second.myChildren.end());
				}
				else { //else, pass the update down
					if ( myStartPosition+1 < myItemset.size() ) {
						deleteHelp(tid,
							myItemset,
							myIsNew,
							myStartPosition+1,
							FPTree,
							pos1->second);
					}

					if ( pos1->second.childrenSupport < pos1->second.mySupport ) {
						pos1->second.isClosed = true;
						closedItemsets.insert(make_pair(pos1->second.myTidSum, 
							make_pair(pos1->second.mySupport,currentPrefix)));
					}
				}

				currentPrefix.pop_back();
			}

			else {//if originally not closed, not isInfrequent, not isUnpromising
				//add the item to the end of currentPrefix
				currentPrefix.push_back(pos1->first);

				if ( myStartPosition+1 < myItemset.size() ) {
					deleteHelp(tid,
						myItemset,
						myIsNew,
						myStartPosition+1,
						FPTree,
						pos1->second);
				}

				if ( pos1->second.childrenSupport < pos1->second.mySupport ) {//impossible
					pos1->second.isClosed = true;
					closedItemsets.insert(make_pair(pos1->second.myTidSum, 
						make_pair(pos1->second.mySupport,currentPrefix)));
				}

				currentPrefix.pop_back();
			}

			myStartPosition++;
		}

		//case 3, a node in the itemset, which just becomes infrequent
		//remove all children, if there are any
		else {
			if ( myNeedErase[myStartPosition] ) {//became infrequent at parent's level
				//do nothing
			}
			else if ( pos1->second.isUnpromising ) pos1->second.isUnpromising = false;
			else {
				//add the item to the end of currentPrefix
				currentPrefix.push_back(pos1->first);
				//recover the OLD support and tidSum, so that can be found
				//in the hash table
				pos1->second.mySupport ++;
				pos1->second.myTidSum += tid;
				cleanNode(pos1->second);
				pos1->second.mySupport --;
				pos1->second.myTidSum -= tid;

				pos1->second.myChildren.erase(pos1->second.myChildren.begin(),
					pos1->second.myChildren.end());
				currentPrefix.pop_back();
				pos1->second.isInfrequent = true;
			}
			myStartPosition++;
		}


	}//end for	

	//step 3, if exist nodes to be removed because it became infrequent at parent's level
	if ( itemsToErase.size() != 0 ) {
		for ( set<unsigned short>::iterator pos9 = itemsToErase.begin(); 
			pos9 != itemsToErase.end(); ++pos9 )
		{
			node.myChildren.erase(*pos9);
		}
	}

	//step 2, update node.childrenSupport, if necessary
	if ( needCount ) {
		node.childrenSupport = 0;
		for ( Family::iterator pos = node.myChildren.begin();
			pos != node.myChildren.end(); ++pos )
		{
			if ( node.childrenSupport < pos->second.mySupport )
				node.childrenSupport = pos->second.mySupport;
		}
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
void  printMe ()

Decription: debugging function 
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CET::printMe(TreeNode & node, unsigned short level)
{
	for ( unsigned short s = 0; s < currentPrefix.size(); s++ )
		cout << currentPrefix[s];
	cout << ": ";

	if ( node.myChildren.size() != 0 ) {
		Family::iterator pos;
		for ( pos = node.myChildren.begin(); pos != node.myChildren.end(); ++pos ) {
			cout << "\"" << pos->first << ' ' << pos->second.mySupport << ' ' << pos->second.myTidSum << ' ' 
				<< (pos->second.isInfrequent? 't' : 'f') << ' '
				<< (pos->second.isUnpromising? 't' : 'f') << ' '
				<< (pos->second.isClosed? 't' : 'f') <<  "\" ";
		}

		cout << endl;

		for ( Family::iterator pos1 = node.myChildren.begin(); 
			pos1 != node.myChildren.end(); ++pos1) 
		{
			currentPrefix.push_back(pos1->first);
			printMe(pos1->second, level + 1);
			currentPrefix.pop_back();
		}
	}
	else
		cout << endl;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
void  printHash ()

Decription: debugging function 
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CET::printHash()
{
	for ( HashClosed::iterator pos = closedItemsets.begin(); pos != closedItemsets.end(); ++pos )
	{
		cout << pos->first << " ";
		cout << pos->second.first << ":";
		for ( int i = 0; i < pos->second.second.size(); i++ ) {
			cout << " " << pos->second.second[i];
		}
		cout << endl;
	}
}


///* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
//void  checkMe ()
//
//Decription: check if the closed itemsets are correct
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
//void CET::checkMe()
//{
//	HashClosed::iterator pos1;
//	for ( pos1 = closedItemsets.begin(); pos1 != closedItemsets.end(); ++pos1 ) {
//		pair<HashClosed::iterator, HashClosed::iterator> p = 
//			closedItemsets.equal_range(pos1->first);
//
//		for ( HashClosed::const_iterator pos = p.first; pos != p.second; ++pos ) {
//			if ( pos == pos1 ) continue;
//
//			else if ( pos->second.first == pos1->second.first 
//				&& isSubset(pos->second.second, pos1->second.second) )
//			{
//				cout << "error: " << endl;
//				cout << pos1->first << " " << pos1->second.first << " ";
//				for ( int i = 0; i < pos1->second.second.size(); i++ )
//					cout << pos1->second.second[i] << "-";
//				cout << endl;
//				cout << pos->first << " " << pos->second.first << " ";
//				for ( int i = 0; i < pos->second.second.size(); i++ )
//					cout << pos->second.second[i] << "-";
//				cout << endl;
//			}
//		}
//	}
//}
