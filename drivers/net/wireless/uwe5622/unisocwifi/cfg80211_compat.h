/*
 * cfg80211 API compatibility for kernel 5.15 - 7.1+
 * Kernel 7.1 changed function signatures: net_device* → wireless_dev*
 */

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0)

/* Get net_device from wireless_dev wrapper */
static inline struct net_device *wdev_to_ndev(struct wireless_dev *wdev)
{
	return wdev ? wdev->netdev : NULL;
}

#endif /* KERNEL_VERSION(7, 1, 0) */
