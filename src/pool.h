/**
    hMARS - A fast and feature-rich Memory Array Redcode Simulator for Corewar
    Copyright (C) 2018  Aritz Erkiaga

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

/*
██████  ██████   ██████   ██████  ██
██   ██ ██   ██ ██    ██ ██      ███
██████  ██████  ██    ██ ██       ██
██      ██   ██ ██    ██ ██       ██
██      ██   ██  ██████   ██████  ██
*/

//weak symbols
#define POOL_PROC1_GLOBALS() \
  PROC1* l_pool_proc1; \
  PROC1** l_pool_fbase_proc1; \
  PROC1** l_pool_ftop_proc1; //actually points over the top for performance reasons

#ifndef TSAFE_CORE
POOL_PROC1_GLOBALS()
#endif

#define alloc_pool_proc1() *(--l_pool_ftop_proc1)

#define free_pool_proc1(ptr) *(l_pool_ftop_proc1++) = ptr;

#define clear_pool_proc1() \
  do { \
    int init_pool_proc1__c; \
    for(init_pool_proc1__c = 0; init_pool_proc1__c < MAXPROCESSES * WARRIORS; ++init_pool_proc1__c) { \
      l_pool_fbase_proc1[init_pool_proc1__c] = &l_pool_proc1[init_pool_proc1__c]; \
    } \
  } while(0); \
  l_pool_ftop_proc1 = &l_pool_fbase_proc1[MAXPROCESSES * WARRIORS];

#define init_pool_proc1() \
  l_pool_proc1 = malloc(MAXPROCESSES * WARRIORS * sizeof(PROC1)); \
  l_pool_fbase_proc1 = malloc(MAXPROCESSES * WARRIORS * sizeof(PROC1*)); \
  clear_pool_proc1();

#define destroy_pool_proc1() \
  free(l_pool_proc1); \
  free(l_pool_fbase_proc1);

  /*
  ██████  ██████   ██████   ██████ ██████
  ██   ██ ██   ██ ██    ██ ██           ██
  ██████  ██████  ██    ██ ██       █████
  ██      ██   ██ ██    ██ ██      ██
  ██      ██   ██  ██████   ██████ ███████
  */

#define POOL_PROC2_GLOBALS() \
  PROC2* l_pool_proc2; \
  PROC2** l_pool_fbase_proc2; \
  PROC2** l_pool_ftop_proc2; //actually points over the top for performance reasons

#ifndef TSAFE_CORE
POOL_PROC2_GLOBALS()
#endif

#define alloc_pool_proc2() *(--l_pool_ftop_proc2)

#define free_pool_proc2(ptr) *(l_pool_ftop_proc2++) = ptr;

#define clear_pool_proc2() \
  do { \
    int init_pool_proc2__c; \
    for(init_pool_proc2__c = 0; init_pool_proc2__c < MAXPROCESSES * WARRIORS; ++init_pool_proc2__c) { \
      l_pool_fbase_proc2[init_pool_proc2__c] = &l_pool_proc2[init_pool_proc2__c]; \
    } \
  } while(0); \
  l_pool_ftop_proc2 = &l_pool_fbase_proc2[MAXPROCESSES * WARRIORS];

#define init_pool_proc2() \
  l_pool_proc2 = malloc(MAXPROCESSES * WARRIORS * sizeof(PROC2)); \
  l_pool_fbase_proc2 = malloc(MAXPROCESSES * WARRIORS * sizeof(PROC2*)); \
  clear_pool_proc2();

#define destroy_pool_proc2() \
  free(l_pool_proc2); \
  free(l_pool_fbase_proc2);
