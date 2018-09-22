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

#include "config.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <math.h>
#include <jit/jit.h>
#include "pcg_variants.h"
#include "entropy.h"

typedef struct t_WARRIOR WARRIOR;
#include "text.h"
#ifdef _COREVIEW_
#ifndef MULTITHREAD
#warning "Coreview enables multithreading automatically."
#endif
#define MULTITHREAD
#endif
#ifdef MULTITHREAD
#define TSAFE_CORE
#endif
#ifdef TSAFE_CORE
#include "multithread.h"
#endif
#ifdef _COREVIEW_
#include "SDL2/SDL.h"
#endif

#define PHI 1.6180339887498948

extern unsigned int ROUNDS;
extern unsigned int WARRIORS;

#if defined(__GNUC__) && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40500)
#define COMPILER_HINT(x) /*do {if(!(x)) __builtin_unreachable();} while(0)*/;
#define HINT_UNREACHABLE() /*__builtin_unreachable()*/;
#else
#define COMPILER_HINT(x) /*do {if(!(x)) puts("Assertion failed: " ## x); abort(2);} while(0)*/;
#define HINT_UNREACHABLE() ;
#endif

#if CORESIZE <= 2
#error "CORESIZE must be at least 3."
#elif CORESIZE <= 256
typedef uint_fast8_t pcell_t;
#elif CORESIZE <= 35768
typedef uint_fast16_t pcell_t;
#else
#error "CORESIZE must be less than or equal to 32768."
#endif
#if PSPACESIZE
#if (CORESIZE % PSPACESIZE)
#warning "PSPACESIZE should be a factor of CORESIZE."
#endif
#endif
#if (CORESIZE < MINDISTANCE*2)
#warning "CORESIZE should be at least equal to MINDISTANCE*2"
#endif
#if (MAXLENGTH > MINDISTANCE)
#warning "MAXLENGTH should not be larger than MINDISTANCE"
#endif

#define BOUND_CORESIZE(x) \
  if((signed)x < 0) x += CORESIZE; \
  else if((signed)x >= CORESIZE) x -= CORESIZE;
#define BOUND_CORESIZE_HIGH(x) \
  if((signed)x >= CORESIZE) x -= CORESIZE;
#define BOUND_CORESIZE_LOW(x) \
  if((signed)x < 0) x += CORESIZE; \

#define O_DAT 0 //default
#define O_MOV 1
#define O_ADD 2
#define O_SUB 3
#define O_JMP 4
#define O_JMZ 5
#define O_CMP 6
#if STANDARD == 84
#define O_DJZ 7
#define O_X 8
#endif
#if STANDARD >= 86
#define O_JMN 7
#define O_DJN 8
#define O_SPL 9
#endif
#if STANDARD >= 88
#define O_SLT 10
#else
#define O_X 10
#endif
#if STANDARD >= 94
#define O_MUL 11
#define O_DIV 12
#define O_MOD 13
#define O_SEQ O_CMP
#define O_SNE 14
#define O_NOP 15
#if PSPACESIZE
#define O_LDP 16
#define O_STP 17
#define O_X 18
#else
#define O_X 16
#endif
#else
#define O_X 11
#endif
#if defined(EXT_XCH_a) || defined(EXT_XCH_b)
#if defined(EXT_XCH_a) && defined(EXT_XCH_b)
#error "Can't use EXT_XCH_a and EXT_XCH_b at the same time."
#endif
#define O_XCH O_X
#define _V_XCH_ O_X + 1
#else
#define _V_XCH_ O_X
#endif
#if defined(EXT_PCT_a) || defined(EXT_PCT_b)
#if defined(EXT_PCT_a) && defined(EXT_PCT_b)
#error "Can't use EXT_PCT_a and EXT_PCT_b at the same time."
#endif
#define O_PCT _V_XCH_
#define _V_PCT_ _V_XCH_ + 1
#else
#define _V_PCT_ _V_XCH_
#endif
#ifdef EXT_DJZ
#if STANDARD == 84
#warning "Opcode DJZ is already included for pre-ICWS ('84) compilation."
#define _V_DJZ_ _V_PCT_
#else
#define O_DJZ _V_PCT_
#define _V_DJZ_ _V_PCT_ + 1
#endif
#else
#define _V_DJZ_ _V_PCT_
#endif
#ifdef EXT_STS
#define O_STS _V_DJZ_
#define _V_STS_ _V_DJZ_ + 1
#else
#define _V_STS_ _V_DJZ_
#endif
#define O_COUNT _V_STS_

