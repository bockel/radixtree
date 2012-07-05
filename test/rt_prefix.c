
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
#include "radixtree.h"

#define ALPHABET "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"
#define ALSIZE (strlen(ALPHABET))

int
main(int argc, char **argv)
{
	rt_tree *t;
	char **arg;
	int i, succ=0;
	rt_iter *iter;

	t = rt_tree_new(ALSIZE,NULL);
	if(!t) {
#ifndef NDEBUG
		printf("ERROR: Could not create rt_tree... Exiting\n");
#endif
		return (-1);
	}
	for(i=1,arg=argv+1;i<argc-1;i++,arg++)
	{
		if(rt_tree_set(t, (unsigned char *)*arg, strlen(*arg), *arg))
			succ++;
#ifndef NDEBUG
		else printf("!!! Adding arg[%d] = %s... FAILED\n",i,*arg);
#endif
	}
#ifndef NDEBUG
	printf("ADD Passed: %d of %d\n",succ,argc-2);
	rt_tree_print(t);
	printf("Searching for \"%s\"...\n", *arg);
#endif
	if(succ!=argc-2) {
		rt_tree_free(t);
		return (-1);
	}
	iter = rt_tree_prefix(t, (unsigned char *)*arg, strlen(*arg));
	succ=0;
	if(iter) {
		while(rt_iter_next(iter)) {
			succ++;
#ifndef NDEBUG
			printf("SUCCESS (%s) = %s\n",rt_iter_key(iter),(char *)rt_iter_value(iter));
#endif
		}
	}
#ifndef NDEBUG
	else printf("FAILED\n");
#endif

	rt_tree_free(t);
	return succ > 0;
}

