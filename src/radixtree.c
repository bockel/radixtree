
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "radixtree.h"

typedef struct _node rt_node;

struct _node {
	uint8_t klen;
	uint8_t lcnt;
	uint8_t lalloc;
	char *key;
	void *value;
	struct _node **leaf;
};

struct _rt_tree {
	void (*free)(void *);
	void (*vfree)(void *);
	void * (* malloc)(size_t);
	void * (* realloc)(void *,size_t);
	rt_node *root;
};

static void
rt_node_free(const rt_tree *t, rt_node *n)
{
	uint8_t i;
	rt_node **l;
	if(!n || !t) return;
	for(i=0,l=n->leaf;i<n->lcnt;i++,*l++)
		rt_node_free(t, *l);
	if(n->value && t->vfree) t->vfree(n->value);
	if(n->key) t->free(n->key);
	if(n->leaf) t->free(n->leaf);
	t->free(n);
}

static rt_node *
rt_node_new(const rt_tree *t, uint8_t c, const char *key, size_t keylen)
{
	rt_node *n = NULL;
	uint8_t s = c > 0 && c < NODE_LEAF_MAX ? c : NODE_INIT_SIZE;
	size_t sz = sizeof(*n);
	if(!t || !t->malloc) return NULL;
	/* n = malloc(sz); */
	n = t->malloc(sz);
	if(!n) return NULL;
	memset(n,0,sz);
	n->lalloc = s;
	n->klen = keylen;
	n->leaf = t->malloc(s*sizeof(n));
	if(!n->leaf) goto fail;
	if(key && keylen>0) {
		n->key = t->malloc(keylen+1);
		if(!n->key) goto fail;
		memcpy(n->key,key,keylen);
		n->key[keylen] = 0;
	}
	return n;
fail:
	rt_node_free(t,n);
	return NULL;
}

static void
rt_node_print(rt_node *n, int depth)
{
	int i;
	rt_node **l;
	for(i=0;i<depth;i++) printf("\t");
	if(n)
	{
		if(n->key) printf("\"%.*s\"",n->klen,n->key);
		else       printf("NULL");
		if(n->value) printf(" = \"%s\"\n",n->value);
		else         printf(" = NULL\n");
		for(i=0,l=n->leaf;i<n->lcnt;i++,*l++) rt_node_print(*l,depth+1);
	} else printf("NULL\n");
}

static size_t
_maxmatch(const char *s1, const char *s2, size_t len)
{
	register char *m1 = (char *)s1, *m2 = (char *)s2;
	char *me1 = m1+len, *me2 = m2+len;
	size_t ret;
	while(*m1 == *m2 && m1<me1 && m2<me2) {
		*m1++; *m2++;
	}
	ret = m1 - s1;
	return ret;
}

static int
_cmp_nodes(const void *v1, const void *v2)
{
	register const char *n1 = (const char *)v1;
	register const rt_node *n2 = *(const rt_node **)v2;
	return n1[0] - n2->key[0];
}

/*
 * Implement a custom binary search that returns the last search location.
 * This location is either a match or the location where the node should be inserted +-1
 */
static int
rt_bsearch(const char *key, const rt_node **leaf, size_t leafcnt, rt_node ***match)
{
	size_t left = 0, right = leafcnt, index;
	int cmp;
	while(left < right)
	{
		index = (right+left)/2;
		cmp = *key - leaf[index]->key[0];
		if(cmp < 0)
			right = index;
		else if (cmp > 0)
			left = index+1;
		else break;
	}
	*match = (rt_node **)&leaf[index];
	return cmp;
}

static size_t
rt_node_grow(const rt_tree *t, rt_node *n)
{
	size_t ns = n->lalloc;
	rt_node **rt;
	ns *= 2;
	if(ns>ALPHABET_SIZE) ns = ALPHABET_SIZE;
	if(t->realloc) {
		rt = t->realloc(n->leaf,ns*sizeof(rt));
		if(!rt) return 0;
	} else {
		rt = t->malloc(ns*sizeof(rt));
		if(!rt) return 0;
		memcpy(rt,n->leaf,n->lalloc);
		t->free(n->leaf);
	}
	n->lalloc = ns;
	n->leaf = rt;
	return ns;
}

