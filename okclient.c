#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "str.h"
#include "ip4.h"
#include "okclient.h"
#include "log.h"

static char fn[3 + IP4_FMT];
static char **ip_list;
static int ip_count;

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
    d_name = malloc(strlen(de->d_name)* sizeof(char));
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
