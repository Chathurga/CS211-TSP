all:
	gcc -std=gnu99 main.c -o main.o -c -lrt -static -fPIC -I./ -I/opt/ibm/ILOG/CPLEX_Studio124/cplex/include
	gcc -std=gnu99 main.o -o main -lrt -static -fPIC -I/opt/ibm/ILOG/CPLEX_Studio124/cplex/include -L/opt/ibm/ILOG/CPLEX_Studio124/cplex/lib/x86-64_sles10_4.1/static_pic -lcplex -lm -lpthread