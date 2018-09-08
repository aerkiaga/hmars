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

//#include "Python.h"
#include "defs.h"

pcg32_random_t randomgen;
#ifdef _COREVIEW_
int algorithm_select = 6; // 1 and 2
#else
int algorithm_select = 6; // only 2
#endif
jit_context_t jit_context;
#if defined(TSAFE_CORE) && PSPACESIZE
extern MUTEX mutex_pwarriors;
#endif

void load1(WARRIOR* w, LINE* txt) { //is used by load2
  w->code1 = malloc(w->len * sizeof(INSTR1));
  int c;
  txt = txt->next;
  for(c = 0; c < w->len; ++c, txt = txt->next) {
    char* in = txt->data;
    int i = 7;
    uint32_t s_op = in[i]; s_op <<= 8;
    s_op |= in[++i]; s_op <<= 8;
    s_op |= in[++i];
    uint8_t op = 0;
    switch(s_op) {
      #ifdef O_DAT
      case 0x444154: op = O_DAT; break;
      #endif
      #ifdef O_MOV
      case 0x4d4f56: op = O_MOV; break;
      #endif
      #ifdef O_ADD
      case 0x414444: op = O_ADD; break;
      #endif
      #ifdef O_SUB
      case 0x535542: op = O_SUB; break;
      #endif
      #ifdef O_JMP
      case 0x4a4d50: op = O_JMP; break;
      #endif
      #ifdef O_JMZ
      case 0x4a4d5a: op = O_JMZ; break;
      #endif
      #ifdef O_CMP
      case 0x434d50: op = O_CMP; break;
      #endif
      #ifdef O_JMN
      case 0x4a4d4e: op = O_JMN; break;
      #endif
      #ifdef O_DJN
      case 0x444a4e: op = O_DJN; break;
      #endif
      #ifdef O_SPL
      case 0x53504c: op = O_SPL; break;
      #endif
      #ifdef O_SLT
      case 0x534c54: op = O_SLT; break;
      #endif
      #ifdef O_MUL
      case 0x4d554c: op = O_MUL; break;
      #endif
      #ifdef O_DIV
      case 0x444956: op = O_DIV; break;
      #endif
      #ifdef O_MOD
      case 0x4d4f44: op = O_MOD; break;
      #endif
      #ifdef O_SEQ
      case 0x534551: op = O_SEQ; break;
      #endif
      #ifdef O_SNE
      case 0x534e45: op = O_SNE; break;
      #endif
      #ifdef O_NOP
      case 0x4e4f50: op = O_NOP; break;
      #endif
      #ifdef O_LDP
      case 0x4c4450: op = O_LDP; break;
      #endif
      #ifdef O_STP
      case 0x535450: op = O_STP; break;
      #endif
      #ifdef O_DJZ
      case 0x444a5a: op = O_DJZ; break;
      #endif
      #ifdef O_PCT
      case 0x504354: op = O_PCT; break;
      #endif
      #ifdef O_XCH
      case 0x584348: op = O_XCH; break;
      #endif
      #ifdef O_STS
      case 0x535453: op = O_STS; break;
      #endif
      case 0x585858: //XXX
        op = pcg32_boundedrand_r(&randomgen, O_COUNT);
        #ifdef O_STS
        if(op == O_STS) op = O_NOP; //we don't want undesired output
        //sure there's a better way to do it
        #endif
        break;
      default:
        error("Unknown opcode: %c%c%c", in[i-2], in[i-1], in[i]);
    }
    w->code1[c]._O = op;
    ++i;
    uint8_t mod = 0;
    switch(in[++i]) {
      case 'A':
        if(in[i+1] == 'B') {
          mod = M_AB;
        }
        else mod = M_A;
        break;
      case 'B':
        if(in[i+1] == 'A') {
          mod = M_BA;
        }
        else mod = M_B;
        break;
      case 'F': mod = M_F; break;
      case 'X': mod = M_X; break;
      case 'I': mod = M_I; break;
    }
    if(s_op == 0x585858) {
      mod = pcg32_boundedrand_r(&randomgen, M_COUNT);
    }
    w->code1[c]._M = mod;
    i += 2;
    uint8_t adA = 0;
    switch(in[++i]) {
      #ifdef A_IMM
      case '#': adA = A_IMM; break;
      #endif
      #ifdef A_DIR
      case '$': adA = A_DIR; break;
      #endif
      #ifdef A_INA
      case '*': adA = A_INA; break;
      #endif
      #ifdef A_INB
      case '@': adA = A_INB; break;
      #endif
      #ifdef A_PDA
      case '{': adA = A_PDA; break;
      #endif
      #ifdef A_PDB
      case '<': adA = A_PDB; break;
      #else
      #ifdef A_ADB
      case '<': adA = A_ADB; break;
      #endif
      #endif
      #ifdef A_PIA
      case '}': adA = A_PIA; break;
      #endif
      #ifdef A_PIB
      case '>': adA = A_PIB; break;
      #endif
    }
    if(s_op == 0x585858) {
      adA = pcg32_boundedrand_r(&randomgen, A_COUNT);
    }
    w->code1[c]._aA = adA;
    int valA = 0;
    while(isblank(in[++i])) ;
    int neg;
    if(in[i] == '-') {
      neg = 1;
      ++i;
    } else neg = 0;
    while(in[i] != ',') valA = (valA * 10) + (in[i++] - '0');
    ++i;
    if(neg) valA = CORESIZE - valA;
    w->code1[c]._A = valA;
    uint8_t adB = 0;
    switch(in[++i]) {
      #ifdef A_IMM
      case '#': adB = A_IMM; break;
      #endif
      #ifdef A_DIR
      case '$': adB = A_DIR; break;
      #endif
      #ifdef A_INA
      case '*': adB = A_INA; break;
      #endif
      #ifdef A_INB
      case '@': adB = A_INB; break;
      #endif
      #ifdef A_PDA
      case '{': adB = A_PDA; break;
      #endif
      #ifdef A_PDB
      case '<': adB = A_PDB; break;
      #else
      #ifdef A_ADB
      case '<': adB = A_ADB; break;
      #endif
      #endif
      #ifdef A_PIA
      case '}': adB = A_PIA; break;
      #endif
      #ifdef A_PIB
      case '>': adB = A_PIB; break;
      #endif
    }
    if(s_op == 0x585858) {
      adB = pcg32_boundedrand_r(&randomgen, A_COUNT);
    }
    w->code1[c]._aB = adB;
    int valB = 0;
    while(isblank(in[++i])) ;
    if(in[i] == '-') {
      neg = 1;
      ++i;
    } else neg = 0;
    while(isdigit(in[i])) valB = (valB * 10) + (in[i++] - '0');
    if(neg) valB = CORESIZE - valB;
    w->code1[c]._B = valB;
    if(s_op == 0x585858) {
      if(valB < valA) valB += CORESIZE;
      w->code1[c]._A = valA + pcg32_boundedrand_r(&randomgen, valB - valA + 1);
      w->code1[c]._B = valA + pcg32_boundedrand_r(&randomgen, valB - valA + 1);
      if(w->code1[c]._A >= CORESIZE) w->code1[c]._A -= CORESIZE;
      if(w->code1[c]._B >= CORESIZE) w->code1[c]._B -= CORESIZE;
    }
  }
  return;
}

typedef struct tDATA2_ELEM {
  jitfunc2_t fn;
  uint32_t oma;
} DATA2_ELEM;

struct {
  DATA2_ELEM hasht[64];
  int nentr;
  int allocd;
  DATA2_ELEM* list;
  int curhpos;
}
  g_data2 = {{{NULL, 0}}, 0, 0, NULL, -1};

jitfunc2_t instr1to2(uint32_t oma) {
  INSTR1 i;
  i._OMA = oma;

  uint8_t hash = i._O ^ i._M ^ i._aA ^ i._aB;
  if(g_data2.hasht[hash].fn == NULL) { //hash table miss
    g_data2.hasht[hash].oma = i._OMA;
    g_data2.curhpos = hash; //set .fn later
    goto _nocallback;
  }
  else if(g_data2.hasht[hash].fn == (jitfunc2_t)-1) { //hash table collision
    int c;
    for(c = 0; c < g_data2.nentr; ++c) {
      if(g_data2.list[c].oma == i._OMA) return g_data2.list[c].fn;
    }
    g_data2.curhpos = -1; //don't bother about hash table
    goto _nocallback;
  }
  else { //hash table hit
    if(g_data2.hasht[hash].oma == i._OMA) { //correct element
      return g_data2.hasht[hash].fn;
    }
    else { //must add new
      g_data2.hasht[hash].fn = (jitfunc2_t)-1; //now there's a collision
      g_data2.curhpos = -1; //don't bother about hash table
      goto _nocallback;
    }
  }
  _nocallback:
  ++g_data2.nentr; //no callback registered, add new
  if(g_data2.nentr > g_data2.allocd) g_data2.allocd += 16;
  g_data2.list = (DATA2_ELEM*) realloc(g_data2.list, g_data2.allocd * sizeof(DATA2_ELEM));
  g_data2.list[g_data2.nentr-1].oma = oma;
  return NULL;
}

#define JIT_CONST(x, type) jit_value_create_nint_constant(function, (type), (jit_nint)(x))
#define JIT_CORE2_L(a) jit_insn_load_elem_address(function, core2, (a), jit_type_instr2)
#define JIT_INSTR2_L(s, i)  jit_insn_load_relative(function, (s), offsetof(INSTR2, i), jit_type_addr2)
#define JIT_INSTR2_fn_L(s)  jit_insn_load_relative(function, (s), offsetof(INSTR2, fn), jit_type_void_ptr)
#define JIT_INSTR2_S(s, i, v) jit_insn_store_relative(function, (s), offsetof(INSTR2, i), (v))
#define BOUND_CORESIZE_LOW_JIT(y, x) do { \
  jit_label_t label1 = jit_label_undefined; \
  jit_label_t label3 = jit_label_undefined; \
  (y) = jit_value_create(function, jit_type_addr2); \
  jit_insn_branch_if_not(function, jit_insn_lt(function, (x), JIT_CONST(0, jit_type_addr2s)), &label1); \
    jit_insn_store(function, (y), jit_insn_add(function, (x), JIT_CONST(CORESIZE, jit_type_addr2))); \
  jit_insn_branch(function, &label3); \
  jit_insn_label(function, &label1); \
    jit_insn_store(function, (y), jit_insn_load(function, (x))); \
  jit_insn_label(function, &label3); \
  /*if((signed)x < 0) x += CORESIZE;*/ \
} while(0)
#define BOUND_CORESIZE_HIGH_JIT(y, x) do { \
  jit_label_t label2 = jit_label_undefined; \
  jit_label_t label3 = jit_label_undefined; \
  (y) = jit_value_create(function, jit_type_addr2); \
  jit_insn_branch_if_not(function, jit_insn_ge(function, (x), JIT_CONST(CORESIZE, jit_type_addr2s)), &label2); \
    jit_insn_store(function, (y), jit_insn_sub(function, (x), JIT_CONST(CORESIZE, jit_type_addr2))); \
  jit_insn_branch(function, &label3); \
  jit_insn_label(function, &label2); \
    jit_insn_store(function, (y), jit_insn_load(function, (x))); \
  jit_insn_label(function, &label3); \
  /*if((signed)x >= CORESIZE) x -= CORESIZE;*/ \
} while(0)
#define JIT_LPROC2_L(a) jit_insn_load_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), (a), jit_type_void_ptr)
#define JIT_PROC2_next_L(p) jit_insn_load_relative(function, (p), offsetof(PROC2, next), jit_type_void_ptr)
#define JIT_PROC2_pos_L(p) jit_insn_load_relative(function, (p), offsetof(PROC2, pos), jit_type_addr2)
#define JIT_PROC2_pos_S(p, v) jit_insn_store_relative(function, (p), offsetof(PROC2, pos), (v))
#define JIT_W_ADDR() jit_insn_add_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_w2))
#define JIT_W() jit_insn_load_relative(function, w_addr, 0, jit_type_ulong)
#define JIT_TYPE(s)  (((jit_type_t[]){jit_type_ubyte, jit_type_ushort, (jit_type_t) 0, jit_type_uint, (jit_type_t) 0, (jit_type_t) 0, (jit_type_t) 0, jit_type_ulong})[sizeof(s)-1])

