run: hw4
	./hw4 -data data_dir -output submission.csv -tree 30 -thread 10
hw4: main.o pool.o test.o train.o
	gcc -lpthread -pthread -std=c11 main.o pool.o test.o train.o -o hw4
main.o: main.c 
	gcc -std=c11 -c main.c
pool.o: pool.c pool.h
	gcc -std=c11 -c pool.c
test.o: test.c test.h
	gcc -std=c11 -c test.c
train.o: train.c train.h
	gcc -std=c11 -c train.c
clean:
	rm *.o
	rm hw4
	rm submission.csv
