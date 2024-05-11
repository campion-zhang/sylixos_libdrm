#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
//#include <sys/sysmacros.h>
#endif

#include "xf86drm.h"
#include "libdrm_macros.h"
#include "internal.h"

#define PATH_SIZE 512

static int
linux_name_from_sysfs(int fd, char **out)
{
#if 0
	char path[PATH_SIZE+1] = ""; /* initialize to please valgrind */
	char link[PATH_SIZE+1] = "";
	struct stat buffer;
	unsigned maj, min;
	char* slash_name;
	int ret;

	/* 
	 * Inside the sysfs directory for the device there is a symlink
	 * to the directory representing the driver module, that path
	 * happens to hold the name of the driver.
	 *
	 * So lets get the symlink for the drm device. Then read the link
	 * and filter out the last directory which happens to be the name
	 * of the driver, which we can use to load the correct interface.
	 *
	 * Thanks to Ray Strode of Plymouth for the code.
	 */

	ret = fstat(fd, &buffer);
	if (ret)
		return -EINVAL;

	if (!S_ISCHR(buffer.st_mode))
		return -EINVAL;

	maj = major(buffer.st_rdev);
	min = minor(buffer.st_rdev);

	snprintf(path, PATH_SIZE, "/sys/dev/char/%d:%d/device/driver", maj, min);

	if (readlink(path, link, PATH_SIZE) < 0)
		return -EINVAL;

	/* link looks something like this: ../../../bus/pci/drivers/intel */
	slash_name = strrchr(link, '/');
	if (!slash_name)
		return -EINVAL;

	/* copy name and at the same time remove the slash */
	*out = strdup(slash_name + 1);
#endif
//  /sys/dev/char/226:0/device/driver$
//  /sys/bus/pci/drivers/gb

	return -EINVAL;
}

static int
linux_from_sysfs(int fd, struct kms_driver **out)
{
	char *name;
	int ret;

	ret = linux_name_from_sysfs(fd, &name);
	if (ret)
		return ret;

	ret = -ENOSYS;

	free(name);
	return ret;
}

drm_private int
linux_create(int fd, struct kms_driver **out)
{
	if (!dumb_create(fd, out))
		return 0;

	return linux_from_sysfs(fd, out);
}