int remove_proc_jit(_corefun0) { //returns 1 if battle ends, 0 otherwise
  --l_nprocs[l_w2];
  if(!l_nprocs[l_w2]) { /*warrior loses*/
    free_pool_proc2(l_proc2[l_w2]);
    l_running[l_indices[l_w2]] = 0;
    --l_nrunning;
    if(l_nrunning == 1) {
      return 1;
    }
    unsigned long v;
    for(v = l_w2; v < l_nrunning; ++v) {
      if(v+1 >= WARRIORS) __builtin_unreachable(); /*GCC builtin*/
      l_proc2[v] = l_proc2[v+1];
      l_nprocs[v] = l_nprocs[v+1];
      l_indices[v] = l_indices[v+1];
    }
  }
  else {
    l_proc2[l_w2]->prev->next = l_proc2[l_w2]->next;
    l_proc2[l_w2]->next->prev = l_proc2[l_w2]->prev;
    free_pool_proc2(l_proc2[l_w2]); /*note that the memory is not actually free'd*/
    l_proc2[l_w2] = l_proc2[l_w2]->next; /*it can be used safely afterwards*/
  }
  return 0;
}
#ifdef TSAFE_CORE
#define REMOVE_PROC_JIT() jit_insn_return(function, jit_insn_call_native(function, NULL, remove_proc_jit, signature_remove_proc_jit, (jit_value_t[]){local_core_jit}, 1, JIT_CALL_NOTHROW))
#else
#define REMOVE_PROC_JIT() jit_insn_return(function, jit_insn_call_native(function, NULL, remove_proc_jit, signature_remove_proc_jit, NULL, 0, JIT_CALL_NOTHROW))
#endif
#ifdef opt_SPL86
void spl_jit(_corefun0, addr2_t bp) {
#else
void spl_jit(_corefun0, addr2_t ap) {
#endif
  PROC2* newproc = alloc_pool_proc2();
  #ifdef opt_SPL86
  newproc->pos = bp;
  #else
  newproc->pos = ap;
  #endif
  #ifdef opt_SPL86
  newproc->next = l_proc2[l_w2];
  newproc->prev = l_proc2[l_w2]->prev;
  l_proc2[l_w2]->prev->next = newproc;
  l_proc2[l_w2]->prev = newproc;
  l_proc2[l_w2]->pos++;
  BOUND_CORESIZE(l_proc2[l_w2]->pos);
  l_proc2[l_w2] = l_proc2[l_w2]->next;
  #else
  newproc->prev = l_proc2[l_w2];
  newproc->next = l_proc2[l_w2]->next;
  l_proc2[l_w2]->next->prev = newproc;
  l_proc2[l_w2]->next = newproc;
  l_proc2[l_w2]->pos++;
  BOUND_CORESIZE(l_proc2[l_w2]->pos);
  l_proc2[l_w2] = newproc->next;
  #endif
  ++l_nprocs[l_w2];
  return;
}
void mlock_helper_jit() {
  mlock(mutex_pwarriors);
  return;
}
#ifdef O_STS
void sts_jit(_corefunc uint8_t mode, addr2_t v1/*a*/, addr2_t v2/*b*/, jitfunc2_t vf) {
  switch(mode) {
    case M_I:
      printf("%s: %p\t%d\t%d\n", warriors[l_indices[l_w2]].name, vf, (int)v1, (int)v2);
      break;
    case M_A: case M_AB:
      printf("%s: %d\n", warriors[l_indices[l_w2]].name, (int)v1);
      break;
    case M_B: case M_BA:
      printf("%s: %d\n", warriors[l_indices[l_w2]].name, (int)v2);
      break;
    case M_F: case M_X:
      printf("%s: %d\t%d\n", warriors[l_indices[l_w2]].name, (int)v1, (int)v2);
      break;
  }
  return;
}
#endif

int _hardcoded_dat(_corefunc INSTR2* core2, addr2_t pc, addr2_t a, addr2_t b) { //instruction filling core
  return remove_proc_jit(_corecal0);
}

void load2(WARRIOR* w, LINE* txt) {
  INSTR1* c1; //get some temporary code
  if(w->code1 != NULL) c1 = w->code1;
  else {
    WARRIOR* tw = malloc(sizeof(WARRIOR));
    memcpy(tw, w, sizeof(WARRIOR));
    load1(tw, txt);
    c1 = tw->code1;
    free(tw);
  }

  w->code2 = malloc(w->len * sizeof(INSTR2));

  #ifdef O_PCT
  int warning_pct_given = 0;
  #endif
  //JIT compiler
  jit_context_build_start(jit_context);
  jit_type_t fields[3] = {jit_type_void_ptr, jit_type_addr2, jit_type_addr2};
  jit_type_t jit_type_instr2 = jit_type_create_struct(fields, 3, 1);
  #ifdef TSAFE_CORE
  jit_type_t signature_remove_proc_jit = jit_type_create_signature(jit_abi_cdecl, jit_type_nint, (jit_type_t[]){jit_type_void_ptr}, 1, 1);
  #else
  jit_type_t signature_remove_proc_jit = jit_type_create_signature(jit_abi_cdecl, jit_type_nint, NULL, 0, 1);
  #endif
  #ifdef O_SPL
  #ifdef TSAFE_CORE
  jit_type_t signature_spl_jit = jit_type_create_signature(jit_abi_cdecl, jit_type_void, (jit_type_t[]){jit_type_void_ptr, jit_type_addr2}, 2, 1);
  #else
  jit_type_t signature_spl_jit = jit_type_create_signature(jit_abi_cdecl, jit_type_void, (jit_type_t[]){jit_type_addr2}, 1, 1);
  #endif
  #endif
  jit_type_t signature_mlock_helper_jit = jit_type_create_signature(jit_abi_cdecl, jit_type_void, NULL, 0, 1);
  jit_type_t jit_type_pcell = JIT_TYPE(pcell_t);
  jit_type_t fields3[] = {jit_type_sys_ulong, jit_type_void_ptr, jit_type_void_ptr, jit_type_sys_uint, jit_type_void_ptr, jit_type_void_ptr,
  #if PSPACESIZE
    jit_type_void_ptr, jit_type_pcell, jit_type_sys_ulong, jit_type_sys_int,
  #endif
    jit_type_sys_uint, jit_type_sys_uint, jit_type_sys_uint, jit_type_sys_uint
  #ifdef TSAFE_CORE
    , JIT_TYPE(MUTEX)
  #endif
  #ifdef _COREVIEW_
    , jit_type_uint
  #endif
  };
  //#define was here
  jit_type_t jit_type_warrior = jit_type_create_struct(fields3, 3, 1);
  #ifdef O_STS
  #ifdef TSAFE_CORE
  jit_type_t signature_sts_jit = jit_type_create_signature(jit_abi_cdecl, jit_type_void, (jit_type_t[]){jit_type_void_ptr, jit_type_uint, jit_type_addr2, jit_type_addr2, jit_type_void_ptr}, 5, 1);
  #else
  jit_type_t signature_sts_jit = jit_type_create_signature(jit_abi_cdecl, jit_type_void, (jit_type_t[]){jit_type_uint, jit_type_addr2, jit_type_addr2, jit_type_void_ptr}, 4, 1);
  #endif
  #endif

  unsigned int c;
  for(c = 0; c < w->len; ++c) {
    INSTR2 i2;
    i2.fn = instr1to2(c1[c]._OMA); //only translates if duplicate
    i2.a = c1[c]._A; //this limits field width to to 15 bits
    i2.b = c1[c]._B;
    if(i2.fn != NULL) {
      w->code2[c] = i2;
    }
    else { //non-duplicate, must compile
      jit_function_t function;
      jit_type_t params[5] = {jit_type_void_ptr, jit_type_void_ptr, jit_type_addr2, jit_type_addr2, jit_type_addr2};
      jit_type_t signature;
      signature = jit_type_create_signature(jit_abi_cdecl, jit_type_nint, params, 5, 1);
      function = jit_function_create(jit_context, signature);
      //jit_insn_mark_breakpoint (function, jit_nint data1, jit_nint data2)

      jit_value_t local_core_jit, core2, pc, a, b;
      #ifdef TSAFE_CORE
      local_core_jit = jit_value_get_param(function, 0);
      core2 = jit_value_get_param(function, 1);
      pc = jit_value_get_param(function, 2);
      a = jit_value_get_param(function, 3);
      b = jit_value_get_param(function, 4);
      #else
      local_core_jit = JIT_CONST(local_core, jit_type_void_ptr);
      core2 = jit_value_get_param(function, 0);
      pc = jit_value_get_param(function, 1);
      a = jit_value_get_param(function, 2);
      b = jit_value_get_param(function, 3);
      #endif

      jit_value_t w_addr = JIT_W_ADDR();

      int need_ap = 0, need_ai = 0, need_bp = 0, go_on = 0; //needs a- and b-pointers, can continue on next instruction
      switch(c1[c]._O) {
        #ifdef O_DAT
        case O_DAT: need_ap = need_ai = need_bp = go_on = 0; break;
        #endif
        #ifdef O_MOV
        case O_MOV: need_ap = need_ai = need_bp = go_on = 1; break;
        #endif
        #ifdef O_ADD
        case O_ADD: need_ap = need_ai = need_bp = go_on = 1; break;
        #endif
        #ifdef O_SUB
        case O_SUB: need_ap = need_ai = need_bp = go_on = 1; break;
        #endif
        #ifdef O_MUL
        case O_MUL: need_ap = need_ai = need_bp = go_on = 1; break;
        #endif
        #ifdef O_DIV
        case O_DIV: need_ap = need_ai = need_bp = go_on = 1; break;
        #endif
        #ifdef O_MOD
        case O_MOD: need_ap = need_ai = need_bp = go_on = 1; break;
        #endif
        #ifdef O_JMP
        case O_JMP: need_ap = 1; need_bp = need_ai = go_on = 0; break;
        #endif
        #ifdef O_JMZ
        case O_JMZ: need_ap = need_bp = go_on = 1; need_ai = 0; break;
        #endif
        #ifdef O_JMN
        case O_JMN: need_ap = need_bp = go_on = 1; need_ai = 0; break;
        #endif
        #ifdef O_DJZ
        case O_DJZ: need_ap = need_bp = 1; go_on = need_ai = 0; break;
        #endif
        #ifdef O_DJN
        case O_DJN: need_ap = need_bp = 1; go_on = need_ai = 0; break;
        #endif
        #if defined(O_CMP) || defined(O_SEQ)
        #ifdef O_CMP
        case O_CMP:
        #else
        case O_SEQ:
        #endif
          need_ap = need_ai = need_bp = 1; go_on = 0; break;
        #endif
        #ifdef O_SNE
        case O_SNE: need_ap = need_ai = need_bp = 1; go_on = 0; break;
        #endif
        #ifdef O_SLT
        case O_SLT: need_ap = need_ai = need_bp = 1; go_on = 0; break;
        #endif
        #ifdef O_SPL
        case O_SPL: need_ap = go_on = 1; need_bp = need_ai = 0; break;
        #endif
        #ifdef O_NOP
        case O_NOP: need_ap = need_ai = need_bp = 0; go_on = 1; break;
        #endif
        #ifdef O_LDP
        case O_LDP: need_ap = need_ai = need_bp = go_on = 1; break;
        #endif
        #ifdef O_STP
        case O_STP: need_ap = need_ai = need_bp = go_on = 1; break;
        #endif
        #ifdef O_XCH
        case O_XCH: need_ap = need_ai = need_bp = go_on = 1; break;
        #endif
        #ifdef O_PCT
        case O_PCT: need_ap = go_on = 1; need_bp = need_ai = 0; break;
        #endif
        #ifdef O_STS
        case O_STS: need_ap = need_ai = go_on = 1; need_bp = 0; break;
        #endif
        default:
          error("Unknown opcode found by JIT-compiler: %d", c1[c]._O);
      }
      jit_value_t ap = NULL, bp = NULL;
      #ifdef A_ADB //auto-decrement
      jit_value_t adpA, adpB; //auto-decrement pointers
      #endif

      if(need_ap) {
        switch(c1[c]._aA) {
          #ifdef A_IMM
          case A_IMM:
            ap = pc;
            break;
          #endif
          #ifdef A_DIR
          case A_DIR: {
            jit_value_t ap_ = jit_insn_add(function, a, pc);
            BOUND_CORESIZE_HIGH_JIT(ap, ap_);
            break; }
          #endif
          #ifdef A_INA
          case A_INA: {
            jit_value_t ap_ = jit_insn_add(function, a, pc);
            jit_value_t ap_2, ap_3;
            BOUND_CORESIZE_HIGH_JIT(ap_2, ap_);
            jit_value_t tmp1 = JIT_CORE2_L(ap_2);
            jit_value_t tmp1_ = JIT_INSTR2_L(tmp1, a);
            ap_3 = jit_insn_add(function, ap_2, tmp1_);
            BOUND_CORESIZE_HIGH_JIT(ap, ap_3);
            break; }
          #endif
          #ifdef A_INB
          case A_INB: {
            jit_value_t ap_ = jit_insn_add(function, a, pc);
            jit_value_t ap_2, ap_3;
            BOUND_CORESIZE_HIGH_JIT(ap_2, ap_);
            jit_value_t tmp1 = JIT_CORE2_L(ap_2);
            jit_value_t tmp1_ = JIT_INSTR2_L(tmp1, b);
            ap_3 = jit_insn_add(function, ap_2, tmp1_);
            BOUND_CORESIZE_HIGH_JIT(ap, ap_3);
            break; }
          #endif
          #ifdef A_PDA
          case A_PDA: {
            jit_value_t ap_ = jit_insn_add(function, a, pc);
            jit_value_t ap_2, ap_3;
            BOUND_CORESIZE_HIGH_JIT(ap_2, ap_);
            jit_value_t tmp1 = JIT_CORE2_L(ap_2);
            jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
            jit_value_t tmpa_ = jit_insn_sub(function, tmpa, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpa_2;
            BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            ap_3 = jit_insn_add(function, ap_2, tmpa_2);
            BOUND_CORESIZE_HIGH_JIT(ap, ap_3);
            break; }
          #endif
          #ifdef A_PDB
          case A_PDB: {
            jit_value_t ap_ = jit_insn_add(function, a, pc);
            jit_value_t ap_2, ap_3;
            BOUND_CORESIZE_HIGH_JIT(ap_2, ap_);
            jit_value_t tmp1 = JIT_CORE2_L(ap_2);
            jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
            jit_value_t tmpb_ = jit_insn_sub(function, tmpb, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpb_2;
            BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            ap_3 = jit_insn_add(function, ap_2, tmpb_2);
            BOUND_CORESIZE_HIGH_JIT(ap, ap_3);
            break;
          }
          #endif
          #ifdef A_ADB
          case A_ADB: {
            jit_value_t ap_ = jit_insn_add(function, a, pc);
            jit_value_t ap_2, ap_3, ap_4, ap_5;
            BOUND_CORESIZE_HIGH_JIT(ap_2, ap_);
            adpA = jit_insn_load(function, ap_2);
            jit_value_t tmp1 = JIT_CORE2_L(ap_2);
            jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
            ap_3 = jit_insn_add(function, tmpb, ap_2);
            BOUND_CORESIZE_HIGH_JIT(ap_4, ap_3);
            ap_5 = jit_insn_sub(ap_4, JIT_CONST(1, jit_type_addr2s));
            BOUND_CORESIZE_LOW_JIT(ap, ap_5);
            break;
          }
          #endif
          #ifdef A_PIA
          case A_PIA: {
            jit_value_t ap2 = jit_insn_add(function, a, pc);
            jit_value_t ap2_;
            BOUND_CORESIZE_HIGH_JIT(ap2_, ap2);
            jit_value_t tmp1 = JIT_CORE2_L(ap2_);
            jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
            jit_value_t ap_ = jit_insn_add(function, tmpa, ap2_);
            BOUND_CORESIZE_HIGH_JIT(ap, ap_);
            jit_value_t tmpa_ = jit_insn_add(function, tmpa, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpa_2;
            BOUND_CORESIZE_HIGH_JIT(tmpa_2, tmpa_);
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            break; }
          #endif
          #ifdef A_PIB
          case A_PIB: {
            jit_value_t ap2 = jit_insn_add(function, a, pc);
            jit_value_t ap2_;
            BOUND_CORESIZE_HIGH_JIT(ap2_, ap2);
            jit_value_t tmp1 = JIT_CORE2_L(ap2_);
            jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
            jit_value_t ap_ = jit_insn_add(function, tmpb, ap2_);
            BOUND_CORESIZE_HIGH_JIT(ap, ap_);
            jit_value_t tmpb_ = jit_insn_add(function, tmpb, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpb_2;
            BOUND_CORESIZE_HIGH_JIT(tmpb_2, tmpb_);
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            break; }
          #endif
          default:
            error("Unknown address mode found by JIT-compiler: %d", c1[c]._aA);
        }
      }
      else { //only in/decrements
        switch(c1[c]._aA) {
          #ifdef A_PDA
          case A_PDA: {
            jit_value_t ap_ = jit_insn_add(function, a, pc);
            jit_value_t ap_2;
            BOUND_CORESIZE_HIGH_JIT(ap_2, ap_);
            jit_value_t tmp1 = JIT_CORE2_L(ap_2);
            jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
            jit_value_t tmpa_ = jit_insn_sub(function, tmpa, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpa_2;
            BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            break; }
          #endif
          #ifdef A_PDB
          case A_PDB: {
            jit_value_t ap_ = jit_insn_add(function, a, pc);
            jit_value_t ap_2;
            BOUND_CORESIZE_HIGH_JIT(ap_2, ap_);
            jit_value_t tmp1 = JIT_CORE2_L(ap_2);
            jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
            jit_value_t tmpb_ = jit_insn_sub(function, tmpb, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpb_2;
            BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            break;
          }
          #endif
          #ifdef A_ADB
          case A_ADB: {
            jit_value_t ap_ = jit_insn_add(function, a, pc);
            jit_value_t ap_2;
            BOUND_CORESIZE_HIGH_JIT(ap_2, ap_);
            adpA = jit_insn_load(function, ap_2);
            break;
          }
          #endif
          #ifdef A_PIA
          case A_PIA: {
            jit_value_t ap_ = jit_insn_add(function, a, pc);
            jit_value_t ap_2;
            BOUND_CORESIZE_HIGH_JIT(ap_2, ap_);
            jit_value_t tmp1 = JIT_CORE2_L(ap_2);
            jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
            jit_value_t tmpa_ = jit_insn_add(function, tmpa, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpa_2;
            BOUND_CORESIZE_HIGH_JIT(tmpa_2, tmpa_);
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            break; }
          #endif
          #ifdef A_PIB
          case A_PIB: {
            jit_value_t ap_ = jit_insn_add(function, a, pc);
            jit_value_t ap_2;
            BOUND_CORESIZE_HIGH_JIT(ap_2, ap_);
            jit_value_t tmp1 = JIT_CORE2_L(ap_2);
            jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
            jit_value_t tmpb_ = jit_insn_add(function, tmpb, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpb_2;
            BOUND_CORESIZE_HIGH_JIT(tmpb_2, tmpb_);
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            break; }
          #endif
          default:
            break;
        }
      }

      int a_imm = 0, b_imm = 0;
      /* no matter if they are actually immediate,
      if the other is pre-/post-/auto-something,
      we can't use immediate optimization*/
      if(c1[c]._aA == A_IMM) {
        switch(c1[c]._aB) {
          case A_IMM: case A_DIR: case A_INA: case A_INB: a_imm = 1;
        }
      }
      if(c1[c]._aB == A_IMM) {
        switch(c1[c]._aA) {
          case A_IMM: case A_DIR: case A_INA: case A_INB: b_imm = 1;
        }
      }
      if(a_imm) need_ai = 0; //if we use a and b, we don't need ai_a and ai_b

      jit_value_t ai_a = NULL, ai_b = NULL;
      if(need_ai) {
        jit_value_t tmp1 = JIT_CORE2_L(ap);
        switch(c1[c]._O) {
          #ifdef O_MOV
          case O_MOV:
          #endif
          #ifdef O_ADD
          case O_ADD:
          #endif
          #ifdef O_SUB
          case O_SUB:
          #endif
          #ifdef O_MUL
          case O_MUL:
          #endif
          #ifdef O_DIV
          case O_DIV:
          #endif
          #ifdef O_MOD
          case O_MOD:
          #endif
          #if defined(O_CMP) || defined(O_SEQ)
          #ifdef O_CMP
          case O_CMP:
          #else
          case O_SEQ:
          #endif
          #endif
          #ifdef O_SNE
          case O_SNE:
          #endif
          #ifdef O_SLT
          case O_SLT:
          #endif
          #ifdef EXT_XCH_b
          case O_XCH:
          #endif
          #ifdef O_STS
          case O_STS:
          #endif
            switch(c1[c]._M) {
              case M_A: case M_AB: {
                ai_a = JIT_INSTR2_L(tmp1, a);
                break; }
              case M_B: case M_BA: {
                ai_b = JIT_INSTR2_L(tmp1, b);
                break; }
              case M_F: case M_X: case M_I: {
                ai_a = JIT_INSTR2_L(tmp1, a);
                ai_b = JIT_INSTR2_L(tmp1, b);
                break; }
            }
            break;

          #ifdef O_LDP
          case O_LDP:
          #endif
          #ifdef O_STP
          case O_STP:
          #endif
            switch(c1[c]._M) {
              case M_A: case M_AB: {
                ai_a = JIT_INSTR2_L(tmp1, a);
                break; }
              case M_B: case M_BA: case M_F: case M_X: case M_I: {
                ai_b = JIT_INSTR2_L(tmp1, b);
                break; }
            }
            break;

          #ifdef EXT_XCH_a
          case O_XCH:
          #endif
            ai_a = JIT_INSTR2_L(tmp1, a);
            ai_b = JIT_INSTR2_L(tmp1, b);
            break;
        }
      }
      else {
        ai_a = a;
        ai_b = b;
      }

      if(need_bp) {
        switch(c1[c]._aB) {
          #ifdef A_IMM
          case A_IMM:
            bp = pc;
            break;
          #endif
          #ifdef A_DIR
          case A_DIR: {
            jit_value_t bp_ = jit_insn_add(function, b, pc);
            BOUND_CORESIZE_HIGH_JIT(bp, bp_);
            break; }
          #endif
          #ifdef A_INA
          case A_INA: {
            jit_value_t bp_ = jit_insn_add(function, b, pc);
            jit_value_t bp_2, bp_3;
            BOUND_CORESIZE_HIGH_JIT(bp_2, bp_);
            jit_value_t tmp1 = JIT_CORE2_L(bp_2);
            jit_value_t tmp1_ = JIT_INSTR2_L(tmp1, a);
            bp_3 = jit_insn_add(function, bp_2, tmp1_);
            BOUND_CORESIZE_HIGH_JIT(bp, bp_3);
            break; }
          #endif
          #ifdef A_INB
          case A_INB: {
            jit_value_t bp_ = jit_insn_add(function, b, pc);
            jit_value_t bp_2, bp_3;
            BOUND_CORESIZE_HIGH_JIT(bp_2, bp_);
            jit_value_t tmp1 = JIT_CORE2_L(bp_2);
            jit_value_t tmp1_ = JIT_INSTR2_L(tmp1, b);
            bp_3 = jit_insn_add(function, bp_2, tmp1_);
            BOUND_CORESIZE_HIGH_JIT(bp, bp_3);
            break; }
          #endif
          #ifdef A_PDA
          case A_PDA: {
            jit_value_t bp_ = jit_insn_add(function, b, pc);
            jit_value_t bp_2, bp_3;
            BOUND_CORESIZE_HIGH_JIT(bp_2, bp_);
            jit_value_t tmp1 = JIT_CORE2_L(bp_2);
            jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
            jit_value_t tmpa_ = jit_insn_sub(function, tmpa, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpa_2;
            BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            bp_3 = jit_insn_add(function, bp_2, tmpa_2);
            BOUND_CORESIZE_HIGH_JIT(bp, bp_3);
            break; }
          #endif
          #ifdef A_PDB
          case A_PDB: {
            jit_value_t bp_ = jit_insn_add(function, b, pc);
            jit_value_t bp_2, bp_3;
            BOUND_CORESIZE_HIGH_JIT(bp_2, bp_);
            jit_value_t tmp1 = JIT_CORE2_L(bp_2);
            jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
            jit_value_t tmpb_ = jit_insn_sub(function, tmpb, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpb_2;
            BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            bp_3 = jit_insn_add(function, bp_2, tmpb_2);
            BOUND_CORESIZE_HIGH_JIT(bp, bp_3);
            break;
          }
          #endif
          #ifdef A_ADB
          case A_ADB: {
            jit_value_t bp_ = jit_insn_add(function, b, pc);
            jit_value_t bp_2, bp_3, bp_4, bp_5;
            BOUND_CORESIZE_HIGH_JIT(bp_2, bp_);
            adpA = jit_insn_load(function, bp_2);
            jit_value_t tmp1 = JIT_CORE2_L(bp_2);
            jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
            bp_3 = jit_insn_add(function, tmpb, bp_2);
            BOUND_CORESIZE_HIGH_JIT(bp_4, bp_3);
            bp_5 = jit_insn_sub(bp_4, JIT_CONST(1, jit_type_addr2s));
            BOUND_CORESIZE_LOW_JIT(bp, bp_5);
            break;
          }
          #endif
          #ifdef A_PIA
          case A_PIA: {
            jit_value_t bp2 = jit_insn_add(function, b, pc);
            jit_value_t bp2_;
            BOUND_CORESIZE_HIGH_JIT(bp2_, bp2);
            jit_value_t tmp1 = JIT_CORE2_L(bp2_);
            jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
            jit_value_t bp_ = jit_insn_add(function, tmpa, bp2_);
            BOUND_CORESIZE_HIGH_JIT(bp, bp_);
            jit_value_t tmpa_ = jit_insn_add(function, tmpa, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpa_2;
            BOUND_CORESIZE_HIGH_JIT(tmpa_2, tmpa_);
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            break; }
          #endif
          #ifdef A_PIB
          case A_PIB: {
            jit_value_t bp2 = jit_insn_add(function, b, pc);
            jit_value_t bp2_;
            BOUND_CORESIZE_HIGH_JIT(bp2_, bp2);
            jit_value_t tmp1 = JIT_CORE2_L(bp2_);
            jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
            jit_value_t bp_ = jit_insn_add(function, tmpb, bp2_);
            BOUND_CORESIZE_HIGH_JIT(bp, bp_);
            jit_value_t tmpb_ = jit_insn_add(function, tmpb, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpb_2;
            BOUND_CORESIZE_HIGH_JIT(tmpb_2, tmpb_);
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            break; }
          #endif
          default:
            error("Unknown address mode found by JIT-compiler: %d", c1[c]._aB);
        }
      }
      else { //only in/decrements
        switch(c1[c]._aB) {
          #ifdef A_PDA
          case A_PDA: {
            jit_value_t bp_ = jit_insn_add(function, b, pc);
            jit_value_t bp_2;
            BOUND_CORESIZE_HIGH_JIT(bp_2, bp_);
            jit_value_t tmp1 = JIT_CORE2_L(bp_2);
            jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
            jit_value_t tmpa_ = jit_insn_sub(function, tmpa, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpa_2;
            BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            break; }
          #endif
          #ifdef A_PDB
          case A_PDB: {
            jit_value_t bp_ = jit_insn_add(function, b, pc);
            jit_value_t bp_2;
            BOUND_CORESIZE_HIGH_JIT(bp_2, bp_);
            jit_value_t tmp1 = JIT_CORE2_L(bp_2);
            jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
            jit_value_t tmpb_ = jit_insn_sub(function, tmpb, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpb_2;
            BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            break;
          }
          #endif
          #ifdef A_ADB
          case A_ADB: {
            jit_value_t bp_ = jit_insn_add(function, b, pc);
            jit_value_t bp_2;
            BOUND_CORESIZE_HIGH_JIT(bp_2, bp_);
            adpA = jit_insn_load(function, bp_2);
            break;
          }
          #endif
          #ifdef A_PIA
          case A_PIA: {
            jit_value_t bp_ = jit_insn_add(function, b, pc);
            jit_value_t bp_2;
            BOUND_CORESIZE_HIGH_JIT(bp_2, bp_);
            jit_value_t tmp1 = JIT_CORE2_L(bp_2);
            jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
            jit_value_t tmpa_ = jit_insn_add(function, tmpa, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpa_2;
            BOUND_CORESIZE_HIGH_JIT(tmpa_2, tmpa_);
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            break; }
          #endif
          #ifdef A_PIB
          case A_PIB: {
            jit_value_t bp_ = jit_insn_add(function, b, pc);
            jit_value_t bp_2;
            BOUND_CORESIZE_HIGH_JIT(bp_2, bp_);
            jit_value_t tmp1 = JIT_CORE2_L(bp_2);
            jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
            jit_value_t tmpb_ = jit_insn_add(function, tmpb, JIT_CONST(1, jit_type_addr2s));
            jit_value_t tmpb_2;
            BOUND_CORESIZE_HIGH_JIT(tmpb_2, tmpb_);
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            break; }
          #endif
          default:
            break;
        }
      }

      #ifdef A_ADB //auto-decrement
      if(c1[c]._aA == A_ADB) {
        jit_value_t tmp1 = JIT_CORE2_L(adpA);
        jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
        jit_value_t tmpb_ = jit_insn_sub(function, tmpb, JIT_CONST(1, jit_type_addr2s));
        jit_value_t tmpb_2;
        BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
        JIT_INSTR2_S(tmp1, b, tmpb_2);
      }
      if(c1[c]._aB == A_ADB) {
        jit_value_t tmp1 = JIT_CORE2_L(adpB);
        jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
        jit_value_t tmpb_ = jit_insn_sub(function, tmpb, JIT_CONST(1, jit_type_addr2s));
        jit_value_t tmpb_2;
        BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
        JIT_INSTR2_S(tmp1, b, tmpb_2);
      }
      #endif

      switch(c1[c]._O) { //execute
        #ifdef O_DAT
        case O_DAT:
          //jit_type_t params2[4] = {jit_type_void_ptr, jit_type_addr2, jit_type_addr2, jit_type_addr2};
          REMOVE_PROC_JIT();
          break;
        #endif
        #ifdef O_MOV
        case O_MOV:
          switch(c1[c]._M) {
            case M_I: {
              jit_value_t tmp1 = JIT_CORE2_L(ap);
              jit_value_t tmpi = JIT_INSTR2_fn_L(tmp1);
              jit_value_t tmp1_ = JIT_CORE2_L(bp);
              JIT_INSTR2_S(tmp1_, fn, tmpi);
              JIT_INSTR2_S(tmp1_, a, ai_a);
              JIT_INSTR2_S(tmp1_, b, ai_b);
              break;
            }
            case M_A: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              JIT_INSTR2_S(tmp1, a, ai_a);
              break;
            }
            case M_B: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              JIT_INSTR2_S(tmp1, b, ai_b);
              break;
            }
            case M_AB: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              JIT_INSTR2_S(tmp1, b, ai_a);
              break;
            }
            case M_BA: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              JIT_INSTR2_S(tmp1, a, ai_b);
              break;
            }
            case M_F: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              JIT_INSTR2_S(tmp1, a, ai_a);
              JIT_INSTR2_S(tmp1, b, ai_b);
              break;
            }
            case M_X: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              JIT_INSTR2_S(tmp1, b, ai_a);
              JIT_INSTR2_S(tmp1, a, ai_b);
              break;
            }
          }
          break;
        #endif
        #ifdef O_ADD
        case O_ADD:
          switch(c1[c]._M) {
            case M_A: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
              jit_value_t tmpa_ = jit_insn_add(function, tmpa, ai_a);
              jit_value_t tmpa_2;
              BOUND_CORESIZE_HIGH_JIT(tmpa_2, tmpa_);
              JIT_INSTR2_S(tmp1, a, tmpa_2);
              break;
            }
            case M_B: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
              jit_value_t tmpb_ = jit_insn_add(function, tmpb, ai_b);
              jit_value_t tmpb_2;
              BOUND_CORESIZE_HIGH_JIT(tmpb_2, tmpb_);
              JIT_INSTR2_S(tmp1, b, tmpb_2);
              break;
            }
            case M_AB: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
              jit_value_t tmpb_ = jit_insn_add(function, tmpb, ai_a);
              jit_value_t tmpb_2;
              BOUND_CORESIZE_HIGH_JIT(tmpb_2, tmpb_);
              JIT_INSTR2_S(tmp1, b, tmpb_2);
              break;
            }
            case M_BA: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
              jit_value_t tmpa_ = jit_insn_add(function, tmpa, ai_b);
              jit_value_t tmpa_2;
              BOUND_CORESIZE_HIGH_JIT(tmpa_2, tmpa_);
              JIT_INSTR2_S(tmp1, a, tmpa_2);
              break;
            }
            case M_I: case M_F: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
              jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
              jit_value_t tmpa_ = jit_insn_add(function, tmpa, ai_a);
              jit_value_t tmpb_ = jit_insn_add(function, tmpb, ai_b);
              jit_value_t tmpa_2, tmpb_2;
              BOUND_CORESIZE_HIGH_JIT(tmpa_2, tmpa_);
              BOUND_CORESIZE_HIGH_JIT(tmpb_2, tmpb_);
              JIT_INSTR2_S(tmp1, a, tmpa_2);
              JIT_INSTR2_S(tmp1, b, tmpb_2);
              break;
            }
            case M_X: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
              jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
              jit_value_t tmpa_ = jit_insn_add(function, tmpa, ai_b);
              jit_value_t tmpb_ = jit_insn_add(function, tmpb, ai_a);
              jit_value_t tmpa_2, tmpb_2;
              BOUND_CORESIZE_HIGH_JIT(tmpa_2, tmpa_);
              BOUND_CORESIZE_HIGH_JIT(tmpb_2, tmpb_);
              JIT_INSTR2_S(tmp1, a, tmpa_2);
              JIT_INSTR2_S(tmp1, b, tmpb_2);
              break;
            }
          }
          break;
        #endif
        #ifdef O_SUB
        case O_SUB:
          switch(c1[c]._M) {
              case M_A: {
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                jit_value_t tmpa_ = jit_insn_sub(function, tmpa, ai_a);
                jit_value_t tmpa_2;
                BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
                JIT_INSTR2_S(tmp1, a, tmpa_2);
                break;
              }
              case M_B: {
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                jit_value_t tmpb_ = jit_insn_sub(function, tmpb, ai_b);
                jit_value_t tmpb_2;
                BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
                JIT_INSTR2_S(tmp1, b, tmpb_2);
                break;
              }
              case M_AB: {
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                jit_value_t tmpb_ = jit_insn_sub(function, tmpb, ai_a);
                jit_value_t tmpb_2;
                BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
                JIT_INSTR2_S(tmp1, b, tmpb_2);
                break;
              }
              case M_BA: {
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                jit_value_t tmpa_ = jit_insn_sub(function, tmpa, ai_b);
                jit_value_t tmpa_2;
                BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
                JIT_INSTR2_S(tmp1, a, tmpa_2);
                break;
              }
              case M_I: case M_F: {
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                jit_value_t tmpa_ = jit_insn_sub(function, tmpa, ai_a);
                jit_value_t tmpb_ = jit_insn_sub(function, tmpb, ai_b);
                jit_value_t tmpa_2, tmpb_2;
                BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
                BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
                JIT_INSTR2_S(tmp1, a, tmpa_2);
                JIT_INSTR2_S(tmp1, b, tmpb_2);
                break;
              }
              case M_X: {
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                jit_value_t tmpa_ = jit_insn_sub(function, tmpa, ai_b);
                jit_value_t tmpb_ = jit_insn_sub(function, tmpb, ai_a);
                jit_value_t tmpa_2, tmpb_2;
                BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
                BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
                JIT_INSTR2_S(tmp1, a, tmpa_2);
                JIT_INSTR2_S(tmp1, b, tmpb_2);
                break;
              }
            }
          break;
        #endif
        #ifdef O_MUL
        case O_MUL:
          switch(c1[c]._M) {
              case M_A: {
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                jit_value_t tmpa_ = jit_insn_mul(function, ai_a, tmpa);
                jit_value_t tmpa_2 = jit_insn_rem(function, tmpa_, JIT_CONST(CORESIZE, jit_type_addr2));
                JIT_INSTR2_S(tmp1, a, tmpa_2);
                break;
              }
              case M_B: {
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                jit_value_t tmpb_ = jit_insn_mul(function, ai_b, tmpb);
                jit_value_t tmpb_2 = jit_insn_rem(function, tmpb_, JIT_CONST(CORESIZE, jit_type_addr2));
                JIT_INSTR2_S(tmp1, b, tmpb_2);
                break;
              }
              case M_AB: {
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                jit_value_t tmpb_ = jit_insn_mul(function, ai_a, tmpb);
                jit_value_t tmpb_2 = jit_insn_rem(function, tmpb_, JIT_CONST(CORESIZE, jit_type_addr2));
                JIT_INSTR2_S(tmp1, b, tmpb_2);
                break;
              }
              case M_BA: {
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                jit_value_t tmpa_ = jit_insn_mul(function, ai_b, tmpa);
                jit_value_t tmpa_2 = jit_insn_rem(function, tmpa_, JIT_CONST(CORESIZE, jit_type_addr2));
                JIT_INSTR2_S(tmp1, a, tmpa_2);
                break;
              }
              case M_F: case M_I: {
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                jit_value_t tmpa_ = jit_insn_mul(function, ai_a, tmpa);
                jit_value_t tmpb_ = jit_insn_mul(function, ai_b, tmpb);
                jit_value_t tmpa_2 = jit_insn_rem(function, tmpa_, JIT_CONST(CORESIZE, jit_type_addr2));
                jit_value_t tmpb_2 = jit_insn_rem(function, tmpb_, JIT_CONST(CORESIZE, jit_type_addr2));
                JIT_INSTR2_S(tmp1, a, tmpa_2);
                JIT_INSTR2_S(tmp1, b, tmpb_2);
                break;
              }
              case M_X: {
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                jit_value_t tmpa_ = jit_insn_mul(function, ai_b, tmpa);
                jit_value_t tmpb_ = jit_insn_mul(function, ai_a, tmpb);
                jit_value_t tmpa_2 = jit_insn_rem(function, tmpa_, JIT_CONST(CORESIZE, jit_type_addr2));
                jit_value_t tmpb_2 = jit_insn_rem(function, tmpb_, JIT_CONST(CORESIZE, jit_type_addr2));
                JIT_INSTR2_S(tmp1, a, tmpa_2);
                JIT_INSTR2_S(tmp1, b, tmpb_2);
                break;
              }
            }
          break;
        #endif
        #ifdef O_DIV
        case O_DIV: {
          switch(c1[c]._M) {
              case M_A: {
                jit_label_t labele = jit_label_undefined;
                jit_label_t labelb = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele);
                  jit_value_t tmp1 = JIT_CORE2_L(bp);
                  jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                  jit_value_t tmpa_ = jit_insn_div(function, tmpa, ai_a);
                  JIT_INSTR2_S(tmp1, a, tmpa_);
                jit_insn_branch(function, &labelb);
                jit_insn_label(function, &labele);
                  REMOVE_PROC_JIT(); //automatically jit_insn_returns
                jit_insn_label(function, &labelb);
                break; }
              case M_B: {
                jit_label_t labele = jit_label_undefined;
                jit_label_t labelb = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_b), &labele);
                  jit_value_t tmp1 = JIT_CORE2_L(bp);
                  jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                  jit_value_t tmpb_ = jit_insn_div(function, tmpb, ai_b);
                  JIT_INSTR2_S(tmp1, b, tmpb_);
                jit_insn_branch(function, &labelb);
                jit_insn_label(function, &labele);
                  REMOVE_PROC_JIT(); //automatically jit_insn_returns
                jit_insn_label(function, &labelb);
                break; }
              case M_AB: {
                jit_label_t labele = jit_label_undefined;
                jit_label_t labelb = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele);
                  jit_value_t tmp1 = JIT_CORE2_L(bp);
                  jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                  jit_value_t tmpb_ = jit_insn_div(function, tmpb, ai_a);
                  JIT_INSTR2_S(tmp1, b, tmpb_);
                jit_insn_branch(function, &labelb);
                jit_insn_label(function, &labele);
                  REMOVE_PROC_JIT(); //automatically jit_insn_returns
                jit_insn_label(function, &labelb);
                break; }
              case M_BA: {
                jit_label_t labele = jit_label_undefined;
                jit_label_t labelb = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_b), &labele);
                  jit_value_t tmp1 = JIT_CORE2_L(bp);
                  jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                  jit_value_t tmpa_ = jit_insn_div(function, tmpa, ai_b);
                  JIT_INSTR2_S(tmp1, a, tmpa_);
                jit_insn_branch(function, &labelb);
                jit_insn_label(function, &labele);
                  REMOVE_PROC_JIT(); //automatically jit_insn_returns
                jit_insn_label(function, &labelb);
                break; }
              case M_F: case M_I: {
                jit_value_t chkend = JIT_CONST(0, jit_type_sys_int);
                jit_label_t labele1 = jit_label_undefined;
                jit_label_t labelb1 = jit_label_undefined;
                jit_label_t labele2 = jit_label_undefined;
                jit_label_t labelb2 = jit_label_undefined;
                jit_label_t labelb3 = jit_label_undefined;
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_b), &labele1);
                  jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                  jit_value_t tmpb_ = jit_insn_div(function, tmpb, ai_b);
                  JIT_INSTR2_S(tmp1, b, tmpb_);
                jit_insn_branch(function, &labelb1);
                jit_insn_label(function, &labele1);
                  jit_insn_store(function, chkend, JIT_CONST(1, jit_type_sys_int));
                jit_insn_label(function, &labelb1);
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele2);
                  jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                  jit_value_t tmpa_ = jit_insn_div(function, tmpa, ai_a);
                  JIT_INSTR2_S(tmp1, a, tmpa_);
                jit_insn_branch(function, &labelb2);
                jit_insn_label(function, &labele2);
                  jit_insn_store(function, chkend, JIT_CONST(1, jit_type_sys_int));
                jit_insn_label(function, &labelb2);

                jit_insn_branch_if_not(function, jit_insn_to_bool(function, chkend), &labelb3);
                  REMOVE_PROC_JIT();
                jit_insn_label(function, &labelb3);
                break; }
              case M_X: {
                jit_value_t chkend = JIT_CONST(0, jit_type_sys_int);
                jit_label_t labele1 = jit_label_undefined;
                jit_label_t labelb1 = jit_label_undefined;
                jit_label_t labele2 = jit_label_undefined;
                jit_label_t labelb2 = jit_label_undefined;
                jit_label_t labelb3 = jit_label_undefined;
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_b), &labele1);
                  jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                  jit_value_t tmpa_ = jit_insn_div(function, tmpa, ai_b);
                  JIT_INSTR2_S(tmp1, a, tmpa_);
                jit_insn_branch(function, &labelb1);
                jit_insn_label(function, &labele1);
                  jit_insn_store(function, chkend, JIT_CONST(1, jit_type_sys_int));
                jit_insn_label(function, &labelb1);
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele2);
                  jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                  jit_value_t tmpb_ = jit_insn_div(function, tmpb, ai_a);
                  JIT_INSTR2_S(tmp1, b, tmpb_);
                jit_insn_branch(function, &labelb2);
                jit_insn_label(function, &labele2);
                  jit_insn_store(function, chkend, JIT_CONST(1, jit_type_sys_int));
                jit_insn_label(function, &labelb2);

                jit_insn_branch_if_not(function, jit_insn_to_bool(function, chkend), &labelb3);
                  REMOVE_PROC_JIT();
                jit_insn_label(function, &labelb3);
                break; }
            }
          break; }
        #endif
        #ifdef O_MOD
        case O_MOD: {
          switch(c1[c]._M) {
              case M_A: {
                jit_label_t labele = jit_label_undefined;
                jit_label_t labelb = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele);
                  jit_value_t tmp1 = JIT_CORE2_L(bp);
                  jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                  jit_value_t tmpa_ = jit_insn_rem(function, tmpa, ai_a);
                  JIT_INSTR2_S(tmp1, a, tmpa_);
                jit_insn_branch(function, &labelb);
                jit_insn_label(function, &labele);
                  REMOVE_PROC_JIT(); //automatically jit_insn_returns
                jit_insn_label(function, &labelb);
                break; }
              case M_B: {
                jit_label_t labele = jit_label_undefined;
                jit_label_t labelb = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_b), &labele);
                  jit_value_t tmp1 = JIT_CORE2_L(bp);
                  jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                  jit_value_t tmpb_ = jit_insn_rem(function, tmpb, ai_b);
                  JIT_INSTR2_S(tmp1, b, tmpb_);
                jit_insn_branch(function, &labelb);
                jit_insn_label(function, &labele);
                  REMOVE_PROC_JIT(); //automatically jit_insn_returns
                jit_insn_label(function, &labelb);
                break; }
              case M_AB: {
                jit_label_t labele = jit_label_undefined;
                jit_label_t labelb = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele);
                  jit_value_t tmp1 = JIT_CORE2_L(bp);
                  jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                  jit_value_t tmpb_ = jit_insn_rem(function, tmpb, ai_a);
                  JIT_INSTR2_S(tmp1, b, tmpb_);
                jit_insn_branch(function, &labelb);
                jit_insn_label(function, &labele);
                  REMOVE_PROC_JIT(); //automatically jit_insn_returns
                jit_insn_label(function, &labelb);
                break; }
              case M_BA: {
                jit_label_t labele = jit_label_undefined;
                jit_label_t labelb = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_b), &labele);
                  jit_value_t tmp1 = JIT_CORE2_L(bp);
                  jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                  jit_value_t tmpa_ = jit_insn_rem(function, tmpa, ai_b);
                  JIT_INSTR2_S(tmp1, a, tmpa_);
                jit_insn_branch(function, &labelb);
                jit_insn_label(function, &labele);
                  REMOVE_PROC_JIT(); //automatically jit_insn_returns
                jit_insn_label(function, &labelb);
                break; }
              case M_F: case M_I: {
                jit_value_t chkend = JIT_CONST(0, jit_type_sys_int);
                jit_label_t labele1 = jit_label_undefined;
                jit_label_t labelb1 = jit_label_undefined;
                jit_label_t labele2 = jit_label_undefined;
                jit_label_t labelb2 = jit_label_undefined;
                jit_label_t labelb3 = jit_label_undefined;
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_b), &labele1);
                  jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                  jit_value_t tmpb_ = jit_insn_rem(function, tmpb, ai_b);
                  JIT_INSTR2_S(tmp1, b, tmpb_);
                jit_insn_branch(function, &labelb1);
                jit_insn_label(function, &labele1);
                  jit_insn_store(function, chkend, JIT_CONST(1, jit_type_sys_int));
                jit_insn_label(function, &labelb1);
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele2);
                  jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                  jit_value_t tmpa_ = jit_insn_rem(function, tmpa, ai_a);
                  JIT_INSTR2_S(tmp1, a, tmpa_);
                jit_insn_branch(function, &labelb2);
                jit_insn_label(function, &labele2);
                  jit_insn_store(function, chkend, JIT_CONST(1, jit_type_sys_int));
                jit_insn_label(function, &labelb2);

                jit_insn_branch_if_not(function, jit_insn_to_bool(function, chkend), &labelb3);
                  REMOVE_PROC_JIT();
                jit_insn_label(function, &labelb3);
                break; }
              case M_X: {
                jit_value_t chkend = JIT_CONST(0, jit_type_sys_int);
                jit_label_t labele1 = jit_label_undefined;
                jit_label_t labelb1 = jit_label_undefined;
                jit_label_t labele2 = jit_label_undefined;
                jit_label_t labelb2 = jit_label_undefined;
                jit_label_t labelb3 = jit_label_undefined;
                jit_value_t tmp1 = JIT_CORE2_L(bp);
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_b), &labele1);
                  jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
                  jit_value_t tmpa_ = jit_insn_rem(function, tmpa, ai_b);
                  JIT_INSTR2_S(tmp1, a, tmpa_);
                jit_insn_branch(function, &labelb1);
                jit_insn_label(function, &labele1);
                  jit_insn_store(function, chkend, JIT_CONST(1, jit_type_sys_int));
                jit_insn_label(function, &labelb1);
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele2);
                  jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
                  jit_value_t tmpb_ = jit_insn_rem(function, tmpb, ai_a);
                  JIT_INSTR2_S(tmp1, b, tmpb_);
                jit_insn_branch(function, &labelb2);
                jit_insn_label(function, &labele2);
                  jit_insn_store(function, chkend, JIT_CONST(1, jit_type_sys_int));
                jit_insn_label(function, &labelb2);

                jit_insn_branch_if_not(function, jit_insn_to_bool(function, chkend), &labelb3);
                  REMOVE_PROC_JIT();
                jit_insn_label(function, &labelb3);
                break; }
            }
          break; }
        #endif
        #ifdef O_JMP
        case O_JMP:  {
          jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
          JIT_PROC2_pos_S(tmp1, ap);
          jit_value_t next = JIT_PROC2_next_L(tmp1);
          jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next);
          break; }
        #endif
        #ifdef O_JMZ
        case O_JMZ:
          if(b_imm) {
            switch(c1[c]._M) {
              case M_A: case M_BA: {
                jit_label_t labele = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, a), &labele);
                  jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
                  JIT_PROC2_pos_S(tmp1, ap);
                  jit_value_t next = JIT_PROC2_next_L(tmp1);
                  jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
                  jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
                jit_insn_label(function, &labele);
                break; }
              case M_B: case M_AB: {
                jit_label_t labele = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, b), &labele);
                  jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
                  JIT_PROC2_pos_S(tmp1, ap);
                  jit_value_t next = JIT_PROC2_next_L(tmp1);
                  jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
                  jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
                jit_insn_label(function, &labele);
                break; }
              case M_F: case M_I: case M_X: {
                jit_label_t labele = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, a), &labele);
                jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, b), &labele);
                  jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
                  JIT_PROC2_pos_S(tmp1, ap);
                  jit_value_t next = JIT_PROC2_next_L(tmp1);
                  jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
                  jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
                jit_insn_label(function, &labele);
                break; }
            }
          }
          else {
            switch(c1[c]._M) {
              case M_A: case M_BA: {
                jit_label_t labele = jit_label_undefined;
                jit_value_t tmp2 = JIT_CORE2_L(bp);
                jit_value_t ba = JIT_INSTR2_L(tmp2, a);
                jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, ba), &labele);
                  jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
                  JIT_PROC2_pos_S(tmp1, ap);
                  jit_value_t next = JIT_PROC2_next_L(tmp1);
                  jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
                  jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
                jit_insn_label(function, &labele);
                break; }
              case M_B: case M_AB: {
                jit_label_t labele = jit_label_undefined;
                jit_value_t tmp2 = JIT_CORE2_L(bp);
                jit_value_t bb = JIT_INSTR2_L(tmp2, b);
                jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, bb), &labele);
                  jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
                  JIT_PROC2_pos_S(tmp1, ap);
                  jit_value_t next = JIT_PROC2_next_L(tmp1);
                  jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
                  jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
                jit_insn_label(function, &labele);
                break; }
              case M_F: case M_I: case M_X: {
                jit_label_t labele = jit_label_undefined;
                jit_value_t tmp2 = JIT_CORE2_L(bp);
                jit_value_t ba = JIT_INSTR2_L(tmp2, a);
                jit_value_t bb = JIT_INSTR2_L(tmp2, b);
                jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, ba), &labele);
                jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, bb), &labele);
                  jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
                  JIT_PROC2_pos_S(tmp1, ap);
                  jit_value_t next = JIT_PROC2_next_L(tmp1);
                  jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
                  jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
                jit_insn_label(function, &labele);
                break; }
            }
          }
          break;
        #endif
        #ifdef O_JMN
        case O_JMN:
          if(b_imm) {
            switch(c1[c]._M) {
              case M_A: case M_BA: {
                jit_label_t labele = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, a), &labele);
                  jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
                  JIT_PROC2_pos_S(tmp1, ap);
                  jit_value_t next = JIT_PROC2_next_L(tmp1);
                  jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
                  jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
                jit_insn_label(function, &labele);
                break; }
              case M_B: case M_AB: {
                jit_label_t labele = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, b), &labele);
                  jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
                  JIT_PROC2_pos_S(tmp1, ap);
                  jit_value_t next = JIT_PROC2_next_L(tmp1);
                  jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
                  jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
                jit_insn_label(function, &labele);
                break; }
              case M_F: case M_I: case M_X: {
                jit_label_t labele = jit_label_undefined;
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, a), &labele);
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, b), &labele);
                  jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
                  JIT_PROC2_pos_S(tmp1, ap);
                  jit_value_t next = JIT_PROC2_next_L(tmp1);
                  jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
                  jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
                jit_insn_label(function, &labele);
                break; }
            }
          }
          else {
            switch(c1[c]._M) {
              case M_A: case M_BA: {
                jit_label_t labele = jit_label_undefined;
                jit_value_t tmp2 = JIT_CORE2_L(bp);
                jit_value_t ba = JIT_INSTR2_L(tmp2, a);
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ba), &labele);
                  jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
                  JIT_PROC2_pos_S(tmp1, ap);
                  jit_value_t next = JIT_PROC2_next_L(tmp1);
                  jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
                  jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
                jit_insn_label(function, &labele);
                break; }
              case M_B: case M_AB: {
                jit_label_t labele = jit_label_undefined;
                jit_value_t tmp2 = JIT_CORE2_L(bp);
                jit_value_t bb = JIT_INSTR2_L(tmp2, b);
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, bb), &labele);
                  jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
                  JIT_PROC2_pos_S(tmp1, ap);
                  jit_value_t next = JIT_PROC2_next_L(tmp1);
                  jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
                  jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
                jit_insn_label(function, &labele);
                break; }
              case M_F: case M_I: case M_X: {
                jit_label_t labele = jit_label_undefined;
                jit_value_t tmp2 = JIT_CORE2_L(bp);
                jit_value_t ba = JIT_INSTR2_L(tmp2, a);
                jit_value_t bb = JIT_INSTR2_L(tmp2, b);
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, ba), &labele);
                jit_insn_branch_if_not(function, jit_insn_to_bool(function, bb), &labele);
                  jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
                  JIT_PROC2_pos_S(tmp1, ap);
                  jit_value_t next = JIT_PROC2_next_L(tmp1);
                  jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;                  jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
                jit_insn_label(function, &labele);
                break; }
            }
          }
          break;
        #endif
        #ifdef O_DJZ
        case O_DJZ: {
          jit_value_t tmp2 = JIT_LPROC2_L(JIT_W());
          jit_label_t labele = jit_label_undefined, labelb = jit_label_undefined;
          switch(c1[c]._M) {
            case M_A: case M_BA: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
              jit_value_t tmpa_ = jit_insn_sub(function, tmpa, JIT_CONST(1, jit_type_addr2s));
              jit_value_t tmpa_2;
              BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
              JIT_INSTR2_S(tmp1, a, tmpa_2);
              jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, tmpa_2), &labele);
              break; }
            case M_B: case M_AB: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
              jit_value_t tmpb_ = jit_insn_sub(function, tmpb, JIT_CONST(1, jit_type_addr2s));
              jit_value_t tmpb_2;
              BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
              JIT_INSTR2_S(tmp1, b, tmpb_2);
              jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, tmpb_2), &labele);
              break; }
            case M_F: case M_X: case M_I:  {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
              jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
              jit_value_t tmpa_ = jit_insn_sub(function, tmpa, JIT_CONST(1, jit_type_addr2s));
              jit_value_t tmpb_ = jit_insn_sub(function, tmpb, JIT_CONST(1, jit_type_addr2s));
              jit_value_t tmpa_2, tmpb_2;
              BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
              BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
              JIT_INSTR2_S(tmp1, a, tmpa_2);
              JIT_INSTR2_S(tmp1, b, tmpb_2);
              jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, tmpa_2), &labele);
              jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, tmpb_2), &labele);
              break; }
          }
            JIT_PROC2_pos_S(tmp2, ap);
          jit_insn_branch(function, &labelb);
          jit_insn_label(function, &labele);
            jit_value_t pos = JIT_PROC2_pos_L(tmp2);
            jit_value_t pos_ = jit_insn_add(function, pos, JIT_CONST(1, jit_type_addr2s));
            jit_value_t pos_2;
            BOUND_CORESIZE_HIGH_JIT(pos_2, pos_);
            JIT_PROC2_pos_S(tmp2, pos_2);
          jit_insn_label(function, &labelb);
          jit_value_t next = JIT_PROC2_next_L(tmp2);
          jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;                jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
          break; }
        #endif
        #ifdef O_DJN
        case O_DJN: {
          jit_value_t tmp2 = JIT_LPROC2_L(JIT_W());
          jit_label_t labele = jit_label_undefined, labelb = jit_label_undefined;
          switch(c1[c]._M) {
            case M_A: case M_BA: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
              jit_value_t tmpa_ = jit_insn_sub(function, tmpa, JIT_CONST(1, jit_type_addr2s));
              jit_value_t tmpa_2;
              BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
              JIT_INSTR2_S(tmp1, a, tmpa_2);
              jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpa_2), &labele);
              break; }
            case M_B: case M_AB: {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
              jit_value_t tmpb_ = jit_insn_sub(function, tmpb, JIT_CONST(1, jit_type_addr2s));
              jit_value_t tmpb_2;
              BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
              JIT_INSTR2_S(tmp1, b, tmpb_2);
              jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpb_2), &labele);
              break; }
            case M_F: case M_X: case M_I:  {
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpa = JIT_INSTR2_L(tmp1, a);
              jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
              jit_value_t tmpa_ = jit_insn_sub(function, tmpa, JIT_CONST(1, jit_type_addr2s));
              jit_value_t tmpb_ = jit_insn_sub(function, tmpb, JIT_CONST(1, jit_type_addr2s));
              jit_value_t tmpa_2, tmpb_2;
              BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
              BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
              JIT_INSTR2_S(tmp1, a, tmpa_2);
              JIT_INSTR2_S(tmp1, b, tmpb_2);
              jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpa_2), &labele);
              jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpb_2), &labele);
              break; }
          }
            JIT_PROC2_pos_S(tmp2, ap);
          jit_insn_branch(function, &labelb);
          jit_insn_label(function, &labele);
            jit_value_t pos = JIT_PROC2_pos_L(tmp2);
            jit_value_t pos_ = jit_insn_add(function, pos, JIT_CONST(1, jit_type_addr2s));
            jit_value_t pos_2;
            BOUND_CORESIZE_HIGH_JIT(pos_2, pos_);
            JIT_PROC2_pos_S(tmp2, pos_2);
          jit_insn_label(function, &labelb);
          jit_value_t next = JIT_PROC2_next_L(tmp2);
          jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;                jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
          break; }
        #endif
        #if defined(O_CMP) || defined(O_SEQ)
        #ifdef O_CMP
        case O_CMP:
        #else
        case O_SEQ:
        #endif
        {
          jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
          jit_value_t pos = JIT_PROC2_pos_L(tmp1);
          jit_label_t labele = jit_label_undefined;
          jit_label_t labelb = jit_label_undefined;
          jit_value_t pos_ = jit_value_create(function, jit_type_addr2s);
          switch(c1[c]._M) {
            case M_I: {
              jit_value_t tmpa = JIT_CORE2_L(ap);
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t ai = JIT_INSTR2_fn_L(tmpa);
              jit_value_t aa = ai_a;
              jit_value_t ab = ai_b;
              jit_value_t bi = JIT_INSTR2_fn_L(tmpb);
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_eq(function, ai, bi), &labele);
              jit_insn_branch_if_not(function, jit_insn_eq(function, aa, ba), &labele);
              jit_insn_branch_if_not(function, jit_insn_eq(function, ab, bb), &labele);
              break; }
            case M_A: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t aa = ai_a;
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_insn_branch_if_not(function, jit_insn_eq(function, aa, ba), &labele);
              break; }
            case M_B: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t ab = ai_b;
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_eq(function, ab, bb), &labele);
              break; }
            case M_AB: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t aa = ai_a;
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_eq(function, aa, bb), &labele);
              break; }
            case M_BA: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t ab = ai_b;
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_insn_branch_if_not(function, jit_insn_eq(function, ab, ba), &labele);
              break; }
            case M_F: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t aa = ai_a;
              jit_value_t ab = ai_b;
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_eq(function, aa, ba), &labele);
              jit_insn_branch_if_not(function, jit_insn_eq(function, ab, bb), &labele);
              break; }
            case M_X: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t aa = ai_a;
              jit_value_t ab = ai_b;
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_eq(function, aa, bb), &labele);
              jit_insn_branch_if_not(function, jit_insn_eq(function, ab, ba), &labele);
              break; }
          }

            jit_insn_store(function, pos_, jit_insn_add(function, pos, JIT_CONST(2, jit_type_addr2)));
          jit_insn_branch(function, &labelb);
          jit_insn_label(function, &labele);
            jit_insn_store(function, pos_, jit_insn_add(function, pos, JIT_CONST(1, jit_type_addr2)));
          jit_insn_label(function, &labelb);
          jit_value_t pos_2;
          BOUND_CORESIZE_HIGH_JIT(pos_2, pos_);
          JIT_PROC2_pos_S(tmp1, pos_2);
          jit_value_t next = JIT_PROC2_next_L(tmp1);
          jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next);
          break; }
        #endif
        #ifdef O_SNE
        case O_SNE: {
          jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
          jit_value_t pos = JIT_PROC2_pos_L(tmp1);
          jit_label_t labele = jit_label_undefined;
          jit_label_t labelb = jit_label_undefined;
          jit_value_t pos_ = jit_value_create(function, jit_type_addr2s);
          switch(c1[c]._M) {
            case M_I: {
              jit_value_t tmpa = JIT_CORE2_L(ap);
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t ai = JIT_INSTR2_fn_L(tmpa);
              jit_value_t aa = ai_a;
              jit_value_t ab = ai_b;
              jit_value_t bi = JIT_INSTR2_fn_L(tmpb);
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_eq(function, ai, bi), &labele);
              jit_insn_branch_if_not(function, jit_insn_eq(function, aa, ba), &labele);
              jit_insn_branch_if_not(function, jit_insn_eq(function, ab, bb), &labele);
              break; }
            case M_A: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t aa = ai_a;
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_insn_branch_if_not(function, jit_insn_eq(function, aa, ba), &labele);
              break; }
            case M_B: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t ab = ai_b;
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_eq(function, ab, bb), &labele);
              break; }
            case M_AB: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t aa = ai_a;
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_eq(function, aa, bb), &labele);
              break; }
            case M_BA: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t ab = ai_b;
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_insn_branch_if_not(function, jit_insn_eq(function, ab, ba), &labele);
              break; }
            case M_F: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t aa = ai_a;
              jit_value_t ab = ai_b;
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_eq(function, aa, ba), &labele);
              jit_insn_branch_if_not(function, jit_insn_eq(function, ab, bb), &labele);
              break; }
            case M_X: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t aa = ai_a;
              jit_value_t ab = ai_b;
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_eq(function, aa, bb), &labele);
              jit_insn_branch_if_not(function, jit_insn_eq(function, ab, ba), &labele);
              break; }
          }

            jit_insn_store(function, pos_, jit_insn_add(function, pos, JIT_CONST(1, jit_type_addr2)));
          jit_insn_branch(function, &labelb);
          jit_insn_label(function, &labele);
            jit_insn_store(function, pos_, jit_insn_add(function, pos, JIT_CONST(2, jit_type_addr2)));
          jit_insn_label(function, &labelb);
          jit_value_t pos_2;
          BOUND_CORESIZE_HIGH_JIT(pos_2, pos_);
          JIT_PROC2_pos_S(tmp1, pos_2);
          jit_value_t next = JIT_PROC2_next_L(tmp1);
          jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next);
          break; }
        #endif
        #ifdef O_SLT
        case O_SLT: {
          jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
          jit_value_t pos = JIT_PROC2_pos_L(tmp1);
          jit_label_t labele = jit_label_undefined;
          jit_label_t labelb = jit_label_undefined;
          jit_value_t pos_ = jit_value_create(function, jit_type_addr2s);
          switch(c1[c]._M) {
            case M_A: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t aa = ai_a;
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_insn_branch_if_not(function, jit_insn_lt(function, aa, ba), &labele);
              break; }
            case M_B: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t ab = ai_b;
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_lt(function, ab, bb), &labele);
              break; }
            case M_AB: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t aa = ai_a;
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_lt(function, aa, bb), &labele);
              break; }
            case M_BA: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t ab = ai_b;
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_insn_branch_if_not(function, jit_insn_lt(function, ab, ba), &labele);
              break; }
            case M_F: case M_I: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t aa = ai_a;
              jit_value_t ab = ai_b;
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_lt(function, aa, ba), &labele);
              jit_insn_branch_if_not(function, jit_insn_lt(function, ab, bb), &labele);
              break; }
            case M_X: {
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t aa = ai_a;
              jit_value_t ab = ai_b;
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              jit_insn_branch_if_not(function, jit_insn_lt(function, aa, bb), &labele);
              jit_insn_branch_if_not(function, jit_insn_lt(function, ab, ba), &labele);
              break; }
          }

            jit_insn_store(function, pos_, jit_insn_add(function, pos, JIT_CONST(2, jit_type_addr2)));
          jit_insn_branch(function, &labelb);
          jit_insn_label(function, &labele);
            jit_insn_store(function, pos_, jit_insn_add(function, pos, JIT_CONST(1, jit_type_addr2)));
          jit_insn_label(function, &labelb);
          jit_value_t pos_2;
          BOUND_CORESIZE_HIGH_JIT(pos_2, pos_);
          JIT_PROC2_pos_S(tmp1, pos_2);
          jit_value_t next = JIT_PROC2_next_L(tmp1);
          jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next);
          break; }
        #endif
        #ifdef O_SPL
        case O_SPL: {
          jit_label_t labele = jit_label_undefined;

          jit_insn_branch_if_not(function, jit_insn_lt(function,
              jit_insn_load_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_nprocs), jit_type_void_ptr), JIT_W(), jit_type_sys_ulong),
              JIT_CONST(MAXPROCESSES, jit_type_sys_ulong)), &labele);
            #ifdef opt_SPL86
            jit_value_t xp = bp;
            #else
            jit_value_t xp = ap;
            #endif
            #ifdef TSAFE_CORE
            jit_insn_call_native(function, NULL, spl_jit, signature_spl_jit, (jit_value_t[]){local_core_jit, xp}, 2, JIT_CALL_NOTHROW);
            #else
            jit_insn_call_native(function, NULL, spl_jit, signature_spl_jit, (jit_value_t[]){xp}, 1, JIT_CALL_NOTHROW);
            #endif
            jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
          jit_insn_label(function, &labele);
          break; }
        #endif
        #ifdef O_PCT
        case O_PCT: {
          if(!warning_pct_given) {
            puts("Warning: JIT-compiler does not support PCT extension for performance reasons, it will behave like NOP.");
            warning_pct_given = 1;
          }
        }
        #endif
        #ifdef O_NOP
        case O_NOP:
          break;
        #endif
        #ifdef O_LDP
        case O_LDP: {
          #ifdef TSAFE_CORE
          jit_label_t labelb = jit_label_undefined;
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, jit_insn_load_relative(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_pspace_local_accessed), jit_type_sys_int), 0, jit_type_sys_int)), &labelb);
            jit_insn_call_native(function, NULL, mlock_helper_jit, signature_mlock_helper_jit, NULL, 0, JIT_CALL_NOTHROW);
            jit_insn_store_relative(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_pspace_local_accessed), jit_type_sys_int), 0, JIT_CONST(1, jit_type_sys_int));
          jit_insn_label(function, &labelb);
          #endif
          switch(c1[c]._M) {
            case M_A: {
              jit_value_t tmpx = ai_a;
              jit_value_t tmpx_ = jit_insn_rem(function, tmpx, JIT_CONST(PSPACESIZE, jit_type_addr2));
              jit_label_t labele2 = jit_label_undefined;
              jit_label_t labelb2 = jit_label_undefined;
              jit_value_t tmpv = jit_value_create(function, jit_type_addr2);

              jit_value_t tmp2 = jit_insn_load_elem_address(function, JIT_CONST(warriors, jit_type_void_ptr), JIT_W(), jit_type_warrior); //warriors[w]
              jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpx_), &labele2);
                jit_value_t pspace = jit_insn_load_relative(function, tmp2, offsetof(WARRIOR, pspace), jit_type_void_ptr); //.pspace
                jit_insn_store(function, tmpv, jit_insn_load_elem(function, pspace, tmpx_, jit_type_pcell));
              jit_insn_branch(function, &labelb2);
              jit_insn_label(function, &labele2);
                jit_insn_store(function, tmpv, jit_insn_load_relative(function, tmp2, offsetof(WARRIOR, pspace), jit_type_void_ptr)); //.psp0
              jit_insn_label(function, &labelb2);

              jit_value_t tmp1 = JIT_CORE2_L(bp);
              JIT_INSTR2_S(tmp1, a, tmpv);
              break; }
            case M_B: case M_I: case M_F: case M_X: {
              jit_value_t tmpx = ai_b;
              jit_value_t tmpx_ = jit_insn_rem(function, tmpx, JIT_CONST(PSPACESIZE, jit_type_addr2));
              jit_label_t labele2 = jit_label_undefined;
              jit_label_t labelb2 = jit_label_undefined;
              jit_value_t tmpv = jit_value_create(function, jit_type_addr2);

              jit_value_t tmp2 = jit_insn_load_elem_address(function, JIT_CONST(warriors, jit_type_void_ptr), JIT_W(), jit_type_warrior); //warriors[w]
              jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpx_), &labele2);
                jit_value_t pspace = jit_insn_load_relative(function, tmp2, offsetof(WARRIOR, pspace), jit_type_void_ptr); //.pspace
                jit_insn_store(function, tmpv, jit_insn_load_elem(function, pspace, tmpx_, jit_type_pcell));
              jit_insn_branch(function, &labelb2);
              jit_insn_label(function, &labele2);
                jit_insn_store(function, tmpv, jit_insn_load_relative(function, tmp2, offsetof(WARRIOR, pspace), jit_type_void_ptr)); //.psp0
              jit_insn_label(function, &labelb2);

              jit_value_t tmp1 = JIT_CORE2_L(bp);
              JIT_INSTR2_S(tmp1, b, tmpv);
              break; }
            case M_AB: {
              jit_value_t tmpx = ai_a;
              jit_value_t tmpx_ = jit_insn_rem(function, tmpx, JIT_CONST(PSPACESIZE, jit_type_addr2));
              jit_label_t labele2 = jit_label_undefined;
              jit_label_t labelb2 = jit_label_undefined;
              jit_value_t tmpv = jit_value_create(function, jit_type_addr2);

              jit_value_t tmp2 = jit_insn_load_elem_address(function, JIT_CONST(warriors, jit_type_void_ptr), JIT_W(), jit_type_warrior); //warriors[w]
              jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpx_), &labele2);
                jit_value_t pspace = jit_insn_load_relative(function, tmp2, offsetof(WARRIOR, pspace), jit_type_void_ptr); //.pspace
                jit_insn_store(function, tmpv, jit_insn_load_elem(function, pspace, tmpx_, jit_type_pcell));
              jit_insn_branch(function, &labelb2);
              jit_insn_label(function, &labele2);
                jit_insn_store(function, tmpv, jit_insn_load_relative(function, tmp2, offsetof(WARRIOR, pspace), jit_type_void_ptr)); //.psp0
              jit_insn_label(function, &labelb2);

              jit_value_t tmp1 = JIT_CORE2_L(bp);
              JIT_INSTR2_S(tmp1, b, tmpv);
              break; }
            case M_BA: {
              jit_value_t tmpx = ai_b;
              jit_value_t tmpx_ = jit_insn_rem(function, tmpx, JIT_CONST(PSPACESIZE, jit_type_addr2));
              jit_label_t labele2 = jit_label_undefined;
              jit_label_t labelb2 = jit_label_undefined;
              jit_value_t tmpv = jit_value_create(function, jit_type_addr2);

              jit_value_t tmp2 = jit_insn_load_elem_address(function, JIT_CONST(warriors, jit_type_void_ptr), JIT_W(), jit_type_warrior); //warriors[w]
              jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpx_), &labele2);
                jit_value_t pspace = jit_insn_load_relative(function, tmp2, offsetof(WARRIOR, pspace), jit_type_void_ptr); //.pspace
                jit_insn_store(function, tmpv, jit_insn_load_elem(function, pspace, tmpx_, jit_type_pcell));
              jit_insn_branch(function, &labelb2);
              jit_insn_label(function, &labele2);
                jit_insn_store(function, tmpv, jit_insn_load_relative(function, tmp2, offsetof(WARRIOR, pspace), jit_type_void_ptr)); //.psp0
              jit_insn_label(function, &labelb2);

              jit_value_t tmp1 = JIT_CORE2_L(bp);
              JIT_INSTR2_S(tmp1, a, tmpv);
              break; }
          }
          break; }
        #endif
        #ifdef O_STP
        case O_STP: {
          #ifdef TSAFE_CORE
          jit_label_t labelb = jit_label_undefined;
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, jit_insn_load_relative(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_pspace_local_accessed), jit_type_sys_int), 0, jit_type_sys_int)), &labelb);
            jit_insn_call_native(function, NULL, mlock_helper_jit, signature_mlock_helper_jit, NULL, 0, JIT_CALL_NOTHROW);
            jit_insn_store_relative(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_pspace_local_accessed), jit_type_sys_int), 0, JIT_CONST(1, jit_type_sys_int));
          jit_insn_label(function, &labelb);
          #endif
          switch(c1[c]._M) {
            case M_A: {
              jit_value_t tmp2 = JIT_CORE2_L(bp);
              jit_value_t tmpx = JIT_INSTR2_L(tmp2, a);
              jit_value_t tmpx_ = jit_insn_rem(function, tmpx, JIT_CONST(PSPACESIZE, jit_type_addr2));
              jit_label_t labelb2 = jit_label_undefined;
              jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpx_), &labelb2);
                jit_value_t tmp2_ = jit_insn_load_elem_address(function, JIT_CONST(warriors, jit_type_void_ptr), JIT_W(), jit_type_warrior); //warriors[w]
                jit_value_t pspace = jit_insn_load_relative(function, tmp2_, offsetof(WARRIOR, pspace), jit_type_void_ptr); //.pspace
                jit_value_t tmpv = ai_a;
                jit_insn_store_elem(function, pspace, tmpx_, tmpv);
              jit_insn_label(function, &labelb2);
              break; }
            case M_B: case M_I: case M_F: case M_X: {
              jit_value_t tmp2 = JIT_CORE2_L(bp);
              jit_value_t tmpx = JIT_INSTR2_L(tmp2, b);
              jit_value_t tmpx_ = jit_insn_rem(function, tmpx, JIT_CONST(PSPACESIZE, jit_type_addr2));
              jit_label_t labelb2 = jit_label_undefined;
              jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpx_), &labelb2);
                jit_value_t tmp2_ = jit_insn_load_elem_address(function, JIT_CONST(warriors, jit_type_void_ptr), JIT_W(), jit_type_warrior); //warriors[w]
                jit_value_t pspace = jit_insn_load_relative(function, tmp2_, offsetof(WARRIOR, pspace), jit_type_void_ptr); //.pspace
                jit_value_t tmpv = ai_b;
                jit_insn_store_elem(function, pspace, tmpx_, tmpv);
              jit_insn_label(function, &labelb2);
              break; }
            case M_AB: {
              jit_value_t tmp2 = JIT_CORE2_L(bp);
              jit_value_t tmpx = JIT_INSTR2_L(tmp2, b);
              jit_value_t tmpx_ = jit_insn_rem(function, tmpx, JIT_CONST(PSPACESIZE, jit_type_addr2));
              jit_label_t labelb2 = jit_label_undefined;
              jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpx_), &labelb2);
                jit_value_t tmp2_ = jit_insn_load_elem_address(function, JIT_CONST(warriors, jit_type_void_ptr), JIT_W(), jit_type_warrior); //warriors[w]
                jit_value_t pspace = jit_insn_load_relative(function, tmp2_, offsetof(WARRIOR, pspace), jit_type_void_ptr); //.pspace
                jit_value_t tmpv = ai_a;
                jit_insn_store_elem(function, pspace, tmpx_, tmpv);
              jit_insn_label(function, &labelb2);
              break; }
            case M_BA: {
              jit_value_t tmp2 = JIT_CORE2_L(bp);
              jit_value_t tmpx = JIT_INSTR2_L(tmp2, a);
              jit_value_t tmpx_ = jit_insn_rem(function, tmpx, JIT_CONST(PSPACESIZE, jit_type_addr2));
              jit_label_t labelb2 = jit_label_undefined;
              jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpx_), &labelb2);
                jit_value_t tmp2_ = jit_insn_load_elem_address(function, JIT_CONST(warriors, jit_type_void_ptr), JIT_W(), jit_type_warrior); //warriors[w]
                jit_value_t pspace = jit_insn_load_relative(function, tmp2_, offsetof(WARRIOR, pspace), jit_type_void_ptr); //.pspace
                jit_value_t tmpv = ai_b;
                jit_insn_store_elem(function, pspace, tmpx_, tmpv);
              jit_insn_label(function, &labelb2);
              break; }
          }
          break; }
        #endif
        #ifdef O_XCH
        case O_XCH: {
          #ifdef EXT_XCH_a
          jit_value_t tmp1 = JIT_CORE2_L(ap);
          JIT_INSTR2_S(tmp1, a, ai_b);
          JIT_INSTR2_S(tmp1, b, ai_a);
          #else
          switch(c1[c]._M) {
            case M_I: {
              jit_value_t tmpa = JIT_CORE2_L(ap);
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t ai = JIT_INSTR2_fn_L(tmpa);
              jit_value_t bi = JIT_INSTR2_fn_L(tmpb);
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              JIT_INSTR2_S(tmpa, fn, bi);
              JIT_INSTR2_S(tmpa, a, ba);
              JIT_INSTR2_S(tmpa, b, bb);
              JIT_INSTR2_S(tmpb, fn, ai);
              JIT_INSTR2_S(tmpb, a, ai_a);
              JIT_INSTR2_S(tmpb, b, ai_b);
              break; }
            case M_A: {
              jit_value_t tmpa = JIT_CORE2_L(ap);
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              JIT_INSTR2_S(tmpa, a, ba);
              JIT_INSTR2_S(tmpb, a, ai_a);
              break; }
            case M_B: {
              jit_value_t tmpa = JIT_CORE2_L(ap);
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              JIT_INSTR2_S(tmpa, b, bb);
              JIT_INSTR2_S(tmpb, b, ai_b);
              break; }
            case M_AB: {
              jit_value_t tmpa = JIT_CORE2_L(ap);
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              JIT_INSTR2_S(tmpa, a, bb);
              JIT_INSTR2_S(tmpb, b, ai_a);
              break; }
            case M_BA: {
              jit_value_t tmpa = JIT_CORE2_L(ap);
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              JIT_INSTR2_S(tmpa, b, ba);
              JIT_INSTR2_S(tmpb, a, ai_b);
              break; }
            case M_F: {
              jit_value_t tmpa = JIT_CORE2_L(ap);
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              JIT_INSTR2_S(tmpa, a, ba);
              JIT_INSTR2_S(tmpa, b, bb);
              JIT_INSTR2_S(tmpb, a, ai_a);
              JIT_INSTR2_S(tmpb, b, ai_b);
              break; }
            case M_X: {
              jit_value_t tmpa = JIT_CORE2_L(ap);
              jit_value_t tmpb = JIT_CORE2_L(bp);
              jit_value_t ba = JIT_INSTR2_L(tmpb, a);
              jit_value_t bb = JIT_INSTR2_L(tmpb, b);
              JIT_INSTR2_S(tmpa, b, ba);
              JIT_INSTR2_S(tmpa, a, bb);
              JIT_INSTR2_S(tmpb, b, ai_a);
              JIT_INSTR2_S(tmpb, a, ai_b);
              break; }
          }
          #endif
          break; }
        #endif
        #ifdef O_STS
        case O_STS: {
          #ifdef TSAFE_CORE
            switch(c1[c]._M) {
              case M_I: {
                jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
                  (jit_value_t[]){local_core_jit, JIT_CONST(M_I, jit_type_uint), ai_a, ai_b,
                  JIT_INSTR2_fn_L(JIT_CORE2_L(ap))}, 5, JIT_CALL_NOTHROW);
                break; }
              case M_A: case M_AB: {
                jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
                  (jit_value_t[]){local_core_jit, JIT_CONST(c1[c]._M, jit_type_ubyte), ap, JIT_CONST(0, jit_type_addr2),
                  JIT_CONST(0, jit_type_void_ptr)}, 5, JIT_CALL_NOTHROW);
                break; }
              case M_B: case M_BA: {
                jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
                  (jit_value_t[]){local_core_jit, JIT_CONST(c1[c]._M, jit_type_ubyte), JIT_CONST(0, jit_type_addr2), ai_b,
                  JIT_CONST(0, jit_type_void_ptr)}, 5, JIT_CALL_NOTHROW);
                break; }
              case M_F: case M_X: {
                jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
                  (jit_value_t[]){local_core_jit, JIT_CONST(c1[c]._M, jit_type_ubyte), ai_a, ai_b,
                  JIT_CONST(0, jit_type_void_ptr)}, 5, JIT_CALL_NOTHROW);
                break; }
            }
          #else
            switch(c1[c]._M) {
              case M_I: {
                jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
                  (jit_value_t[]){JIT_CONST(M_I, jit_type_uint), ai_a, ai_b,
                  JIT_INSTR2_fn_L(JIT_CORE2_L(ap))}, 4, JIT_CALL_NOTHROW);
                break; }
              case M_A: case M_AB: {
                jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
                  (jit_value_t[]){JIT_CONST(c1[c]._M, jit_type_ubyte), ai_a, JIT_CONST(0, jit_type_addr2),
                  JIT_CONST(0, jit_type_void_ptr)}, 4, JIT_CALL_NOTHROW);
                break; }
              case M_B: case M_BA: {
                jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
                  (jit_value_t[]){JIT_CONST(c1[c]._M, jit_type_ubyte), JIT_CONST(0, jit_type_addr2), ai_b,
                  JIT_CONST(0, jit_type_void_ptr)}, 4, JIT_CALL_NOTHROW);
                break; }
              case M_F: case M_X: {
                jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
                  (jit_value_t[]){JIT_CONST(c1[c]._M, jit_type_ubyte), ai_a, ai_b,
                  JIT_CONST(0, jit_type_void_ptr)}, 4, JIT_CALL_NOTHROW);
                break; }
            }
          #endif
          break; }
        #endif
      }

      if(go_on) { //enqueue next instruction
        jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
        jit_value_t pos = JIT_PROC2_pos_L(tmp1);
        jit_value_t pos_ = jit_insn_add(function, pos, JIT_CONST(1, jit_type_addr2s));
        jit_value_t pos_2;
        BOUND_CORESIZE_HIGH_JIT(pos_2, pos_);
        JIT_PROC2_pos_S(tmp1, pos_2);
        jit_value_t next = JIT_PROC2_next_L(tmp1);
        jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next);
      }

      jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
      jit_function_compile(function);
      jitfunc2_t r = (jitfunc2_t) jit_function_to_closure(function);
      i2.fn = r;
      g_data2.list[g_data2.nentr-1].fn = r;
      if(g_data2.curhpos != -1) {
        g_data2.hasht[g_data2.curhpos].fn = r;
      }
    }
    w->code2[c] = i2;
  }
  jit_context_build_end(jit_context);

  if(w->code1 == NULL) free(c1); //free if not from w
  return;
}

