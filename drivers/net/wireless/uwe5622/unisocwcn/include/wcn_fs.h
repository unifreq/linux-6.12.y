#ifndef __WCN_FS_H__
#define __WCN_FS_H__

#include <linux/fs.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(7, 3, 0)
#include <linux/fs_struct.h>
#endif

/*
 * Since 7.3, kthreads no longer share fs_struct with init. Their root is
 * the initial rootfs, so absolute paths such as /lib/firmware fail.
 * Resolve paths against the init root, as kernel_read_file does.
 */
static inline struct file *wcn_filp_open(const char *name, int flags,
					 umode_t mode)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(7, 3, 0)
	struct file *file;

	scoped_with_init_fs()
		file = filp_open(name, flags, mode);

	return file;
#else
	return filp_open(name, flags, mode);
#endif
}

#endif
