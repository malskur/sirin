#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define	START			1
#define	STOP			2
#define	SELECT_IFACE	3
#define	SHOW_IP_COUNT	4
#define	STAT_IFACE		5

char *value;

int parse(char *str){
	if(strcmp(str, "start")==0)
		return START;
	
	if(strcmp(str, "stop")==0)
		return STOP;
	
	if(strncmp(str, "select iface ", 13)==0){
		value = strtok(str+13, " ");
		return SELECT_IFACE;
	}
	
	if(strncmp(str, "show ", 5)==0){
		value = strtok(str+4, " ");
		return SHOW_IP_COUNT;
	}
		
	if(strncmp(str, "stat", 4)==0){
		if(strlen(str)>5)
			value = strtok(str+4, " ");
		else
			strcpy(value, "all");
		return STAT_IFACE;
	}
	
	return 0;
}

int get_ip_directory(void)
{
	char *pch;
	pch=strchr(value,'.');
	while (pch!=NULL)
	{
		strncpy(pch,"/",1);
		pch=strchr(pch+1,'.');
	}
	return 0;
}

void show_ip(){
	char buf[64];
	sprintf(buf, "cat /tmp/ifaces/%s/cnt", value);
	system(buf);
}

void show_directory(){
	char buf[64];
	sprintf(buf, "cat /tmp/ifaces/%s", value);
	system(buf);
}

void print_help(void){
	printf("\nusage information:\nstart\t(packets are being sniffed from now on from default iface(eth0))\n\
stop\t(packets are not sniffed)\n\
show [ip] count\t(print number of packets received from ip address)\n\
select iface [iface]\t(select interface for sniffing eth0, wlan0, ethN, wlanN...)\n\
stat [iface]\tshow all collected statistics for particular interface, if iface omitted - for all interfaces.\n\
--help\t(show usage information)\n");
}

int send_command(char *cmd){
	char buf[64];
	sprintf(buf, "echo '%s' > /tmp/pipe",cmd);
	system(buf);
}

int main(void){
	
	char command[32];
	
	value = malloc(sizeof(char));

	while(1){
		gets(command);
		
		switch(parse(command)){
			
			case(START)://start
				send_command(command);
				break;
				
			case(STOP)://stop
				send_command(command);
				break;
				
			case(SHOW_IP_COUNT)://show [ip] count
				get_ip_directory();
				show_ip();
				break;
				
			case(SELECT_IFACE)://select iface [iface]
				send_command(value);
				break;
				
			case(STAT_IFACE)://stat [iface]
				show_directory();
				break;
				
			default:
				print_help();
				break;
		}
	}
	
	return 0;
}