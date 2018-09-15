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

#include "defs.h"
#include "multithread.h"

#ifndef TSAFE_CORE
LOCAL_CORE g_local_core;
#endif

//GLOBALS
WARRIOR* warriors;
#if defined(TSAFE_CORE) && PSPACESIZE
MUTEX mutex_pwarriors;
#endif
int terminating = 0;
#ifdef _COREVIEW_
MUTEX mutex_commun_global;
PAUSED_CORE* paused_cores = NULL;
#endif

#ifdef _COREVIEW_
void inline signal_terminate() {
  mlock(mutex_commun_global);
  terminating = 1;
  PAUSED_CORE* p;
  for(p = paused_cores; p != NULL; p = p->next) { //unpause all cores
    mlock(*p->cmutex);
    csignal(*p->cond);
    munlock(*p->cmutex);
  }
  munlock(mutex_commun_global);
  return;
}

int inline check_terminate() {
  int r = terminating;
  return r;
}
#endif

/*
██   ██  ██████   ██████  ██   ██ ███████
██   ██ ██    ██ ██    ██ ██  ██  ██
███████ ██    ██ ██    ██ █████   ███████
██   ██ ██    ██ ██    ██ ██  ██       ██
██   ██  ██████   ██████  ██   ██ ███████
*/
#if defined(EXT_PCT_b) || defined(_COREVIEW_)
#define HOOK_ONEXEC
void inline hook_onexec(_corefunc int16_t ptr, unsigned long w) {
  //w is unordered warrior index, ordered index can be obtained as l_indices[w]
  #if defined(_COREVIEW_)
  l_hook_lastw_ordered = l_indices[w];
  l_coreviewdata[ptr].warrior = l_hook_lastw_ordered;
  l_coreviewdata[ptr].type = CVCT_CURR;
  mlock(l_mutex_mode);
  /********************************/
  /*Here pause execution as needed*/
  /********************************/
  int dolock = 1;
  switch(l_runmode) {
    case RUN_FAST:
      if(l_rundata > 0) dolock = 0;
      else l_rundata = l_runparam;
      break;
    case RUN_NOWAIT: break;
    case RUN_CLOCK: {
      unsigned int tmp = l_runparam;
      munlock(l_mutex_mode); //don't block other threads
      sleep_ms(tmp);
      mlock(l_mutex_mode);
      break; }
    case RUN_PAUSED:
      mlock(mutex_commun_global);
      if(paused_cores == NULL) {
        paused_cores = malloc(sizeof(PAUSED_CORE));
        paused_cores->next = NULL;
      } else {
        PAUSED_CORE* tmp = paused_cores;
        paused_cores = malloc(sizeof(PAUSED_CORE));
        paused_cores->next = tmp;
      }
      paused_cores->cond = &l_cond_exec;
      paused_cores->cmutex = &l_mutex_mode;
      munlock(mutex_commun_global);

      cwait(l_cond_exec, l_mutex_mode);

      mlock(mutex_commun_global);
      paused_cores = paused_cores->next;
      free(paused_cores);
      munlock(mutex_commun_global);
      break;
  }
  munlock(l_mutex_mode);
  if(dolock) mlock(l_mutex_exec);
  #endif
  #ifdef EXT_PCT_b
  l_isPCT[ptr] = 0;
  #endif
  return;
}
#else
#define hook_onexec(x, y) ;
#endif

#if defined(_COREVIEW_)
#define HOOK_ONENDEXEC
extern void inline hook_onendexec(_corefunc int16_t ptr) {
  #if defined(_COREVIEW_)
  l_coreviewdata[ptr].type = CVCT_EXEC;
  mlock(l_mutex_mode);
  if(l_runmode == RUN_FAST) {
    --l_rundata;
    if(l_rundata <= 0) munlock(l_mutex_exec);
  }
  else munlock(l_mutex_exec);
  munlock(l_mutex_mode);
  #endif
  return;
}
#else
#define hook_onendexec(x) ;
#endif

#if defined(_COREVIEW_)
#define HOOK_ONREAD
uint64_t inline hook_onread(_corefunc int16_t ptr) { //return instruction as ._I
  #if defined(_COREVIEW_)
  l_coreviewdata[ptr].warrior = l_hook_lastw_ordered;
  l_coreviewdata[ptr].type = CVCT_READ;
  #endif
  return l_core1[ptr]._I;
}
#else
#define hook_onread(x) l_core1[x]._I
#endif

#if defined(O_PCT) || defined(_COREVIEW_)
#define HOOK_ONWRITE
void inline hook_onwrite_A(_corefunc int16_t ptr, int16_t val) { //perform write (A-field)
  #ifdef O_PCT
  if(l_isPCT[ptr]) {
    #ifdef EXT_PCT_a
    l_isPCT[ptr] = 0;
    #endif
    return;
  }
  #endif
  l_core1[ptr]._A = val;
  #if defined(_COREVIEW_)
  l_coreviewdata[ptr].warrior = l_hook_lastw_ordered;
  l_coreviewdata[ptr].type = CVCT_WRITE;
  #endif
  return;
}
void inline hook_onwrite_B(_corefunc int16_t ptr, int16_t val) { //perform write (B-field)
  #ifdef O_PCT
  if(l_isPCT[ptr]) {
    #ifdef EXT_PCT_a
    l_isPCT[ptr] = 0;
    #endif
    return;
  }
  #endif
  l_core1[ptr]._B = val;
  #if defined(_COREVIEW_)
  l_coreviewdata[ptr].warrior = l_hook_lastw_ordered;
  l_coreviewdata[ptr].type = CVCT_WRITE;
  #endif
  return;
}
void inline hook_onwrite_F(_corefunc int16_t ptr, uint32_t val) { //perform write (both fields)
  #ifdef O_PCT
  if(l_isPCT[ptr]) {
    #ifdef EXT_PCT_a
    l_isPCT[ptr] = 0;
    #endif
    return;
  }
  #endif
  l_core1[ptr]._F = val;
  #if defined(_COREVIEW_)
  l_coreviewdata[ptr].warrior = l_hook_lastw_ordered;
  l_coreviewdata[ptr].type = CVCT_WRITE;
  #endif
  return;
}
void inline hook_onwrite_I(_corefunc int16_t ptr, uint64_t val) { //perform write (whole instruction)
  #ifdef O_PCT
  if(l_isPCT[ptr]) {
    #ifdef EXT_PCT_a
    l_isPCT[ptr] = 0;
    #endif
    return;
  }
  #endif
  l_core1[ptr]._I = val;
  #if defined(_COREVIEW_)
  l_coreviewdata[ptr].warrior = l_hook_lastw_ordered;
  l_coreviewdata[ptr].type = CVCT_WRITE;
  INSTR1 tmp;
  tmp._I = val; //hope the compiler optimizes this
  l_coreviewdata[ptr].opcode = tmp._O; //into some simple cast
  #endif
  return;
}
#else
#define hook_onwrite_A(x, y) l_core1[x]._A = y
#define hook_onwrite_B(x, y) l_core1[x]._B = y
#define hook_onwrite_F(x, y) l_core1[x]._F = y
#define hook_onwrite_I(x, y) l_core1[x]._I = y
#endif

#if defined(O_PCT) || defined(_COREVIEW_)
#define HOOK_ONINC
void inline hook_oninc_A(_corefunc int16_t ptr) { //perform increment (A-field)
  #ifdef O_PCT
  if(l_isPCT[ptr]) {
    #ifdef EXT_PCT_a
    l_isPCT[ptr] = 0;
    #endif
    return;
  }
  #endif
  l_core1[ptr]._A++;
  BOUND_CORESIZE(l_core1[ptr]._A);
  #if defined(_COREVIEW_)
  l_coreviewdata[ptr].warrior = l_hook_lastw_ordered;
  l_coreviewdata[ptr].type = CVCT_INC;
  #endif
  return;
}
void inline hook_oninc_B(_corefunc int16_t ptr) { //perform increment (B-field)
  #ifdef O_PCT
  if(l_isPCT[ptr]) {
    #ifdef EXT_PCT_a
    l_isPCT[ptr] = 0;
    #endif
    return;
  }
  #endif
  l_core1[ptr]._B++;
  BOUND_CORESIZE(l_core1[ptr]._B);
  #if defined(_COREVIEW_)
  l_coreviewdata[ptr].warrior = l_hook_lastw_ordered;
  l_coreviewdata[ptr].type = CVCT_INC;
  #endif
  return;
}
#else
#define hook_oninc_A(x) l_core1[x]._A++; BOUND_CORESIZE(l_core1[x]._A)
#define hook_oninc_B(x) l_core1[x]._B++; BOUND_CORESIZE(l_core1[x]._B)
#endif

