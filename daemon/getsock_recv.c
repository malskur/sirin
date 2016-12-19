
#include <linux/socket.h>
#include <linux/types.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#ifndef PF_PACKET
#define PF_PACKET       17 
#endif
#ifndef SOCK_RAW
#define SOCK_RAW		3
#endif

int getsock_recv(int index)
{
    int sd; 
    struct sockaddr_ll s_ll;

    sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(sd < 0) return -1;

    memset((void *)&s_ll, 0, sizeof(struct sockaddr_ll));

    s_ll.sll_family = PF_PACKET;
    s_ll.sll_protocol = htons(ETH_P_ALL);
    s_ll.sll_ifindex = index;

    if(bind(sd, (struct sockaddr *)&s_ll, sizeof(struct sockaddr_ll)) < 0) {
		close(sd);
		return -1;
    }

    return sd;
}

