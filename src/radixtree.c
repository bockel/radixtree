
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

static rt_node *
rt_node_find(	const rt_tree *root, rt_node *n,
		const char *key, size_t lkey, int add)
{
	rt_node *node = NULL;
	/* printf("LOG: rt_node_find(node(%s),\"%.*s\",%d,%d)\n",n->key,lkey,key,lkey,add); */
	if(!root || !n || lkey < 1 || !key) return NULL;
	/* if we are only adding, we can do a more efficient binary search */
	if(!add) {
		size_t mm,mlen;
		if(n->lcnt == 0 || !n->leaf) return NULL;
		node = *(rt_node **)bsearch(key,n->leaf,n->lcnt,sizeof(void*),_cmp_nodes);
		if(node==NULL) return NULL;
		mlen = node->klen < lkey ? node->klen : lkey;
		mm = _maxmatch(node->key,key,mlen);
		if(mm < node->klen) return NULL;
		else if(mm==mlen && node->klen==lkey) return node;
		else return rt_node_find(root,node,key+mm,lkey-mm,add);
	}

	/* otherwise, do a tail insertion sort */
	if(n->lcnt == 0)
	{
		node = rt_node_new(root,0,key,lkey);
		if(!node) return NULL;
		n->leaf[0] = node;
		n->lcnt++;
	} else {
		/* Search for the key
		 * If not found, return the location for it to be added */
		int i = n->lcnt-1, diff;
		rt_node **index;
		if(n->lcnt >= n->lalloc) {
			size_t ns = n->lalloc;
			rt_node **rt;
			ns *= 2;
			if(ns>ALPHABET_SIZE) ns = ALPHABET_SIZE;
			if(root->realloc) {
				rt = root->realloc(n->leaf,ns*sizeof(rt));
				if(!rt) return NULL;
			} else {
				rt = root->malloc(ns*sizeof(rt));
				if(!rt) return NULL;
				memcpy(rt,n->leaf,n->lalloc);
				root->free(n->leaf);
			}
			n->lalloc = ns;
			n->leaf = rt;
		}
		index = &n->leaf[i];
		while(i >= 0) {
			/* Only need to compare first char */
			diff = key[0] - (*index)->key[0];
			if(diff > 0) {
				node = rt_node_new(root,0,key,lkey);
				if(!node) return NULL;
				memmove(index+2,index+1,sizeof(void*)*(n->lcnt-i-1));
				index[1] = node;
				n->lcnt++;
				break;
			} else if(diff < 0) {
				if(i==0) {
					node = rt_node_new(root,0,key,lkey);
					if(!node) return NULL;
					memmove(index+1,index,n->lcnt*sizeof(void*));
					*index = node;
					n->lcnt++;
					break;
				}
			} else {
				/*
				 * if matches, recurse to next node level
				 * otherwise,  update this node to have the shared string;
				 * then add children for different suffixes
				 */
				rt_node *p = *index, *child;
				size_t mm = _maxmatch(p->key, key, p->klen < lkey ? p->klen : lkey);
				if(mm < p->klen) {
					rt_node **tmp;
					/* otherwise, split
					 * add child and update this node */
					child = rt_node_new(root,0,p->key+mm,p->klen-mm);
					if(!child) {
						/* failed to split and add child node */
						return NULL;
					}
					tmp = root->malloc(NODE_INIT_SIZE*sizeof(rt_node *));
					if(!tmp) return NULL;
					child->lalloc = p->lalloc;
					child->lcnt   = p->lcnt;
					child->leaf   = p->leaf;
					child->value  = p->value;
					p->klen    = mm;
					p->key[mm] = 0;
					p->value   = NULL;
					p->leaf    = tmp;
					p->lalloc  = NODE_INIT_SIZE;
					p->lcnt    = 1;
					p->leaf[0] = child;
				}
				if(mm==lkey && p->klen==lkey) {
					return p;
				} else {
					return rt_node_find(root,p,key+mm,lkey-mm,add);
				}
			}
			i--;*index--;
		}
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

