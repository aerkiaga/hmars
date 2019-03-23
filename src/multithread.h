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

#define TM_NEED_THREAD

#if defined(unix) || defined(__unix__) || defined(__unix)
#define PLATFORM_UNIX
#include <unistd.h>
#endif

/*If any of the #include's below fail,
just comment its block out completely*/

#if defined(THREAD_USE_C11)
  #ifdef TM_THREAD_DONE
    #error "Specify only one thread option."
  #endif
  #if __STDC_VERSION__ < 201112L
    #warning "C11 standard may not be supported by this system."
  #endif
  #include <threads.h>
  #define TM_THREAD_DONE
  #define MT_USE_C11 //C11 threads

#elif defined(THREAD_USE_c11threads)
  #ifdef TM_THREAD_DONE
    #error "Specify only one thread option."
  #endif
  #if __STDC_VERSION__ < 201112L
    #warning "C11 standard may not be supported by this system."
  #endif
  #include <c11threads.h>
  #define TM_THREAD_DONE
  #define MT_USE_C11 //C11 threads

#elif defined(THREAD_USE_POSIX)
  #ifdef TM_THREAD_DONE
    #error "Specify only one thread option."
  #endif
  #if !defined(_POSIX_VERSION) || _POSIX_VERSION < 199506L
    #warning "POSIX standard may not be supported by this system."
  #endif
  #include <pthread.h>
  #define TM_THREAD_DONE
  #define MT_USE_POSIX //POSIX threads
#endif

#ifndef TM_THREAD_DONE
  #error "Multithreading impossible, or no option selected."
#else
  #undef TM_THREAD_DONE
#endif

#ifdef MT_USE_C11
typedef thrd_t THREAD;
typedef mtx_t MUTEX;
typedef cnd_t CONDITION;
typedef int _TPROC_TYPE;
#define _TPROC_RET() return thrd_success

#define tcreate(t, f, p) thrd_create(t, f, p)
#define tjoin(t) thrd_join(t, NULL)
#define minit(m) mtx_init(&m, mtx_plain)
#define mlock(m) mtx_lock(&m)
#define munlock(m) mtx_unlock(&m)
#define mdestroy(m) mtx_destroy(&m)
#define cinit(c) cnd_init(&c)
#define cwait(c, m) cnd_wait(&c, &m)
#define csignal(c) cnd_signal(&c)
#define cdestroy(c) cnd_destroy(&c)
#endif

#ifdef MT_USE_POSIX
typedef pthread_t THREAD;
typedef pthread_mutex_t MUTEX;
typedef pthread_cond_t CONDITION;
typedef void* _TPROC_TYPE;
#define _TPROC_RET() return NULL

#define tcreate(t, f, p) pthread_create(t, NULL, f, p)
#define tjoin(t) pthread_join(t, NULL)
#define minit(m) pthread_mutex_init(&m, NULL)
#define mlock(m) pthread_mutex_lock(&m)
#define munlock(m) pthread_mutex_unlock(&m)
#define mdestroy(m) pthread_mutex_destroy(&m)
#define cinit(c) pthread_cond_init(&c, NULL)
#define cwait(c, m) pthread_cond_wait(&c, &m)
#define csignal(c) pthread_cond_signal(&c)
#define cdestroy(c) pthread_cond_destroy(&c)
#endif