#if defined(O_PCT) || defined(_COREVIEW_)
#define HOOK_ONDEC
void inline hook_ondec_A(_corefunc int16_t ptr) { //perform decrement (A-field)
  #ifdef O_PCT
  if(l_isPCT[ptr]) {
    #ifdef EXT_PCT_a
    l_isPCT[ptr] = 0;
    #endif
    return;
  }
  #endif
  --l_core1[ptr]._A;
  BOUND_CORESIZE(l_core1[ptr]._A);
  #if defined(_COREVIEW_)
  l_coreviewdata[ptr].warrior = l_hook_lastw_ordered;
  l_coreviewdata[ptr].type = CVCT_DEC;
  #endif
  return;
}
void inline hook_ondec_B(_corefunc int16_t ptr) { //perform decrement (B-field)
  #ifdef O_PCT
  if(l_isPCT[ptr]) {
    #ifdef EXT_PCT_a
    l_isPCT[ptr] = 0;
    #endif
    return;
  }
  #endif
  --l_core1[ptr]._B;
  BOUND_CORESIZE(l_core1[ptr]._B);
  #if defined(_COREVIEW_)
  l_coreviewdata[ptr].warrior = l_hook_lastw_ordered;
  l_coreviewdata[ptr].type = CVCT_DEC;
  #endif
  return;
}
#else
#define hook_ondec_A(x) --l_core1[x]._A; BOUND_CORESIZE(l_core1[x]._A);
#define hook_ondec_B(x) --l_core1[x]._B; BOUND_CORESIZE(l_core1[x]._B);
#endif

#if PSPACESIZE
#if defined(_COREVIEW_not_yet)
#define HOOK_ONPREAD
uint16_t inline hook_onpread(unsigned long w, int16_t pos) { //return value
  return (pos)? warriors[w].pspace[pos] : warriors[w].psp0;
}
#else
#define hook_onpread(w, x) (x)? warriors[w].pspace[x] : warriors[w].psp0
#endif
#if defined(_COREVIEW_not_yet)
#define HOOK_ONPWRITE
void inline hook_onpwrite(unsigned long w, int16_t pos, int16_t val) { //write value to P-space
  if(pos) warriors[w].pspace[pos] = val;
  return;
}
#else
#define hook_onpwrite(w, x, y) if(x) warriors[w].pspace[x] = y
#endif
#endif

inline int16_t BOUND_DOWN(int16_t x) {
  int16_t r;
  r = x;
  if(r >= CORESIZE) r -= CORESIZE;
  return r;
}

inline int16_t BOUND_UP(int16_t x) {
  int16_t r;
  r = x;
  if(r < 0) r += CORESIZE;
  return r;
}

#define S16UDS_BINS 16
#define S16UDS_ALLOC 8
void sort16u_desc_sparse(uint16_t* buf, unsigned int num, unsigned int min, unsigned int range) {
  if(num <= 2) {
    switch(num) {
      case 1: return;
      case 2:
        if(buf[0] < buf[1]) {
          uint16_t tmp = buf[0];
          buf[0] = buf[1];
          buf[1] = tmp;
        }
        return;
    }
  } else {
    unsigned int* nums = malloc(S16UDS_BINS * sizeof(int));
    unsigned int* mins = malloc(S16UDS_BINS * sizeof(int));
    unsigned int* maxs = malloc(S16UDS_BINS * sizeof(int));
    uint16_t** bins = malloc(S16UDS_BINS * sizeof(void*));
    memset(bins, 0, S16UDS_BINS * sizeof(void*));
    int c;
    for(c = 0; c < num; ++c) {
      unsigned int bin;
      bin = (buf[c] - min)*(S16UDS_BINS-1) / (range+1);
      if(bins[bin] == NULL) {
        bins[bin] = malloc(S16UDS_ALLOC * 2);
        nums[bin] = 0;
        mins[bin] = (unsigned) -1;
        maxs[bin] = 0;
      }
      bins[bin][nums[bin]] = buf[c];
      if(buf[c] > maxs[bin]) maxs[bin] = buf[c];
      if(buf[c] < mins[bin]) mins[bin] = buf[c];
      ++nums[bin];
      if(!(nums[bin] % S16UDS_ALLOC)) {
        bins[bin] = realloc(bins[bin], nums[bin] + S16UDS_ALLOC);
      }
    }
    unsigned int n = 0;
    for(c = S16UDS_BINS - 1; c >= 0; --c) {
      if(bins[c] != NULL) {
        sort16u_desc_sparse(bins[c], nums[c], mins[c], maxs[c] - mins[c]);
        memcpy(buf + n, bins[c], nums[c] * 2);
        n += nums[c];
        free(bins[c]);
      }
    }
    free(nums);
    free(mins);
    free(maxs);
    for(c = 0; c < num; ++c) {
    }
  }
  return;
}

void place_generic(_corefun0) {
  int c;
  l_positions[0] = 0;
  unsigned long g;
  uint16_t* posbuffer = malloc(WARRIORS * 2); //element at index 0 is not used
  for(c = 1; c < WARRIORS; ++c) { //generate numbers
    posbuffer[c] = MINDISTANCE + pcg32_boundedrand_r(&randomgen, CORESIZE - MINDISTANCE*WARRIORS + 1);
  }
  //sort all positions except 0 (map all simplices to one)
  sort16u_desc_sparse(posbuffer + 1, WARRIORS - 1, MINDISTANCE, CORESIZE - MINDISTANCE*WARRIORS + 1);
  for(c = 1; c < WARRIORS-1; ++c) { //transformation matrix
    l_positions[c] = posbuffer[c] - posbuffer[c+1] + MINDISTANCE;
  }
  if(WARRIORS > 1) l_positions[WARRIORS-1] = posbuffer[WARRIORS-1];
  free(posbuffer);

  g = 0;
  for(c = 1; c < WARRIORS; ++c) { //distances to positions
    g += l_positions[c];
    l_positions[c] = g;
  }

  for(c = WARRIORS-1; c; --c) { //Fisher-Yates shuffling algorithm (shuffle warriors)
    unsigned long c2 = pcg32_boundedrand_r(&randomgen, c+1);
    uint16_t tmp = l_positions[c];
    l_positions[c] = l_positions[c2];
    l_positions[c2] = tmp;
  }
  return;
}

/*
███████ ██    ██ ███    ██  ██████ ████████ ██  ██████  ███    ██ ███████      ██  ██ ██
██      ██    ██ ████   ██ ██         ██    ██ ██    ██ ████   ██ ██          ██  ███  ██
█████   ██    ██ ██ ██  ██ ██         ██    ██ ██    ██ ██ ██  ██ ███████     ██   ██  ██
██      ██    ██ ██  ██ ██ ██         ██    ██ ██    ██ ██  ██ ██      ██     ██   ██  ██
██       ██████  ██   ████  ██████    ██    ██  ██████  ██   ████ ███████      ██  ██ ██
*/

void place1(_corefun0) {
  int c;
  place_generic(_corecal0); //set positions

  for(c = 0; c < WARRIORS; ++c) {
    if(l_positions[c] + warriors[c].len <= CORESIZE) {
      memcpy(&l_core1[l_positions[c]], warriors[c].code1, warriors[c].len * sizeof(INSTR1));
      #ifdef _COREVIEW_
      int k;
      for(k = 0; k < warriors[c].len; ++k) {
        l_coreviewdata[l_positions[c] + k].opcode = warriors[c].code1[k]._O;
      }
      #endif
    }
    else {
      memcpy(&l_core1[l_positions[c]], warriors[c].code1, (CORESIZE - l_positions[c]) * sizeof(INSTR1));
      memcpy(l_core1, &warriors[c].code1[CORESIZE - l_positions[c]], (warriors[c].len - CORESIZE + l_positions[c]) * sizeof(INSTR1));
      #ifdef _COREVIEW_
      int k;
      for(k = 0; k < (CORESIZE - l_positions[c]); ++k) {
        l_coreviewdata[l_positions[c] + k].opcode = warriors[c].code1[k]._O;
      }
      for(/*current k value*/; k < warriors[c].len; ++k) {
        l_coreviewdata[k - (CORESIZE - l_positions[c])].opcode = warriors[c].code1[k]._O;
      }
      #endif
    }
  }
  return;
}

