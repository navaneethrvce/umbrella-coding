#ifndef OKCLIENT_H
#define OKCLIENT_H


#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

extern int okclient(char *);
extern int watch_ip_list();
extern int build_ok_ip_list();

#endif
