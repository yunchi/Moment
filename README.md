MomentFP: a system for mining and maintaining closed frequent itemsets over data streams [MomentFP.tar.gz] 

Citation information:

[1] Yun Chi, Haixun Wang, Philip S. Yu, and Richard R. Muntz. Catch the moment: maintaining closed frequent itemsets over a data stream sliding window. Knowledge and Information Systems, 10(3):265-294, 2006.

[2] Yun Chi, Haixun Wang, Philip S. Yu, and Richard R. Muntz. Moment: Maintaining closed frequent itemsets over a stream sliding window. In ICDM '04: Proceedings of the Fourth IEEE International Conference on Data Mining, pages 59-66, Washington, DC, USA, 2004. IEEE Computer Society.

Note: The MOA (Massive Online Analysis) project has a Java implementation of Moment, MOA-Moment.

The following codes were implemented in MS VC++ 6.0 and successfully compiled under Redhat Linux 6.0 using g++ 2.96.

The usage of the program is: "momentFP window_size support item_size input_file output_file", where:
window_size: the sliding window size
support: the minimum support (as integer)
itemsize: number of distinct items in the data set
input_file: name of the input file
output_file: name of the output file

The input file should be in the format defined by the IBM synthetic data generator, i.e.,
<CustID, TransID, NumItems, List-Of-Items>

In the output file:
(1) The first line gives the size of the FP-tree for the first sliding window.
(2) The second line gives the window index (0) for the first sliding window, the running time for
loading the first window (including building the FP-tree and the CET), the number of closed
frequent itemsets in the first sliding window, and the size of CET for the first sliding window.
(3) Each of the following line represent one sliding window. The data contains for the current
sliding window: the index of the window, the running time, the number of closed frequent itemsets, 
the size of the CET, the number of calls for the Explore() function, the number of nodes added to 
the CET, the number of nodes deleted from the CET, the number of closed frequent itemsets before
the deletion (after the addition).
(4) Finally, the average of the above numbers.

For example, a toy example included in this package is test.ascii, which contains 6 transactions
and 4 distinct items. So if we want to run with the sliding window size to be 4 and the support
to be 2 (50%), then we use the following command:
./momentFP 4 2 4 test.ascii output.txt

As another example, the first 1100 transactions of the mushroom data set are included in this
pakcage as mushroom1000.ascii. It contains 120 distinct itemsets. E.g., if we want to run it
with sliding window size to be 1000 and the support to be 10 (1%), we can use:
./momentFP 1000 10 120 mushroom1000.ascii output.txt

