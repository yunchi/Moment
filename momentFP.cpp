//variables defined for book-keeping
//FP::numberOfFPNodes
//CET::numberOfCETNodes

#include "misc.h"
#include "FP.h"
#include "CET.h"

unsigned int WINDOW_SIZE;
unsigned int SUPPORT;
unsigned short MAX_ITEM;
unsigned int TRANSACTION_SIZE;

const unsigned short ENDSHORT = 65535;
const int ENDINT = 2000000001;

long FPNode::numberOfFPNodes = 0;
long TreeNode::numberOfCETNodes = 0;

long numberOfExploreCall = 0; //number of newly frequent and newly promising, for an addition

int main(int argc, char* argv[])
{

	if ( argc != 6 ) {	
		cout << "Usage: momentFP window_size support item_size input_file output_file" << endl;
		exit (1);
	}

	istringstream iss1(argv[1]);
	iss1 >> WINDOW_SIZE;
	if(!iss1) {
		cerr << "Invalid window_size, not an integer value!" << endl;
		exit (1);
	}

	istringstream iss2(argv[2]);
	iss2 >> SUPPORT;
	if(!iss2) {
		cerr << "Invalid support, not an integer value!" << endl;
		exit (1);
	}

	if ( SUPPORT > WINDOW_SIZE ) {
		cerr << "support should be less than or equal to window_size!" << endl;
		exit (1);
	}

	istringstream iss3(argv[3]);
	iss3 >> MAX_ITEM;
	if(!iss3 && MAX_ITEM <= 0) {
		cerr << "Invalid number of items, not a positive integer value!" << endl;
		exit (1);
	}

	string inputFile = argv[4];
	string outputFile = argv[5];

	ofstream outFile(outputFile.c_str());
	if(!outFile) {
		cerr << "cannot open OUTPUT file!" << endl;
		exit(1);
	}

	ifstream inFile(inputFile.c_str());
	if(!inFile) {
		cerr << "cannot open INPUT file!" << endl;
		outFile.close();
		exit(1);
	}

	FP mainFPTree(MAX_ITEM);

	string dummyString;
	unsigned short dummyShort;
	float dummyFloat;
	int dummyInt;

	for ( int i = WINDOW_SIZE; i > 0; i-- ) {
		inFile >> dummyInt;
		if ( !inFile ) {
			cerr << "not enough transactions for one window!" << endl;
			inFile.close();
			outFile.close();
			exit(1);
		}
		inFile >> dummyInt;
		int length;
		inFile >> length;
		vector<unsigned short> items;
		for ( int j = 0; j < length; j++ ) {
			inFile >> dummyShort;
			items.push_back(dummyShort);
		}
		mainFPTree.addItemset(items, dummyInt);
	}

	outFile << "FP_tree_size: " << FPNode::numberOfFPNodes << endl;

	clock_t t1 = clock();

	CET mainCET;
	mainCET.initialize(mainFPTree);

	clock_t t2 = clock();
	outFile << "0 " << static_cast<float>( t2 - t1 ) / CLOCKS_PER_SEC 
		<< " " << mainCET.closedItemsets.size() 
			<< " " << TreeNode::numberOfCETNodes << endl;

	////debug
	//cout << "***********************************" << endl;
	//mainCET.printMe(mainCET.CETRoot,0);
	//cout << endl;
	//mainCET.printHash();
	//cout << endl;

	float totalTime = 0;
	long totalExplore = 0;
	long totalAddedNodes = 0;
	long totalDeletedNodes = 0;
	long totalClosed = 0;
	long totalCETNodes = 0;

	long previousNodes;

	long addedNodes;
	long deletedNodes;


	int i;
	for ( i = 0; i < 100; i++ ) {

		numberOfExploreCall = 0;
		addedNodes = 0;
		deletedNodes = 0;
		previousNodes = TreeNode::numberOfCETNodes;

		inFile >> dummyInt;
		if ( inFile.eof() ) break;
		inFile >> dummyInt;
		int length;
		inFile >> length;
		vector<unsigned short> myItems;
		for ( int j = 0; j < length; j++ ) {
			inFile >> dummyShort;
			myItems.push_back(dummyShort);
		}

		t1 = clock();
		//add the new itemset
		mainFPTree.addItemset(myItems, dummyInt);
		mainCET.addition(dummyInt,myItems,mainFPTree);

		////debug
		//cout << "***********************************" << endl;
		//mainCET.printMe(mainCET.CETRoot,0);
		//cout << endl;
		//mainCET.printHash();
		//cout << endl;


		addedNodes = TreeNode::numberOfCETNodes - previousNodes;

		previousNodes = TreeNode::numberOfCETNodes;

		long temp = mainCET.closedItemsets.size();

		//delete the old itemset
		myItems.clear();
		mainFPTree.deleteItemset(myItems, dummyInt);
		mainCET.deletion(dummyInt, myItems, mainFPTree);
		t2 = clock();

		////debug
		//cout << "***********************************" << endl;
		//mainCET.printMe(mainCET.CETRoot,0);
		//cout << endl;
		//cout << i << ":" << endl;
		//mainCET.printHash();
		//cout << endl;

		deletedNodes = previousNodes - TreeNode::numberOfCETNodes;

		float tempTime = static_cast<float>( t2 - t1 ) / CLOCKS_PER_SEC;
		totalTime += tempTime;
		outFile << i+1 << " " << tempTime << " " << mainCET.closedItemsets.size() 
			<< " " << TreeNode::numberOfCETNodes
			<< " " << numberOfExploreCall 
			<< " " << addedNodes
			<< " " << deletedNodes
			<< " " << temp
			<< endl;

		totalExplore += numberOfExploreCall;
		totalAddedNodes += addedNodes;
		totalDeletedNodes += deletedNodes;
		totalClosed += mainCET.closedItemsets.size();
		totalCETNodes += TreeNode::numberOfCETNodes;

		//if ( i+1 == 15 ) {
		//	cout << "checking closed itemsets..." << endl;
		//	mainCET.checkMe();
		//}
	}
	outFile << endl;
	outFile << "average time    : " << totalTime/i << endl;
	outFile << "closed itemset #: " << totalClosed*1.0/i << endl;
	outFile << "CET node       #: " << totalCETNodes*1.0/i << endl;
	outFile << "Explore call   #: " << totalExplore*1.0/i << endl;
	outFile << "Added node     #: " << totalAddedNodes*1.0/i << endl;
	outFile << "Deleted node   #: " << totalDeletedNodes*1.0/i << endl;

	inFile.close();
	outFile.close();
	return 0;
}

