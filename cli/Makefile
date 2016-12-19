.PHONY = clean

cli: cli.o
	gcc $^ -o $@

%.o: %.c
	gcc -c $^

clean:
	rm -f *.o
	rm -f ./cli