#if STANDARD == 86
#define opt_SPL86 //uses B-field. Next is new process
#endif //else SPL uses A-field, and next is following instruction

#define M_F 0 //default
#define M_X 1
#define M_A 2
#define M_B 3
#define M_AB 4
#define M_BA 5
#define M_I 6
#define M_COUNT 7

#if STANDARD <= 86
#define A_IMM 0 //default
#define A_DIR 1
#else
#define A_DIR 0 //default
#define A_IMM 1
#endif
#define A_INB 2
#if STANDARD == 86
#define A_ADB 3
#define A_COUNT 4
#elif STANDARD > 86
#define A_PDB 3
#if STANDARD >= 94
#define A_PIB 4
#define A_INA 5
#define A_PDA 6
#define A_PIA 7
#define A_COUNT 8
#else
#define A_COUNT 4
#endif
#endif

#ifdef TSAFE_CORE
#define _corefunc LOCAL_CORE* local_core,
#define _corefun0 LOCAL_CORE* local_core
#define _COREMACR //add local_core parameter to avoid preprocessing errors
#define _corecall local_core,
#define _corecal0 local_core
#else
#define _corefunc
#define _corefun0 void
#define _corecall
#define _corecal0
#endif

/*
<xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx> (uint64_t)
 |------| |------| |------| |------| |---------------| |---------------|
  Opcode  Modifier AddrmodA AddrmodB      A-field           B-field
*/

typedef union t_INSTR1 {
  uint64_t I;
  struct t_INSTR1_i {
    union t_INSTR1_i_f {
      uint32_t F;
      struct t_INSTR1_i_f_f2 {
        int16_t B;
        int16_t A;
      } f2;
    } f;
    union t_INSTR1_i_oma {
      uint32_t OMA;
      struct t_INSTR1_i_oma_oma2 {
        uint8_t aB;
        uint8_t aA;
        uint8_t M;
        uint8_t O;
      } oma2;
    } oma;
  } i;
} INSTR1;

#define _I I
#define _F i.f.F
#define _A i.f.f2.A
#define _B i.f.f2.B
#define _OMA i.oma.OMA
#define _aB i.oma.oma2.aB
#define _aA i.oma.oma2.aA
#define _M i.oma.oma2.M
#define _O i.oma.oma2.O

typedef uint32_t addr2_t;
#define jit_type_addr2 jit_type_uint
#define jit_type_addr2s jit_type_int
#ifdef TSAFE_CORE
typedef int (*jitfunc2_t)(void*, void*, addr2_t, addr2_t, addr2_t);
#else
typedef int (*jitfunc2_t)(void*, addr2_t, addr2_t, addr2_t);
#endif
typedef struct tINSTR2 {
  jitfunc2_t fn;
  addr2_t a, b;
} INSTR2;

struct t_WARRIOR {
  unsigned long org;
  char* name; //;name
  char* author; //;author
  char* version; //;version
  char* date; //;date
  char* strategy; //;strategy, multiple lines
  unsigned int len;
  INSTR1* code1;
  INSTR2* code2;
  #if PSPACESIZE
  pcell_t* pspace;
  pcell_t psp0;
  unsigned long pin;
  int haspin;
  #endif
  unsigned int score, wins, losses, ties;
  #ifdef TSAFE_CORE
  MUTEX mutex;
  #endif
  #ifdef _COREVIEW_
  uint32_t color; //l_coreviewdata warrior color
  #endif
}; //when changing this, update JIT definition

