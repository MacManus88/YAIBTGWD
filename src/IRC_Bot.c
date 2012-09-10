/**
* @file IRC_Bot.c
* @brief The file includes the complete Bot, without the Daemon Function
* 
* In this file you can find the complete IRC_Bot Code including the sqlite3 Code.
* This IRC_Bot was programmed as part of the lecture "Linux Programmierung 2012".
* The Lecturere is Christoph Hahn.
* 
* @author Pascal König
* @version 1.0
* @date 2012
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>

#include "sqlite3.h"

#define DATABASE_FILE "sqlite3_db.sql"

#define BUFFER_SIZE 1000

int logging = 1;

char buf[1000];
char tok[200];

char read_data_buf[1000];

char server[200];
char port[4];
char channel[100];
char nick[100];

char uhrzeit[99];

/**
* @brief Creating the Table
* 
* This function creates the Table in the Database
* 
* @param *ptr This is the Database Handle
*/
void create_table(sqlite3 *ptr)
{
	sqlite3_exec(ptr, "CREATE TABLE log (id integer primary key, uhrzeit text, nick text, server text, channel text, nachricht text);", NULL, NULL, NULL);
	sqlite3_exec(ptr, "INSERT INTO log (nachricht) VALUES ('Tabelle erstellt');", NULL, NULL, NULL);
}

/**
* @brief split and log the received Data
* 
* This function splits and logs the received Data in the sqlite3 Database.
* 
* @param *ptr This is the Database Handle
* @param buffer This is the received Data
*/
void zeile_splitten_loggen(sqlite3 *ptr, char buffer[512])
{
	if (logging == 1)
	{
		char logbuf[700];
		char log_temp[700];
		char log_nick[100];
		char log_channel[100];
		char log_nachricht[700];
		strcpy(log_nick, strtok(buffer,"!"));
		strcpy(log_temp, strtok(NULL," "));
		strcpy(log_temp, strtok(NULL," "));
		strcpy(log_channel, strtok(NULL," "));
		strcpy(log_nachricht, strtok(NULL,"\n"));
		sprintf(logbuf, "INSERT INTO log (uhrzeit, nick, server, channel, nachricht) VALUES ('%s', '%s', '%s', '%s', '%s');", uhrzeit, log_nick, server, log_channel, log_nachricht);
		sqlite3_exec(ptr, logbuf, NULL, NULL, NULL);
	}
}

/**
* @brief split and log the send Data
* 
* This function splits and logs the send Data in the sqlite3 Database.
* 
* @param *ptr This is the Database Handle
* @param buffer This is the send Data
*/
void zeile_send_splitten_loggen(sqlite3 *ptr, char buffer[512])
{
	if (logging == 1)
	{
		char logbuf[700];
		char log_temp[700];
		char log_channel[100];
		char log_nachricht[700];
		strcpy(log_temp, strtok(buffer," "));
		strcpy(log_channel, strtok(NULL," "));
		strcpy(log_nachricht, strtok(NULL,"\n"));
		sprintf(logbuf, "INSERT INTO log (uhrzeit, nick, server, channel, nachricht) VALUES ('%s', '%s', '%s', '%s', '%s');", uhrzeit, nick, server, log_channel, log_nachricht);
		sqlite3_exec(ptr, logbuf, NULL, NULL, NULL);
	}
}

/**
* @brief log the Data without splitting
* 
* This function logs the complete Data without splitting in the sqlite3 Database.
* Only the Values "Uhrzeit" and "Nachricht" are used.
* The complete Data will be written in the Value "Nachricht".
* 
* @param *ptr This is the Database Handle
* @param buffer This is the Data
*/
void zeile_loggen(sqlite3 *ptr, char buffer[512])
{
	if (logging == 1)
	{
		char logbuf[700];
		sprintf(logbuf, "INSERT INTO log (uhrzeit, nick, server, channel, nachricht) VALUES ('%s','','','','%s');", uhrzeit, buffer);
		sqlite3_exec(ptr, logbuf, NULL, NULL, NULL);
	}
}

/**
* @brief This function reads the config File
* 
* This function reads the Config File ".config" and parse the Data to it's variable.
* 
*/
void config_einlesen(void)
{
	FILE *fpData = NULL;
	
	if ((fpData = fopen( ".config", "r" )) == NULL)
	{
	  printf( "Datei konnte nicht geöffnet werden.\n" );
	}
	
	while(fgets(buf, BUFFER_SIZE, fpData) != NULL)
	{		
		strcpy(server, strtok(buf,";"));
		strcpy(port, strtok(NULL,";"));
		strcpy(channel, strtok(NULL,";"));
		strcpy(nick, strtok(NULL,"\n"));
	}
	fclose(fpData);
}