void simulate1(_corefun0) {
  //place warriors
  place1(_corecal0);

  //init process queues
  l_nrunning = WARRIORS;
  unsigned long c;
  for(c = 0; c < WARRIORS; ++c) {
    l_proc1[c] = alloc_pool_proc1();
    l_proc1[c]->pos = l_positions[c] + warriors[c].org;
    if(l_proc1[c]->pos >= CORESIZE) l_proc1[c]->pos -= CORESIZE;
    l_proc1[c]->next = l_proc1[c]->prev = l_proc1[c];
    l_nprocs[c] = 1;
    l_indices[c] = c;

    l_running[c] = 1;
  }
  for(c = WARRIORS-1; c; --c) { //Fisher-Yates shuffling algorithm
    unsigned long c2 = pcg32_boundedrand_r(&randomgen, c+1);
    PROC1* tmp = l_proc1[c];
    l_proc1[c] = l_proc1[c2];
    l_proc1[c2] = tmp;
    unsigned long tmp2 = l_nprocs[c];
    l_nprocs[c] = l_nprocs[c2];
    l_nprocs[c2] = tmp2;
    tmp2 = l_indices[c];
    l_indices[c] = l_indices[c2];
    l_indices[c2] = tmp2;
  }

  #ifdef O_PCT
  memset(l_isPCT, 0, CORESIZE * sizeof(uint_fast8_t)); //unprotected
  #endif

  #if defined(TSAFE_CORE) && PSPACESIZE
  l_pspace_local_accessed = 0;
  #endif

  //run in a loop
  for(c = 0; c < MAXCYCLES; ++c) {
    unsigned long w;
    for(w = 0; w < l_nrunning; ++w) {
      int16_t pc = l_proc1[w]->pos;
      #ifdef _COREVIEW_
      if(check_terminate()) {
        mlock(l_mutex_mode);
        if(l_runmode == RUN_FAST && l_rundata > 0) munlock(l_mutex_exec);
        munlock(l_mutex_mode);
        return;
      }
      #endif
      #ifdef HOOK_ONEXEC
      hook_onexec(_corecall pc, w);
      #else
      hook_onexec(pc, w);
      #endif
      INSTR1 I;
      I._I = l_core1[pc]._I;
      //printf("[##] "); debug_println1(l_core1[##]._I); //D
      //debug_println1(I._I); //D
      int16_t ap = 0;
      INSTR1 ai;
      #ifdef A_ADB
      int16_t adpA = -1;
      #endif
      switch(I._aA) {
        #ifdef A_IMM
        case A_IMM:
          ap = pc;
          #ifdef HOOK_ONREAD
          ai._I = hook_onread(_corecall pc);
          #else
          ai._I = I._I;
          #endif
          break;
        #endif
        #ifdef A_DIR
        case A_DIR:
          ap = I._A + pc;
          BOUND_CORESIZE(ap)
          ai._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall ap);
          #else
            hook_onread(ap);
          #endif
          break;
        #endif
        #ifdef A_INA
        case A_INA:
          ap = I._A + pc;
          BOUND_CORESIZE(ap);
          ap = l_core1[ap]._A + ap;
          BOUND_CORESIZE(ap);
          ai._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall ap);
          #else
            hook_onread(ap);
          #endif
          break;
        #endif
        #ifdef A_INB
        case A_INB:
          ap = I._A + pc;
          BOUND_CORESIZE(ap);
          ap = l_core1[ap]._B + ap;
          BOUND_CORESIZE(ap);
          ai._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall ap);
          #else
            hook_onread(ap);
          #endif
          break;
        #endif
        #ifdef A_PDA
        case A_PDA:
          ap = I._A + pc;
          BOUND_CORESIZE(ap);
          #ifdef HOOK_ONDEC
          hook_ondec_A(_corecall ap);
          #else
          hook_ondec_A(ap);
          #endif
          ap = l_core1[ap]._A + ap;
          BOUND_CORESIZE(ap);
          ai._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall ap);
          #else
            hook_onread(ap);
          #endif
          break;
        #endif
        #ifdef A_PDB
        case A_PDB:
          ap = I._A + pc;
          BOUND_CORESIZE(ap);
          #ifdef HOOK_ONDEC
          hook_ondec_B(_corecall ap);
          #else
          hook_ondec_B(ap);
          #endif
          ap = l_core1[ap]._B + ap;
          BOUND_CORESIZE(ap);
          ai._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall ap);
          #else
            hook_onread(ap);
          #endif
          break;
        #endif
        #ifdef A_ADB
        case A_ADB:
          #error "Auto-decrement implementation in progress"
          ap = I._A + pc;
          BOUND_CORESIZE(ap);
          adpA = ap;
          ap = l_core1[ap]._B - 1 + ap;
          BOUND_CORESIZE(ap);
          ai._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall ap);
          #else
            hook_onread(ap);
          #endif
          break;
        #endif
        #ifdef A_PIA
        case A_PIA: {
          int16_t ap2 = I._A + pc;
          BOUND_CORESIZE(ap2);
          ap = l_core1[ap2]._A + ap2;
          BOUND_CORESIZE(ap);
          ai._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall ap);
          #else
            hook_onread(ap);
          #endif
          #ifdef HOOK_ONINC
          hook_oninc_A(_corecall ap2);
          #else
          hook_oninc_A(ap2);
          #endif
          break; }
        #endif
        #ifdef A_PIB
        case A_PIB: {
          int16_t ap2 = I._A + pc;
          BOUND_CORESIZE(ap2);
          ap = l_core1[ap2]._B + ap2;
          BOUND_CORESIZE(ap);
          ai._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall ap);
          #else
            hook_onread(ap);
          #endif
          #ifdef HOOK_ONINC
          hook_oninc_B(_corecall ap2);
          #else
          hook_oninc_B(ap2);
          #endif
          break; }
        #endif
        default: HINT_UNREACHABLE();
      }
      COMPILER_HINT(ap >= 0 && ap < CORESIZE);
      COMPILER_HINT(ai._A >= 0 && ai._A < CORESIZE);
      COMPILER_HINT(ai._B >= 0 && ai._B < CORESIZE);
      int16_t bp = 0;
      INSTR1 bi;
      #ifdef A_ADB
      int16_t adpB = -1;
      #endif
      switch(I._aB) {
        #ifdef A_IMM
        case A_IMM:
          bp = pc;
          #ifdef HOOK_ONREAD
          bi._I = hook_onread(_corecall pc);
          #else
          bi._I = I._I;
          #endif
          break;
        #endif
        #ifdef A_DIR
        case A_DIR:
          bp = I._B + pc;
          BOUND_CORESIZE(bp)
          bi._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall bp);
          #else
            hook_onread(bp);
          #endif
          break;
        #endif
        #ifdef A_INA
        case A_INA:
          bp = I._B + pc;
          BOUND_CORESIZE(bp);
          bp = l_core1[bp]._A + bp;
          BOUND_CORESIZE(bp);
          bi._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall bp);
          #else
            hook_onread(bp);
          #endif
          break;
        #endif
        #ifdef A_INB
        case A_INB:
          bp = I._B + pc;
          BOUND_CORESIZE(bp);
          bp = l_core1[bp]._B + bp;
          BOUND_CORESIZE(bp);
          bi._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall bp);
          #else
            hook_onread(bp);
          #endif
          break;
        #endif
        #ifdef A_PDA
        case A_PDA:
          bp = I._B + pc;
          BOUND_CORESIZE(bp);
          #ifdef HOOK_ONDEC
          hook_ondec_A(_corecall bp);
          #else
          hook_ondec_A(bp);
          #endif
          bp = l_core1[bp]._A + bp;
          BOUND_CORESIZE(bp);
          bi._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall bp);
          #else
            hook_onread(bp);
          #endif
          break;
        #endif
        #ifdef A_PDB
        case A_PDB:
          bp = I._B + pc;
          BOUND_CORESIZE(bp);
          #ifdef HOOK_ONDEC
          hook_ondec_B(_corecall bp);
          #else
          hook_ondec_B(bp);
          #endif
          bp = l_core1[bp]._B + bp;
          BOUND_CORESIZE(bp);
          bi._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall bp);
          #else
            hook_onread(bp);
          #endif
          break;
        #endif
        #ifdef A_ADB
        case A_ADB:
          #error "Auto-decrement implementation in progress"
          bp = I._B + pc;
          BOUND_CORESIZE(bp);
          adpB = bp;
          bp = l_core1[bp]._B - 1 + bp;
          BOUND_CORESIZE(bp);
          bi._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall bp);
          #else
            hook_onread(bp);
          #endif
          break;
        #endif
        #ifdef A_PIA
        case A_PIA: {
          int16_t bp2 = I._B + pc;
          BOUND_CORESIZE(bp2);
          bp = l_core1[bp2]._A + bp2;
          BOUND_CORESIZE(bp);
          bi._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall bp);
          #else
            hook_onread(bp);
          #endif
          #ifdef HOOK_ONINC
          hook_oninc_A(_corecall bp2);
          #else
          hook_oninc_A(bp2);
          #endif
          break; }
        #endif
        #ifdef A_PIB
        case A_PIB: {
          int16_t bp2 = I._B + pc;
          BOUND_CORESIZE(bp2);
          bp = l_core1[bp2]._B + bp2;
          BOUND_CORESIZE(bp);
          bi._I =
          #ifdef HOOK_ONREAD
            hook_onread(_corecall bp);
          #else
            hook_onread(bp);
          #endif
          #ifdef HOOK_ONINC
          hook_oninc_B(_corecall bp2);
          #else
          hook_oninc_B(bp2);
          #endif
          break; }
        #endif
        default: HINT_UNREACHABLE();
      }
      COMPILER_HINT(bp >= 0 && bp < CORESIZE);
      COMPILER_HINT(bi._A >= 0 && bi._A < CORESIZE);
      COMPILER_HINT(bi._B >= 0 && bi._B < CORESIZE);
      #ifdef A_ADB
      if(adpA >= 0) {
        #ifdef HOOK_ONDEC
        hook_ondec_B(_corecall adpA);
        #else
        hook_ondec_B(adpA);
        #endif
      }
      if(adpB >= 0) {
        #ifdef HOOK_ONDEC
        hook_ondec_B(_corecall adpB);
        #else
        hook_ondec_B(adpB);
        #endif
      }
      #endif

      #ifdef HOOK_ONENDEXEC
      #define ENDINSTR() hook_onendexec(_corecall pc);
      #else
      #define ENDINSTR() hook_onendexec(pc);
      #endif
      #define REMOVE_PROC() { \
        --l_nprocs[w]; \
        if(!l_nprocs[w]) { /*warrior loses*/ \
          free_pool_proc1(l_proc1[w]); \
          l_running[l_indices[w]] = 0; \
          --l_nrunning; \
          if(l_nrunning == 1) { \
            ENDINSTR(); /*Otherwise, it would not be executed*/ \
            goto _label_endbattle; \
          } \
          unsigned long v; \
          for(v = w; v < l_nrunning; ++v) { \
            if(v+1 >= WARRIORS) __builtin_unreachable(); /*GCC builtin*/ \
            l_proc1[v] = l_proc1[v+1]; \
            l_nprocs[v] = l_nprocs[v+1]; \
            l_indices[v] = l_indices[v+1]; \
          } \
        } \
        else { \
          l_proc1[w]->prev->next = l_proc1[w]->next; \
          l_proc1[w]->next->prev = l_proc1[w]->prev; \
          free_pool_proc1(l_proc1[w]); /*note that the memory is not actually free'd*/ \
          l_proc1[w] = l_proc1[w]->next; /*it can be used safely afterwards*/ \
        }}
      switch(I._O) {
        #ifdef O_DAT
        case O_DAT:
          REMOVE_PROC();
          break;
        #endif
        #ifdef O_MOV
        case O_MOV:
          #ifdef HOOK_ONWRITE
          switch(I._M) {
            case M_I: hook_onwrite_I(_corecall bp, ai._I); break;
            case M_A: hook_onwrite_A(_corecall bp, ai._A); break;
            case M_B: hook_onwrite_B(_corecall bp, ai._B); break;
            case M_AB: hook_onwrite_B(_corecall bp, ai._A); break;
            case M_BA: hook_onwrite_A(_corecall bp, ai._B); break;
            case M_F: hook_onwrite_F(_corecall bp, ai._F); break;
            case M_X:
              hook_onwrite_A(_corecall bp, ai._B);
              hook_onwrite_B(_corecall bp, ai._A);
              break;
          }
          #else
          switch(I._M) {
            case M_I: hook_onwrite_I(bp, ai._I); break;
            case M_A: hook_onwrite_A(bp, ai._A); break;
            case M_B: hook_onwrite_B(bp, ai._B); break;
            case M_AB: hook_onwrite_B(bp, ai._A); break;
            case M_BA: hook_onwrite_A(bp, ai._B); break;
            case M_F: hook_onwrite_F(bp, ai._F); break;
            case M_X:
              hook_onwrite_A(bp, ai._B);
              hook_onwrite_B(bp, ai._A);
              break;
          }
          #endif
          l_proc1[w]->pos++;
          BOUND_CORESIZE(l_proc1[w]->pos);
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_ADD
        case O_ADD:
          #ifdef HOOK_ONWRITE
          switch(I._M) {
            case M_A: hook_onwrite_A(_corecall bp, BOUND_DOWN(bi._A + ai._A)); break;
            case M_B: hook_onwrite_B(_corecall bp, BOUND_DOWN(bi._B + ai._B)); break;
            case M_AB: hook_onwrite_B(_corecall bp, BOUND_DOWN(bi._B + ai._A)); break;
            case M_BA: hook_onwrite_A(_corecall bp, BOUND_DOWN(bi._A + ai._B)); break;
            case M_F: case M_I:
              hook_onwrite_A(_corecall bp, BOUND_DOWN(bi._A + ai._A));
              hook_onwrite_B(_corecall bp, BOUND_DOWN(bi._B + ai._B));
              break;
            case M_X:
              hook_onwrite_A(_corecall bp, BOUND_DOWN(bi._A + ai._B));
              hook_onwrite_B(_corecall bp, BOUND_DOWN(bi._B + ai._A));
              break;
          }
          #else
          switch(I._M) {
            case M_A: hook_onwrite_A(bp, BOUND_DOWN(bi._A + ai._A)); break;
            case M_B: hook_onwrite_B(bp, BOUND_DOWN(bi._B + ai._B)); break;
            case M_AB: hook_onwrite_B(bp, BOUND_DOWN(bi._B + ai._A)); break;
            case M_BA: hook_onwrite_A(bp, BOUND_DOWN(bi._A + ai._B)); break;
            case M_F: case M_I:
              hook_onwrite_A(bp, BOUND_DOWN(bi._A + ai._A));
              hook_onwrite_B(bp, BOUND_DOWN(bi._B + ai._B));
              break;
            case M_X:
              hook_onwrite_A(bp, BOUND_DOWN(bi._A + ai._B));
              hook_onwrite_B(bp, BOUND_DOWN(bi._B + ai._A));
              break;
          }
          #endif
          l_proc1[w]->pos++;
          BOUND_CORESIZE(l_proc1[w]->pos);
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_SUB
        case O_SUB:
          #ifdef HOOK_ONWRITE
          switch(I._M) {
            case M_A: hook_onwrite_A(_corecall bp, BOUND_UP(bi._A - ai._A)); break;
            case M_B: hook_onwrite_B(_corecall bp, BOUND_UP(bi._B - ai._B)); break;
            case M_AB: hook_onwrite_B(_corecall bp, BOUND_UP(bi._B - ai._A)); break;
            case M_BA: hook_onwrite_A(_corecall bp, BOUND_UP(bi._A - ai._B)); break;
            case M_F: case M_I:
              hook_onwrite_A(_corecall bp, BOUND_UP(bi._A - ai._A));
              hook_onwrite_B(_corecall bp, BOUND_UP(bi._B - ai._B));
              break;
            case M_X:
              hook_onwrite_A(_corecall bp, BOUND_UP(bi._A - ai._B));
              hook_onwrite_B(_corecall bp, BOUND_UP(bi._B - ai._A));
              break;
          }
          #else
          switch(I._M) {
            case M_A: hook_onwrite_A(bp, BOUND_UP(bi._A - ai._A)); break;
            case M_B: hook_onwrite_B(bp, BOUND_UP(bi._B - ai._B)); break;
            case M_AB: hook_onwrite_B(bp, BOUND_UP(bi._B - ai._A)); break;
            case M_BA: hook_onwrite_A(bp, BOUND_UP(bi._A - ai._B)); break;
            case M_F: case M_I:
              hook_onwrite_A(bp, BOUND_UP(bi._A - ai._A));
              hook_onwrite_B(bp, BOUND_UP(bi._B - ai._B));
              break;
            case M_X:
              hook_onwrite_A(bp, BOUND_UP(bi._A - ai._B));
              hook_onwrite_B(bp, BOUND_UP(bi._B - ai._A));
              break;
          }
          #endif
          l_proc1[w]->pos++;
          BOUND_CORESIZE(l_proc1[w]->pos);
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_MUL
        case O_MUL: {
          uint_fast32_t val1, val2;
          switch(I._M) {
            case M_A:
              val1 = bi._A * ai._A;
              #ifdef HOOK_ONWRITE
              hook_onwrite_A(_corecall bp, val1 % CORESIZE);
              #else
              hook_onwrite_A(bp, val1 % CORESIZE);
              #endif
              break;
            case M_B:
              val1 = bi._B * ai._B;
              #ifdef HOOK_ONWRITE
              hook_onwrite_B(_corecall bp, val1 % CORESIZE);
              #else
              hook_onwrite_B(bp, val1 % CORESIZE);
              #endif
              break;
            case M_AB:
              val1 = bi._B * ai._A;
              #ifdef HOOK_ONWRITE
              hook_onwrite_B(_corecall bp, val1 % CORESIZE);
              #else
              hook_onwrite_B(bp, val1 % CORESIZE);
              #endif
              break;
            case M_BA:
              val1 = bi._A * ai._B;
              #ifdef HOOK_ONWRITE
              hook_onwrite_A(_corecall bp, val1 % CORESIZE);
              #else
              hook_onwrite_A(bp, val1 % CORESIZE);
              #endif
              break;
            case M_F: case M_I:
              val1 = bi._A * ai._A;
              val2 = bi._B * ai._B;
              #ifdef HOOK_ONWRITE
              hook_onwrite_A(_corecall bp, val1 % CORESIZE);
              #else
              hook_onwrite_A(bp, val1 % CORESIZE);
              #endif
              #ifdef HOOK_ONWRITE
              hook_onwrite_B(_corecall bp, val2 % CORESIZE);
              #else
              hook_onwrite_B(bp, val2 % CORESIZE);
              #endif
              break;
            case M_X:
              val1 = bi._A * ai._B;
              val2 = bi._B * ai._A;
              #ifdef HOOK_ONWRITE
              hook_onwrite_A(_corecall bp, val1 % CORESIZE);
              #else
              hook_onwrite_A(bp, val1 % CORESIZE);
              #endif
              #ifdef HOOK_ONWRITE
              hook_onwrite_B(_corecall bp, val2 % CORESIZE);
              #else
              hook_onwrite_B(bp, val2 % CORESIZE);
              #endif
              break;
          }
          l_proc1[w]->pos++;
          BOUND_CORESIZE(l_proc1[w]->pos);
          l_proc1[w] = l_proc1[w]->next;
          break; }
        #endif
        #ifdef O_DIV
        case O_DIV: {
          int chkend = 0;
          switch(I._M) {
            case M_A:
              if(ai._A) {
                #ifdef HOOK_ONWRITE
              hook_onwrite_A(_corecall bp, bi._A / ai._A);
              #else
              hook_onwrite_A(bp, bi._A / ai._A);
              #endif
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
                l_proc1[w] = l_proc1[w]->next;
              }
              else REMOVE_PROC();
              break;
            case M_B:
              if(ai._B) {
                #ifdef HOOK_ONWRITE
              hook_onwrite_B(_corecall bp, bi._B / ai._B);
              #else
              hook_onwrite_B(bp, bi._B / ai._B);
              #endif
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
                l_proc1[w] = l_proc1[w]->next;
              }
              else REMOVE_PROC();
              break;
            case M_AB:
              if(ai._A) {
                #ifdef HOOK_ONWRITE
              hook_onwrite_B(_corecall bp, bi._B / ai._A);
              #else
              hook_onwrite_B(bp, bi._B / ai._A);
              #endif
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
                l_proc1[w] = l_proc1[w]->next;
              }
              else REMOVE_PROC();
              break;
            case M_BA:
              if(ai._B) {
                #ifdef HOOK_ONWRITE
              hook_onwrite_A(_corecall bp, bi._A / ai._B);
              #else
              hook_onwrite_A(bp, bi._A / ai._B);
              #endif
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
                l_proc1[w] = l_proc1[w]->next;
              }
              else REMOVE_PROC();
              break;
            case M_F: case M_I:
              if(ai._B) {
                #ifdef HOOK_ONWRITE
                hook_onwrite_B(_corecall bp, bi._B / ai._B);
                #else
                hook_onwrite_B(bp, bi._B / ai._B);
                #endif
              }
              else chkend = 1;
              if(ai._A) {
                #ifdef HOOK_ONWRITE
                hook_onwrite_A(_corecall bp, bi._A / ai._A);
                #else
                hook_onwrite_A(bp, bi._A / ai._A);
                #endif
              }
              else chkend = 1;
              if(chkend) {REMOVE_PROC();}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
                l_proc1[w] = l_proc1[w]->next;
              }
              break;
            case M_X:
              if(ai._B) {
                #ifdef HOOK_ONWRITE
                hook_onwrite_A(_corecall bp, bi._A / ai._B);
                #else
                hook_onwrite_A(bp, bi._A / ai._B);
                #endif
              }
              else chkend = 1;
              if(ai._A) {
                #ifdef HOOK_ONWRITE
                hook_onwrite_B(_corecall bp, bi._B / ai._A);
                #else
                hook_onwrite_B(bp, bi._B / ai._A);
                #endif
              }
              else chkend = 1;
              if(chkend) {REMOVE_PROC();}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
                l_proc1[w] = l_proc1[w]->next;
              }
              break;
          }
          break; }
        #endif
        #ifdef O_MOD
        case O_MOD: {
          int chkend = 0;
          switch(I._M) {
            case M_A:
              if(ai._A) {
                #ifdef HOOK_ONWRITE
                hook_onwrite_A(_corecall bp, bi._A % ai._A);
                #else
                hook_onwrite_A(bp, bi._A % ai._A);
                #endif
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
                l_proc1[w] = l_proc1[w]->next;
              }
              else REMOVE_PROC();
              break;
            case M_B:
              if(ai._B) {
                #ifdef HOOK_ONWRITE
                hook_onwrite_B(_corecall bp, bi._B % ai._B);
                #else
                hook_onwrite_B(bp, bi._B % ai._B);
                #endif
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
                l_proc1[w] = l_proc1[w]->next;
              }
              else REMOVE_PROC();
              break;
            case M_AB:
              if(ai._A) {
                #ifdef HOOK_ONWRITE
                hook_onwrite_B(_corecall bp, bi._B % ai._A);
                #else
                hook_onwrite_B(bp, bi._B % ai._A);
                #endif
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
                l_proc1[w] = l_proc1[w]->next;
              }
              else REMOVE_PROC();
              break;
            case M_BA:
              if(ai._B) {
                #ifdef HOOK_ONWRITE
                hook_onwrite_A(_corecall bp, bi._A % ai._B);
                #else
                hook_onwrite_A(bp, bi._A % ai._B);
                #endif
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
                l_proc1[w] = l_proc1[w]->next;
              }
              else REMOVE_PROC();
              break;
            case M_F: case M_I:
              if(ai._B) {
                #ifdef HOOK_ONWRITE
                hook_onwrite_B(_corecall bp, bi._B % ai._B);
                #else
                hook_onwrite_B(bp, bi._B % ai._B);
                #endif
              }
              else chkend = 1;
              if(ai._A) {
                #ifdef HOOK_ONWRITE
                hook_onwrite_A(_corecall bp, bi._A % ai._A);
                #else
                hook_onwrite_A(bp, bi._A % ai._A);
                #endif
              }
              else chkend = 1;
              if(chkend) {REMOVE_PROC();}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
                l_proc1[w] = l_proc1[w]->next;
              }
              break;
            case M_X:
              if(ai._B) {
                #ifdef HOOK_ONWRITE
                hook_onwrite_A(_corecall bp, bi._A % ai._B);
                #else
                hook_onwrite_A(bp, bi._A % ai._B);
                #endif
              }
              else chkend = 1;
              if(ai._A) {
                #ifdef HOOK_ONWRITE
                hook_onwrite_B(_corecall bp, bi._B % ai._A);
                #else
                hook_onwrite_B(bp, bi._B % ai._A);
                #endif
              }
              else chkend = 1;
              if(chkend) {REMOVE_PROC();}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
                l_proc1[w] = l_proc1[w]->next;
              }
              break;
          }
          break; }
        #endif
        #ifdef O_JMP
        case O_JMP:
          l_proc1[w]->pos = ap;
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_JMZ
        case O_JMZ:
          switch(I._M) {
            case M_A: case M_BA:
              if(!bi._A) {l_proc1[w]->pos = ap;}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
              }
              break;
            case M_B: case M_AB:
              if(!bi._B) {l_proc1[w]->pos = ap;}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
              }
              break;
            case M_F: case M_I: case M_X:
              if(!bi._A && !bi._B) {l_proc1[w]->pos = ap;}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
              }
              break;
          }
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_JMN
        case O_JMN:
          switch(I._M) {
            case M_A: case M_BA:
              if(bi._A) {l_proc1[w]->pos = ap;}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
              }
              break;
            case M_B: case M_AB:
              if(bi._B) {l_proc1[w]->pos = ap;}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
              }
              break;
            case M_F: case M_I: case M_X:
              if(bi._A || bi._B) {l_proc1[w]->pos = ap;}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
              }
              break;
          }
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_DJZ
        case O_DJZ:
          switch(I._M) {
            case M_A: case M_BA:
              if(!--bi._A) {l_proc1[w]->pos = ap;}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
              }
              #ifdef HOOK_ONDEC
              hook_ondec_A(_corecall bp);
              #else
              hook_ondec_A(bp);
              #endif
              break;
            case M_B: case M_AB:
              if(!--bi._B) {l_proc1[w]->pos = ap;}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
              }
              #ifdef HOOK_ONDEC
              hook_ondec_B(_corecall bp);
              #else
              hook_ondec_B(bp);
              #endif
              break;
            case M_F: case M_X: case M_I:
              if(!--bi._A && !--bi._B) {l_proc1[w]->pos = ap;}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
              }
              #ifdef HOOK_ONDEC
              hook_ondec_A(_corecall bp);
              #else
              hook_ondec_A(bp);
              #endif
              #ifdef HOOK_ONDEC
              hook_ondec_B(_corecall bp);
              #else
              hook_ondec_B(bp);
              #endif
              break;
          }
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_DJN
        case O_DJN:
          switch(I._M) {
            case M_A: case M_BA:
              if(--bi._A) {l_proc1[w]->pos = ap;}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
              }
              #ifdef HOOK_ONDEC
              hook_ondec_A(_corecall bp);
              #else
              hook_ondec_A(bp);
              #endif
              break;
            case M_B: case M_AB:
              if(--bi._B) {l_proc1[w]->pos = ap;}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
              }
              #ifdef HOOK_ONDEC
              hook_ondec_B(_corecall bp);
              #else
              hook_ondec_B(bp);
              #endif
              break;
            case M_F: case M_X: case M_I:
              if(--bi._A || --bi._B) {l_proc1[w]->pos = ap;}
              else {
                l_proc1[w]->pos++;
                BOUND_CORESIZE(l_proc1[w]->pos);
              }
              #ifdef HOOK_ONDEC
              hook_ondec_A(_corecall bp);
              #else
              hook_ondec_A(bp);
              #endif
              #ifdef HOOK_ONDEC
              hook_ondec_B(_corecall bp);
              #else
              hook_ondec_B(bp);
              #endif
              break;
          }
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #if defined(O_CMP) || defined(O_SEQ)
        #ifdef O_CMP
        case O_CMP:
        #else
        case O_SEQ:
        #endif
          switch(I._M) {
            case M_I:
              if(ai._I == bi._I) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_A:
              if(ai._A == bi._A) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_B:
              if(ai._B == bi._B) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_AB:
              if(ai._A == bi._B) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_BA:
              if(ai._B == bi._A) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_F:
              if(ai._F == bi._F) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_X:
              if(ai._A == bi._B && ai._B == bi._A) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
          }
          BOUND_CORESIZE(l_proc1[w]->pos);
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_SNE
        case O_SNE:
          switch(I._M) {
            case M_I:
              if(ai._I != bi._I) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_A:
              if(ai._A != bi._A) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_B:
              if(ai._B != bi._B) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_AB:
              if(ai._A != bi._B) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_BA:
              if(ai._B != bi._A) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_F:
              if(ai._F != bi._F) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_X:
              if(ai._A != bi._B || ai._B != bi._A) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
          }
          BOUND_CORESIZE(l_proc1[w]->pos);
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_SLT
        case O_SLT:
          switch(I._M) {
            case M_A:
              if(ai._A < bi._A) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_B:
              if(ai._B < bi._B) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_AB:
              if(ai._A < bi._B) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_BA:
              if(ai._B < bi._A) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_F: case M_I:
              if(ai._A < bi._A && ai._B < bi._B) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
            case M_X:
              if(ai._A < bi._B && ai._B < bi._A) l_proc1[w]->pos += 2;
              else l_proc1[w]->pos++;
              break;
          }
          BOUND_CORESIZE(l_proc1[w]->pos);
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_SPL
        case O_SPL:
          if(l_nprocs[w] < MAXPROCESSES) {
            PROC1* newproc = alloc_pool_proc1();
            #ifdef opt_SPL86
            newproc->pos = bp;
            #else
            newproc->pos = ap; //SIGSEGV occurs here
            #endif
            #ifdef opt_SPL86
            newproc->next = l_proc1[w];
            newproc->prev = l_proc1[w]->prev;
            l_proc1[w]->prev->next = newproc;
            l_proc1[w]->prev = newproc;
            l_proc1[w]->pos++;
            BOUND_CORESIZE(l_proc1[w]->pos);
            l_proc1[w] = l_proc1[w]->next;
            #else
            newproc->prev = l_proc1[w];
            newproc->next = l_proc1[w]->next;
            l_proc1[w]->next->prev = newproc;
            l_proc1[w]->next = newproc;
            l_proc1[w]->pos++;
            BOUND_CORESIZE(l_proc1[w]->pos);
            l_proc1[w] = newproc->next;
            #endif
            ++l_nprocs[w];
          }
          else {
            l_proc1[w]->pos++;
            BOUND_CORESIZE(l_proc1[w]->pos);
            l_proc1[w] = l_proc1[w]->next;
          }
          break;
        #endif
        #ifdef O_NOP
        case O_NOP:
          l_proc1[w]->pos++;
          BOUND_CORESIZE(l_proc1[w]->pos);
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_LDP
        case O_LDP:
          #ifdef TSAFE_CORE
          if(!l_pspace_local_accessed) {
            mlock(mutex_pwarriors);
            l_pspace_local_accessed = 1;
          }
          #endif
          #ifdef HOOK_ONWRITE
          switch(I._M) {
            case M_A: hook_onwrite_A(_corecall bp, hook_onpread(w, ai._A % PSPACESIZE)); break;
            case M_B: case M_I: case M_F: case M_X: hook_onwrite_B(_corecall bp, hook_onpread(w, ai._B % PSPACESIZE)); break;
            case M_AB: hook_onwrite_B(_corecall bp, hook_onpread(w, ai._A % PSPACESIZE)); break;
            case M_BA: hook_onwrite_A(_corecall bp, hook_onpread(w, ai._B % PSPACESIZE)); break;
          }
          #else
          switch(I._M) {
            case M_A: hook_onwrite_A(bp, hook_onpread(w, ai._A % PSPACESIZE)); break;
            case M_B: case M_I: case M_F: case M_X: hook_onwrite_B(bp, hook_onpread(w, ai._B % PSPACESIZE)); break;
            case M_AB: hook_onwrite_B(bp, hook_onpread(w, ai._A % PSPACESIZE)); break;
            case M_BA: hook_onwrite_A(bp, hook_onpread(w, ai._B % PSPACESIZE)); break;
          }
          #endif
          l_proc1[w]->pos++;
          BOUND_CORESIZE(l_proc1[w]->pos);
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_STP
        case O_STP: {
          #ifdef TSAFE_CORE
          if(!l_pspace_local_accessed) {
            mlock(mutex_pwarriors);
            l_pspace_local_accessed = 1;
          }
          #endif
          int16_t pos;
          switch(I._M) {
            case M_A:
              pos = bi._A % PSPACESIZE;
              hook_onpwrite(w, pos, ai._A);
              break;
            case M_B: case M_I: case M_F: case M_X:
              pos = bi._B % PSPACESIZE;
              hook_onpwrite(w, pos, ai._B);
              break;
            case M_AB:
              pos = bi._B % PSPACESIZE;
              hook_onpwrite(w, pos, ai._A);
              break;
            case M_BA:
              pos = bi._A % PSPACESIZE;
              hook_onpwrite(w, pos, ai._B);
              break;
          }
          l_proc1[w]->pos++;
          BOUND_CORESIZE(l_proc1[w]->pos);
          l_proc1[w] = l_proc1[w]->next;
          break; }
        #endif
        #ifdef O_XCH
        case O_XCH:
          #ifdef EXT_XCH_a
          #ifdef HOOK_ONWRITE
              hook_onwrite_A(_corecall ap, ai._B);
              #else
              hook_onwrite_A(ap, ai._B);
              #endif
          #ifdef HOOK_ONWRITE
              hook_onwrite_B(_corecall ap, ai._A);
              #else
              hook_onwrite_B(ap, ai._A);
              #endif
          #else
          switch(I._M) {
            case M_I:
              #ifdef HOOK_ONWRITE
              hook_onwrite_I(_corecall ap, bi._I);
              #else
              hook_onwrite_I(ap, bi._I);
              #endif
              #ifdef HOOK_ONWRITE
              hook_onwrite_I(_corecall bp, ai._I);
              #else
              hook_onwrite_I(bp, ai._I);
              #endif
              break;
            case M_A:
              #ifdef HOOK_ONWRITE
              hook_onwrite_A(_corecall ap, bi._A);
              #else
              hook_onwrite_A(ap, bi._A);
              #endif
              #ifdef HOOK_ONWRITE
              hook_onwrite_A(_corecall bp, ai._A);
              #else
              hook_onwrite_A(bp, ai._A);
              #endif
              break;
            case M_B:
              #ifdef HOOK_ONWRITE
              hook_onwrite_B(_corecall ap, bi._B);
              #else
              hook_onwrite_B(ap, bi._B);
              #endif
              #ifdef HOOK_ONWRITE
              hook_onwrite_B(_corecall bp, ai._B);
              #else
              hook_onwrite_B(bp, ai._B);
              #endif
              break;
            case M_AB:
              #ifdef HOOK_ONWRITE
              hook_onwrite_A(_corecall ap, bi._B);
              #else
              hook_onwrite_A(ap, bi._B);
              #endif
              #ifdef HOOK_ONWRITE
              hook_onwrite_B(_corecall bp, ai._A);
              #else
              hook_onwrite_B(bp, ai._A);
              #endif
              break;
            case M_BA:
              #ifdef HOOK_ONWRITE
              hook_onwrite_B(_corecall ap, bi._A);
              #else
              hook_onwrite_B(ap, bi._A);
              #endif
              #ifdef HOOK_ONWRITE
              hook_onwrite_A(_corecall bp, ai._B);
              #else
              hook_onwrite_A(bp, ai._B);
              #endif
              break;
            case M_F:
              #ifdef HOOK_ONWRITE
              hook_onwrite_F(_corecall ap, bi._F);
              #else
              hook_onwrite_F(ap, bi._F);
              #endif
              #ifdef HOOK_ONWRITE
              hook_onwrite_F(_corecall bp, ai._F);
              #else
              hook_onwrite_F(bp, ai._F);
              #endif
              break;
            case M_X:
              #ifdef HOOK_ONWRITE
              hook_onwrite_A(_corecall ap, bi._B);
              #else
              hook_onwrite_A(ap, bi._B);
              #endif
              #ifdef HOOK_ONWRITE
              hook_onwrite_B(_corecall ap, bi._A);
              #else
              hook_onwrite_B(ap, bi._A);
              #endif
              #ifdef HOOK_ONWRITE
              hook_onwrite_A(_corecall bp, ai._B);
              #else
              hook_onwrite_A(bp, ai._B);
              #endif
              #ifdef HOOK_ONWRITE
              hook_onwrite_B(_corecall bp, ai._A);
              #else
              hook_onwrite_B(bp, ai._A);
              #endif
              break;
          }
          #endif
          l_proc1[w]->pos++;
          BOUND_CORESIZE(l_proc1[w]->pos);
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_PCT
        case O_PCT:
          l_isPCT[ap] = 1;
          l_proc1[w]->pos++;
          BOUND_CORESIZE(l_proc1[w]->pos);
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        #ifdef O_STS
        case O_STS:
          switch(I._M) {
            case M_I:
              printf("%s: ", warriors[l_indices[w]].name);
              debug_println1(ai._I);
              break;
            case M_A: case M_AB:
              printf("%s: %d\n", warriors[l_indices[w]].name, (int)ai._A);
              break;
            case M_B: case M_BA:
              printf("%s: %d\n", warriors[l_indices[w]].name, (int)ai._B);
              break;
            case M_F: case M_X:
              printf("%s: %d\t%d\n", warriors[l_indices[w]].name, (int)ai._A, (int)ai._B);
              break;
          }
          l_proc1[w]->pos++;
          BOUND_CORESIZE(l_proc1[w]->pos);
          l_proc1[w] = l_proc1[w]->next;
          break;
        #endif
        default: HINT_UNREACHABLE();
      }
      ENDINSTR();
    }
  }
  _label_endbattle:

  #if defined(TSAFE_CORE) && PSPACESIZE
  if(l_pspace_local_accessed) munlock(mutex_pwarriors);
  #endif
  #ifdef _COREVIEW_
  mlock(l_mutex_mode);
  if(l_runmode == RUN_FAST && l_rundata > 0) {
    munlock(l_mutex_exec);
  }
  munlock(l_mutex_mode);
  #endif
  //clear process queues
  clear_pool_proc1(); //this may be inefficient! Should try free_pool_proc1() for each process instead
  return;
}