typedef struct t_PROC1 {
  uint16_t pos;
  struct t_PROC1* prev;
  struct t_PROC1* next;
} PROC1;

typedef struct t_PROC2 {
  addr2_t pos;
  struct t_PROC2* prev;
  struct t_PROC2* next;
} PROC2;

#ifdef _COREVIEW_
typedef struct t_CVCELL {
  unsigned long warrior; //ordered warrior index
  uint8_t type;
    #define CVCT_NONE 0
    #define CVCT_READ 1
    #define CVCT_WRITE 2
    #define CVCT_EXEC 3
    #define CVCT_INC 4
    #define CVCT_DEC 5
    #define CVCT_CURR 6
  uint8_t opcode; //same values as INSTR1._O
} CVCELL;

typedef struct t_PAUSED_CORE {
  CONDITION* cond;
  MUTEX* cmutex;
  struct t_PAUSED_CORE* next;
} PAUSED_CORE;
#endif

extern WARRIOR* warriors;
extern int opt_VERBOSE;
extern pcg32_random_t randomgen;
extern int algorithm_select;
extern jit_context_t jit_context;
#ifdef _COREVIEW_
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern int terminating;
extern MUTEX mutex_commun_global;
#endif

#include "pool.h"

/*
██       ██████   ██████  █████  ██               ██████  ██████  ██████  ███████
██      ██    ██ ██      ██   ██ ██              ██      ██    ██ ██   ██ ██
██      ██    ██ ██      ███████ ██              ██      ██    ██ ██████  █████
██      ██    ██ ██      ██   ██ ██              ██      ██    ██ ██   ██ ██
███████  ██████   ██████ ██   ██ ███████ ███████  ██████  ██████  ██   ██ ███████
*/
typedef struct t_LOCAL_CORE {
  INSTR1* l_core1;
  INSTR2* l_core2;
  PROC1** l_proc1; //unordered
  PROC2** la_proc2; //unordered
  uint16_t* l_positions; //ordered
  unsigned long* la_nprocs; //unordered
  unsigned long* l_indices; //maps unordered to ordered
  unsigned long l_nrunning; //number of warriors that remain alive
  int* l_running; //whether they are alive (ordered)
  #ifdef O_PCT
  uint_fast8_t l_isPCT[CORESIZE];
  #endif
  POOL_PROC1_GLOBALS()
  POOL_PROC2_GLOBALS()
  #ifdef _COREVIEW_
  CVCELL* l_coreviewdata; //coreview data
  MUTEX l_mutex_exec; //execution mutex, used for synchronizing core view
  MUTEX l_mutex_mode; //runmode mutex, used for sending/receiving data
  CONDITION l_cond_exec; //condition variable, same function
  unsigned int l_runmode; //synchronization mode
    #define RUN_NOWAIT 0
    #define RUN_CLOCK 1
    #define RUN_PAUSED 2
    #define RUN_FAST 3
  unsigned int l_runparam;
  int l_rundata;
  #endif
  #ifdef TSAFE_CORE
  THREAD l_thread; //thread running core
  #if PSPACESIZE
  int la_pspace_local_accessed;
  #endif
  #endif
  unsigned long l_hook_lastw_ordered; //ordered index of warrior executing current instruction
  unsigned long la_w2; //same, but unordered
} LOCAL_CORE;
#ifndef TSAFE_CORE
#define local_core (&g_local_core)
#endif
#define l_core1 local_core->l_core1
#define l_core2 local_core->l_core2
#define l_positions local_core->l_positions
#define l_proc1 local_core->l_proc1
#define l_proc2 local_core->la_proc2 //different alias for standalone member use
#define l_nprocs local_core->la_nprocs //different alias for standalone member use
#define l_indices local_core->l_indices
#define l_nrunning local_core->l_nrunning
#define l_running local_core->l_running
#define l_pool_proc1 local_core->l_pool_proc1
#define l_pool_fbase_proc1 local_core->l_pool_fbase_proc1
#define l_pool_ftop_proc1 local_core->l_pool_ftop_proc1
#define l_pool_proc2 local_core->l_pool_proc2
#define l_pool_fbase_proc2 local_core->l_pool_fbase_proc2
#define l_pool_ftop_proc2 local_core->l_pool_ftop_proc2
#ifdef O_PCT
#define l_isPCT local_core->l_isPCT
#endif
#ifdef _COREVIEW_
#define l_coreviewdata local_core->l_coreviewdata
#define l_mutex_exec local_core->l_mutex_exec
#define l_mutex_mode local_core->l_mutex_mode
#define l_cond_exec local_core->l_cond_exec
#define l_runmode local_core->l_runmode
#define l_runparam local_core->l_runparam
#define l_rundata local_core->l_rundata
#endif
#ifdef TSAFE_CORE
#define l_thread local_core->l_thread
#if PSPACESIZE
#define l_pspace_local_accessed local_core->la_pspace_local_accessed //different alias for standalone member use
#endif
#endif
#define l_hook_lastw_ordered local_core->l_hook_lastw_ordered
#define l_w2 local_core->la_w2 //different alias for standalone member use