#define COPY_STR(d, s) \
  if(!s) error("Returned string is NULL."); \
  d = malloc(strlen(s) + 1); \
  strcpy(d, s);

int parse_load(char** redfn, char** lfn, char* pname) { //nonzero = failed
  /*char** loadfiles = malloc(WARRIORS * sizeof(char*));
  long orgs_c;
  char* names_c;
  char* authors_c;
  long lengths_c;
  long havepin_c;
  long pins_c;*/

  warriors = malloc(WARRIORS * sizeof(WARRIOR));

  /******** THIS WAS OLD PYTHON PARSER CODE ********/
  /*
  wchar_t *program = Py_DecodeLocale(pname, NULL);
  if (program == NULL) {
      fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
      exit(1);
  }
  Py_SetProgramName(program);

  Py_Initialize();
  if(opt_VERBOSE) puts("Parsing warriors...\n--------------------------------");
  unsigned long c;
  for(c = 0; c < WARRIORS; ++c) {
    FILE* parser = fopen(PARSER_PATH, "rt");
    if(!parser) error("can't open %s", PARSER_PATH);

    PyObject* globals = PyDict_New();
    if (!globals) error("PyDict_New() failed.");
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());

    PyObject* CONST = PyDict_New();
    if (!CONST) error("PyDict_New() failed.");
    PyDict_SetItemString(CONST, "CORESIZE", PyLong_FromLong(CORESIZE));
    PyDict_SetItemString(CONST, "PSPACESIZE", PyLong_FromLong(PSPACESIZE));
    PyDict_SetItemString(CONST, "MAXCYCLES", PyLong_FromLong(MAXCYCLES));
    PyDict_SetItemString(CONST, "MAXPROCESSES", PyLong_FromLong(MAXPROCESSES));
    PyDict_SetItemString(CONST, "WARRIORS", PyLong_FromLong(2));
    PyDict_SetItemString(CONST, "MAXLENGTH", PyLong_FromLong(MAXLENGTH));
    PyDict_SetItemString(CONST, "CURLINE", PyLong_FromLong(0));
    PyDict_SetItemString(CONST, "MINDISTANCE", PyLong_FromLong(MINDISTANCE));
    PyDict_SetItemString(CONST, "VERSION", PyLong_FromLong(VERSION));
    PyDict_SetItemString(CONST, "ROUNDS", PyLong_FromLong(ROUNDS));
    PyDict_SetItemString(globals, "CONST", CONST);

    FILE* f = fopen(redfn[c], "rt");
    if(!f) error("can't open %s", redfn[c]);
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* s_red = malloc(fsize+1);
    fread(s_red, fsize, 1, f);
    s_red[fsize] = '\0';
    fclose(f);
    PyObject* g_red = Py_BuildValue("s", s_red);
    if(!g_red) error("Can't convert input string to PyObject.");
    PyDict_SetItemString(globals, "red", g_red);
    if(opt_VERBOSE) PyDict_SetItemString(globals, "opt_VERBOSE", Py_True);
    else PyDict_SetItemString(globals, "opt_VERBOSE", Py_False);
    #ifdef opt_SPL86
    PyDict_SetItemString(globals, "opt_SPL86", Py_True);
    #else
    PyDict_SetItemString(globals, "opt_SPL86", Py_False);
    #endif

    PyObject* g_opt_EXTENSIONS = PyList_New(0);
    PyObject* g_opt_EXTENSIONS_defmodf = PyList_New(0);
    #if PY_MAJOR_VERSION >= 3
    #define PyString_FromString(x) PyUnicode_FromString(x)
    #define PyString_AsString(x) PyUnicode_AsUTF8(x)
    #endif

    #ifdef O_XCH
    PyList_Append(g_opt_EXTENSIONS, PyString_FromString("xch"));
    #ifdef EXT_XCH_a
    PyList_Append(g_opt_EXTENSIONS_defmodf, PyString_FromString("F"));
    #else
    PyList_Append(g_opt_EXTENSIONS_defmodf, PyString_FromString("M"));
    #endif
    #endif
    #ifdef O_PCT
    PyList_Append(g_opt_EXTENSIONS, PyString_FromString("pct"));
    PyList_Append(g_opt_EXTENSIONS_defmodf, PyString_FromString("F"));
    #endif
    #ifdef O_DJZ
    PyList_Append(g_opt_EXTENSIONS, PyString_FromString("djz"));
    PyList_Append(g_opt_EXTENSIONS_defmodf, PyString_FromString("B"));
    #endif
    #ifdef O_STS
    PyList_Append(g_opt_EXTENSIONS, PyString_FromString("sts"));
    PyList_Append(g_opt_EXTENSIONS_defmodf, PyString_FromString("V"));
    #endif
    PyDict_SetItemString(globals, "opt_EXTENSIONS", g_opt_EXTENSIONS);
    PyDict_SetItemString(globals, "opt_EXTENSIONS_defmodf", g_opt_EXTENSIONS_defmodf);

    PyObject* g_lf;
    if(lfn[c]) {
      f = fopen(lfn[c], "wt");
      if(!f) error("can't open %s", lfn[c]);
      g_lf = PyFile_FromFd((intptr_t)f, lfn[c], "wt", -1, NULL, NULL, NULL, 0);
      fclose(f);
    }
    else {
      g_lf = Py_None;
    }
    PyDict_SetItemString(globals, "lf", g_lf);
    PyDict_SetItemString(globals, "isEMBEDDED", Py_True);

    PyRun_FileEx(parser, PARSER_PATH, Py_file_input, globals, NULL, 0);

    if(PyErr_Occurred()) {
      puts("Parser Error");
      PyErr_Print();
      exit(0);
    }

    lengths_c = PyLong_AsLong(PyDict_GetItemString(globals, "SIZE"));
    orgs_c = PyLong_AsLong(PyDict_GetItemString(globals, "ORG"));
    havepin_c = PyLong_AsLong(PyDict_GetItemString(globals, "HASPIN"));
    if(havepin_c) {
      pins_c = PyLong_AsLong(PyDict_GetItemString(globals, "PIN"));
      if(opt_VERBOSE) printf("Result:\nLENGTH: %ld,  ORG: %ld,  PIN: %lu\n", lengths_c, orgs_c, pins_c);
    }
    else if(opt_VERBOSE) printf("Result:\nLENGTH: %ld,  ORG: %ld,  PIN: none\n", lengths_c, orgs_c);

    COPY_STR(names_c, PyString_AsString(PyDict_GetItemString(globals, "NAME")));
    COPY_STR(authors_c, PyString_AsString(PyDict_GetItemString(globals, "AUTHOR")));
    if(opt_VERBOSE) printf("Name: %s\nAuthor: %s\n", names_c, authors_c);

    COPY_STR(loadfiles[c], PyString_AsString(PyDict_GetItemString(globals, "red")));
    if(opt_VERBOSE >= 2) puts(loadfiles[c]);
    if(opt_VERBOSE) puts("");

    Py_DECREF(g_red);
    free(s_red);
    fclose(parser);

    //Now load
    warriors[c].org = orgs_c;
    warriors[c].name = names_c;
    warriors[c].author = authors_c;
    warriors[c].len = lengths_c;
    #if PSPACESIZE
    warriors[c].pspace = NULL;
    warriors[c].psp0 = CORESIZE-1;
    warriors[c].haspin = havepin_c;
    if(havepin_c) {
      warriors[c].pin = pins_c;
      unsigned long k;
      for(k = 0; k < c; ++k) {
        if(warriors[k].haspin && warriors[k].pin == warriors[c].pin) {
          warriors[c].pspace = warriors[k].pspace;
        }
      }
    }
    if(warriors[c].pspace == NULL)
      warriors[c].pspace = calloc(PSPACESIZE, sizeof(pcell_t));
    #endif
    //here was loading code
    #ifdef TSAFE_CORE
    minit(warriors[c].mutex);
    #endif
  }

  Py_Finalize();
  for(c = 0; c < WARRIORS; ++c) { //loading code (e.g. libjit) isolated from Python
    if(algorithm_select & (1 << 1)) load1(&warriors[c], loadfiles[c]);
    else warriors[c].code1 = NULL;
    if(algorithm_select & (1 << 2)) load2(&warriors[c], loadfiles[c]);
    else warriors[c].code2 = NULL;
    free(loadfiles[c]);
  }
  free(loadfiles);
  if(opt_VERBOSE) puts("--------------------------------\nDone.");
  */
  /******** END OF IT ********/

  /******** NEW C PARSER ********/
  long c;
  for(c = 0; c < WARRIORS; ++c) {
    FILE* red = fopen(redfn[c], "rt");
    if(red == NULL) {
      error("Could not open %s.", redfn[c]);
    }
    LINE* redt = file2text(red);
    fclose(red);
    LINE* loadt = parse(redt, &warriors[c]);
    freetext(redt);
    if(loadt == NULL) {
      free(warriors);
      return 1;
    }
    if(lfn[c] != NULL) {
      FILE* lf = fopen(lfn[c], "wt");
      text2file(loadt, lf);
      fclose(lf);
    }

    if(algorithm_select & (1 << 1)) load1(&warriors[c], loadt);
    else warriors[c].code1 = NULL;
    if(algorithm_select & (1 << 2)) load2(&warriors[c], loadt);
    else warriors[c].code2 = NULL;
    freetext(loadt);

    #if PSPACESIZE
    warriors[c].pspace = NULL;
    warriors[c].psp0 = CORESIZE-1;
    if(warriors[c].haspin) {
      unsigned long k;
      for(k = 0; k < c; ++k) {
        if(warriors[k].haspin && warriors[k].pin == warriors[c].pin) {
          warriors[c].pspace = warriors[k].pspace;
        }
      }
    }
    if(warriors[c].pspace == NULL)
      warriors[c].pspace = calloc(PSPACESIZE, sizeof(pcell_t));
    #endif

    #ifdef TSAFE_CORE
    minit(warriors[c].mutex);
    #endif
  }

  /******** END OF IT ********/

  //Generate a color for each warrior
  #ifdef _COREVIEW_
  if(WARRIORS == 1) { //1 warrior -> white
    warriors[0].color = 0xFFFFFFFF;
  }
  else if(WARRIORS <= 3) { //2-3 -> yellow, cyan, magenta
    warriors[0].color = 0xFFFFFF00;
    warriors[1].color = 0xFF00FFFF;
    if(WARRIORS == 3) {
      warriors[2].color = 0xFFFF00FF;
    }
  }
  else { //4-7+ -> white, red, green, blue, yellow, cyan, magenta
    warriors[0].color = 0xFFFFFFFF;
    warriors[1].color = 0xFFFF0000;
    warriors[2].color = 0xFF00FF00;
    warriors[3].color = 0xFF0000FF;
    if(WARRIORS >= 5) {
      warriors[4].color = 0xFFFFFF00;
      if(WARRIORS >= 6) {
        warriors[5].color = 0xFF00FFFF;
        if(WARRIORS >= 7) {
          warriors[6].color = 0xFFFF00FF;
        }
      }
    }
  }
  if(WARRIORS > 7) {
    unsigned int delta = 1 << (int) ceil(log2(ceil((3.0 + sqrt(12.0 * WARRIORS - 3.0)) / 6.0) - 1.0));
    unsigned int side = delta;
    unsigned int c = 7;
    for(; side > 1; side >>= 1) {
    //for(c = 7; c < WARRIORS; ++c)
      unsigned int cx, cy;
      float x, y;
      uint8_t a, b;
      //int k = (c-1)/3;
      //
      for(cy = delta >> 1; cy <= side; cy += delta) {
        for(cx = delta >> 1; cx < side; cx += delta) {
          int t;
          for(t = 0; t < 3; ++t) {
            y = ((float) cy) / side;
            x = ((float) cx) / side;
            y *= 255.0;
            x *= 255.0;
            a = (y > 255.0)? 255 : y;
            b = (x > 255.0)? 255 : x;
            switch((c-1)%3) {
              case 0: warriors[c].color = 0xFFFF0000 | a << 8 | b; break;
              case 1: warriors[c].color = 0xFF00FF00 | b | a << 16; break;
              case 2: warriors[c].color = 0xFF0000FF | a << 16 | b << 8; break;
            }
            ++c;
            if(c >= WARRIORS) goto _label_endcolor;
          }
        }
      }
      for(cy = 0; cy <= side; cy += delta) {
        for(cx = delta >> 1; cx < side; cx += delta) {
          int t;
          for(t = 0; t < 3; ++t) {
            y = ((float) cy) / side;
            x = ((float) cx) / side;
            y *= 255.0;
            x *= 255.0;
            a = (y > 255.0)? 255 : y;
            b = (x > 255.0)? 255 : x;
            switch((c-1)%3) {
              case 0: warriors[c].color = 0xFFFF0000 | a << 8 | b; break;
              case 1: warriors[c].color = 0xFF00FF00 | b | a << 16; break;
              case 2: warriors[c].color = 0xFF0000FF | a << 16 | b << 8; break;
            }
            ++c;
            if(c >= WARRIORS) goto _label_endcolor;
          }
        }
      }
      for(cy = delta >> 1; cy <= side; cy += delta) {
        for(cx = 0; cx < side; cx += delta) {
          int t;
          for(t = 0; t < 3; ++t) {
            y = ((float) cy) / side;
            x = ((float) cx) / side;
            y *= 255.0;
            x *= 255.0;
            a = (y > 255.0)? 255 : y;
            b = (x > 255.0)? 255 : x;
            switch((c-1)%3) {
              case 0: warriors[c].color = 0xFFFF0000 | a << 8 | b; break;
              case 1: warriors[c].color = 0xFF00FF00 | b | a << 16; break;
              case 2: warriors[c].color = 0xFF0000FF | a << 16 | b << 8; break;
            }
            ++c;
            if(c >= WARRIORS) goto _label_endcolor;
          }
        }
      }
    }
    _label_endcolor:;
  }
  #endif

  return 0;
}

