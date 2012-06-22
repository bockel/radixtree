
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
	size_t klen;
	uint8_t lcnt;
	uint8_t lalloc;
	char *key;
	void *value;
	rt_node *parent;
	struct _node **leaf;
};

struct _rt_tree {
	uint8_t alsize;
	void (*free)(void *);
	void (*vfree)(void *);
	void * (* malloc)(size_t);
	void * (* realloc)(void *,size_t);
	rt_node *root;
};

struct _rt_iter {
	const rt_tree *t;
	const rt_node *root;
	rt_node *curr;
};

static void
rt_node_free(const rt_tree *t, rt_node *n)
{
	uint8_t i;
	rt_node **l;
	if(!n || !t) return;
	for(i=0,l=n->leaf;i<n->lcnt;i++,l++)
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
	uint8_t s = c;
	size_t sz = sizeof(*n);
	if(!t || !t->malloc) return NULL;

	n = t->malloc(sz);
	if(!n) return NULL;
	memset(n,0,sz);
	if(s < 1) s = NODE_INIT_SIZE;
	else if(s > t->alsize) s = t->alsize;
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
		if(n->key) printf("\"%.*s\"",(int)n->klen,n->key);
		else       printf("NULL");
		if(n->value) printf(" = addr(%p)\n",n->value);
		else         printf(" = NULL\n");
		for(i=0,l=n->leaf;i<n->lcnt;i++,l++) rt_node_print(*l,depth+1);
	} else printf("NULL\n");
}

static size_t
_maxmatch(const char *key, const char *match, size_t len)
{
	register char *m1 = (char *)key, *m2 = (char *)match;
	char *me1 = m1+len, *me2 = m2+len;
	while(*m1 == *m2 && m1<me1 && m2<me2) {
		m1++; m2++;
	}
	return m1-key;
}

/*
 * Implement a custom binary search that returns the last search location.
 * This location is either a match or the location where the node should be inserted +-1
 */
static int
rt_bsearch(const char *key, const rt_node **leaf,
		size_t leafcnt, rt_node ***match)
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
	if(match) *match = (rt_node **)&leaf[index];
	return cmp;
}

