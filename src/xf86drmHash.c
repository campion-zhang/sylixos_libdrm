#include <stdio.h>
#include <stdlib.h>

#include "libdrm_macros.h"
#include "xf86drm.h"
#include "xf86drmHash.h"

#define HASH_MAGIC 0xdeadbeef

static unsigned long HashHash(unsigned long key)
{
    unsigned long        hash = 0;
    unsigned long        tmp  = key;
    static int           init = 0;
    static unsigned long scatter[256];
    int                  i;

    if (!init) {
	void *state;
	state = drmRandomCreate(37);
	for (i = 0; i < 256; i++) scatter[i] = drmRandom(state);
	drmRandomDestroy(state);
	++init;
    }

    while (tmp) {
	hash = (hash << 1) + scatter[tmp & 0xff];
	tmp >>= 8;
    }

    hash %= HASH_SIZE;
    return hash;
}

drm_public void *drmHashCreate(void)
{
    HashTablePtr table;

    table           = drmMalloc(sizeof(*table));
    if (!table) return NULL;
    table->magic    = HASH_MAGIC;

    return table;
}

drm_public int drmHashDestroy(void *t)
{
    HashTablePtr  table = (HashTablePtr)t;
    HashBucketPtr bucket;
    HashBucketPtr next;
    int           i;

    if (table->magic != HASH_MAGIC) return -1; /* Bad magic */

    for (i = 0; i < HASH_SIZE; i++) {
	for (bucket = table->buckets[i]; bucket;) {
	    next = bucket->next;
	    drmFree(bucket);
	    bucket = next;
	}
    }
    drmFree(table);
    return 0;
}

/* Find the bucket and organize the list so that this bucket is at the
   top. */

static HashBucketPtr HashFind(HashTablePtr table,
			      unsigned long key, unsigned long *h)
{
    unsigned long hash = HashHash(key);
    HashBucketPtr prev = NULL;
    HashBucketPtr bucket;

    if (h) *h = hash;

    for (bucket = table->buckets[hash]; bucket; bucket = bucket->next) {
	if (bucket->key == key) {
	    if (prev) {
				/* Organize */
		prev->next           = bucket->next;
		bucket->next         = table->buckets[hash];
		table->buckets[hash] = bucket;
		++table->partials;
	    } else {
		++table->hits;
	    }
	    return bucket;
	}
	prev = bucket;
    }
    ++table->misses;
    return NULL;
}

drm_public int drmHashLookup(void *t, unsigned long key, void **value)
{
    HashTablePtr  table = (HashTablePtr)t;
    HashBucketPtr bucket;

    if (!table || table->magic != HASH_MAGIC) return -1; /* Bad magic */

    bucket = HashFind(table, key, NULL);
    if (!bucket) return 1;	/* Not found */
    *value = bucket->value;
    return 0;			/* Found */
}

drm_public int drmHashInsert(void *t, unsigned long key, void *value)
{
    HashTablePtr  table = (HashTablePtr)t;
    HashBucketPtr bucket;
    unsigned long hash;

    if (table->magic != HASH_MAGIC) return -1; /* Bad magic */

    if (HashFind(table, key, &hash)) return 1; /* Already in table */

    bucket               = drmMalloc(sizeof(*bucket));
    if (!bucket) return -1;	/* Error */
    bucket->key          = key;
    bucket->value        = value;
    bucket->next         = table->buckets[hash];
    table->buckets[hash] = bucket;
    return 0;			/* Added to table */
}

drm_public int drmHashDelete(void *t, unsigned long key)
{
    HashTablePtr  table = (HashTablePtr)t;
    unsigned long hash;
    HashBucketPtr bucket;

    if (table->magic != HASH_MAGIC) return -1; /* Bad magic */

    bucket = HashFind(table, key, &hash);

    if (!bucket) return 1;	/* Not found */

    table->buckets[hash] = bucket->next;
    drmFree(bucket);
    return 0;
}

drm_public int drmHashNext(void *t, unsigned long *key, void **value)
{
    HashTablePtr  table = (HashTablePtr)t;

    while (table->p0 < HASH_SIZE) {
	if (table->p1) {
	    *key       = table->p1->key;
	    *value     = table->p1->value;
	    table->p1  = table->p1->next;
	    return 1;
	}
	table->p1 = table->buckets[table->p0];
	++table->p0;
    }
    return 0;
}

drm_public int drmHashFirst(void *t, unsigned long *key, void **value)
{
    HashTablePtr  table = (HashTablePtr)t;

    if (table->magic != HASH_MAGIC) return -1; /* Bad magic */

    table->p0 = 0;
    table->p1 = table->buckets[0];
    return drmHashNext(table, key, value);
}
