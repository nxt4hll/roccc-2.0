all: initializeFP createScript

clean: 
	rm -f *~ *.o addCore dumpCores initializeFP createScript

initializeFP: initialize.cc
	g++ -Wall -I${LLVM_PATH}/sqlite-3.6.11/ -L${LLVM_PATH}/sqlite-3.6.11/.libs/  -o initializeFP initialize.cc -lsqlite3

createScript.o: createScript.cc scriptGenerator.h
	g++ -Wall -g -c createScript.cc

scriptGenerator.o: scriptGenerator.cc scriptGenerator.h
	g++ -Wall -g -c scriptGenerator.cc

createScript: scriptGenerator.o createScript.o
	g++ -Wall -g -o createScript scriptGenerator.o createScript.o