unsigned int battle1(_corefunc unsigned long nrounds) { //returns actual number of rounds
  unsigned int cround;
  l_core1 = malloc(CORESIZE * sizeof(INSTR1));
  l_positions = malloc(WARRIORS * 2);
  l_proc1 = malloc(CORESIZE * sizeof(PROC1*));
  l_nprocs = malloc(CORESIZE * sizeof(unsigned long));
  l_indices = malloc(CORESIZE * sizeof(unsigned long));
  l_running = malloc(CORESIZE * sizeof(int));
  #ifdef _COREVIEW_
  l_coreviewdata = malloc(CORESIZE * sizeof(CVCELL));
  #endif
  init_pool_proc1();
  unsigned long w;
  for(w = 0; w < WARRIORS; ++w) {
    warriors[w].score = 0;
    warriors[w].wins = 0;
    warriors[w].losses = 0;
    warriors[w].ties = 0;
  }
  for(cround = 0; cround < nrounds; ++cround) {
    memset(l_core1, 0, CORESIZE * sizeof(INSTR1));
    #ifdef _COREVIEW_
    memset(l_coreviewdata, 0, CORESIZE * sizeof(CVCELL));
    #endif
    simulate1(_corecal0);
    unsigned long c;
    for(c = 0; c < WARRIORS; ++c) {
      #ifdef TSAFE_CORE
      mlock(warriors[c].mutex);
      #endif
      if(l_running[c]) {
        if(l_nrunning > 1) {
          warriors[c].ties++;
        }
        else {
          warriors[c].wins++;
        }
        warriors[c].score += (WARRIORS*WARRIORS-1)/l_nrunning;
        #if PSPACESIZE
        #ifdef TSAFE_CORE
        mlock(mutex_pwarriors);
        #endif
        warriors[c].psp0 = l_nrunning;
        #ifdef TSAFE_CORE
        munlock(mutex_pwarriors);
        #endif
        #endif
      }
      else {
        warriors[c].losses++;
        #if PSPACESIZE
        #ifdef TSAFE_CORE
        mlock(mutex_pwarriors);
        #endif
        warriors[c].psp0 = 0;
        #ifdef TSAFE_CORE
        munlock(mutex_pwarriors);
        #endif
        #endif
      }
      #ifdef TSAFE_CORE
      munlock(warriors[c].mutex);
      #endif
    }

    #ifdef _COREVIEW_
    if(check_terminate()) break;
    #endif
  }
  #ifdef _COREVIEW_
  mlock(l_mutex_exec);
  free(l_coreviewdata);
  l_coreviewdata = NULL;
  munlock(l_mutex_exec);
  #endif
  free(l_core1);
  free(l_positions);
  free(l_proc1);
  free(l_nprocs);
  free(l_indices);
  free(l_running);
  destroy_pool_proc1();
  return cround + 1;
}