/**
* @brief Entering the programm
* 
* The Program starts here.
* 
* @param argc The Number of Arguments
* @param argv The Arguments
*/
int main(int argc, char *argv[]) 
{
	config_einlesen();
  if(argc != 5 && strstr(server, "SERVER")) 
  {
  	printf("Bitte folgendes Format einhalten:\n%s [server] [port] [channel (ohne '#')] [nick]\n\nOder Alternativ die Datei .config ändern.\n", argv[0]);
    return 0;
  }
   
  if (argc == 5)
	{
		strcpy(server, argv[1]);
	  strcpy(port, argv[2]);
	  strcpy(channel, argv[3]);
	  strcpy(nick, argv[4]);
  }
        
  //Vor Release aktivieren - zum debuggen deaktiviert lassen
	//daemonize();
    
  //Datenbank
  int id;
	sqlite3 *db_handler;
	sqlite3_open(DATABASE_FILE, &db_handler);
	create_table(db_handler);
    
  int sock;
  int rec;
  char recvbuf[512];
  char sendbuf[512];
  char *tok;
  struct addrinfo *results, hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  int gai_err = getaddrinfo( server, port, &hints, &results);
  if(gai_err != 0) 
  {
    fprintf(stderr, "%s\n", gai_strerror(gai_err));
    exit(EXIT_FAILURE);
  }
  sock = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
  if(sock == -1) 
  {
  	perror("sock()");
   	freeaddrinfo(results);
   	exit(EXIT_FAILURE);
  }
  if(connect(sock, results->ai_addr, results->ai_addrlen) == -1) 
  {
   	perror("connect()");
   	freeaddrinfo(results);
   	exit(EXIT_FAILURE);
  }
  sprintf(sendbuf, "NICK %s\r\nUSER %s 0 * :Pascal König\r\n", nick, nick);
  if(send(sock, sendbuf, strlen(sendbuf), 0) == -1)
  {
   	perror("send()");
   	freeaddrinfo(results);
   	exit(EXIT_FAILURE);
  }
   
  while(rec = recv(sock, recvbuf, sizeof(recvbuf), 0))
  {
    if(rec <= 0)
   	break;
        
    struct tm *zeit;	
    time_t sekunde;
    time( &sekunde );
    zeit = localtime(&sekunde);
    strftime(uhrzeit, sizeof(uhrzeit), "%Y-%m-%d %H:%M", zeit);
      
    char recvbuf_temp[512];
    strcpy(recvbuf_temp, recvbuf);
      
    if(strstr(recvbuf_temp, "PRIVMSG"))
    {
	    zeile_splitten_loggen(db_handler, recvbuf_temp);
	  }
	  else
	  {
	    zeile_loggen(db_handler, recvbuf_temp);
	  }
	    
    printf("%s", recvbuf);
       
    if(strstr(recvbuf, "MODE"))
    {
      sprintf(sendbuf, "JOIN #%s\r\n", channel);
      send(sock, sendbuf, strlen(sendbuf), 0);
      printf("%s", sendbuf);
	    zeile_loggen(db_handler, sendbuf);
      memset(sendbuf, 0, 512);
    }
    if(strstr(recvbuf, "PING"))
    {
      tok = strtok(recvbuf, "PING :");
      sprintf(sendbuf, "PONG :%s", tok);
      send(sock, sendbuf, strlen(sendbuf), 0);
      printf("%s", sendbuf);
	    zeile_loggen(db_handler, sendbuf);
      memset(sendbuf, 0, 512);
    }
    if(strstr(recvbuf, "PRIVMSG") && strstr(recvbuf, nick) && strstr(recvbuf, "Uhrzeit"))
    {
      sprintf(sendbuf, "PRIVMSG #%s :%s\r\n", channel, uhrzeit);
      send(sock, sendbuf, strlen(sendbuf), 0);
      printf("%s", sendbuf);
	    zeile_send_splitten_loggen(db_handler, sendbuf);
      memset(sendbuf, 0, 512);
    }
    else if(strstr(recvbuf, "PRIVMSG") && strstr(recvbuf, nick) && strstr(recvbuf, "Kill dich"))
    {
	  	sprintf(sendbuf, "QUIT\r\n");
	    send(sock, sendbuf, strlen(sendbuf), 0);
	    printf("%s", sendbuf);
	    zeile_loggen(db_handler, sendbuf);
     	memset(sendbuf, 0, 512);
    }
    else if(strstr(recvbuf, "PRIVMSG") && strstr(recvbuf, nick) && strstr(recvbuf, "LastSeen"))
		{
    	char *recv_nick_buf;
			strcpy(recv_nick_buf, strtok(recvbuf," "));
			strcpy(recv_nick_buf, strtok(NULL," "));
			strcpy(recv_nick_buf, strtok(NULL," "));
			strcpy(recv_nick_buf, strtok(NULL," "));
			strcpy(recv_nick_buf, strtok(NULL," "));
			strcpy(recv_nick_buf, strtok(NULL,"\r"));
		
			sqlite3_stmt *vm;
			char *sql_query;
			sql_query = sqlite3_mprintf("SELECT * FROM log WHERE nick = ':%s' ORDER BY id DESC LIMIT 1", recv_nick_buf);
			printf("%s\n", sql_query);
			sqlite3_prepare_v2(db_handler, sql_query, strlen(sql_query), &vm, NULL);
			while(sqlite3_step(vm) != SQLITE_DONE)
			{
				sprintf(sendbuf, "PRIVMSG #%s :Der User %s wurde zuletzt am %s auf dem Server %s im Channel %s gesehen. Seine letzte Nachricht: %s\r\n", channel, sqlite3_column_text(vm, 2), sqlite3_column_text(vm, 1), sqlite3_column_text(vm, 3), sqlite3_column_text(vm, 4), sqlite3_column_text(vm, 5));
				send(sock, sendbuf, strlen(sendbuf), 0);
		    printf("%s", sendbuf);
	     	memset(sendbuf, 0, 512);
	    }
			sqlite3_finalize(vm);
    }
		else if(strstr(recvbuf, "PRIVMSG") && strstr(recvbuf, nick) && strstr(recvbuf, "Log ausgeben"))
    {
	  	sqlite3_stmt *vm;
			sqlite3_prepare(db_handler, "SELECT * FROM log", -1, &vm, NULL);
			while(sqlite3_step(vm) != SQLITE_DONE)
			{
				sprintf(sendbuf, "PRIVMSG #%s :%i\t%s\t%s\t%s\t%s\t%s\n", channel, sqlite3_column_int(vm, 0), sqlite3_column_text(vm, 1), sqlite3_column_text(vm, 2), sqlite3_column_text(vm, 3), sqlite3_column_text(vm, 4), sqlite3_column_text(vm, 5));
		    send(sock, sendbuf, strlen(sendbuf), 0);
		    printf("%s", sendbuf);
		    memset(sendbuf, 0, 512);
			}
			sqlite3_finalize(vm);
    }
    else if(strstr(recvbuf, "PRIVMSG") && strstr(recvbuf, nick) && strstr(recvbuf, "Switch Logging"))
    {
    	if (logging == 1)
     	{
     		sprintf(sendbuf, "PRIVMSG #%s :Logging deaktiviert\r\n", channel);
        send(sock, sendbuf, strlen(sendbuf), 0);
        printf("%s", sendbuf);
		    zeile_send_splitten_loggen(db_handler, sendbuf);
        memset(sendbuf, 0, 512);
     		logging = 0;
     	}
      else
      {
       	logging = 1;
       	sprintf(sendbuf, "PRIVMSG #%s :Logging aktiviert\r\n", channel);
        send(sock, sendbuf, strlen(sendbuf), 0);
        printf("%s", sendbuf);
		    zeile_send_splitten_loggen(db_handler, sendbuf);
        memset(sendbuf, 0, 512);
      }
    }
    else
    {
		  if(strstr(recvbuf, "PRIVMSG") && strstr(recvbuf, nick) && strstr(recvbuf, "#"))	//Nachricht im Channel
		  {
		    sprintf(sendbuf, "PRIVMSG #%s :Ich nehme folgende Befehle entgegen: '%s Uhrzeit', '%s Kill dich', '%s LastSeen NICK', '%s Log ausgeben' und '%s Switch Logging'\r\n", channel, nick, nick, nick, nick, nick);
		    send(sock, sendbuf, strlen(sendbuf), 0);
		    printf("%s", sendbuf);
			  zeile_send_splitten_loggen(db_handler, sendbuf);
		    memset(sendbuf, 0, 512);
		  }
		  if(strstr(recvbuf, "PRIVMSG") && strstr(recvbuf, nick) && !strstr(recvbuf, "#"))	//PN erhalten
		  {
		    sprintf(sendbuf, "PRIVMSG #%s :Ich nehme auch per PN Befehle entgegen, die Ausgabe erfolgt jedoch in den Channel.\r\n", channel);
		    send(sock, sendbuf, strlen(sendbuf), 0);
		    printf("%s", sendbuf);
			  zeile_send_splitten_loggen(db_handler, sendbuf);
		    memset(sendbuf, 0, 512);
		  }
		}
    memset(recvbuf, 0, 512);
  }
  freeaddrinfo(results);
  sqlite3_close(db_handler);
  return 0;
}
