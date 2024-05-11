#include <stdio.h>
#include <stdlib.h>

#include "libdrm_macros.h"
#include "xf86drm.h"
#include "xf86drmRandom.h"

#define RANDOM_MAGIC 0xfeedbeef

drm_public void *drmRandomCreate(unsigned long seed)
{
    RandomState  *state;

    state           = drmMalloc(sizeof(*state));
    if (!state) return NULL;
    state->magic    = RANDOM_MAGIC;
#if 0
				/* Park & Miller, October 1988 */
    state->a        = 16807;
    state->m        = 2147483647;
    state->check    = 1043618065; /* After 10000 iterations */
#else
				/* Park, Miller, and Stockmeyer, July 1993 */
    state->a        = 48271;
    state->m        = 2147483647;
    state->check    = 399268537; /* After 10000 iterations */
#endif
    state->q        = state->m / state->a;
    state->r        = state->m % state->a;

    state->seed     = seed;
				/* Check for illegal boundary conditions,
                                   and choose closest legal value. */
    if (state->seed <= 0)        state->seed = 1;
    if (state->seed >= state->m) state->seed = state->m - 1;

    return state;
}

drm_public int drmRandomDestroy(void *state)
{
    drmFree(state);
    return 0;
}

drm_public unsigned long drmRandom(void *state)
{
    RandomState   *s = (RandomState *)state;
    unsigned long hi;
    unsigned long lo;

    hi      = s->seed / s->q;
    lo      = s->seed % s->q;
    s->seed = s->a * lo - s->r * hi;
    if ((s->a * lo) <= (s->r * hi)) s->seed += s->m;

    return s->seed;
}

drm_public double drmRandomDouble(void *state)
{
    RandomState *s = (RandomState *)state;
    
    return (double)drmRandom(state)/(double)s->m;
}
