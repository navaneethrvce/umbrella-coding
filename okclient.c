#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include "str.h"
#include "ip4.h"
#include "okclient.h"
#include "log.h"

static char fn[3 + IP4_FMT];
static char **ip_list;
static int ip_count;
static int inotifyFd;

void ip_sigio_handler(int sig)
{
  char buf[BUF_LEN] __attribute__ ((aligned(8)));
  ssize_t numRead = read(inotifyFd, buf, BUF_LEN);
  if(numRead > 0){
    log_dbg("Detected change in access list, rebuilding ip_list");
    build_ok_ip_list();
  }
}

int watch_ip_list()
{

  int flags,wd;	
  inotifyFd = inotify_init();
  if(inotifyFd == -1) return -1;
  
  //Establish Signal Handler
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = ip_sigio_handler;
  if(sigaction(SIGIO, &sa, NULL) == -1) return -1;
  
  if (fcntl(inotifyFd, F_SETOWN, getpid()) == -1) return -1;
  
  //Make IO non blocking here
  flags = fcntl(inotifyFd, F_GETFL);
  if (fcntl(inotifyFd, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1) return -1;
  wd = inotify_add_watch(inotifyFd,"ip/" , IN_CREATE|IN_MODIFY|IN_DELETE);
  if(wd == -1) return -1;

}

int build_ok_ip_list()
{
  log_dbg("Executing build_ok_ip_list");
  struct dirent *de;
  char *d_name;
  DIR *dr = opendir("ip/");
  if(dr == NULL) return -1;
  ip_count = 0;
  while ((de = readdir(dr)) != NULL) {
    ip_count +=1;	  
    ip_list = (char**)realloc(ip_list, ip_count*sizeof(char*));
    if(!ip_list) return -1;	    
    d_name = malloc(strlen(de->d_name)* sizeof(char));
    if(!d_name) return -1;
    strcpy(d_name,de->d_name);
    ip_list[ip_count-1] = d_name;
  }
}

int check_ip_exists(char *s)
{
  log_dbg("Executing check_ip_exists");
  int i;
  char *ip_addr = s+3;
  for(i = 0; i < ip_count; i++) {
    if(strcmp(ip_addr,ip_list[i]) ==0) return 0;
  } 
  return 1; 	  
}

int okclient(char ip[4])
{
  struct stat st;
  int i;
  fn[0] = 'i';
  fn[1] = 'p';
  fn[2] = '/';
  fn[3 + ip4_fmt(fn + 3,ip)] = 0;
  for (;;) {
    if (check_ip_exists(fn) == 0) return 1;
    /* treat temporary error as rejection */
    i = str_rchr(fn,'.');
    if (!fn[i]) return 0;
    fn[i] = 0;
  }
}
