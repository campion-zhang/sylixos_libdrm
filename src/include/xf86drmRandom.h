#ifndef __XF86DRM_RANDOM_H__
#define __XF86DRM_RANDOM_H__

typedef struct RandomState {
    unsigned long magic;
    unsigned long a;
    unsigned long m;
    unsigned long q;		/* m div a */
    unsigned long r;		/* m mod a */
    unsigned long check;
    unsigned long seed;
} RandomState;

#endif