unsigned int inline battle1_single(unsigned long nrounds) { //returns actual number of rounds
  #ifdef TSAFE_CORE
  LOCAL_CORE* local_core = malloc(sizeof(LOCAL_CORE));
  #endif

  unsigned int r = battle1(_corecall nrounds);

  #ifdef TSAFE_CORE
  free(local_core);
  #endif
  return r;
}

#ifdef TSAFE_CORE
_TPROC_TYPE tproc_battle1(void* pnrounds) {
  battle1_single(*(unsigned long*) pnrounds);
  _TPROC_RET();
}

void battle1_multithread(unsigned long nrounds, unsigned int nthreads) {
  unsigned long* vnrounds = malloc(nthreads * sizeof(unsigned long));
  unsigned int c;
  for(c = 0; c < nthreads; ++c) {
    vnrounds[c] = nrounds / (nthreads - c);
    nrounds -= vnrounds[c];
  }
  THREAD* threads = malloc(nthreads * sizeof(THREAD));
  for(c = 0; c < nthreads; ++c) {
    tcreate(&threads[c], tproc_battle1, &vnrounds[c]);
  }
  for(c = 0; c < nthreads; ++c) {
    tjoin(threads[c]);
  }
  return;
}
#endif

#ifdef _COREVIEW_

typedef struct tTPB1A_struct {
  LOCAL_CORE* local_core;
  unsigned long nrounds;
  CONDITION cond;
  MUTEX mutex;
} TPB1A_struct;