static size_t
rt_node_grow(const rt_tree *t, rt_node *n)
{
	size_t ns = n->lalloc;
	rt_node **rt = NULL;
	ns *= 2;
	if(ns>t->alsize) ns = t->alsize;
	if(ns == n->lalloc) return 0;
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

typedef enum {
	NODE_SET,
	NODE_GET,
	NODE_PREFIX
} rt_get_mode;

static rt_node *
rt_node_get(	const rt_tree *root, rt_node *n,
		const char *key, const char *ptr,
		size_t lkey, rt_get_mode mode)
{
	rt_node *node = NULL, **p;
	size_t len;
	int diff;
	if(!root || !n || !key || !ptr || ptr > key+lkey) return NULL;

	len = lkey - (ptr - key);
	if(n->lcnt == 0) {
		if(mode!=NODE_SET) return NULL;
		node = rt_node_new(root,0,ptr,len);
		if(!node) return NULL;
		n->leaf[0] = node;
		node->parent = n;
		n->lcnt++;
		return node;
	}

	/* Search for the key
	 * If not found, return the location for it to be added */
	diff = rt_bsearch(ptr,(const rt_node **)n->leaf,n->lcnt,&p);
	if(!p) return NULL;
	if(diff==0) /* found (partial?) match */
	{
		rt_node *index = *p, *child;
		size_t mm = _maxmatch(ptr,index->key, index->klen < len ? index->klen : len);
		if(mode == NODE_SET) {
			/* if(n->lcnt >= n->lalloc && rt_node_grow(root,n)<1) return NULL; */
			if(mm < index->klen) {
				rt_node **tmp;
				int i;
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
				child->parent = index;
				index->klen    = mm;
				index->key[mm] = 0;
				index->value   = NULL;
				index->leaf    = tmp;
				index->lalloc  = NODE_INIT_SIZE;
				index->lcnt    = 1;
				index->leaf[0] = child;
				/* need to update child parent refs, if necessary */
				for(i=0,tmp=child->leaf;i<child->lcnt&&*tmp;i++,tmp++)
					(*tmp)->parent=child;
			}
		}
		if(mm==len && (index->klen==mm || (mode==NODE_PREFIX && index->klen>mm))) return index;
		return rt_node_get(root,index,key,ptr+mm,lkey,mode);
	} else if(mode==NODE_SET) {
		size_t offset = p - n->leaf;
		/* rt_node_grow may realloc the n->leaf location
		 * 1 save the p offset
		 * 2 redeclare p based on the new n->leaf location and offset
		 */
		if(mode == NODE_SET && n->lcnt >= n->lalloc && rt_node_grow(root,n)<1) return NULL;
		p = n->leaf + offset;
		node = rt_node_new(root,0,ptr,len);
		if(!node) return NULL;
		node->parent = n;
		if(diff > 0) {
			memmove(p+2,p+1,sizeof(p)*(n->lcnt-offset-1));
			p[1] = node;
		} else {
			memmove(p+1,p,sizeof(p)*(n->lcnt-offset));
			*p = node;
		}
		n->lcnt++;
		return node;
	}

	return NULL;
}

rt_tree *
rt_tree_new(uint8_t albet_size, void (*_vfree)(void*))
{
	return rt_tree_custom(albet_size, malloc, realloc, free, _vfree);
}

rt_tree *
rt_tree_custom(	uint8_t albet_size,
		void* (*_malloc)(size_t),
		void* (*_realloc)(void *,size_t),
		void (*_free)(void*),
		void (*_vfree)(void*))
{
	rt_tree *t = NULL;
	if(!_malloc || !_free || albet_size<1) return NULL;
	t = _malloc(sizeof(rt_tree));
	if(!t) return NULL;
	t->malloc = _malloc;
	t->realloc = _realloc;
	t->free = _free;
	t->vfree = _vfree;
	t->alsize = albet_size > MAX_ALPHABET_SIZE ? MAX_ALPHABET_SIZE : albet_size;
	t->root = rt_node_new(t,0,NULL,0);
	t->root->parent = NULL;
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
rt_tree_get(const rt_tree *t, const char *key, size_t lkey, void ** value)
{
	rt_node *n;
	if(!t) {
		*value = NULL;
		return 0;
	}
	n = rt_node_get(t,t->root,key,key,lkey<MAX_KEY_LENGTH?lkey:MAX_KEY_LENGTH,NODE_GET);
	if(n && n->value) {
		*value = n->value;
		return 1;
	} else {
		*value = NULL;
		return 0;
	}
}

int
rt_tree_set(const rt_tree *t, const char *key, size_t lkey, void *value)
{
	rt_node *n;
	if(!t) return 0;
	n = rt_node_get(t,t->root,key,key,lkey<MAX_KEY_LENGTH?lkey:MAX_KEY_LENGTH,NODE_SET);
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
	for(i=0,l=r->leaf;i < r->lcnt;i++,l++) rt_node_print(*l,0);
}

rt_iter *
rt_tree_prefix(const rt_tree *t, const char *prefix, size_t prefixlen)
{
	static rt_iter iter;
	rt_node *result;
	if(!t) return NULL;
	result = rt_node_get(t, t->root, prefix, prefix, prefixlen<MAX_KEY_LENGTH?prefixlen:MAX_KEY_LENGTH, NODE_PREFIX);
	if(!result) return NULL;
	iter.root = result;
	iter.curr = NULL;
	iter.t = t;
	return &iter;
}

int
rt_iter_next(rt_iter *iter)
{
	rt_node *c,**t;
	char *pkey;
	if(!iter) return 0;
	if(iter->curr == NULL) {
		iter->curr = (rt_node*)iter->root;
		if(iter->root->value != NULL) return 1;
	}

	/* First try to recurse through child nodes */
	while(iter->curr->lcnt > 0) {
		iter->curr = iter->curr->leaf[0];
		if(iter->curr->value) return 1;
	}
	c = iter->curr;
	while(1) {
		pkey = c->key;
		if(!c->parent) return 0;
		c = c->parent;
		/* Go up to parent to scan siblings */
		/* printf("key(%s) c(%s) parent(%s)\n",pkey,c->key,c->parent->key); */
		if(c->lcnt > 0) {
			rt_bsearch(pkey,(const rt_node **)c->leaf,c->lcnt,&t);
			while(t && *t && t<c->leaf+c->lcnt-1) {
				t++;
				if((*t)->value != NULL) {
					iter->curr = *t;
					return 1;
				} else if((*t)->lcnt > 0) {
					t = &(*t)->leaf[0];
					c=(*t)->parent;
					if((*t)->value) {
						iter->curr = *t;
						return 1;
					}
				}
			}
		}
		if(c==iter->root) return 0;
	}
	return 0;
}

const char *
rt_iter_key(const rt_iter *iter)
{
	rt_node *n;
	static char ret[MAX_KEY_LENGTH+1], *ptr;
	size_t len = MAX_KEY_LENGTH,i;
	if(!iter || !iter->curr) return NULL;
	/* This builds up the string by traversing the tree
	 * from the current node to the root.
	 */
	ptr = ret+MAX_KEY_LENGTH;
	*ptr = 0; ptr--;
	n = iter->curr;
	while(len>0 && n) {
		i = n->klen;
		ptr-=i;
		len-=i;

		memcpy(ptr,n->key,i);
		n = n->parent;
	}
	return ptr;
}

const void *
rt_iter_value(const rt_iter *iter)
{
	return iter->curr ? iter->curr->value : NULL;
}

