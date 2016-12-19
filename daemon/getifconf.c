#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/ioctl.h>
#include <net/if.h>
#include <linux/in.h>

#include "statistic.h"

int getifconf(char *intf, struct ifparam *ifp, int mode)
{
	int fd;
	struct sockaddr_in s;
	struct ifreq ifr;

	memset((void *)&ifr, 0, sizeof(struct ifreq));
	if((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0)	return (-1);

	sprintf(ifr.ifr_name,"%s",intf);

	if(mode){
		if(ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
			perror("ioctl SIOCGIFADDR");
			return -1;
		}
		memset((void *)&s, 0, sizeof(struct sockaddr_in));
		memcpy((void *)&s, (void *)&ifr.ifr_addr, sizeof(struct sockaddr));
		memcpy((void *)&ifp->ip, (void *)&s.sin_addr.s_addr, sizeof(__u32));

		if(ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) {
			perror("ioctl SIOCGIFNETMASK");
			return -1;
		}
		memset((void *)&s, 0, sizeof(struct sockaddr_in));
		memcpy((void *)&s, (void *)&ifr.ifr_netmask, sizeof(struct sockaddr));
		memcpy((void *)&ifp->mask, (void *)&s.sin_addr.s_addr, sizeof(u_long));

		if(ioctl(fd, SIOCGIFMTU, &ifr) < 0) {
			perror("ioctl SIOCGIFMTU");
			return -1;
		}
		ifp->mtu = ifr.ifr_mtu;

		if(ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
			perror("ioctl SIOCGIFINDEX");
			return -1;
		}
		ifp->index = ifr.ifr_ifindex;
	}

	if(ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
		perror("ioctl SIOCGIFFLAGS");
		close(fd);
		return -1;
	}

	if(mode) ifr.ifr_flags |= IFF_PROMISC;
	else ifr.ifr_flags &= ~(IFF_PROMISC);

	if(ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
		perror("ioctl SIOCSIFFLAGS");
		close(fd);
		return (-1);
	}

	return 0;
}