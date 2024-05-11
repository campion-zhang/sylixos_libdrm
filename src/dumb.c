#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"

#include <sys/ioctl.h>
#include "xf86drm.h"
#include "libdrm_macros.h"

struct dumb_bo
{
	struct kms_bo base;
	unsigned map_count;
};

static int
dumb_get_prop(struct kms_driver *kms, unsigned key, unsigned *out)
{
	switch (key) {
	case KMS_BO_TYPE:
		*out = KMS_BO_TYPE_SCANOUT_X8R8G8B8 | KMS_BO_TYPE_CURSOR_64X64_A8R8G8B8;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int
dumb_destroy(struct kms_driver *kms)
{
	free(kms);
	return 0;
}

static int
dumb_bo_create(struct kms_driver *kms,
		 const unsigned width, const unsigned height,
		 const enum kms_bo_type type, const unsigned *attr,
		 struct kms_bo **out)
{
	struct drm_mode_create_dumb arg;
	struct dumb_bo *bo;
	int i, ret;

	for (i = 0; attr[i]; i += 2) {
		switch (attr[i]) {
		case KMS_WIDTH:
		case KMS_HEIGHT:
			break;
		case KMS_BO_TYPE:
			break;
		default:
			return -EINVAL;
		}
	}

	bo = calloc(1, sizeof(*bo));
	if (!bo)
		return -ENOMEM;

	memset(&arg, 0, sizeof(arg));

	/* All BO_TYPE currently are 32bpp formats */
	arg.bpp = 32;
	arg.width = width;
	arg.height = height;

	ret = drmIoctl(kms->fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg);
	if (ret)
		goto err_free;

	bo->base.kms = kms;
	bo->base.handle = arg.handle;
	bo->base.size = arg.size;
	bo->base.pitch = arg.pitch;

	*out = &bo->base;

	return 0;

err_free:
	free(bo);
	return ret;
}

static int
dumb_bo_get_prop(struct kms_bo *bo, unsigned key, unsigned *out)
{
	switch (key) {
	default:
		return -EINVAL;
	}
}

static int
dumb_bo_map(struct kms_bo *_bo, void **out)
{
	struct dumb_bo *bo = (struct dumb_bo *)_bo;
	struct drm_mode_map_dumb arg;
	void *map = NULL;
	int ret;

	if (bo->base.ptr) {
		bo->map_count++;
		*out = bo->base.ptr;
		return 0;
	}

	memset(&arg, 0, sizeof(arg));
	arg.handle = bo->base.handle;

	ret = drmIoctl(bo->base.kms->fd, DRM_IOCTL_MODE_MAP_DUMB, &arg);
	if (ret)
		return ret;

	map = drm_mmap(0, bo->base.size, PROT_READ | PROT_WRITE, MAP_SHARED, bo->base.kms->fd, arg.offset);
	if (map == MAP_FAILED)
		return -errno;

	bo->base.ptr = map;
	bo->map_count++;
	*out = bo->base.ptr;

	return 0;
}

static int
dumb_bo_unmap(struct kms_bo *_bo)
{
	struct dumb_bo *bo = (struct dumb_bo *)_bo;
	bo->map_count--;
	return 0;
}

static int
dumb_bo_destroy(struct kms_bo *_bo)
{
	struct dumb_bo *bo = (struct dumb_bo *)_bo;
	struct drm_mode_destroy_dumb arg;
	int ret;

	if (bo->base.ptr) {
		/* XXX Sanity check map_count */
		drm_munmap(bo->base.ptr, bo->base.size);
		bo->base.ptr = NULL;
	}

	memset(&arg, 0, sizeof(arg));
	arg.handle = bo->base.handle;

	ret = drmIoctl(bo->base.kms->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);
	if (ret)
		return -errno;

	free(bo);
	return 0;
}

drm_private int
dumb_create(int fd, struct kms_driver **out)
{
	struct kms_driver *kms;
	int ret;
	uint64_t cap = 0;

	ret = drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &cap);
	if (ret || cap == 0)
		return -EINVAL;

	kms = calloc(1, sizeof(*kms));
	if (!kms)
		return -ENOMEM;

	kms->fd = fd;

	kms->bo_create = dumb_bo_create;
	kms->bo_map = dumb_bo_map;
	kms->bo_unmap = dumb_bo_unmap;
	kms->bo_get_prop = dumb_bo_get_prop;
	kms->bo_destroy = dumb_bo_destroy;
	kms->get_prop = dumb_get_prop;
	kms->destroy = dumb_destroy;
	*out = kms;

	return 0;
}
