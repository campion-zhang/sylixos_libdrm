#ifndef LIBDRM_LIBDRM_H
#define LIBDRM_LIBDRM_H

#if HAVE_VISIBILITY
#  define drm_private __attribute__((visibility("hidden")))
#  define drm_public  __attribute__((visibility("default")))
#else
#  define drm_private
#  define drm_public
#endif


/**
 * Static (compile-time) assertion.
 * Basically, use COND to dimension an array.  If COND is false/zero the
 * array size will be -1 and we'll get a compilation error.
 */
#define STATIC_ASSERT(COND) \
   do { \
      (void) sizeof(char [1 - 2*!(COND)]); \
   } while (0)


#include <sys/mman.h>

#if defined(ANDROID) && !defined(__LP64__)
#include <errno.h> /* for EINVAL */

static inline void *drm_mmap(void *addr, size_t length, int prot, int flags,
                             int fd, loff_t offset)
{
   /* offset must be aligned to 4096 (not necessarily the page size) */
   if (offset & 4095) {
      errno = EINVAL;
      return MAP_FAILED;
   }

   return mmap64(addr, length, prot, flags, fd, offset);
}

#  define drm_munmap(addr, length) \
              munmap(addr, length)


#else

/* assume large file support exists */
#  define drm_mmap(addr, length, prot, flags, fd, offset) \
              mmap(addr, length, prot, flags, fd, offset)


static inline int drm_munmap(void *addr, size_t length)
{
   /* Copied from configure code generated by AC_SYS_LARGEFILE */
#define LARGE_OFF_T ((((off_t) 1 << 31) << 31) - 1 + \
                     (((off_t) 1 << 31) << 31))
   STATIC_ASSERT(LARGE_OFF_T % 2147483629 == 721 &&
                 LARGE_OFF_T % 2147483647 == 1);
#undef LARGE_OFF_T

   return munmap(addr, length);
}
#endif

#endif