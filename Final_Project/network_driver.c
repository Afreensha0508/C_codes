#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#define DRV_NAME "dummy_net"
static struct net_device *dummy_netdev; // Represents a network interface


// Function is transmit handler for network device. 
static netdev_tx_t dummy_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    pr_info("%s: Simulated TX packet, len = %u\n", DRV_NAME, skb->len);
    dev_kfree_skb(skb);  //Releases socket buffer back to kernel after a packet is transmitted 
    dev->stats.tx_packets++;
    dev->stats.tx_bytes += skb->len;
    return NETDEV_TX_OK;
}


// Function is called when network interface is up
static int dummy_open(struct net_device *dev)
{
    pr_info("%s: device opened\n", DRV_NAME);
    netif_start_queue(dev);  // Tells networking stack that device's transmit queue is ready to accept packets
    return 0;
}


// Function is called when network interface is down
static int dummy_stop(struct net_device *dev)
{
    pr_info("%s: device stopped\n", DRV_NAME);
    netif_stop_queue(dev);  // Tells networking queue to stop sending packets to device's transmit queue
    return 0;
}

// Collection of function pointers that define core network operations
static const struct net_device_ops dummy_netdev_ops = {
    .ndo_open       = dummy_open,
    .ndo_stop       = dummy_stop,
    .ndo_start_xmit = dummy_start_xmit,
};


// Call back function used to allocate net_device structure
static void dummy_setup(struct net_device *dev)
{
    ether_setup(dev);
    dev->netdev_ops = &dummy_netdev_ops;
    dev->flags |= IFF_NOARP;
    dev->features |= NETIF_F_HW_CSUM;
    eth_hw_addr_random(dev);  // Assign random MAC
}

// Module Initialisation function 
static int __init dummy_init(void)
{
    dummy_netdev = alloc_netdev(0, "dummy%d", NET_NAME_UNKNOWN, dummy_setup); // Allocates new net device
    if (!dummy_netdev)
        return -ENOMEM;
    if (register_netdev(dummy_netdev)) //Registers network device with kernel's networking sub system
    {
        free_netdev(dummy_netdev);
        return -ENODEV;
    }
    pr_info("%s: loaded\n", DRV_NAME);
    return 0;
}


// Module De-initialisation function
static void __exit dummy_exit(void)
{
    unregister_netdev(dummy_netdev); // De-register the network device 
    free_netdev(dummy_netdev);    // Free the network device 
    pr_info("%s: unloaded\n", DRV_NAME);
}

module_init(dummy_init);
module_exit(dummy_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Batch5");
MODULE_DESCRIPTION("Minimal Dummy Network Driver");