void unload_all() {
  unsigned long c;
  for(c = 0; c < WARRIORS; ++c) {
    if(warriors[c].code1 != NULL) free(warriors[c].code1);
    if(warriors[c].code2 != NULL) free(warriors[c].code2);
    free(warriors[c].name);
    free(warriors[c].author);
    #if PSPACESIZE
    unsigned long k;
    int check = 0;
    if(warriors[c].haspin) {
      for(k = 0; k < c; ++k) {
        if(warriors[c].pin == warriors[k].pin) check = 1;
      }
    }
    if(!check) free(warriors[c].pspace);
    #endif
    #ifdef TSAFE_CORE
    mdestroy(warriors[c].mutex);
    #endif
  }
  free(warriors);
  return;
}

extern char* cell_icons_default[][7];
extern char*** cell_icons;
extern int cell_icon_max_size;

void initialize() {
  uint64_t randomstateA;
  uint64_t randomstateB;
  entropy_getbytes(&randomstateA, 8);
  entropy_getbytes(&randomstateB, 8);
  pcg32_srandom_r(&randomgen, randomstateA, randomstateB);

  jit_init();
  jit_context = jit_context_create();

  #if defined(TSAFE_CORE) && PSPACESIZE
  minit(mutex_pwarriors);
  #endif

  #ifdef _COREVIEW_
  minit(mutex_commun_global);
  SDL_Init(SDL_INIT_VIDEO);
  window = SDL_CreateWindow("hMARS core view",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,
    SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1,
    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  cell_icons = malloc((cell_icon_max_size + 1) * sizeof(void*));
  int c;
  for(c = 0; c <= cell_icon_max_size; ++c) {
    cell_icons[c] = malloc(7 * sizeof(void*));
    int k;
    for(k = 0; k < 7; ++k) {
      cell_icons[c][k] = malloc(strlen(cell_icons_default[c][k]) + 1);
      strcpy(cell_icons[c][k], cell_icons_default[c][k]);
    }
  }
  #endif

  if((algorithm_select >> 2) & 1) { //add _hardcoded_dat to instruction list
    g_data2.hasht[0].oma = 0;
    g_data2.hasht[0].fn = (jitfunc2_t) _hardcoded_dat;
    ++g_data2.nentr;
    if(g_data2.nentr > g_data2.allocd) g_data2.allocd += 16;
    g_data2.list = (DATA2_ELEM*) realloc(g_data2.list, g_data2.allocd * sizeof(DATA2_ELEM));
    g_data2.list[g_data2.nentr-1].oma = 0;
    g_data2.list[g_data2.nentr-1].fn = (jitfunc2_t) _hardcoded_dat;
  }
  return;
}

void finalize() {
  #ifdef _COREVIEW_
  signal_terminate();
  #endif

  jit_context_destroy(jit_context);

  #if defined(TSAFE_CORE) && PSPACESIZE
  mdestroy(mutex_pwarriors);
  #endif

  #ifdef _COREVIEW_
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  mdestroy(mutex_commun_global);

  int c;
  for(c = 0; c < cell_icon_max_size; ++c) {
    int k;
    for(k = 0; k < 7; ++k) {
      free(cell_icons[c][k]);
    }
    free(cell_icons[c]);
  }
  free(cell_icons);
  #endif
  return;
}
