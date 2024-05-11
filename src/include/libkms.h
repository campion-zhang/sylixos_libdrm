#ifndef _LIBKMS_H_
#define _LIBKMS_H_

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * \file
 *
 */

struct kms_driver;
struct kms_bo;

enum kms_attrib
{
	KMS_TERMINATE_PROP_LIST,
#define KMS_TERMINATE_PROP_LIST KMS_TERMINATE_PROP_LIST
	KMS_BO_TYPE,
#define KMS_BO_TYPE KMS_BO_TYPE
	KMS_WIDTH,
#define KMS_WIDTH KMS_WIDTH
	KMS_HEIGHT,
#define KMS_HEIGHT KMS_HEIGHT
	KMS_PITCH,
#define KMS_PITCH KMS_PITCH
	KMS_HANDLE,
#define KMS_HANDLE KMS_HANDLE
};

enum kms_bo_type
{
	KMS_BO_TYPE_SCANOUT_X8R8G8B8 = (1 << 0),
#define KMS_BO_TYPE_SCANOUT_X8R8G8B8 KMS_BO_TYPE_SCANOUT_X8R8G8B8
	KMS_BO_TYPE_CURSOR_64X64_A8R8G8B8 =  (1 << 1),
#define KMS_BO_TYPE_CURSOR_64X64_A8R8G8B8 KMS_BO_TYPE_CURSOR_64X64_A8R8G8B8
};

int kms_create(int fd, struct kms_driver **out);
int kms_get_prop(struct kms_driver *kms, unsigned key, unsigned *out);
int kms_destroy(struct kms_driver **kms);

int kms_bo_create(struct kms_driver *kms, const unsigned *attr, struct kms_bo **out);
int kms_bo_get_prop(struct kms_bo *bo, unsigned key, unsigned *out);
int kms_bo_map(struct kms_bo *bo, void **out);
int kms_bo_unmap(struct kms_bo *bo);
int kms_bo_destroy(struct kms_bo **bo);

#if defined(__cplusplus)
};
#endif

#endif
