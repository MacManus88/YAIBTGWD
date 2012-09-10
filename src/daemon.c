/**
* @file daemon.c
* @brief The File includes Functions to daemonize the Program
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "daemon.h"

/**
* @brief daemonize the Program
* 
* With this Function, you can daemonize your Program.
* After the call of this Function, the Program runs in the Background and exists even after your Logout.
*/
void daemonize()
{
  pid_t pid, sid;
  if (getppid() == 1) return;
  pid = fork();
  if ( pid < 0 ) exit (1);
  /* Wenn ein Kindprozess erzeugt wurde, kann der 
     Eltern Prozess sich beenden 
  */
  if (pid > 0) exit (0);
  /* Neues file mode mask */
  umask(0);
  /* Neue SID Umgebung */
  sid = setsid();
  if (sid < 0) exit(1);
  /* Aendern des Arbeitsverzeichnisses */
  if ((chdir("/") < 0)) exit(1); 
  /* Aendern der default Filedescriptoren */
  freopen("/dev/null", "r", stdin);
  freopen("/dev/null", "w", stdout);
  freopen("/dev/null", "w", stderr);
}
