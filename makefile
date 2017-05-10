all : send_file receive_file
.PHONY : all
send_file : send_file.c transfer.h
	gcc -Wall -O2 send_file.c -o send_file
receive_file : receive_file.c transfer.h
	gcc -Wall -O2 receive_file.c -o receive_file
clean :
	rm send_file receive_file