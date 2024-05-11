#include <SylixOS.h>
#include <stdlib.h>
#include <string.h>

#include "libdrm_macros.h"
#include "internal.h"

drm_public int kms_create(int fd, struct kms_driver **out)
{
	return linux_create(fd, out);
}

drm_public int kms_get_prop(struct kms_driver *kms, unsigned key, unsigned *out)
{
	switch (key) {
	case KMS_BO_TYPE:
		break;
	default:
		return -EINVAL;
	}
	return kms->get_prop(kms, key, out);
}

drm_public int kms_destroy(struct kms_driver **kms)
{
	if (!(*kms))
		return 0;

	free(*kms);
	*kms = NULL;
	return 0;
}

drm_public int kms_bo_create(struct kms_driver *kms, const unsigned *attr, struct kms_bo **out)
{
	unsigned width = 0;
	unsigned height = 0;
	enum kms_bo_type type = KMS_BO_TYPE_SCANOUT_X8R8G8B8;
	int i;

	for (i = 0; attr[i];) {
		unsigned key = attr[i++];
		unsigned value = attr[i++];

		switch (key) {
		case KMS_WIDTH:
			width = value;
			break;
		case KMS_HEIGHT:
			height = value;
			break;
		case KMS_BO_TYPE:
			type = value;
			break;
		default:
			return -EINVAL;
		}
	}

	if (width == 0 || height == 0)
		return -EINVAL;

	/* XXX sanity check type */

	if (type == KMS_BO_TYPE_CURSOR_64X64_A8R8G8B8 &&
	    (width != 64 || height != 64))
		return -EINVAL;

	return kms->bo_create(kms, width, height, type, attr, out);
}

drm_public int kms_bo_get_prop(struct kms_bo *bo, unsigned key, unsigned *out)
{
	switch (key) {
	case KMS_PITCH:
		*out = bo->pitch;
		break;
	case KMS_HANDLE:
		*out = bo->handle;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

drm_public int kms_bo_map(struct kms_bo *bo, void **out)
{
	return bo->kms->bo_map(bo, out);
}

drm_public int kms_bo_unmap(struct kms_bo *bo)
{
	return bo->kms->bo_unmap(bo);
}

drm_public int kms_bo_destroy(struct kms_bo **bo)
{
	int ret;

	if (!(*bo))
		return 0;

	ret = (*bo)->kms->bo_destroy(*bo);
	if (ret)
		return ret;

	*bo = NULL;
	return 0;
}
