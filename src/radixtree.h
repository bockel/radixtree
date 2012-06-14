
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

#ifndef ALPHABET
#define ALPHABET "01234567890_abcdefghijklmnopqrstuvwxyz"
#endif

#define ALPHABET_SIZE (strlen(ALPHABET))
#define NODE_LEAF_MAX  ALPHABET_SIZE

typedef struct _rt_tree rt_tree;

rt_tree * rt_tree_new(	void* (*_malloc)(size_t),
		void* (*_realloc)(void *,size_t),
		void (*_free)(void*),
		void (*_vfree)(void*));
void rt_tree_free(rt_tree *t);

int rt_tree_find(const rt_tree *t, const char *key, uint8_t lkey, void ** value);
int rt_tree_add(const rt_tree *t, const char *key, uint8_t lkey, void *value);
void rt_tree_print(const rt_tree *t);

#ifdef __cplusplus
}
#endif