_TPROC_TYPE tproc_battle1_cv(void* pstruct) {
  TPB1A_struct* tstruct = pstruct;
  LOCAL_CORE* local_core = tstruct->local_core;
  unsigned long nrounds = tstruct->nrounds;
  mlock(tstruct->mutex);
  csignal(tstruct->cond);
  munlock(tstruct->mutex);
  battle1(_corecall nrounds);
  _TPROC_RET();
}

LOCAL_CORE* battle1_async(unsigned long nrounds) {
  LOCAL_CORE* local_core = malloc(sizeof(LOCAL_CORE));
  minit(l_mutex_exec);
  minit(l_mutex_mode);
  cinit(l_cond_exec);
  #ifdef _COREVIEW_
  l_coreviewdata = NULL;
  l_runmode = RUN_PAUSED;
  l_runparam = 0;
  #endif

  TPB1A_struct* volatile tmps = malloc(sizeof(TPB1A_struct)); //must be in heap
  tmps->nrounds = nrounds;
  tmps->local_core = local_core;
  cinit(tmps->cond);
  minit(tmps->mutex);
  tcreate(&l_thread, tproc_battle1_cv, tmps);
  mlock(tmps->mutex);
  cwait(tmps->cond, tmps->mutex);
  mdestroy(tmps->mutex);
  cdestroy(tmps->cond);
  free(tmps);
  return local_core;
}
#endif

