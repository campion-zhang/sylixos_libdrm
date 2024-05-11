#ifndef INTERNAL_H_
#define INTERNAL_H_

#include "libdrm_macros.h"
#include "libkms.h"

struct kms_driver
{
	int (*get_prop)(struct kms_driver *kms, const unsigned key,
			unsigned *out);
	int (*destroy)(struct kms_driver *kms);

	int (*bo_create)(struct kms_driver *kms,
			 unsigned width,
			 unsigned height,
			 enum kms_bo_type type,
			 const unsigned *attr,
			 struct kms_bo **out);
	int (*bo_get_prop)(struct kms_bo *bo, const unsigned key,
			   unsigned *out);
	int (*bo_map)(struct kms_bo *bo, void **out);
	int (*bo_unmap)(struct kms_bo *bo);
	int (*bo_destroy)(struct kms_bo *bo);

	int fd;
};

struct kms_bo
{
	struct kms_driver *kms;
	void *ptr;
	size_t size;
	size_t offset;
	size_t pitch;
	unsigned handle;
};

drm_private int linux_create(int fd, struct kms_driver **out);

drm_private int vmwgfx_create(int fd, struct kms_driver **out);

drm_private int intel_create(int fd, struct kms_driver **out);

drm_private int dumb_create(int fd, struct kms_driver **out);

drm_private int nouveau_create(int fd, struct kms_driver **out);

drm_private int radeon_create(int fd, struct kms_driver **out);

drm_private int exynos_create(int fd, struct kms_driver **out);

#endif
