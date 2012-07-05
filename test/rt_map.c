
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
#include <string.h>
#include "radixtree.h"

#define ALPHABET "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"
#define ALSIZE (strlen(ALPHABET))

typedef struct {
	size_t ncount;
} ctxt;

void
print_node(void *user_ctxt, unsigned char *key, size_t klen, void *value)
{
	if(!strncmp((char*)key,(char*)value,klen))
#ifndef NDEBUG
		printf("\t[%lu] %.*s(%lu) = %s\n", ++((ctxt *)user_ctxt)->ncount, (int)klen, key, klen, (char *)value);
	else
		printf("FAILED (key(%s) != value(%s))\n", key, (char *)value);
#else
	++((ctxt*)user_ctxt)->ncount;
#endif
}

int
main(int argc, char **argv)
{
	rt_tree *t;
	char **arg;
	int i, succ=0;
	ctxt context;

	t = rt_tree_new(ALSIZE,NULL);
	if(!t) {
#ifndef NDEBUG
		printf("ERROR: Could not create rt_tree... Exiting\n");
#endif
		return (-1);
	}
	for(i=1,arg=argv+1;i<argc;i++,arg++)
	{
		if(rt_tree_set(t, (unsigned char *)*arg, strlen(*arg), *arg))
			succ++;
#ifndef NDEBUG
		else printf("!!! Adding arg[%d] = %s... FAILED\n",i,*arg);
#endif
	}
#ifndef NDEBUG
	printf("ADD Passed: %d of %d\n",succ,argc-1);
	rt_tree_print(t);
#endif
	if(succ!=argc-1) {
		rt_tree_free(t);
		return (-1);
	}
	context.ncount = 0;
	rt_tree_map(t, &context, print_node);
#ifndef NDEBUG
	printf("%s... Found %lu of %d\n", context.ncount==((size_t)argc)-1?"SUCCESS":"FAILURE", context.ncount, argc-1);
#endif

	rt_tree_free(t);
	return context.ncount==((size_t)argc)-1;
}