#ifdef _COREVIEW_
typedef struct tCOREVIEW {
  LOCAL_CORE* local_core;
  unsigned int px, py; //position on window, in destination pixels
  unsigned int tgmode; //target mode, combination of flags:
    #define TG_PW_AM 1 //pw at most
    #define TG_PW_AL 2 //pw at least
    #define TG_PH_AM 4 //pw at most
    #define TG_PH_AL 8 //pw at least
    #define TG_CSIZE 16 //target cell size
  unsigned int tg_pw_am, tg_pw_al, tg_ph_am, tg_ph_al, tg_csize; //values
  int instr_colorize; //colorize instructions
  //fields below set automatically
  SDL_Texture* img_tex;
  uint32_t* data;
  unsigned int cw, ch; //width and height in core cells
  unsigned int csize; //source pixels per core cell
  unsigned int pw, ph; //width and height, in destination pixels
  float scale; //destination/source ratio, should be integer
} COREVIEW;

#define sleep_ms(x) SDL_Delay(x)
#endif

//Thread-local
extern void error(const char*, ...);
extern unsigned int battle1_single(unsigned long); //single-thread, blocking
extern unsigned int battle2_single(unsigned long); //single-thread, blocking
extern void battle1_multithread(unsigned long, unsigned int); //multithread, blocking
extern LOCAL_CORE* battle1_async(unsigned long); //one new thread, non-blocking
void wait_for_core(LOCAL_CORE*); //wait for battle termination
extern void debug_println1(uint64_t);
extern void debug_println2(INSTR2);
extern void signal_terminate(void);
extern int check_terminate(void);
extern int _hardcoded_dat(_corefunc INSTR2*, addr2_t, addr2_t, addr2_t);
extern void load1(WARRIOR*, LINE*);
extern void load2(WARRIOR*, LINE*);
//Global for all threads
extern void initialize(void);
extern void finalize(void);
extern int parse_load(char**, char**, char*);
extern void unload_all(void);
#ifdef _COREVIEW_
extern void init_coreview(void);
extern void draw_coreview(COREVIEW*);
extern COREVIEW* new_coreview(void);
extern void set_coreview_pos(COREVIEW*, unsigned int, unsigned int);
extern void set_coreview_core(COREVIEW*, LOCAL_CORE*);
extern void set_coreview_target(COREVIEW*, unsigned int, ...);
extern void set_core_runmode(COREVIEW*, unsigned int, unsigned int);
extern void destroy_coreview(COREVIEW*);
#endif