static rt_node *
rt_node_find(	const rt_tree *root, rt_node *n,
		const char *key, size_t lkey, int add)
{
	rt_node *node = NULL, **p;
	int diff;
	/* printf("LOG: rt_node_find(node(%s),\"%.*s\",%d,%d)\n",n->key,lkey,key,lkey,add); */
	if(!root || !n || lkey < 1 || !key) return NULL;

	if(n->lcnt == 0) {
		if(!add) return NULL;
		node = rt_node_new(root,0,key,lkey);
		if(!node) return NULL;
		n->leaf[0] = node;
		n->lcnt++;
		return node;
	}

	/* Search for the key
	 * If not found, return the location for it to be added */
	diff = rt_bsearch(key,(const rt_node **)n->leaf,n->lcnt,&p);
	if(!index || !index) return NULL;
	if(diff==0) /* found (partial?) match */
	{
		rt_node *index = *p, *child;
		size_t mm = _maxmatch(index->key, key, index->klen < lkey ? index->klen : lkey);
		if(add) {
			if(n->lcnt >= n->lalloc && rt_node_grow(root,n)<1) return NULL;
			if(mm < index->klen) {
				rt_node **tmp;
				/* otherwise, split
				 * add child and update this node */
				child = rt_node_new(root,0,index->key+mm,index->klen-mm);
				if(!child) {
					/* failed to split and add child node */
					return NULL;
				}
				tmp = root->malloc(NODE_INIT_SIZE*sizeof(rt_node *));
				if(!tmp) return NULL;
				child->lalloc = index->lalloc;
				child->lcnt   = index->lcnt;
				child->leaf   = index->leaf;
				child->value  = index->value;
				index->klen    = mm;
				index->key[mm] = 0;
				index->value   = NULL;
				index->leaf    = tmp;
				index->lalloc  = NODE_INIT_SIZE;
				index->lcnt    = 1;
				index->leaf[0] = child;
			}
		}
		if(mm==lkey && index->klen==lkey) {
			return index;
		} else {
			return rt_node_find(root,index,key+mm,lkey-mm,add);
		}
	} else {
		if(!add) return NULL;
		if(n->lcnt >= n->lalloc && rt_node_grow(root,n)<1) return NULL;
		node = rt_node_new(root,0,key,lkey);
		if(!node) return NULL;
		if(diff > 0) {
			memmove(p+2,p+1,sizeof(p)*(n->lcnt-(p - n->leaf)-1));
			p[1] = node;
		} else {
			memmove(p+1,p,sizeof(p)*(n->lcnt-(p - n->leaf)));
			*p = node;
		}
		n->lcnt++;
	}

	return node;
}

rt_tree *
rt_tree_new(	void* (*_malloc)(size_t),
		void* (*_realloc)(void *,size_t),
		void (*_free)(void*),
		void (*_vfree)(void*))
{
	rt_tree *t = NULL;
	if(!_malloc || !_free) return NULL;
	t = _malloc(sizeof(rt_tree));
	if(!t) return NULL;
	t->malloc = _malloc;
	t->realloc = _realloc;
	t->free = _free;
	t->vfree = _vfree;
	t->root = rt_node_new(t,0,NULL,0);
	return t;
}

void
rt_tree_free(rt_tree *t)
{
	if(!t) return;
	rt_node_free(t,t->root);
	t->free(t);
}

int
rt_tree_find(const rt_tree *t, const char *key, uint8_t lkey, void ** value)
{
	rt_node *n;
	if(!t) {
		*value = NULL;
		return 0;
	}
	n = rt_node_find(t,t->root,key,lkey,0);
	if(n) {
		*value = n->value;
		return 1;
	} else {
		*value = NULL;
		return 0;
	}
}

int
rt_tree_add(const rt_tree *t, const char *key, uint8_t lkey, void *value)
{
	rt_node *n;
	if(!t) return 0;
	n = rt_node_find(t,t->root,key,lkey,1);
	if(n) {
		n->value = value;
		return 1;
	} else {
		return 0;
	}
}

void
rt_tree_print(const rt_tree *t)
{
	int i;
	rt_node *r, **l;
	if(!t || !t->root) printf("NULL");
	r = t->root;
	for(i=0,l=r->leaf;i < r->lcnt;i++,*l++) rt_node_print(*l,0);
}