/*
███████ ██    ██ ███    ██  ██████ ████████ ██  ██████  ███    ██ ███████      ██ ██████  ██
██      ██    ██ ████   ██ ██         ██    ██ ██    ██ ████   ██ ██          ██       ██  ██
█████   ██    ██ ██ ██  ██ ██         ██    ██ ██    ██ ██ ██  ██ ███████     ██   █████   ██
██      ██    ██ ██  ██ ██ ██         ██    ██ ██    ██ ██  ██ ██      ██     ██  ██       ██
██       ██████  ██   ████  ██████    ██    ██  ██████  ██   ████ ███████      ██ ███████ ██
*/

void place2(_corefun0) {
  int c;
  place_generic(_corecal0); //set positions

  for(c = 0; c < WARRIORS; ++c) {
    if(l_positions[c] + warriors[c].len <= CORESIZE) {
      memcpy(&l_core2[l_positions[c]], warriors[c].code2, warriors[c].len * sizeof(INSTR2));
    }
    else {
      memcpy(&l_core2[l_positions[c]], warriors[c].code2, (CORESIZE - l_positions[c]) * sizeof(INSTR2));
      memcpy(l_core2, &warriors[c].code2[CORESIZE - l_positions[c]], (warriors[c].len - CORESIZE + l_positions[c]) * sizeof(INSTR2));
    }
  }
  return;
}

