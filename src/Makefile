CC = gcc
PREFIX = /usr/local/bin
CFLAGS = 
INCLUDE = .

all: yaibtgwd

clean:
	rm *.o
	rm yaibtgwd
	rm sqlite3.h
	rm shell.c
	rm sqlite3ext.h
	rm sqlite3.c

install: all
	cp yaibtgwd $(PREFIX)

uninstall:
	rm $(PREFIX)/yaibtgwd

get_sqlite:
	wget http://www.sqlite.org/sqlite-amalgamation-3071100.zip
	unzip sqlite-amalgamation-3071100.zip
	cp sqlite-amalgamation-3071100/sqlite3.h sqlite3.h
	cp sqlite-amalgamation-3071100/shell.c shell.c
	cp sqlite-amalgamation-3071100/sqlite3ext.h sqlite3ext.h
	cp sqlite-amalgamation-3071100/sqlite3.c sqlite3.c
	rm -r sqlite-amalgamation-3071100
	rm sqlite-amalgamation-3071100.zip

yaibtgwd: get_sqlite IRC_Bot.o sqlite3.o daemon.o
	$(CC) -I$(INCLUDE)  $(CFLAGS) -o yaibtgwd IRC_Bot.o sqlite3.o daemon.o -lpthread -ldl

IRC_Bot.o: IRC_Bot.c sqlite3.h
	$(CC) -I$(INCLUDE) $(CFLAGS) -c IRC_Bot.c

sqlite3.o: get_sqlite sqlite3.c sqlite3.h
	$(CC) -I$(INCLUDE) $(CFLAGS) -c sqlite3.c

daemon.o: daemon.c daemon.h
	$(CC) -I$(INCLUDE) $(CFLAGS) -c daemon.c
