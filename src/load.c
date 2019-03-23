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
#if PSPACESIZE
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

DATA2 g_data2 = {{{-2, 0}}, 0, 0, NULL, -1};

jitind_t instr1to2(uint32_t oma) {
  INSTR1 i;
  i._OMA = oma;

  uint8_t hash = i._O ^ i._M ^ i._aA ^ i._aB;
  hash &= 0x3F;
  if(g_data2.hasht[hash].in == -2) { //hash table miss
    g_data2.hasht[hash].oma = i._OMA;
    g_data2.curhpos = hash; //set .in later
    goto _noindex;
  }
  else if(g_data2.hasht[hash].in == -1) { //hash table collision
    int c;
    for(c = 0; c < g_data2.nentr; ++c) {
      if(g_data2.oma[c] == i._OMA) return c;
    }
    g_data2.curhpos = -1; //don't bother about hash table
    goto _noindex;
  }
  else { //hash table hit
    if(g_data2.hasht[hash].oma == i._OMA) { //correct element
      return g_data2.hasht[hash].in;
    }
    else { //must add new
      g_data2.hasht[hash].in = -1; //now there's a collision
      g_data2.curhpos = -1; //don't bother about hash table
      goto _noindex;
    }
  }
  _noindex:
  ;
  jitind_t r = g_data2.nentr;

  ++g_data2.nentr; //no callback registered, add new
  jit_invalidate();
  if(g_data2.nentr > g_data2.allocd) {
    g_data2.allocd += 16;
    g_data2.oma = (uint32_t*) realloc(g_data2.oma, g_data2.allocd * sizeof(DATA2_ELEM));
  }
  g_data2.oma[g_data2.nentr-1] = oma;

  if(g_data2.curhpos != -1) {
    g_data2.hasht[g_data2.curhpos].in = r;
  }
  return r;
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

  unsigned int c;
  for(c = 0; c < w->len; ++c) {
    INSTR2 i2;
    i2.in = instr1to2(c1[c]._OMA); //only translates if duplicate
    i2.a = c1[c]._A; //this limits field width to to 15 bits
    i2.b = c1[c]._B;
    w->code2[c] = i2;
  }

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

    minit(warriors[c].mutex);
  }

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

void unload_warrior(int c) {
  if(warriors[c].code1 != NULL) {free(warriors[c].code1); warriors[c].code1 = NULL;}
  if(warriors[c].code2 != NULL) {free(warriors[c].code2); warriors[c].code2 = NULL;}
  free(warriors[c].name); warriors[c].name = NULL;
  free(warriors[c].author); warriors[c].author = NULL;
  #if PSPACESIZE
  unsigned long k;
  int check = 0;
  if(warriors[c].haspin) {
    for(k = 0; k < c; ++k) {
      if(warriors[c].pin == warriors[k].pin) check = 1;
    }
  }
  if(!check) free(warriors[c].pspace);
  warriors[c].pspace = NULL;
  #endif
  mdestroy(warriors[c].mutex);
  return;
}

void unload_all() {
  unsigned long c;
  for(c = 0; c < WARRIORS; ++c) {
    unload_warrior(c);
  }
  free(warriors);
  return;
}

void set_nwarriors(int nw) {
  if(nw < WARRIORS) {
    int c;
    for(c = nw; c < WARRIORS; ++c) {
      unload_warrior(c);
    }
  }
  WARRIORS = nw;
  warriors = (WARRIOR*) realloc(warriors, WARRIORS * sizeof(WARRIOR));
  return;
}

void reset_warrior(WARRIOR* w) {
  memset(w->pspace, 0, PSPACESIZE * sizeof(pcell_t));
  w->psp0 = CORESIZE - 1;
  w->wins = w->losses = 0;
  return;
}

void init_warrior(WARRIOR* w) {
  #if PSPACESIZE
  w->pspace = malloc(PSPACESIZE * sizeof(pcell_t));
  #endif
  minit(w->mutex);
  reset_warrior(w);
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

  #if PSPACESIZE
  minit(mutex_pwarriors);
  #endif

  if((algorithm_select >> 2) & 1) {
    jit_init();
    hasht_reset();
    add_hdat();
    jit_invalidate();
  }
  return;
}

void finalize() {
  #ifdef _COREVIEW_
  signal_terminate();
  #endif

  jit_clear();

  #if PSPACESIZE
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
