all:
	gcc ./src/main.c -o tsp_solve.o -c -std=gnu99 -fPIC -I./ -I/opt/ibm/ILOG/CPLEX_Studio124/cplex/include
	gcc tsp_solve.o -o tsp_solve -lcplex -lpthread -lm -lrt -L/opt/ibm/ILOG/CPLEX_Studio124/cplex/lib/x86-64_sles10_4.1/static_pic
	@rm tsp_solve.o