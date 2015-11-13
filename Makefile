branchpredictor:
	gcc -g -Wall branchpredictor.c -o sim -lm

bimodal:
	./sim bimodal 6 traces/gcc_trace.txt > 1
	./sim bimodal 12 traces/gcc_trace.txt > 2
	./sim bimodal 4 traces/jpeg_trace.txt > 3
	./sim bimodal 5 traces/perl_trace.txt > 4
	gvimdiff 1 ./ValidationRuns/val_bimodal_1.txt
	gvimdiff 2 ./ValidationRuns/val_bimodal_2.txt
	gvimdiff 3 ./ValidationRuns/val_bimodal_3.txt
	gvimdiff 4 ./ValidationRuns/val_bimodal_4.txt


clean:
	rm -f branchpredictor.o