void simulate2(_corefun0) {
  //place warriors
  place2(_corecal0);

  //init process queues
  l_nrunning = WARRIORS;
  unsigned long c;
  for(c = 0; c < WARRIORS; ++c) {
    l_proc2[c] = alloc_pool_proc2();
    l_proc2[c]->pos = l_positions[c] + warriors[c].org;
    if(l_proc2[c]->pos >= CORESIZE) l_proc2[c]->pos -= CORESIZE;
    l_proc2[c]->next = l_proc2[c]->prev = l_proc2[c];
    l_nprocs[c] = 1;
    l_indices[c] = c;

    l_running[c] = 1;
  }
  for(c = WARRIORS-1; c; --c) { //Fisher-Yates shuffling algorithm
    unsigned long c2 = pcg32_boundedrand_r(&randomgen, c+1);
    PROC2* tmp = l_proc2[c];
    l_proc2[c] = l_proc2[c2];
    l_proc2[c2] = tmp;
    unsigned long tmp2 = l_nprocs[c];
    l_nprocs[c] = l_nprocs[c2];
    l_nprocs[c2] = tmp2;
    tmp2 = l_indices[c];
    l_indices[c] = l_indices[c2];
    l_indices[c2] = tmp2;
  }

  #if defined(TSAFE_CORE) && PSPACESIZE
  l_pspace_local_accessed = 0;
  #endif

  //run in a loop
  for(c = 0; c < MAXCYCLES; ++c) {
    for(l_w2 = 0; l_w2 < l_nrunning; ++l_w2) {
      addr2_t pc = l_proc2[l_w2]->pos;
      addr2_t a, b;
      a = l_core2[pc].a;
      b = l_core2[pc].b;
      //printf("[##] "); debug_println2(l_core2[##]); //D
      //debug_println2(l_core2[pc]); //D
      if(l_core2[pc].fn(_corecall l_core2, pc, a, b)) goto _label_endbattle; //<here sigsegv
    }
  }
  _label_endbattle:

  #if defined(TSAFE_CORE) && PSPACESIZE
  if(l_pspace_local_accessed) munlock(mutex_pwarriors);
  #endif
  //clear process queues
  clear_pool_proc2(); //this may be inefficient! Should try free_pool_proc1() for each process instead
  return;
}

unsigned int battle2(_corefunc unsigned long nrounds) { //returns actual number of rounds
  unsigned int cround;
  l_core2 = malloc(CORESIZE * sizeof(INSTR2));
  l_positions = malloc(WARRIORS * 2);
  l_proc2 = malloc(CORESIZE * sizeof(PROC2*));
  l_nprocs = malloc(CORESIZE * sizeof(unsigned long));
  l_indices = malloc(CORESIZE * sizeof(unsigned long));
  l_running = malloc(CORESIZE * sizeof(int));
  init_pool_proc2();
  unsigned long w;
  for(w = 0; w < WARRIORS; ++w) {
    warriors[w].score = 0;
    warriors[w].wins = 0;
    warriors[w].losses = 0;
    warriors[w].ties = 0;
  }
  for(cround = 0; cround < nrounds; ++cround) {
    unsigned long c;
    for(c = 0; c < CORESIZE; ++c) {
      l_core2[c].fn = (jitfunc2_t) _hardcoded_dat;
      l_core2[c].a = 0;
      l_core2[c].b = 0;
    }
    simulate2(_corecal0);
    for(c = 0; c < WARRIORS; ++c) {
      #ifdef TSAFE_CORE
      mlock(warriors[c].mutex);
      #endif
      if(l_running[c]) {
        if(l_nrunning > 1) {
          warriors[c].ties++;
        }
        else {
          warriors[c].wins++;
        }
        warriors[c].score += (WARRIORS*WARRIORS-1)/l_nrunning;
        #if PSPACESIZE
        #ifdef TSAFE_CORE
        mlock(mutex_pwarriors);
        #endif
        warriors[c].psp0 = l_nrunning;
        #ifdef TSAFE_CORE
        munlock(mutex_pwarriors);
        #endif
        #endif
      }
      else {
        warriors[c].losses++;
        #if PSPACESIZE
        #ifdef TSAFE_CORE
        mlock(mutex_pwarriors);
        #endif
        warriors[c].psp0 = 0;
        #ifdef TSAFE_CORE
        munlock(mutex_pwarriors);
        #endif
        #endif
      }
      #ifdef TSAFE_CORE
      munlock(warriors[c].mutex);
      #endif
    }
  }
  free(l_core2);
  free(l_positions);
  free(l_proc2);
  free(l_nprocs);
  free(l_indices);
  free(l_running);
  destroy_pool_proc2();
  return cround + 1;
}

unsigned int inline battle2_single(unsigned long nrounds) { //returns actual number of rounds
  #ifdef TSAFE_CORE
  LOCAL_CORE* local_core = malloc(sizeof(LOCAL_CORE));
  #endif

  unsigned int r = battle2(_corecall nrounds);

  #ifdef TSAFE_CORE
  free(local_core);
  #endif
  return r;
}

#ifdef _COREVIEW_
void wait_for_core(LOCAL_CORE* local_core) {
  tjoin(l_thread);
  mdestroy(l_mutex_exec);
  mdestroy(l_mutex_mode);
  cdestroy(l_cond_exec);
  return;
}
#endif
