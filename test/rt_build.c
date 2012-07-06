
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

#ifndef NDEBUG
#define DEBUG
#define _DEBUG(x,...) printf(x,__VA_ARGS__)
#else
#define _DEBUG(x,...)
#endif

int
main(int argc, char **argv)
{
    rt_tree *t;
    char **arg;
    int i, succ=0, def=0;

    t = rt_tree_new(ALSIZE,NULL);
    if(!t) {
        printf("ERROR: Could not create rt_tree... Exiting\n");
        return (-1);
    }
    arg = argv+1;
    i = 1;
    if(argc>1 && *arg && !strcmp(*arg,"-d")) {
        def = 1;
        arg++; i++;
    }

    if(!def) {
        _DEBUG("Using rt_tree_set(%d)\n",def);
        for(;i<argc;i++,arg++)
        {
            if(rt_tree_set(t, (unsigned char *)*arg, strlen(*arg), *arg))
                succ++;
            else{_DEBUG("!!! Adding arg[%d] = %s... FAILED\n",i,*arg);}
        }
    } else {
        char *r;
        _DEBUG("Using rt_tree_setdefault(%d)\n",def);
        for(;i<argc;i++,arg++)
        {
            if((r = rt_tree_setdefault(t, (unsigned char *)*arg,
                            strlen(*arg), *arg)) != NULL) {
                if(r == *arg) succ++;
            } else{_DEBUG("!!! Adding arg[%d] = %s... FAILED\n",i,*arg);}
        }
    }
#ifdef DEBUG
    printf("ADD Passed: %d of %d\n",succ,argc-def-1);
    rt_tree_print(t);
#endif
    if(succ!=argc-def-1) {
        rt_tree_free(t);
        return (-1);
    }
    succ = 0;
    for(i=argc-def-1,arg--;i>0;i--,arg--)
    {
        char *val;
        if((val = (char*)rt_tree_get(t, (unsigned char *)*arg, strlen(*arg)))
                != NULL) {
            if(!strcmp(val,*arg)) succ++;
            else{_DEBUG("!!! Value mismatch (%s != %s)\n",val,*arg);}
        }
        else{_DEBUG("!!! Searching for \"%s\"(arg[%d])... FAILED\n",*arg,i);}
    }
    _DEBUG("SEARCH Passed: %d of %d\n",succ,argc-def-1);

    rt_tree_free(t);
    return succ==argc-def-1;
}
