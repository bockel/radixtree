
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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define NODE_INIT_SIZE 6
#define MAX_ALPHABET_SIZE 128
#define MAX_KEY_LENGTH 128
#define WILDCARD_CHAR '*'

typedef struct _rt_tree rt_tree;
typedef struct _rt_iter rt_iter;

rt_tree * rt_tree_new(	uint8_t albet_size,
		void (*_vfree)(void*));

rt_tree * rt_tree_custom(	uint8_t albet_size,
		void* (*_malloc)(size_t),
		void* (*_realloc)(void *,size_t),
		void (*_free)(void*),
		void (*_vfree)(void*));

void rt_tree_free(rt_tree *t);

int rt_tree_get(const rt_tree *t, const char *key, size_t lkey, void ** value);
int rt_tree_set(const rt_tree *t, const char *key, size_t lkey, void *value);
void rt_tree_print(const rt_tree *t);

rt_iter *rt_tree_find(const rt_tree *t, const char *key, size_t lkey);
int rt_iter_next(rt_iter *iter);
const char *rt_iter_key(const rt_iter *iter);
const void *rt_iter_value(const rt_iter *iter);

#ifdef __cplusplus
}
#endif
