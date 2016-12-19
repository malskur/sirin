
#include <linux/types.h>

#define PROMISC_MODE_ON 1
#define PROMISC_MODE_OFF 0

struct ifparam {
    __u32 ip;
    __u32 mask;
    int mtu;
    int index;
} ifp;
