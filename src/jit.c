#include "defs.h"

extern MUTEX mutex_pwarriors;

extern DATA2 g_data2;

jit_context_t jit_context;

#define JIT_CONST(x, type) jit_value_create_nint_constant(function, (type), (jit_nint)(x))
#define JIT_CORE2_L(a) jit_insn_load_elem_address(function, core2, (a), jit_type_instr2)
#define JIT_INSTR2_L(s, i)  jit_insn_load_relative(function, (s), offsetof(INSTR2, i), jit_type_addr2)
#define JIT_INSTR2_in_L(s)  jit_insn_load_relative(function, (s), offsetof(INSTR2, in), jit_type_void_ptr)
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
  --l_nprocs[l_w2]; //Here crash (memory clobbered before allocated block)
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
#define REMOVE_PROC_JIT() jit_insn_store(function, jit_locals.r, jit_insn_call_native(function, NULL, remove_proc_jit, signature_remove_proc_jit, (jit_value_t[]){local_core_jit}, 1, JIT_CALL_NOTHROW)); jit_insn_branch(function, &jit_locals.after);
#else
#define REMOVE_PROC_JIT() jit_insn_store(function, jit_locals.r, jit_insn_call_native(function, NULL, remove_proc_jit, signature_remove_proc_jit, NULL, 0, JIT_CALL_NOTHROW)); jit_insn_branch(function, &jit_locals.after);
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
void sts_jit(_corefunc uint8_t mode, addr2_t v1/*a*/, addr2_t v2/*b*/, jitind_t vi) {
  switch(mode) {
    case M_I:
      printf("%s: %ld\t%d\t%d\n", warriors[l_indices[l_w2]].name, vi, (int)v1, (int)v2);
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

void (*jit_main_loop)(_corefun0) = NULL;
jit_function_t jit_func_main_loop;

#ifdef LIBJIT_NESTING
#warning "This is not thread-safe!"
struct {
  jit_function_t function;
  jit_value_t local_core, pc, a, b, r;
  jit_label_t after;
} jit_locals;
#endif

jit_type_t compile_jit_type_instr2() {
  jit_type_t fields[3] = {JIT_TYPE(int_fast32_t), jit_type_addr2, jit_type_addr2};
  return jit_type_create_struct(fields, 3, 1);
}

jit_type_t compile_jit_type_warrior(jit_type_t jit_type_pcell) {
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
  return jit_type_create_struct(fields3, 3, 1);
}

jit_type_t compile_jit_type_proc2() {
  jit_type_t fields3[] = {jit_type_addr2, jit_type_void_ptr, jit_type_void_ptr};
  return jit_type_create_struct(fields3, 3, 1);
}

void _Noreturn jit_error(char* str) {
  error(str);
}

void compile_instr(INSTR1 c1_c) {
  //jit_insn_mark_breakpoint (function, jit_nint data1, jit_nint data2)

  jit_function_t function = jit_locals.function;
  jit_insn_store(function, jit_locals.r, JIT_CONST(0, jit_type_sys_int));

  jit_type_t jit_type_instr2 = compile_jit_type_instr2();
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
  jit_type_t jit_type_warrior = compile_jit_type_warrior(jit_type_pcell);
  #ifdef O_STS
  #ifdef TSAFE_CORE
  jit_type_t signature_sts_jit = jit_type_create_signature(jit_abi_cdecl, jit_type_void, (jit_type_t[]){jit_type_void_ptr, jit_type_uint, jit_type_addr2, jit_type_addr2, jit_type_void_ptr}, 5, 1);
  #else
  jit_type_t signature_sts_jit = jit_type_create_signature(jit_abi_cdecl, jit_type_void, (jit_type_t[]){jit_type_uint, jit_type_addr2, jit_type_addr2, jit_type_void_ptr}, 4, 1);
  #endif
  #endif
  jit_type_t signature_jit_error = jit_type_create_signature(jit_abi_cdecl, jit_type_void, (jit_type_t[]){jit_type_void_ptr}, 1, 1);

  jit_value_t local_core_jit, core2, pc, a, b;
  #ifdef LIBJIT_NESTING
    local_core_jit = jit_locals.local_core;
    core2 = jit_insn_load_relative(function, jit_locals.local_core, offsetof(LOCAL_CORE, la_core2), jit_type_void_ptr);
    pc = jit_locals.pc;
    a = jit_locals.a;
    b = jit_locals.b;
  #else
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
  #endif

  /*jit_value_t stsargs[] = {local_core_jit, JIT_CONST(M_I, jit_type_uint), a, b, JIT_INSTR2_in_L(JIT_CORE2_L(pc))};
  jit_insn_call_native(function, "sts_jit", sts_jit, signature_sts_jit, stsargs, 5, 0);*/ //D

  jit_value_t w_addr = JIT_W_ADDR();

  int need_ap = 0, need_ai = 0, need_bp = 0, need_bi = 0, go_on = 0; //needs a- and b-pointers, can continue on next instruction
  switch(c1_c._O) { //need_*, go_on
    #ifdef O_DAT
    case O_DAT: need_ap = need_ai = need_bp = need_bi = go_on = 0; break;
    #endif
    #ifdef O_MOV
    case O_MOV: need_ap = need_ai = need_bp = go_on = 1; need_bi = 0; break;
    #endif
    #ifdef O_ADD
    case O_ADD: need_ap = need_ai = need_bp = need_bi = go_on = 1; break;
    #endif
    #ifdef O_SUB
    case O_SUB: need_ap = need_ai = need_bp = need_bi = go_on = 1; break;
    #endif
    #ifdef O_MUL
    case O_MUL: need_ap = need_ai = need_bp = need_bi = go_on = 1; break;
    #endif
    #ifdef O_DIV
    case O_DIV: need_ap = need_ai = need_bp = need_bi = go_on = 1; break;
    #endif
    #ifdef O_MOD
    case O_MOD: need_ap = need_ai = need_bp = need_bi = go_on = 1; break;
    #endif
    #ifdef O_JMP
    case O_JMP: need_ap = 1; need_bp = need_ai = need_bi = go_on = 0; break;
    #endif
    #ifdef O_JMZ
    case O_JMZ: need_ap = need_bp = need_bi = go_on = 1; need_ai = 0; break;
    #endif
    #ifdef O_JMN
    case O_JMN: need_ap = need_bp = need_bi = go_on = 1; need_ai = 0; break;
    #endif
    #ifdef O_DJZ
    case O_DJZ: need_ap = need_bp = need_bi = 1; go_on = need_ai = 0; break;
    #endif
    #ifdef O_DJN
    case O_DJN: need_ap = need_bp = need_bi = 1; go_on = need_ai = 0; break;
    #endif
    #if defined(O_CMP) || defined(O_SEQ)
    #ifdef O_CMP
    case O_CMP:
    #else
    case O_SEQ:
    #endif
      need_ap = need_ai = need_bp = need_bi = 1; go_on = 0; break;
    #endif
    #ifdef O_SNE
    case O_SNE: need_ap = need_ai = need_bp = need_bi = 1; go_on = 0; break;
    #endif
    #ifdef O_SLT
    case O_SLT: need_ap = need_ai = need_bp = need_bi = 1; go_on = 0; break;
    #endif
    #ifdef O_SPL
    case O_SPL: need_ap = go_on = 1; need_bp = need_ai = need_bi = 0; break;
    #endif
    #ifdef O_NOP
    case O_NOP: need_ap = need_ai = need_bp = need_bi = 0; go_on = 1; break;
    #endif
    #ifdef O_LDP
    case O_LDP: need_ap = need_ai = need_bp = go_on = 1; need_bi = 0; break;
    #endif
    #ifdef O_STP
    case O_STP: need_ap = need_ai = need_bp = need_bi = go_on = 1; break;
    #endif
    #ifdef EXT_XCH_a
    case O_XCH: need_ap = need_ai = go_on = 1; need_bp = need_bi = 0; break;
    #endif
    #ifdef EXT_XCH_b
    case O_XCH: need_ap = need_ai = need_bp = need_bi = go_on = 1; break;
    #endif
    #ifdef O_PCT
    case O_PCT: need_ap = go_on = 1; need_bp = need_ai = need_bi = 0; break;
    #endif
    #ifdef O_STS
    case O_STS: need_ap = need_ai = go_on = 1; need_bp = need_bi = 0; break;
    #endif
    default:
      error("Unknown opcode found by JIT-compiler: %d", c1_c._O);
  }
  jit_value_t ap = NULL, bp = NULL;
  #ifdef A_ADB //auto-decrement
  jit_value_t adpA, adpB; //auto-decrement pointers
  #endif

  jit_value_t ai_a = NULL, ai_b = NULL;
  jit_value_t a_pi_tmp1 = NULL, a_pi_tmpx = NULL;

  if(need_ap) {
    switch(c1_c._aA) {
      #ifdef A_IMM
      case A_IMM:
        ap = pc;
        ai_a = a;
        ai_b = b;
        need_ai = 0;
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
        a_pi_tmp1 = JIT_CORE2_L(ap2_);
        a_pi_tmpx = JIT_INSTR2_L(a_pi_tmp1, a);
        jit_value_t ap_ = jit_insn_add(function, a_pi_tmpx, ap2_);
        BOUND_CORESIZE_HIGH_JIT(ap, ap_);
        break; }
      #endif
      #ifdef A_PIB
      case A_PIB: {
        jit_value_t ap2 = jit_insn_add(function, a, pc);
        jit_value_t ap2_;
        BOUND_CORESIZE_HIGH_JIT(ap2_, ap2);
        a_pi_tmp1 = JIT_CORE2_L(ap2_);
        a_pi_tmpx = JIT_INSTR2_L(a_pi_tmp1, b);
        jit_value_t ap_ = jit_insn_add(function, a_pi_tmpx, ap2_);
        BOUND_CORESIZE_HIGH_JIT(ap, ap_);
        break; }
      #endif
      default:
        error("Unknown address mode found by JIT-compiler: %d", c1_c._aA);
    }
  }
  else { //only in/decrements
    switch(c1_c._aA) {
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

  if(need_ai) {
    jit_value_t tmp1 = JIT_CORE2_L(ap);
    switch(c1_c._O) {
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
        switch(c1_c._M) {
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
        switch(c1_c._M) {
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

  if(need_ap) switch(c1_c._aA) { //apply postincrements after buffering (only if ap was obtained)
    case A_PIA: {
      jit_value_t tmpx_ = jit_insn_add(function, a_pi_tmpx, JIT_CONST(1, jit_type_addr2s));
      jit_value_t tmpx_2;
      BOUND_CORESIZE_HIGH_JIT(tmpx_2, tmpx_);
      JIT_INSTR2_S(a_pi_tmp1, a, tmpx_2);
      break; }
    case A_PIB: {
      jit_value_t tmpx_ = jit_insn_add(function, a_pi_tmpx, JIT_CONST(1, jit_type_addr2s));
      jit_value_t tmpx_2;
      BOUND_CORESIZE_HIGH_JIT(tmpx_2, tmpx_);
      JIT_INSTR2_S(a_pi_tmp1, b, tmpx_2);
      break; }
  }

  jit_value_t bi_a = NULL, bi_b = NULL;
  jit_value_t b_pi_tmp1 = NULL, b_pi_tmpx = NULL;

  if(need_bp) {
    switch(c1_c._aB) {
      #ifdef A_IMM
      case A_IMM:
        bp = pc;
        bi_a = a;
        bi_b = b;
        need_bi = 0;
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
        b_pi_tmp1 = JIT_CORE2_L(bp2_);
        b_pi_tmpx = JIT_INSTR2_L(b_pi_tmp1, a);
        jit_value_t bp_ = jit_insn_add(function, b_pi_tmpx, bp2_);
        BOUND_CORESIZE_HIGH_JIT(bp, bp_);
        break; }
      #endif
      #ifdef A_PIB
      case A_PIB: {
        jit_value_t bp2 = jit_insn_add(function, b, pc);
        jit_value_t bp2_;
        BOUND_CORESIZE_HIGH_JIT(bp2_, bp2);
        b_pi_tmp1 = JIT_CORE2_L(bp2_);
        b_pi_tmpx = JIT_INSTR2_L(b_pi_tmp1, b);
        jit_value_t bp_ = jit_insn_add(function, b_pi_tmpx, bp2_);
        BOUND_CORESIZE_HIGH_JIT(bp, bp_);
        break; }
      #endif
      default:
        error("Unknown address mode found by JIT-compiler: %d", c1_c._aB);
    }
  }
  else { //only in/decrements
    switch(c1_c._aB) {
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

  if(need_bi) {
    jit_value_t tmp1 = JIT_CORE2_L(bp);
    switch(c1_c._O) {
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
      #ifdef O_JMZ
      case O_JMZ:
      #endif
      #ifdef O_JMN
      case O_JMN:
      #endif
      #ifdef O_DJZ
      case O_DJZ:
      #endif
      #ifdef O_DJN
      case O_DJN:
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
        switch(c1_c._M) {
          case M_A: case M_BA: {
            bi_a = JIT_INSTR2_L(tmp1, a);
            break; }
          case M_B: case M_AB: {
            bi_b = JIT_INSTR2_L(tmp1, b);
            break; }
          case M_F: case M_X: case M_I: {
            bi_a = JIT_INSTR2_L(tmp1, a);
            bi_b = JIT_INSTR2_L(tmp1, b);
            break; }
        }
        break;

      #ifdef O_LDP
      case O_LDP:
      #endif
      #ifdef O_STP
      case O_STP:
      #endif
        switch(c1_c._M) {
          case M_A: case M_BA: {
            bi_a = JIT_INSTR2_L(tmp1, a);
            break; }
          case M_B: case M_AB: case M_F: case M_X: case M_I: {
            bi_b = JIT_INSTR2_L(tmp1, b);
            break; }
        }
        break;

      #ifdef EXT_XCH_a
      case O_XCH:
      #endif
        bi_a = JIT_INSTR2_L(tmp1, a);
        bi_b = JIT_INSTR2_L(tmp1, b);
        break;
    }
  }

  if(need_bp) switch(c1_c._aB) { //apply postincrements after buffering (only if bp was obtained)
    case A_PIA: {
      jit_value_t tmpx_ = jit_insn_add(function, b_pi_tmpx, JIT_CONST(1, jit_type_addr2s));
      jit_value_t tmpx_2;
      BOUND_CORESIZE_HIGH_JIT(tmpx_2, tmpx_);
      JIT_INSTR2_S(b_pi_tmp1, a, tmpx_2);
      break; }
    case A_PIB: {
      jit_value_t tmpx_ = jit_insn_add(function, b_pi_tmpx, JIT_CONST(1, jit_type_addr2s));
      jit_value_t tmpx_2;
      BOUND_CORESIZE_HIGH_JIT(tmpx_2, tmpx_);
      JIT_INSTR2_S(b_pi_tmp1, b, tmpx_2);
      break; }
  }


  #ifdef A_ADB //auto-decrement
  if(c1_c._aA == A_ADB) {
    jit_value_t tmp1 = JIT_CORE2_L(adpA);
    jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
    jit_value_t tmpb_ = jit_insn_sub(function, tmpb, JIT_CONST(1, jit_type_addr2s));
    jit_value_t tmpb_2;
    BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
    JIT_INSTR2_S(tmp1, b, tmpb_2);
  }
  if(c1_c._aB == A_ADB) {
    jit_value_t tmp1 = JIT_CORE2_L(adpB);
    jit_value_t tmpb = JIT_INSTR2_L(tmp1, b);
    jit_value_t tmpb_ = jit_insn_sub(function, tmpb, JIT_CONST(1, jit_type_addr2s));
    jit_value_t tmpb_2;
    BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
    JIT_INSTR2_S(tmp1, b, tmpb_2);
  }
  #endif

  switch(c1_c._O) { //execute
    #ifdef O_DAT
    case O_DAT:
      //jit_type_t params2[4] = {jit_type_void_ptr, jit_type_addr2, jit_type_addr2, jit_type_addr2};
      REMOVE_PROC_JIT();
      break;
    #endif
    #ifdef O_MOV
    case O_MOV:
      switch(c1_c._M) {
        case M_I: {
          jit_value_t tmp1 = JIT_CORE2_L(ap);
          jit_value_t tmpi = JIT_INSTR2_in_L(tmp1);
          jit_value_t tmp1_ = JIT_CORE2_L(bp);
          JIT_INSTR2_S(tmp1_, in, tmpi);
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
      switch(c1_c._M) {
        case M_A: {
          jit_value_t tmp1 = JIT_CORE2_L(bp);
          jit_value_t tmpa_ = jit_insn_add(function, bi_a, ai_a);
          jit_value_t tmpa_2;
          BOUND_CORESIZE_HIGH_JIT(tmpa_2, tmpa_);
          JIT_INSTR2_S(tmp1, a, tmpa_2);
          break;
        }
        case M_B: {
          jit_value_t tmp1 = JIT_CORE2_L(bp);
          jit_value_t tmpb_ = jit_insn_add(function, bi_b, ai_b);
          jit_value_t tmpb_2;
          BOUND_CORESIZE_HIGH_JIT(tmpb_2, tmpb_);
          JIT_INSTR2_S(tmp1, b, tmpb_2);
          break;
        }
        case M_AB: {
          jit_value_t tmp1 = JIT_CORE2_L(bp);
          jit_value_t tmpb_ = jit_insn_add(function, bi_b, ai_a);
          jit_value_t tmpb_2;
          BOUND_CORESIZE_HIGH_JIT(tmpb_2, tmpb_);
          JIT_INSTR2_S(tmp1, b, tmpb_2);
          break;
        }
        case M_BA: {
          jit_value_t tmp1 = JIT_CORE2_L(bp);
          jit_value_t tmpa_ = jit_insn_add(function, bi_a, ai_b);
          jit_value_t tmpa_2;
          BOUND_CORESIZE_HIGH_JIT(tmpa_2, tmpa_);
          JIT_INSTR2_S(tmp1, a, tmpa_2);
          break;
        }
        case M_I: case M_F: {
          jit_value_t tmp1 = JIT_CORE2_L(bp);
          jit_value_t tmpa_ = jit_insn_add(function, bi_a, ai_a);
          jit_value_t tmpb_ = jit_insn_add(function, bi_b, ai_b);
          jit_value_t tmpa_2, tmpb_2;
          BOUND_CORESIZE_HIGH_JIT(tmpa_2, tmpa_);
          BOUND_CORESIZE_HIGH_JIT(tmpb_2, tmpb_);
          JIT_INSTR2_S(tmp1, a, tmpa_2);
          JIT_INSTR2_S(tmp1, b, tmpb_2);
          break;
        }
        case M_X: {
          jit_value_t tmp1 = JIT_CORE2_L(bp);
          jit_value_t tmpa_ = jit_insn_add(function, bi_a, ai_b);
          jit_value_t tmpb_ = jit_insn_add(function, bi_b, ai_a);
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
      switch(c1_c._M) {
          case M_A: {
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_value_t tmpa_ = jit_insn_sub(function, bi_a, ai_a);
            jit_value_t tmpa_2;
            BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            break;
          }
          case M_B: {
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_value_t tmpb_ = jit_insn_sub(function, bi_b, ai_b);
            jit_value_t tmpb_2;
            BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            break;
          }
          case M_AB: {
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_value_t tmpb_ = jit_insn_sub(function, bi_b, ai_a);
            jit_value_t tmpb_2;
            BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            break;
          }
          case M_BA: {
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_value_t tmpa_ = jit_insn_sub(function, bi_a, ai_b);
            jit_value_t tmpa_2;
            BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            break;
          }
          case M_I: case M_F: {
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_value_t tmpa_ = jit_insn_sub(function, bi_a, ai_a);
            jit_value_t tmpb_ = jit_insn_sub(function, bi_b, ai_b);
            jit_value_t tmpa_2, tmpb_2;
            BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
            BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            break;
          }
          case M_X: {
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_value_t tmpa_ = jit_insn_sub(function, bi_a, ai_b);
            jit_value_t tmpb_ = jit_insn_sub(function, bi_b, ai_a);
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
      switch(c1_c._M) {
          case M_A: {
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_value_t tmpa_ = jit_insn_mul(function, ai_a, bi_a);
            jit_value_t tmpa_2 = jit_insn_rem(function, tmpa_, JIT_CONST(CORESIZE, jit_type_addr2));
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            break;
          }
          case M_B: {
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_value_t tmpb_ = jit_insn_mul(function, ai_b, bi_b);
            jit_value_t tmpb_2 = jit_insn_rem(function, tmpb_, JIT_CONST(CORESIZE, jit_type_addr2));
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            break;
          }
          case M_AB: {
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_value_t tmpb_ = jit_insn_mul(function, ai_a, bi_b);
            jit_value_t tmpb_2 = jit_insn_rem(function, tmpb_, JIT_CONST(CORESIZE, jit_type_addr2));
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            break;
          }
          case M_BA: {
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_value_t tmpa_ = jit_insn_mul(function, ai_b, bi_a);
            jit_value_t tmpa_2 = jit_insn_rem(function, tmpa_, JIT_CONST(CORESIZE, jit_type_addr2));
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            break;
          }
          case M_F: case M_I: {
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_value_t tmpa_ = jit_insn_mul(function, ai_a, bi_a);
            jit_value_t tmpb_ = jit_insn_mul(function, ai_b, bi_b);
            jit_value_t tmpa_2 = jit_insn_rem(function, tmpa_, JIT_CONST(CORESIZE, jit_type_addr2));
            jit_value_t tmpb_2 = jit_insn_rem(function, tmpb_, JIT_CONST(CORESIZE, jit_type_addr2));
            JIT_INSTR2_S(tmp1, a, tmpa_2);
            JIT_INSTR2_S(tmp1, b, tmpb_2);
            break;
          }
          case M_X: {
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_value_t tmpa_ = jit_insn_mul(function, ai_b, bi_a);
            jit_value_t tmpb_ = jit_insn_mul(function, ai_a, bi_b);
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
      switch(c1_c._M) {
          case M_A: {
            jit_label_t labele = jit_label_undefined;
            jit_label_t labelb = jit_label_undefined;
            jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele);
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpa_ = jit_insn_div(function, bi_a, ai_a);
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
              jit_value_t tmpb_ = jit_insn_div(function, bi_b, ai_b);
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
              jit_value_t tmpb_ = jit_insn_div(function, bi_b, ai_a);
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
              jit_value_t tmpa_ = jit_insn_div(function, bi_a, ai_b);
              JIT_INSTR2_S(tmp1, a, tmpa_);
            jit_insn_branch(function, &labelb);
            jit_insn_label(function, &labele);
              REMOVE_PROC_JIT(); //automatically jit_insn_returns
            jit_insn_label(function, &labelb);
            break; }
          case M_F: case M_I: {
            jit_value_t chkend = jit_value_create(function, jit_type_sys_int);
            jit_insn_store(function, chkend, JIT_CONST(0, jit_type_sys_int));
            jit_label_t labele1 = jit_label_undefined;
            jit_label_t labelb1 = jit_label_undefined;
            jit_label_t labele2 = jit_label_undefined;
            jit_label_t labelb2 = jit_label_undefined;
            jit_label_t labelb3 = jit_label_undefined;
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_b), &labele1);
              jit_value_t tmpb_ = jit_insn_div(function, bi_b, ai_b);
              JIT_INSTR2_S(tmp1, b, tmpb_);
            jit_insn_branch(function, &labelb1);
            jit_insn_label(function, &labele1);
              jit_insn_store(function, chkend, JIT_CONST(1, jit_type_sys_int));
            jit_insn_label(function, &labelb1);
            jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele2);
              jit_value_t tmpa_ = jit_insn_div(function, bi_a, ai_a);
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
            jit_value_t chkend = jit_value_create(function, jit_type_sys_int);
            jit_insn_store(function, chkend, JIT_CONST(0, jit_type_sys_int));
            jit_label_t labele1 = jit_label_undefined;
            jit_label_t labelb1 = jit_label_undefined;
            jit_label_t labele2 = jit_label_undefined;
            jit_label_t labelb2 = jit_label_undefined;
            jit_label_t labelb3 = jit_label_undefined;
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_b), &labele1);
              jit_value_t tmpa_ = jit_insn_div(function, bi_a, ai_b);
              JIT_INSTR2_S(tmp1, a, tmpa_);
            jit_insn_branch(function, &labelb1);
            jit_insn_label(function, &labele1);
              jit_insn_store(function, chkend, JIT_CONST(1, jit_type_sys_int));
            jit_insn_label(function, &labelb1);
            jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele2);
              jit_value_t tmpb_ = jit_insn_div(function, bi_b, ai_a);
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
      switch(c1_c._M) {
          case M_A: {
            jit_label_t labele = jit_label_undefined;
            jit_label_t labelb = jit_label_undefined;
            jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele);
              jit_value_t tmp1 = JIT_CORE2_L(bp);
              jit_value_t tmpa_ = jit_insn_rem(function, bi_a, ai_a);
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
              jit_value_t tmpb_ = jit_insn_rem(function, bi_b, ai_b);
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
              jit_value_t tmpb_ = jit_insn_rem(function, bi_b, ai_a);
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
              jit_value_t tmpa_ = jit_insn_rem(function, bi_a, ai_b);
              JIT_INSTR2_S(tmp1, a, tmpa_);
            jit_insn_branch(function, &labelb);
            jit_insn_label(function, &labele);
              REMOVE_PROC_JIT(); //automatically jit_insn_returns
            jit_insn_label(function, &labelb);
            break; }
          case M_F: case M_I: {
            jit_value_t chkend = jit_value_create(function, jit_type_sys_int);
            jit_insn_store(function, chkend, JIT_CONST(0, jit_type_sys_int));
            jit_label_t labele1 = jit_label_undefined;
            jit_label_t labelb1 = jit_label_undefined;
            jit_label_t labele2 = jit_label_undefined;
            jit_label_t labelb2 = jit_label_undefined;
            jit_label_t labelb3 = jit_label_undefined;
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_b), &labele1);
              jit_value_t tmpb_ = jit_insn_rem(function, bi_b, ai_b);
              JIT_INSTR2_S(tmp1, b, tmpb_);
            jit_insn_branch(function, &labelb1);
            jit_insn_label(function, &labele1);
              jit_insn_store(function, chkend, JIT_CONST(1, jit_type_sys_int));
            jit_insn_label(function, &labelb1);
            jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele2);
              jit_value_t tmpa_ = jit_insn_rem(function, bi_a, ai_a);
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
            jit_value_t chkend = jit_value_create(function, jit_type_sys_int);
            jit_insn_store(function, chkend, JIT_CONST(0, jit_type_sys_int));
            jit_label_t labele1 = jit_label_undefined;
            jit_label_t labelb1 = jit_label_undefined;
            jit_label_t labele2 = jit_label_undefined;
            jit_label_t labelb2 = jit_label_undefined;
            jit_label_t labelb3 = jit_label_undefined;
            jit_value_t tmp1 = JIT_CORE2_L(bp);
            jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_b), &labele1);
              jit_value_t tmpa_ = jit_insn_rem(function, bi_a, ai_b);
              JIT_INSTR2_S(tmp1, a, tmpa_);
            jit_insn_branch(function, &labelb1);
            jit_insn_label(function, &labele1);
              jit_insn_store(function, chkend, JIT_CONST(1, jit_type_sys_int));
            jit_insn_label(function, &labelb1);
            jit_insn_branch_if_not(function, jit_insn_to_bool(function, ai_a), &labele2);
              jit_value_t tmpb_ = jit_insn_rem(function, bi_b, ai_a);
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
      switch(c1_c._M) {
        case M_A: case M_BA: {
          jit_label_t labele = jit_label_undefined;
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, bi_a), &labele);
            jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
            JIT_PROC2_pos_S(tmp1, ap);
            jit_value_t next = JIT_PROC2_next_L(tmp1);
            jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
            jit_insn_store(function, jit_locals.r, JIT_CONST(0, jit_type_sys_int));
            jit_insn_branch(function, &jit_locals.after);
          jit_insn_label(function, &labele);
          break; }
        case M_B: case M_AB: {
          jit_label_t labele = jit_label_undefined;
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, bi_b), &labele);
            jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
            JIT_PROC2_pos_S(tmp1, ap);
            jit_value_t next = JIT_PROC2_next_L(tmp1);
            jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
            jit_insn_store(function, jit_locals.r, JIT_CONST(0, jit_type_sys_int));
            jit_insn_branch(function, &jit_locals.after);
          jit_insn_label(function, &labele);
          break; }
        case M_F: case M_I: case M_X: {
          jit_label_t labele = jit_label_undefined;
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, bi_a), &labele);
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, bi_b), &labele);
            jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
            JIT_PROC2_pos_S(tmp1, ap);
            jit_value_t next = JIT_PROC2_next_L(tmp1);
            jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
            jit_insn_store(function, jit_locals.r, JIT_CONST(0, jit_type_sys_int));
            jit_insn_branch(function, &jit_locals.after);
          jit_insn_label(function, &labele);
          break; }
      }
      break;
    #endif
    #ifdef O_JMN
    case O_JMN:
      switch(c1_c._M) {
        case M_A: case M_BA: {
          jit_label_t labele = jit_label_undefined;
          jit_insn_branch_if_not(function, jit_insn_to_bool(function, bi_a), &labele);
            jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
            JIT_PROC2_pos_S(tmp1, ap);
            jit_value_t next = JIT_PROC2_next_L(tmp1);
            jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
            jit_insn_store(function, jit_locals.r, JIT_CONST(0, jit_type_sys_int));
            jit_insn_branch(function, &jit_locals.after);
          jit_insn_label(function, &labele);
          break; }
        case M_B: case M_AB: {
          jit_label_t labele = jit_label_undefined;
          jit_insn_branch_if_not(function, jit_insn_to_bool(function, bi_b), &labele);
            jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
            JIT_PROC2_pos_S(tmp1, ap);
            jit_value_t next = JIT_PROC2_next_L(tmp1);
            jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;
            jit_insn_store(function, jit_locals.r, JIT_CONST(0, jit_type_sys_int));
            jit_insn_branch(function, &jit_locals.after);
          jit_insn_label(function, &labele);
          break; }
        case M_F: case M_I: case M_X: {
          jit_label_t labele = jit_label_undefined;
          jit_label_t labelb = jit_label_undefined;
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, bi_a), &labele);
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, bi_b), &labele);
          jit_insn_branch(function, &labelb);
          jit_insn_label(function, &labele);
            jit_value_t tmp1 = JIT_LPROC2_L(JIT_W());
            JIT_PROC2_pos_S(tmp1, ap);
            jit_value_t next = JIT_PROC2_next_L(tmp1);
            jit_insn_store_elem(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_proc2), jit_type_void_ptr), JIT_W(), next); //l_proc2[w] = l_proc2[w]->next;                  jit_insn_return(function, JIT_CONST(0, jit_type_sys_int));
            jit_insn_store(function, jit_locals.r, JIT_CONST(0, jit_type_sys_int));
            jit_insn_branch(function, &jit_locals.after);
          jit_insn_label(function, &labelb);
          break; }
      }
      break;
    #endif
    #ifdef O_DJZ
    case O_DJZ: {
      jit_value_t tmp2 = JIT_LPROC2_L(JIT_W());
      jit_label_t labele = jit_label_undefined, labelb = jit_label_undefined;
      switch(c1_c._M) {
        case M_A: case M_BA: {
          jit_value_t tmp1 = JIT_CORE2_L(bp);
          jit_value_t tmpa_ = jit_insn_sub(function, bi_a, JIT_CONST(1, jit_type_addr2s));
          jit_value_t tmpa_2;
          BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);

          jit_value_t ma = JIT_INSTR2_L(tmp1, a);
          jit_value_t ma_ = jit_insn_sub(function, ma, JIT_CONST(1, jit_type_addr2s));
          jit_value_t ma_2;
          BOUND_CORESIZE_LOW_JIT(ma_2, ma_);
          JIT_INSTR2_S(tmp1, a, ma_2);
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, tmpa_2), &labele);
          break; }
        case M_B: case M_AB: {
          jit_value_t tmp1 = JIT_CORE2_L(bp);
          jit_value_t tmpb_ = jit_insn_sub(function, bi_b, JIT_CONST(1, jit_type_addr2s));
          jit_value_t tmpb_2;
          BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);

          jit_value_t mb = JIT_INSTR2_L(tmp1, b);
          jit_value_t mb_ = jit_insn_sub(function, mb, JIT_CONST(1, jit_type_addr2s));
          jit_value_t mb_2;
          BOUND_CORESIZE_LOW_JIT(mb_2, mb_);
          JIT_INSTR2_S(tmp1, b, mb_2);
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, tmpb_2), &labele);
          break; }
        case M_F: case M_X: case M_I:  {
          jit_value_t tmp1 = JIT_CORE2_L(bp);
          jit_value_t tmpa_ = jit_insn_sub(function, bi_a, JIT_CONST(1, jit_type_addr2s));
          jit_value_t tmpb_ = jit_insn_sub(function, bi_b, JIT_CONST(1, jit_type_addr2s));
          jit_value_t tmpa_2, tmpb_2;
          BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
          BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);

          jit_value_t ma = JIT_INSTR2_L(tmp1, a);
          jit_value_t mb = JIT_INSTR2_L(tmp1, b);
          jit_value_t ma_ = jit_insn_sub(function, ma, JIT_CONST(1, jit_type_addr2s));
          jit_value_t mb_ = jit_insn_sub(function, mb, JIT_CONST(1, jit_type_addr2s));
          jit_value_t ma_2, mb_2;
          BOUND_CORESIZE_LOW_JIT(ma_2, ma_);
          BOUND_CORESIZE_LOW_JIT(mb_2, mb_);
          JIT_INSTR2_S(tmp1, a, ma_2);
          JIT_INSTR2_S(tmp1, b, mb_2);
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
      switch(c1_c._M) {
        case M_A: case M_BA: {
          jit_value_t tmp1 = JIT_CORE2_L(bp);
          jit_value_t tmpa_ = jit_insn_sub(function, bi_a, JIT_CONST(1, jit_type_addr2s));
          jit_value_t tmpa_2;
          BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);

          jit_value_t ma = JIT_INSTR2_L(tmp1, a);
          jit_value_t ma_ = jit_insn_sub(function, ma, JIT_CONST(1, jit_type_addr2s));
          jit_value_t ma_2;
          BOUND_CORESIZE_LOW_JIT(ma_2, ma_);
          JIT_INSTR2_S(tmp1, a, ma_2);
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, tmpa_2), &labele);
          break; }
        case M_B: case M_AB: {
          jit_value_t tmp1 = JIT_CORE2_L(bp);
          jit_value_t tmpb_ = jit_insn_sub(function, bi_b, JIT_CONST(1, jit_type_addr2s));
          jit_value_t tmpb_2;
          BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);

          jit_value_t mb = JIT_INSTR2_L(tmp1, b);
          jit_value_t mb_ = jit_insn_sub(function, mb, JIT_CONST(1, jit_type_addr2s));
          jit_value_t mb_2;
          BOUND_CORESIZE_LOW_JIT(mb_2, mb_);
          JIT_INSTR2_S(tmp1, b, mb_2);
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, tmpb_2), &labele);
          break; }
        case M_F: case M_X: case M_I:  {
          jit_value_t tmp1 = JIT_CORE2_L(bp);
          jit_value_t tmpa_ = jit_insn_sub(function, bi_a, JIT_CONST(1, jit_type_addr2s));
          jit_value_t tmpb_ = jit_insn_sub(function, bi_b, JIT_CONST(1, jit_type_addr2s));
          jit_value_t tmpa_2, tmpb_2;
          BOUND_CORESIZE_LOW_JIT(tmpa_2, tmpa_);
          BOUND_CORESIZE_LOW_JIT(tmpb_2, tmpb_);

          jit_value_t ma = JIT_INSTR2_L(tmp1, a);
          jit_value_t mb = JIT_INSTR2_L(tmp1, b);
          jit_value_t ma_ = jit_insn_sub(function, ma, JIT_CONST(1, jit_type_addr2s));
          jit_value_t mb_ = jit_insn_sub(function, mb, JIT_CONST(1, jit_type_addr2s));
          jit_value_t ma_2, mb_2;
          BOUND_CORESIZE_LOW_JIT(ma_2, ma_);
          BOUND_CORESIZE_LOW_JIT(mb_2, mb_);
          JIT_INSTR2_S(tmp1, a, ma_2);
          JIT_INSTR2_S(tmp1, b, mb_2);
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, tmpa_2), &labele);
          jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, tmpb_2), &labele);
          break; }
      }
        jit_value_t pos = JIT_PROC2_pos_L(tmp2);
        jit_value_t pos_ = jit_insn_add(function, pos, JIT_CONST(1, jit_type_addr2s));
        jit_value_t pos_2;
        BOUND_CORESIZE_HIGH_JIT(pos_2, pos_);
        JIT_PROC2_pos_S(tmp2, pos_2);
      jit_insn_branch(function, &labelb);
      jit_insn_label(function, &labele);
        JIT_PROC2_pos_S(tmp2, ap);
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
      switch(c1_c._M) {
        case M_I: {
          jit_value_t tmpa = JIT_CORE2_L(ap);
          jit_value_t tmpb = JIT_CORE2_L(bp);
          jit_value_t ai = JIT_INSTR2_in_L(tmpa);
          jit_value_t aa = ai_a;
          jit_value_t ab = ai_b;
          jit_value_t bi = JIT_INSTR2_in_L(tmpb);
          jit_value_t ba = bi_a;
          jit_value_t bb = bi_b;
          jit_insn_branch_if_not(function, jit_insn_eq(function, ai, bi), &labele);
          jit_insn_branch_if_not(function, jit_insn_eq(function, aa, ba), &labele);
          jit_insn_branch_if_not(function, jit_insn_eq(function, ab, bb), &labele);
          break; }
        case M_A: {
          jit_value_t aa = ai_a;
          jit_value_t ba = bi_a;
          jit_insn_branch_if_not(function, jit_insn_eq(function, aa, ba), &labele);
          break; }
        case M_B: {
          jit_value_t ab = ai_b;
          jit_value_t bb = bi_b;
          jit_insn_branch_if_not(function, jit_insn_eq(function, ab, bb), &labele);
          break; }
        case M_AB: {
          jit_value_t aa = ai_a;
          jit_value_t bb = bi_b;
          jit_insn_branch_if_not(function, jit_insn_eq(function, aa, bb), &labele);
          break; }
        case M_BA: {
          jit_value_t ab = ai_b;
          jit_value_t ba = bi_a;
          jit_insn_branch_if_not(function, jit_insn_eq(function, ab, ba), &labele);
          break; }
        case M_F: {
          jit_value_t aa = ai_a;
          jit_value_t ab = ai_b;
          jit_value_t ba = bi_a;
          jit_value_t bb = bi_b;
          jit_insn_branch_if_not(function, jit_insn_eq(function, aa, ba), &labele);
          jit_insn_branch_if_not(function, jit_insn_eq(function, ab, bb), &labele);
          break; }
        case M_X: {
          jit_value_t aa = ai_a;
          jit_value_t ab = ai_b;
          jit_value_t ba = bi_a;
          jit_value_t bb = bi_b;
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
      switch(c1_c._M) {
        case M_I: {
          jit_value_t tmpa = JIT_CORE2_L(ap);
          jit_value_t tmpb = JIT_CORE2_L(bp);
          jit_value_t ai = JIT_INSTR2_in_L(tmpa);
          jit_value_t aa = ai_a;
          jit_value_t ab = ai_b;
          jit_value_t bi = JIT_INSTR2_in_L(tmpb);
          jit_value_t ba = bi_a;
          jit_value_t bb = bi_b;
          jit_insn_branch_if_not(function, jit_insn_eq(function, ai, bi), &labele);
          jit_insn_branch_if_not(function, jit_insn_eq(function, aa, ba), &labele);
          jit_insn_branch_if_not(function, jit_insn_eq(function, ab, bb), &labele);
          break; }
        case M_A: {
          jit_value_t aa = ai_a;
          jit_value_t ba = bi_a;
          jit_insn_branch_if_not(function, jit_insn_eq(function, aa, ba), &labele);
          break; }
        case M_B: {
          jit_value_t ab = ai_b;
          jit_value_t bb = bi_b;
          jit_insn_branch_if_not(function, jit_insn_eq(function, ab, bb), &labele);
          break; }
        case M_AB: {
          jit_value_t aa = ai_a;
          jit_value_t bb = bi_b;
          jit_insn_branch_if_not(function, jit_insn_eq(function, aa, bb), &labele);
          break; }
        case M_BA: {
          jit_value_t ab = ai_b;
          jit_value_t ba = bi_a;
          jit_insn_branch_if_not(function, jit_insn_eq(function, ab, ba), &labele);
          break; }
        case M_F: {
          jit_value_t aa = ai_a;
          jit_value_t ab = ai_b;
          jit_value_t ba = bi_a;
          jit_value_t bb = bi_b;
          jit_insn_branch_if_not(function, jit_insn_eq(function, aa, ba), &labele);
          jit_insn_branch_if_not(function, jit_insn_eq(function, ab, bb), &labele);
          break; }
        case M_X: {
          jit_value_t aa = ai_a;
          jit_value_t ab = ai_b;
          jit_value_t ba = bi_a;
          jit_value_t bb = bi_b;
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
      switch(c1_c._M) {
        case M_A: {
          jit_value_t aa = ai_a;
          jit_value_t ba = bi_a;
          jit_insn_branch_if_not(function, jit_insn_lt(function, aa, ba), &labele);
          break; }
        case M_B: {
          jit_value_t ab = ai_b;
          jit_value_t bb = bi_b;
          jit_insn_branch_if_not(function, jit_insn_lt(function, ab, bb), &labele);
          break; }
        case M_AB: {
          jit_value_t aa = ai_a;
          jit_value_t bb = bi_b;
          jit_insn_branch_if_not(function, jit_insn_lt(function, aa, bb), &labele);
          break; }
        case M_BA: {
          jit_value_t ab = ai_b;
          jit_value_t ba = bi_a;
          jit_insn_branch_if_not(function, jit_insn_lt(function, ab, ba), &labele);
          break; }
        case M_F: case M_I: {
          jit_value_t aa = ai_a;
          jit_value_t ab = ai_b;
          jit_value_t ba = bi_a;
          jit_value_t bb = bi_b;
          jit_insn_branch_if_not(function, jit_insn_lt(function, aa, ba), &labele);
          jit_insn_branch_if_not(function, jit_insn_lt(function, ab, bb), &labele);
          break; }
        case M_X: {
          jit_value_t aa = ai_a;
          jit_value_t ab = ai_b;
          jit_value_t ba = bi_a;
          jit_value_t bb = bi_b;
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
        jit_insn_store(function, jit_locals.r, JIT_CONST(0, jit_type_sys_int));
        jit_insn_branch(function, &jit_locals.after);
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
      jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_pspace_local_accessed), jit_type_sys_int)), &labelb);
        jit_insn_call_native(function, NULL, mlock_helper_jit, signature_mlock_helper_jit, NULL, 0, JIT_CALL_NOTHROW);
        jit_insn_store_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_pspace_local_accessed), JIT_CONST(1, jit_type_sys_int));
      jit_insn_label(function, &labelb);
      #endif
      switch(c1_c._M) {
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
            jit_insn_store(function, tmpv, jit_insn_load_relative(function, tmp2, offsetof(WARRIOR, psp0), jit_type_pcell)); //.psp0
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
            jit_insn_store(function, tmpv, jit_insn_load_relative(function, tmp2, offsetof(WARRIOR, psp0), jit_type_pcell)); //.psp0
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
            jit_insn_store(function, tmpv, jit_insn_load_relative(function, tmp2, offsetof(WARRIOR, psp0), jit_type_pcell)); //.psp0
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
            jit_insn_store(function, tmpv, jit_insn_load_relative(function, tmp2, offsetof(WARRIOR, psp0), jit_type_pcell)); //.psp0
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
      jit_insn_branch_if_not(function, jit_insn_to_not_bool(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_pspace_local_accessed), jit_type_sys_int)), &labelb);
        jit_insn_call_native(function, NULL, mlock_helper_jit, signature_mlock_helper_jit, NULL, 0, JIT_CALL_NOTHROW);
        jit_insn_store_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_pspace_local_accessed), JIT_CONST(1, jit_type_sys_int));
      jit_insn_label(function, &labelb);
      #endif
      switch(c1_c._M) {
        case M_A: {
          jit_value_t tmpx = bi_a;
          jit_value_t tmpx_ = jit_insn_rem(function, tmpx, JIT_CONST(PSPACESIZE, jit_type_addr2));
          jit_label_t labelb2 = jit_label_undefined;
          jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpx_), &labelb2);
            jit_value_t tmp2_ = jit_insn_load_elem_address(function, JIT_CONST(warriors, jit_type_void_ptr), JIT_W(), jit_type_warrior); //warriors[w]
            jit_value_t pspace = jit_insn_load_relative(function, tmp2_, offsetof(WARRIOR, pspace), jit_type_void_ptr); //.pspace
            jit_value_t tmpv = jit_insn_convert(function, ai_a, jit_type_pcell, 0);
            jit_insn_store_elem(function, pspace, tmpx_, tmpv);
          jit_insn_label(function, &labelb2);
          break; }
        case M_B: case M_I: case M_F: case M_X: {
          jit_value_t tmpx = bi_b;
          jit_value_t tmpx_ = jit_insn_rem(function, tmpx, JIT_CONST(PSPACESIZE, jit_type_addr2));
          jit_label_t labelb2 = jit_label_undefined;
          jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpx_), &labelb2);
            jit_value_t tmp2_ = jit_insn_load_elem_address(function, JIT_CONST(warriors, jit_type_void_ptr), JIT_W(), jit_type_warrior); //warriors[w]
            jit_value_t pspace = jit_insn_load_relative(function, tmp2_, offsetof(WARRIOR, pspace), jit_type_void_ptr); //.pspace
            jit_value_t tmpv = jit_insn_convert(function, ai_b, jit_type_pcell, 0);
            jit_insn_store_elem(function, pspace, tmpx_, tmpv);
          jit_insn_label(function, &labelb2);
          break; }
        case M_AB: {
          jit_value_t tmpx = bi_b;
          jit_value_t tmpx_ = jit_insn_rem(function, tmpx, JIT_CONST(PSPACESIZE, jit_type_addr2));
          jit_label_t labelb2 = jit_label_undefined;
          jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpx_), &labelb2);
            jit_value_t tmp2_ = jit_insn_load_elem_address(function, JIT_CONST(warriors, jit_type_void_ptr), JIT_W(), jit_type_warrior); //warriors[w]
            jit_value_t pspace = jit_insn_load_relative(function, tmp2_, offsetof(WARRIOR, pspace), jit_type_void_ptr); //.pspace
            jit_value_t tmpv = jit_insn_convert(function, ai_a, jit_type_pcell, 0);
            jit_insn_store_elem(function, pspace, tmpx_, tmpv);
          jit_insn_label(function, &labelb2);
          break; }
        case M_BA: {
          jit_value_t tmpx = bi_a;
          jit_value_t tmpx_ = jit_insn_rem(function, tmpx, JIT_CONST(PSPACESIZE, jit_type_addr2));
          jit_label_t labelb2 = jit_label_undefined;
          jit_insn_branch_if_not(function, jit_insn_to_bool(function, tmpx_), &labelb2);
            jit_value_t tmp2_ = jit_insn_load_elem_address(function, JIT_CONST(warriors, jit_type_void_ptr), JIT_W(), jit_type_warrior); //warriors[w]
            jit_value_t pspace = jit_insn_load_relative(function, tmp2_, offsetof(WARRIOR, pspace), jit_type_void_ptr); //.pspace
            jit_value_t tmpv = jit_insn_convert(function, ai_b, jit_type_pcell, 0);
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
      switch(c1_c._M) {
        case M_I: {
          jit_value_t tmpa = JIT_CORE2_L(ap);
          jit_value_t tmpb = JIT_CORE2_L(bp);
          jit_value_t ai = JIT_INSTR2_in_L(tmpa);
          jit_value_t bi = JIT_INSTR2_in_L(tmpb);
          JIT_INSTR2_S(tmpa, in, bi);
          JIT_INSTR2_S(tmpa, a, bi_a);
          JIT_INSTR2_S(tmpa, b, bi_b);
          JIT_INSTR2_S(tmpb, in, ai);
          JIT_INSTR2_S(tmpb, a, ai_a);
          JIT_INSTR2_S(tmpb, b, ai_b);
          break; }
        case M_A: {
          jit_value_t tmpa = JIT_CORE2_L(ap);
          jit_value_t tmpb = JIT_CORE2_L(bp);
          JIT_INSTR2_S(tmpa, a, bi_a);
          JIT_INSTR2_S(tmpb, a, ai_a);
          break; }
        case M_B: {
          jit_value_t tmpa = JIT_CORE2_L(ap);
          jit_value_t tmpb = JIT_CORE2_L(bp);
          JIT_INSTR2_S(tmpa, b, bi_b);
          JIT_INSTR2_S(tmpb, b, ai_b);
          break; }
        case M_AB: {
          jit_value_t tmpa = JIT_CORE2_L(ap);
          jit_value_t tmpb = JIT_CORE2_L(bp);
          JIT_INSTR2_S(tmpa, a, bi_b);
          JIT_INSTR2_S(tmpb, b, ai_a);
          break; }
        case M_BA: {
          jit_value_t tmpa = JIT_CORE2_L(ap);
          jit_value_t tmpb = JIT_CORE2_L(bp);
          JIT_INSTR2_S(tmpa, b, bi_a);
          JIT_INSTR2_S(tmpb, a, ai_b);
          break; }
        case M_F: {
          jit_value_t tmpa = JIT_CORE2_L(ap);
          jit_value_t tmpb = JIT_CORE2_L(bp);
          JIT_INSTR2_S(tmpa, a, bi_a);
          JIT_INSTR2_S(tmpa, b, bi_b);
          JIT_INSTR2_S(tmpb, a, ai_a);
          JIT_INSTR2_S(tmpb, b, ai_b);
          break; }
        case M_X: {
          jit_value_t tmpa = JIT_CORE2_L(ap);
          jit_value_t tmpb = JIT_CORE2_L(bp);
          JIT_INSTR2_S(tmpa, b, bi_a);
          JIT_INSTR2_S(tmpa, a, bi_b);
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
        switch(c1_c._M) {
          case M_I: {
            jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
              (jit_value_t[]){local_core_jit, JIT_CONST(M_I, jit_type_ubyte), ai_a, ai_b,
              JIT_INSTR2_in_L(JIT_CORE2_L(ap))}, 5, JIT_CALL_NOTHROW);
            break; }
          case M_A: case M_AB: {
            jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
              (jit_value_t[]){local_core_jit, JIT_CONST(c1_c._M, jit_type_ubyte), ai_a, JIT_CONST(0, jit_type_addr2),
              JIT_CONST(0, jit_type_void_ptr)}, 5, JIT_CALL_NOTHROW);
            break; }
          case M_B: case M_BA: {
            jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
              (jit_value_t[]){local_core_jit, JIT_CONST(c1_c._M, jit_type_ubyte), JIT_CONST(0, jit_type_addr2), ai_b,
              JIT_CONST(0, jit_type_void_ptr)}, 5, JIT_CALL_NOTHROW);
            break; }
          case M_F: case M_X: {
            jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
              (jit_value_t[]){local_core_jit, JIT_CONST(c1_c._M, jit_type_ubyte), ai_a, ai_b,
              JIT_CONST(0, jit_type_void_ptr)}, 5, JIT_CALL_NOTHROW);
            break; }
        }
      #else
        switch(c1_c._M) {
          case M_I: {
            jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
              (jit_value_t[]){JIT_CONST(M_I, jit_type_uint), ai_a, ai_b,
              JIT_INSTR2_in_L(JIT_CORE2_L(ap))}, 4, JIT_CALL_NOTHROW);
            break; }
          case M_A: case M_AB: {
            jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
              (jit_value_t[]){JIT_CONST(c1_c._M, jit_type_ubyte), ai_a, JIT_CONST(0, jit_type_addr2),
              JIT_CONST(0, jit_type_void_ptr)}, 4, JIT_CALL_NOTHROW);
            break; }
          case M_B: case M_BA: {
            jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
              (jit_value_t[]){JIT_CONST(c1_c._M, jit_type_ubyte), JIT_CONST(0, jit_type_addr2), ai_b,
              JIT_CONST(0, jit_type_void_ptr)}, 4, JIT_CALL_NOTHROW);
            break; }
          case M_F: case M_X: {
            jit_insn_call_native(function, NULL, sts_jit, signature_sts_jit,
              (jit_value_t[]){JIT_CONST(c1_c._M, jit_type_ubyte), ai_a, ai_b,
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

  return;
}

void compile_jit_all() {
  //JIT compiler
  jit_context = jit_context_create();
  jit_context_build_start(jit_context);
  jit_type_t jit_type_instr2 = compile_jit_type_instr2();
  //jit_type_t jit_type_pcell = JIT_TYPE(pcell_t);
  //jit_type_t jit_type_warrior = compile_jit_type_warrior(jit_type_pcell);
  //jit_type_t jit_type_proc2 = compile_jit_type_proc2();


  jit_function_t function;
  #ifdef TSAFE_CORE
  jit_type_t params[1] = {jit_type_void_ptr};
  #endif
  jit_type_t signature;
  #ifdef TSAFE_CORE
  signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, params, 1, 1);
  #else
  signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, NULL, 0, 1);
  #endif
  function = jit_func_main_loop = jit_function_create(jit_context, signature);

  #ifdef TSAFE_CORE
  jit_value_t local_core_jit = jit_value_get_param(function, 0);
  #else
  jit_value_t local_core_jit = JIT_CONST(local_core, jit_type_void_ptr);
  #endif

  jit_type_t signature_jit_error = jit_type_create_signature(jit_abi_cdecl, jit_type_void, (jit_type_t[]){jit_type_void_ptr}, 1, 1);
  /*#ifdef TSAFE_CORE
  jit_type_t signature_remove_proc_jit = jit_type_create_signature(jit_abi_cdecl, jit_type_nint, (jit_type_t[]){jit_type_void_ptr}, 1, 1);
  #else
  jit_type_t signature_remove_proc_jit = jit_type_create_signature(jit_abi_cdecl, jit_type_nint, NULL, 0, 1);
  #endif*/
  jit_value_t core2 = jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_core2), jit_type_void_ptr);

  jit_label_t labelendbattle = jit_label_undefined;
  jit_value_t c = jit_value_create(function, jit_type_sys_ulong);
  jit_value_t pc = jit_value_create(function, jit_type_addr2);
  jit_value_t _tmp = jit_value_create(function, jit_type_void_ptr);
  jit_value_t a = jit_value_create(function, jit_type_addr2);
  jit_value_t b = jit_value_create(function, jit_type_addr2);

  #ifdef LIBJIT_NESTING
  jit_locals.function = function;
  jit_locals.local_core = local_core_jit;
  jit_locals.pc = pc;
  jit_locals.a = a;
  jit_locals.b = b;
  jit_locals.r = jit_value_create(function, jit_type_sys_int);
  #endif

  jit_insn_store(function, c, JIT_CONST(0, jit_type_sys_ulong));
  jit_label_t labeloutstart = jit_label_undefined;
  jit_label_t labeloutend = jit_label_undefined;
  jit_insn_label(function, &labeloutstart);
  jit_insn_branch_if_not(function, jit_insn_lt(function, c, JIT_CONST(MAXCYCLES, jit_type_sys_ulong)), &labeloutend);
    jit_insn_store_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_w2), JIT_CONST(0, jit_type_sys_ulong));
    jit_label_t labelinstart = jit_label_undefined;
    jit_label_t labelinend = jit_label_undefined;
    jit_insn_label(function, &labelinstart);
    jit_insn_branch_if_not(function, jit_insn_lt(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_w2), jit_type_sys_ulong), jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_nrunning), jit_type_sys_ulong)), &labelinend);
      jit_insn_store(function, pc, JIT_PROC2_pos_L(JIT_LPROC2_L(jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_w2), jit_type_sys_ulong)))); //addr2_t pc = l_proc2[l_w2]->pos
      jit_insn_store(function, _tmp, JIT_CORE2_L(pc)); //(&l_core2[pc])
      jit_insn_store(function, a, jit_insn_load_relative(function, _tmp, offsetof(INSTR2, a), jit_type_addr2)); //addr2_t a = l_core2[pc].a;
      jit_insn_store(function, b, jit_insn_load_relative(function, _tmp, offsetof(INSTR2, b), jit_type_addr2)); //addr2_t b = l_core2[pc].b;

      jit_value_t in = jit_insn_load_relative(function, _tmp, offsetof(INSTR2, in), JIT_TYPE(int_fast32_t));
      jit_label_t* jtable = (jit_label_t*) malloc(g_data2.nentr * sizeof(jit_label_t));
      int_fast32_t k;
      for(k = 0; k < g_data2.nentr; ++k) jtable[k] = jit_label_undefined;
      jit_locals.after = jit_label_undefined;

      jit_insn_jump_table(function, in, jtable, g_data2.nentr);

      //unknown instruction index
      jit_value_t msg = JIT_CONST("JIT code found unknown instruction.", jit_type_void_ptr);
      jit_insn_call_native(function, "jit_error", jit_error, signature_jit_error, &msg, 1, JIT_CALL_NORETURN | JIT_CALL_NOTHROW);

      for(k = 0; k < g_data2.nentr; ++k) {
        jit_insn_label(function, &jtable[k]);
        INSTR1 i1;
        i1._OMA = g_data2.oma[k];

        compile_instr(i1);
        jit_insn_branch(function, &jit_locals.after);
      }
      jit_insn_label(function, &jit_locals.after);

      jit_insn_branch_if(function, jit_insn_to_bool(function, jit_locals.r), &labelendbattle);
    jit_insn_store_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_w2), jit_insn_add(function, jit_insn_load_relative(function, local_core_jit, offsetof(LOCAL_CORE, la_w2), jit_type_sys_ulong), JIT_CONST(1, jit_type_sys_ulong))); //++l_w2
    jit_insn_branch(function, &labelinstart);
    jit_insn_label(function, &labelinend);
  jit_insn_store(function, c, jit_insn_add(function, c, JIT_CONST(1, jit_type_sys_ulong))); //++c
  jit_insn_branch(function, &labeloutstart);
  jit_insn_label(function, &labeloutend);

  jit_insn_label(function, &labelendbattle);
  jit_insn_return(function, NULL);
  jit_function_compile(function);
  jit_main_loop = jit_function_to_closure(function);

  jit_context_build_end(jit_context);
  return;
}

void jit_invalidate() {
  if(jit_main_loop == NULL) return;
  jit_context_destroy(jit_context);
  jit_main_loop = NULL;
  return;
}
