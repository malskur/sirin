#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/agpgart.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "statistic.h"

#define NAMEDPIPE_NAME "/tmp/pipe"
#define BUFSIZE        8

__u8 buff[ETH_FRAME_LEN];
char iface_name[16] = "eth0";
bool start = FALSE;
bool got_command = FALSE;


void mode_off()
{
	if(getifconf(iface_name, &ifp, PROMISC_MODE_OFF) < 0) {
		perror("getifconf");
		exit(EXIT_FAILURE);
	}

	return;
}

void set_iface(char *new_iface){
	strcpy(iface_name, new_iface);
	
	return;
}

void parse(char *str){
	if(strncmp(str, "start", 5)==0) {
		start = TRUE;
	}
	else if(strncmp(str, "stop", 4)==0){
		start = FALSE;
		got_command = TRUE;
	}
	else {
		set_iface(str);
		got_command = TRUE;
	}
	
	return;
}

int pipe_thread(void){
	FILE *fd;
    char buf[BUFSIZE];
    if ( mkfifo(NAMEDPIPE_NAME, 0777) ) {
        perror("mkfifo");
        return 1;
    }

    while(1) {
	    if ( (fd = fopen(NAMEDPIPE_NAME, "r")) <= 0 ) {
			perror("open");
			return 1;
		}
        if ( (fgets(buf, BUFSIZE, fd)) <= 0 ) {
            perror("read");
            close(fd);
            fclose(fd);
            return 1;
        }
		parse(buf);	
    }
}

void inc_count_ifaces_all(void){
	FILE *fd;
	int cnt=0;
	if((fd = fopen("/tmp/ifaces/all", "r"))<=0){
		system("echo '0' > /tmp/ifaces/all");
		fd = fopen("/tmp/ifaces/all", "r");
	}
	fscanf(fd, "%i", &cnt);
	fclose(fd);
	cnt++;
	fd = fopen("/tmp/ifaces/all", "w");
	fprintf(fd, "%i", cnt);
	fclose(fd);
	return;
}

void inc_count_iface(void){
	FILE *fd;
	int cnt=0;
	char buf[32];
	sprintf(buf, "/tmp/ifaces/%s", iface_name);
	if((fd = fopen(buf, "r"))<=0){
		fd = fopen(buf, "a");
		fclose(fd);
		fd = fopen(buf, "r");
	}
	fscanf(fd, "%i", &cnt);
	fclose(fd);
	cnt++;
	fd = fopen(buf, "w");
	fprintf(fd, "%i", cnt);
	fclose(fd);
	return;
}

void check_dir(char *dir){
	char *buf;
	char dir_chk[32];
	DIR* d;
	mode_t mode = 0777 | S_ISVTX;
	
	strcpy(dir_chk,"/tmp/ifaces/");
	
	buf = strtok(dir, "/");
	
	while (buf!=NULL)
	{
		strcat(dir_chk,buf);
		strcat(dir_chk,"/");	
	
		if(!(d = opendir(dir_chk)))
		{
			if (mkdir (dir_chk, mode) == -1) {
				fprintf (stderr, "mkdir() error\n");
				return 0;
			}
		}
		else
			closedir(d);
		
		buf = strtok(NULL, "/");
	}
	
	return;
}

int get_ip_directory(char *ip)
{
	char *pch;
	
	pch=strchr(ip,'.');
	while (pch!=NULL)
	{
		strncpy(pch,"/",1);
		pch=strchr(pch+1,'.');
	}
	
	return 0;
}

void inc_count_ip(char *ip_source){
	FILE *fd;
	int cnt=0;
	char buf[32];
	get_ip_directory(ip_source);
	sprintf(buf, "%s", ip_source);
	check_dir(buf);
	sprintf(buf, "/tmp/ifaces/%s/cnt", ip_source);
	if((fd = fopen(buf, "r"))<=0){
		fd = fopen(buf, "a");
		fclose(fd);
		fd = fopen(buf, "r");
	}
	fscanf(fd, "%i", &cnt);
	fclose(fd);
	cnt++;
	fd = fopen(buf, "w");
	fprintf(fd, "%i", cnt);
	fclose(fd);
	return;
}


int main()
{	
	int iface, rec = 0, ihl = 0;
	struct iphdr ip;
	struct tcphdr tcp;
	struct ethhdr eth;
	static struct sigaction act;
	pthread_t thread_id;
	pid_t pid, sid;

	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	if (pid > 0) {
		printf("pid %i\n", pid);
		exit(EXIT_SUCCESS);
	}

	umask(0);
	
	sid = setsid();
	if (sid < 0) {
		exit(EXIT_FAILURE);
	}

	if ((chdir("/")) < 0) {
		exit(EXIT_FAILURE);
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	
	pthread_create(&thread_id, NULL, &pipe_thread, NULL);
	
	system("rm -r /tmp/ifaces/");
	system("mkdir /tmp/ifaces/");
	
	while(1){

		while(!start){}
			
		if(getifconf(iface_name, &ifp, PROMISC_MODE_ON) < 0) {
			perror("getifconf");
			return -1;
		}

		if((iface = getsock_recv(ifp.index)) < 0) {
			perror("getsock_recv");
			return -1;
		}

		act.sa_handler = mode_off;
		sigfillset(&(act.sa_mask));
		sigaction(SIGINT, &act, NULL);

		got_command = FALSE;

		while(1) {
			memset(buff, 0, ETH_FRAME_LEN);
			rec = recvfrom(iface, (char *)buff, ifp.mtu + 18, 0, NULL, NULL);
			if(rec < 0 || rec > ETH_FRAME_LEN) {
				perror("recvfrom");
				return -1;
			}

			memcpy((void *)&eth, buff, ETH_HLEN);
			memcpy((void *)&ip, buff + ETH_HLEN, sizeof(struct iphdr));
			if((ip.version) != 4) continue;
			memcpy((void *)&tcp, buff + ETH_HLEN + ip.ihl * 4, sizeof(struct tcphdr));
			
			if(ip.protocol == IPPROTO_TCP) {
				inc_count_ip(inet_ntoa(ip.saddr));
			}
			inc_count_ifaces_all();
			inc_count_iface();

			if(got_command)
				break;
		}
	}
	return 0;
}