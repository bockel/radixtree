

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

#ifdef NDEBUG
#define TEST(x) {tests++;if(x){succ++;}}
#else
#define TEST(x) {tests++;if(x){succ++;printf("++ PASS: \"%s\" test\n",#x);}else{printf("-- FAIL: \"%s\" test\n",#x);}}
#endif

static int test1()
{
	rt_tree *t;
	t = rt_tree_new(0,NULL);
	if(t==NULL) return 1;
	else {
		rt_tree_free(t);
		return 0;
	}
}

static int test2()
{
	rt_tree *t;
	t = rt_tree_new(1,NULL);
	if(t) {
		rt_tree_free(t);
		return 1;
	} return 0;
}

/* rt_tree_set tests */
static int test3()
{
	rt_tree *t;
	char *val = "ABCDEFGH";
	int ret = 0;
	t = rt_tree_new(16,NULL);
	if(!t) return 0;

	if(rt_tree_set(NULL,"ABC",3,val)) goto exit;
	if(rt_tree_set(t,NULL,0,val)) goto exit;
	if(rt_tree_set(t,NULL,1,val)) goto exit;
	if(!rt_tree_set(t,"ABC",3,val)) goto exit;
	if(!rt_tree_set(t,"ABC",3,val)) goto exit;
	if(!rt_tree_set(t,"ABC",3,NULL)) goto exit;
	if(!rt_tree_set(t,"ABC",2,val)) goto exit;
	ret = 1;

exit:
	if(!t) return 0;
	rt_tree_free(t);
	return ret;
}

/* rt_tree_get tests */
static int test4()
{
	rt_tree *t;
	char *val = "ABCDEFGH",*r;
	int ret = 0;
	t = rt_tree_new(16,NULL);
	if(!t) return 0;
	if(rt_tree_get(NULL,"ABC",3)) goto exit;
	if(rt_tree_get(t,NULL,3)) goto exit;
	if(rt_tree_get(t,"ABC",3)) goto exit;
	if(rt_tree_set(t,"abc",3,val)) {
		r = rt_tree_get(t,"abc",3);
		if(r!=val) goto exit;
		if(rt_tree_get(t,"abc",2)) goto exit;
		if(rt_tree_get(t,"ABC",3)) goto exit;
	} else goto exit;
	if(rt_tree_set(t,"abc",3,NULL)) {
		if(rt_tree_get(t,"abc",3)) goto exit;
	} else goto exit;
	ret = 1;

exit:
	if(!t) return 0;
	rt_tree_free(t);
	return ret;
}

/* test rt_tree_remove */
static int test5()
{
	rt_tree *t;
	char *val = "ABCDEFGH",*r;
	int ret = 0;
	t = rt_tree_new(16,NULL);
	if(!t) return 0;

	if(rt_tree_remove(NULL,"ABC",3)) goto exit;
	if(rt_tree_remove(t,NULL,2)) goto exit;
	if(rt_tree_remove(t,"ABC",3)) goto exit;
	if(rt_tree_set(t,"ABC",3,val)) {
		r = rt_tree_get(t,"ABC",3);
		if(r!=val) goto exit;
		/* try to remove the key */
		if(!rt_tree_remove(t,"ABC",3)) goto exit;
		/* try to retrive it, should return NULL */
		if(rt_tree_get(t,"ABC",3)) goto exit;
		/* try to remove it again, should fail */
		if(rt_tree_remove(t,"ABC",3)) goto exit;
	} else goto exit;
	ret = 1;

exit:
	if(!t) return 0;
	rt_tree_free(t);
	return ret;
}

static int test6()
{
	return 0;
}

int
main()
{
	unsigned int tests=0, succ=0;

	TEST(test1());
	TEST(test2());
	TEST(test3());
	TEST(test4());
	TEST(test5());

#ifdef DEBUG
	printf("%s: Passed %u of %u tests\n",tests-succ==0?"SUCCESS":"FAILED",succ,tests);
#endif

	return tests-succ;
}
