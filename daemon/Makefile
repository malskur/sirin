.PHONY = clean

statistic: statistic.o getifconf.o getsock_recv.o 
	gcc $^ -g -lpthread -DHAVE_PCAP -o $@
	
cli: cli.o
	gcc -o cli cli.o

%.o: %.c
	gcc -c $^

clean:
	rm -f *.o
	rm -f ./statistic
