#ifndef _LIBSYNC_H
#define _LIBSYNC_H

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <unistd.h>

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef SYNC_IOC_MERGE
/* duplicated from linux/sync_file.h to avoid build-time dependency
 * on new (v4.7) kernel headers.  Once distro's are mostly using
 * something newer than v4.7 drop this and #include <linux/sync_file.h>
 * instead.
 */
struct sync_merge_data {
	char	name[32];
	int32_t	fd2;
	int32_t	fence;
	uint32_t	flags;
	uint32_t	pad;
};
#define SYNC_IOC_MAGIC		'>'
#define SYNC_IOC_MERGE		_IOWR(SYNC_IOC_MAGIC, 3, struct sync_merge_data)
#endif


static inline int sync_wait(int fd, int timeout)
{
	struct pollfd fds = {0};
	int ret;

	fds.fd = fd;
	fds.events = POLLIN;

	do {
		ret = poll(&fds, 1, timeout);
		if (ret > 0) {
			if (fds.revents & (POLLERR | POLLNVAL)) {
				errno = EINVAL;
				return -1;
			}
			return 0;
		} else if (ret == 0) {
			errno = ETIME;
			return -1;
		}
	} while (ret == -1 && (errno == EINTR || errno == EAGAIN));

	return ret;
}

static inline int sync_merge(const char *name, int fd1, int fd2)
{
	struct sync_merge_data data = {0};
	int ret;

	data.fd2 = fd2;
	strncpy(data.name, name, sizeof(data.name));

	do {
		ret = ioctl(fd1, SYNC_IOC_MERGE, &data);
	} while (ret == -1 && (errno == EINTR || errno == EAGAIN));

	if (ret < 0)
		return ret;

	return data.fence;
}

/* accumulate fd2 into fd1.  If *fd1 is not a valid fd then dup fd2,
 * otherwise sync_merge() and close the old *fd1.  This can be used
 * to implement the pattern:
 *
 *    init()
 *    {
 *       batch.fence_fd = -1;
 *    }
 *
 *    // does *NOT* take ownership of fd
 *    server_sync(int fd)
 *    {
 *       if (sync_accumulate("foo", &batch.fence_fd, fd)) {
 *          ... error ...
 *       }
 *    }
 */
static inline int sync_accumulate(const char *name, int *fd1, int fd2)
{
	int ret;

	assert(fd2 >= 0);

	if (*fd1 < 0) {
		*fd1 = dup(fd2);
		return 0;
	}

	ret = sync_merge(name, *fd1, fd2);
	if (ret < 0) {
		/* leave *fd1 as it is */
		return ret;
	}

	close(*fd1);
	*fd1 = ret;

	return 0;
}

#if defined(__cplusplus)
}
#endif

#endif
