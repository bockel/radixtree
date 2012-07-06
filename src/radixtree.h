
/*
 * Copyright 2012 William Heinbockel
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file radixtree.h
 * @brief The exposed radixtree functions
 *
 * @todo map functionality (run method on every node)
 * @todo support multiple wildcards in search
 * @todo figure out how to handle key validation/modification -- keys with "illegal" chars; key.lower(); key.upper()
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/**
 * @def NODE_INIT_SIZE
 * The number of default leafs a rt_tree node contains
 */
#define NODE_INIT_SIZE 4

/**
 * @def MAX_ALPHABET_SIZE
 *
 * The maximum alphabet size of the rt_tree node keys.
 * This is used to place an upper bound on the node leaf size
 */
#define MAX_ALPHABET_SIZE 128

/**
 * @def MAX_KEY_LENGTH
 *
 * The maximum key length for a single key.
 * This bounds the rt_tree depth and search space.
 */
#define MAX_KEY_LENGTH 128

typedef struct _rt_tree rt_tree;
typedef struct _rt_iter rt_iter;

rt_tree * rt_tree_new(
        uint8_t albet_size,
        void (*_vfree)(void*));

rt_tree * rt_tree_custom(
        uint8_t albet_size,
        void (*_vfree)(void*),
        void* (*_malloc)(size_t),
        void* (*_realloc)(void *,size_t),
        void (*_free)(void*));

void rt_tree_free(rt_tree *t);

void * rt_tree_get(
        const rt_tree *t,
        const unsigned char *key,
        size_t lkey);

int rt_tree_set(
        const rt_tree *t,
        const unsigned char *key,
        size_t lkey,
        void *value);

void * rt_tree_setdefault(
        const rt_tree *t,
        const unsigned char *key,
        size_t lkey,
        void *value);

/**
 * @def rt_tree_remove
 *
 * Removes the key @a key from radixtree @a t
 * @param t The radixtree to remove the key from
 * @param key The name of the key to remove
 * @param lkey The length of the key @a key
 *
 * @returns 1 if the key was successfully removed; 0 otherwise
 */
int rt_tree_remove(
        const rt_tree *t,
        const unsigned char *key,
        size_t lkey);

void rt_tree_print(const rt_tree *t);

rt_iter *rt_tree_prefix(
        const rt_tree *t,
        const unsigned char *prefix,
        size_t prefixlen);

int rt_iter_next(rt_iter *iter);

const unsigned char *rt_iter_key(const rt_iter *iter);

const void *rt_iter_value(const rt_iter *iter);

void rt_tree_map(
        rt_tree *tree,
        void *usr_ctxt,
        void (*mapfunc)(void *usr_ctxt,
            unsigned char *key,
            size_t klen,
            void *value));

#ifdef __cplusplus
}
#endif
