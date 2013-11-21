all:
	gcc ./src/main.c -o main.o -c -std=gnu99 -fPIC -I./ -I/opt/ibm/ILOG/CPLEX_Studio124/cplex/include
	gcc main.o -o main -lcplex -lpthread -lm -lrt -L/opt/ibm/ILOG/CPLEX_Studio124/cplex/lib/x86-64_sles10_4.1/static_pic
	@rm main.o