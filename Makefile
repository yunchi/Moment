GCC=g++ -O3
HEADER=CET.h FP.h misc.h
OBJS=momentFP.o CET.o FP.o
momentFP: $(OBJS) $(HEADER)
	$(GCC) -o momentFP ${OBJS}
	rm -f $(OBJS)
momentFP.o: momentFP.cpp $(HEADER)
	$(GCC) -c momentFP.cpp
CET.o: CET.cpp CET.h FP.h misc.h
	$(GCC) -c CET.cpp
FP.o: FP.cpp FP.h misc.h
	$(GCC) -c FP.cpp
