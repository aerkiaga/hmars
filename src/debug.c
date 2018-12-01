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

#ifdef _COREVIEW_
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

uint32_t op_color[] = { //l_coreviewdata opcode color
  #ifdef O_DAT
  0xFF000000,
  #endif
  #ifdef O_MOV
  0xFF000080,
  #endif
  #ifdef O_ADD
  0xFF206020,
  #endif
  #ifdef O_SUB
  0xFF205030,
  #endif
  #ifdef O_MUL
  0xFF305020,
  #endif
  #ifdef O_DIV
  0xFF284028,
  #endif
  #ifdef O_MOD
  0xFF204020,
  #endif
  #ifdef O_JMP
  0xFF602020,
  #endif
  #ifdef O_JMZ
  0xFF502820,
  #endif
  #ifdef O_JMN
  0xFF502028,
  #endif
  #if defined(O_DJZ) && !defined(EXT_DJZ)
  0xFF604010,
  #endif
  #ifdef O_DJN
  0xFF603800,
  #endif
  #if defined(O_CMP) || defined(O_SEQ)
  0xFF505000,
  #endif
  #ifdef O_SNE
  0xFF405000,
  #endif
  #ifdef O_SLT
  0xFF405010,
  #endif
  #ifdef O_SPL
  0xFF602040,
  #endif
  #ifdef O_NOP
  0xFF2A2A2A,
  #endif
  #ifdef O_LDP
  0xFF202050,
  #endif
  #ifdef O_STP
  0xFF203048,
  #endif
  #ifdef O_XCH
  0xFF502050, //default color
  #endif
  #ifdef O_PCT
  0xFF502050, //default color
  #endif
  #if defined(O_DJZ) && defined(EXT_DJZ)
  0xFF604010,
  #endif
  #ifdef O_STS
  0xFF502050, //default color
  #endif
};

const char* cell_icons_default[][7] = { //icons for none, read, write, exec, inc, dec, curr; all sizes
  { //size 0 (unused)
    "", "", "", "", "", "", ""
  },
  { //size 1
    ".", //none
    ".", //read
    "*", //write
    "*", //exec
    "*", //inc
    "*", //dec
    "*" //curr
  },
  { //size 2
    "....", //none
    "*...", //read
    "*..*", //write
    "****", //exec
    "*.*.", //inc
    "**..", //dec
    "****" //curr
  },
  { //size 3
    ".........", //none
    "*........", //read
    "*...*....", //write
    "**.**....", //exec
    "*..*.....", //inc
    "**.......", //dec
    "*********" //curr
  },
  { //size 4
    "................", //none
    ".....*..........", //read
    "*.*..*..*.*.....", //write
    "***.***.***.....", //exec
    ".*..***..*......", //inc
    "....***.........", //dec
    "****************" //curr
  }
};

char*** cell_icons;

int cell_icon_max_size = 4;

void init_coreview() {
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
  return;
}

void update_coreview_metrics(COREVIEW* cv) {
  cv->cw = 100; //    just
  cv->ch = 80; //     some
  cv->csize = 4; //   example
  cv->scale = 1.0; // code

  if(!(cv->tgmode & TG_PW_AL)) cv->tg_pw_al = 0;
  if(!(cv->tgmode & TG_PW_AM)) cv->tg_pw_am = -1;
  if(!(cv->tgmode & TG_PH_AL)) cv->tg_ph_al = 0;
  if(!(cv->tgmode & TG_PH_AM)) cv->tg_ph_am = -1;
  if(!(cv->tgmode & TG_CSIZE)) cv->tg_csize = 4;

  int exactdivisors = 1;
  int naturalscale = 1; //only 0 or 1!!!!!!
  _label_repeat_metrics:;

  float best_aspect_csize_golf = -1.0;
  float best_scale_csize = 0.0;
  unsigned int best_cw_csize = 0;
  unsigned int best_ch_csize = 0;
  //best_csize_csize would always be cv->csize
  float best_aspect_nocsize_golf = -1.0;
  float best_scale_nocsize = 0.0;
  unsigned int best_cw_nocsize = 0;
  unsigned int best_ch_nocsize = 0;
  unsigned int best_csize_nocsize = 0;
  //find all possible divisors of CORESIZE
  unsigned long c;
  unsigned long max = sqrtf((float)CORESIZE);
  if(max*max == CORESIZE) ++max;
  for(c = 1; c < max; ++c) {
    unsigned int cw;
    unsigned int ch;
    if(exactdivisors && CORESIZE%c) continue;
    //first try cw
    cw = c;
    if(exactdivisors) ch = CORESIZE/c;
    else ch = ceil((float)CORESIZE/c);
    for(;;) { //twice
      //satisfy tg_p*_a* constraints and tg_csize
      float max_scale_csize = -1.0;
      float min_scale_csize = -1.0;
      float scale;
      if(naturalscale) {
        scale = floor(((float)cv->tg_pw_am/cw/cv->tg_csize));
        if(max_scale_csize < 0.0 || scale < max_scale_csize) max_scale_csize = scale;
        scale = floor(((float)cv->tg_ph_am/ch/cv->tg_csize));
        if(max_scale_csize < 0.0 || scale < max_scale_csize) max_scale_csize = scale;
        scale = ceil(((float)cv->tg_pw_al/cw/cv->tg_csize));
        if(min_scale_csize < 0.0 || scale > min_scale_csize) min_scale_csize = scale;
        scale = ceil(((float)cv->tg_ph_al/ch/cv->tg_csize));
        if(min_scale_csize < 0.0 || scale > min_scale_csize) min_scale_csize = scale;
      }
      else {
        scale = ((float)cv->tg_pw_am/cw/cv->tg_csize);
        if(max_scale_csize < 0.0 || scale < max_scale_csize) max_scale_csize = scale;
        scale = ((float)cv->tg_ph_am/ch/cv->tg_csize);
        if(max_scale_csize < 0.0 || scale < max_scale_csize) max_scale_csize = scale;
        scale = ((float)cv->tg_pw_al/cw/cv->tg_csize);
        if(min_scale_csize < 0.0 || scale > min_scale_csize) min_scale_csize = scale;
        scale = ((float)cv->tg_ph_al/ch/cv->tg_csize);
        if(min_scale_csize < 0.0 || scale > min_scale_csize) min_scale_csize = scale;
      }
      //if possible
      if(min_scale_csize <= max_scale_csize && max_scale_csize >= (float) naturalscale) {
        float aspect = (float)cw/ch;
        //give score to aspect ratio
        float aspect_golf = fabs(((aspect < 0)? 1.0/aspect : aspect) - PHI);
        if(best_aspect_csize_golf < 0.0 || aspect_golf < best_aspect_csize_golf) {
          best_aspect_csize_golf = aspect_golf;
          best_scale_csize = max_scale_csize;
          best_cw_csize = cw;
          best_ch_csize = ch;
        }
      }
      else if(!best_cw_csize) { //only tg_p*_a* constraints
        unsigned int csize;
        unsigned int csize_start;
        if(max_scale_csize < 1.0) { //cells must be smaller
          csize_start = cv->tg_csize-1;
        }
        else { //try other sizes just in case
          csize_start = cell_icon_max_size;
        }
        for(csize = csize_start; csize > 0; --csize) { //try all the csize's
          float max_scale_nocsize = -1.0;
          float min_scale_nocsize = -1.0;
          if(naturalscale) {
            scale = floor(((float)cv->tg_pw_am/cw/csize));
            if(max_scale_nocsize < 0.0 || scale < max_scale_nocsize) max_scale_nocsize = scale;
            scale = floor(((float)cv->tg_ph_am/ch/csize));
            if(max_scale_nocsize < 0.0 || scale < max_scale_nocsize) max_scale_nocsize = scale;
            scale = ceil(((float)cv->tg_pw_al/cw/csize));
            if(min_scale_nocsize < 0.0 || scale > min_scale_nocsize) min_scale_nocsize = scale;
            scale = ceil(((float)cv->tg_ph_al/ch/csize));
            if(min_scale_nocsize < 0.0 || scale > min_scale_nocsize) min_scale_nocsize = scale;
          }
          //if possible
          if(min_scale_nocsize <= max_scale_nocsize && max_scale_nocsize >= (float) naturalscale) {
            /*if(min_scale_nocsize == 0) min_scale_nocsize = 1;*/
            float aspect = (float)cw/ch;
            //give score to aspect ratio
            float aspect_golf = fabs(((aspect < 0)? 1.0/aspect : aspect) - PHI);
            if(best_aspect_nocsize_golf < 0.0 || aspect_golf < best_aspect_nocsize_golf) {
              best_aspect_nocsize_golf = aspect_golf;
              best_scale_nocsize = max_scale_nocsize;
              best_cw_nocsize = cw;
              best_ch_nocsize = ch;
              best_csize_nocsize = csize;
            }
            break; //aspect ratio does not change with csize
          }
        }
      }

      //then try ch
      if(ch == c) break; //finished both, or c*c = CORESIZE
      ch = c;
      if(exactdivisors) cw = CORESIZE/c;
      else cw = ceil((float)CORESIZE/c);
    }
  }

  if(best_cw_csize) {
    cv->csize = cv->tg_csize;
    cv->cw = best_cw_csize;
    cv->ch = best_ch_csize;
    cv->scale = best_scale_csize;
  }
  else if(best_cw_nocsize) {
    cv->csize = best_csize_nocsize;
    cv->cw = best_cw_nocsize;
    cv->ch = best_ch_nocsize;
    cv->scale = best_scale_nocsize;
  }
  else if(exactdivisors) {
    exactdivisors = 0;
    goto _label_repeat_metrics;
  }
  else if(naturalscale) {
    naturalscale = 0;
    exactdivisors = 1;
    goto _label_repeat_metrics;
  }
  else {
    error("Handling of such extreme constraints still not implemented.");
  }

  cv->pw = cv->cw * cv->csize * cv->scale;
  cv->ph = cv->ch * cv->csize * cv->scale;
  if(cv->img_tex != NULL) SDL_DestroyTexture(cv->img_tex);
  cv->img_tex = SDL_CreateTexture(renderer,
    SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
    cv->cw * cv->csize, cv->ch * cv->csize);
  if(cv->data != NULL) free(cv->data);
  cv->data = malloc(4 * cv->cw * cv->csize * cv->ch * cv->csize);
  memset(cv->data, 0, 4 * cv->cw * cv->csize * cv->ch * cv->csize);
  return;
}

void draw_coreview(COREVIEW* cv) {
  mlock(cv->l_mutex_exec);
  if(cv->l_coreviewdata == NULL) {
    SDL_Rect rect2;
    rect2.x = cv->px;
    rect2.y = cv->py;
    rect2.w = cv->pw;
    rect2.h = cv->ph;
    munlock(cv->l_mutex_exec);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
    SDL_RenderFillRect(renderer, &rect2);
    return;
  }
  int x, y, xx, yy; //Relying on the compiler for strength-reducing this loop
  unsigned long n = 0;
  for(y = 0; y < cv->ch; ++y) {
    for(x = 0; x < cv->cw; ++x) {
      CVCELL* cell = &(cv->l_coreviewdata[y*cv->cw + x]);
      for(yy = 0; yy < cv->csize; ++yy) {
        for(xx = 0; xx < cv->csize; ++xx) {
          if(cell_icons[cv->csize][cell->type][yy*cv->csize + xx] == '*') {
            cv->data[(y*cv->csize + yy)*cv->csize*cv->cw + x*cv->csize + xx] = warriors[cell->warrior].color;
          }
          else {
            if(cv->instr_colorize)
              cv->data[(y*cv->csize + yy)*cv->csize*cv->cw + x*cv->csize + xx] = op_color[cell->opcode];
            else
              cv->data[(y*cv->csize + yy)*cv->csize*cv->cw + x*cv->csize + xx] = 0x0;
          }
        }
      }
      ++n;
      if(n >= CORESIZE) goto _label_endloop_render;
    }
  }
  _label_endloop_render:
  munlock(cv->l_mutex_exec);

  SDL_UpdateTexture(cv->img_tex, NULL, cv->data, 4 * cv->cw * cv->csize);
  SDL_Rect rect;
  rect.x = cv->px;
  rect.y = cv->py;
  rect.w = cv->pw;
  rect.h = cv->ph;
  //SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, cv->img_tex, NULL, &rect);
  return;
}

COREVIEW* new_coreview() {
  COREVIEW* r = malloc(sizeof(COREVIEW));
  r->local_core = NULL;
  r->img_tex = NULL;
  r->data = NULL;
  r->tgmode = TG_CSIZE;
  r->tg_csize = 4;
  r->instr_colorize = 1;
  return r;
}

void set_coreview_pos(COREVIEW* cv, unsigned int x, unsigned int y) {
  cv->px = x;
  cv->py = y;
  return;
}

void set_coreview_core(COREVIEW* cv, LOCAL_CORE* lc) {
  cv->local_core = lc;
  update_coreview_metrics(cv);
  return;
}

void set_coreview_target(COREVIEW* cv, unsigned int tgm, ...) {
  va_list args;
  va_start(args, tgm);
  cv->tgmode = tgm;
  if(tgm & TG_PW_AM) cv->tg_pw_am = va_arg(args, unsigned int);
  if(tgm & TG_PW_AL) cv->tg_pw_al = va_arg(args, unsigned int);
  if(tgm & TG_PH_AM) cv->tg_ph_am = va_arg(args, unsigned int);
  if(tgm & TG_PH_AL) cv->tg_ph_al = va_arg(args, unsigned int);
  if(tgm & TG_CSIZE) cv->tg_csize = va_arg(args, unsigned int);
  va_end(args);
  update_coreview_metrics(cv);
  return;
}

void set_core_runmode(LOCAL_CORE* local_core, unsigned int rm, unsigned int rp) {
  mlock(l_mutex_mode);
  if(l_runmode == RUN_PAUSED) {
    if(rm == RUN_PAUSED) return;
    csignal(l_cond_exec); //unpause core
  }
  l_runmode = rm;
  l_runparam = rp;
  if(rm == RUN_FAST) l_rundata = 0;
  munlock(l_mutex_mode);
  return;
}

void set_coreview_runmode(COREVIEW* cv, unsigned int rm, unsigned int rp) {
  set_core_runmode(cv->local_core, rm, rp);
  return;
}

void destroy_coreview(COREVIEW* cv) {
  if(cv->img_tex != NULL) SDL_DestroyTexture(cv->img_tex);
  if(cv->data != NULL) free(cv->data);
  free(cv);
  return;
}
#endif

void debug_println1(uint64_t i) {
  INSTR1 I;
  I._I = i;
  char* s_op = NULL;
  switch(I._O) {
    #ifdef O_DAT
    case O_DAT: s_op = "DAT"; break;
    #endif
    #ifdef O_MOV
    case O_MOV: s_op = "MOV"; break;
    #endif
    #ifdef O_ADD
    case O_ADD: s_op = "ADD"; break;
    #endif
    #ifdef O_SUB
    case O_SUB: s_op = "SUB"; break;
    #endif
    #ifdef O_JMP
    case O_JMP: s_op = "JMP"; break;
    #endif
    #ifdef O_JMZ
    case O_JMZ: s_op = "JMZ"; break;
    #endif
    #ifdef O_JMN
    case O_JMN: s_op = "JMN"; break;
    #endif
    #ifdef O_DJN
    case O_DJN: s_op = "DJN"; break;
    #endif
    #ifdef O_SPL
    case O_SPL: s_op = "SPL"; break;
    #endif
    #ifdef O_SLT
    case O_SLT: s_op = "SLT"; break;
    #endif
    #ifdef O_MUL
    case O_MUL: s_op = "MUL"; break;
    #endif
    #ifdef O_DIV
    case O_DIV: s_op = "DIV"; break;
    #endif
    #ifdef O_MOD
    case O_MOD: s_op = "MOD"; break;
    #endif
    #ifdef O_SEQ
    case O_SEQ: s_op = "SEQ"; break;
    #else
    #ifdef O_CMP
    case O_CMP: s_op = "CMP"; break;
    #endif
    #endif
    #ifdef O_SNE
    case O_SNE: s_op = "SNE"; break;
    #endif
    #ifdef O_NOP
    case O_NOP: s_op = "NOP"; break;
    #endif
    #ifdef O_LDP
    case O_LDP: s_op = "LDP"; break;
    #endif
    #ifdef O_STP
    case O_STP: s_op = "STP"; break;
    #endif
    #ifdef O_DJZ
    case O_DJZ: s_op = "DJZ"; break;
    #endif
    #ifdef O_PCT
    case O_PCT: s_op = "PCT"; break;
    #endif
    #ifdef O_XCH
    case O_XCH: s_op = "XCH"; break;
    #endif
    #ifdef O_STS
    case O_STS: s_op = "STS"; break;
    #endif
  }
  char* s_mod = NULL;
  switch(I._M) {
    case M_A: s_mod = "A"; break;
    case M_B: s_mod = "B"; break;
    case M_AB: s_mod = "AB"; break;
    case M_BA: s_mod = "BA"; break;
    case M_F: s_mod = "F"; break;
    case M_X: s_mod = "X"; break;
    case M_I: s_mod = "I"; break;
  }
  char c_aA = ' ';
  switch(I._aA) {
    #ifdef A_IMM
    case A_IMM: c_aA = '#'; break;
    #endif
    #ifdef A_DIR
    case A_DIR: c_aA = '$'; break;
    #endif
    #ifdef A_INA
    case A_INA: c_aA = '*'; break;
    #endif
    #ifdef A_INB
    case A_INB: c_aA = '@'; break;
    #endif
    #ifdef A_PDA
    case A_PDA: c_aA = '{'; break;
    #endif
    #ifdef A_PDB
    case A_PDB: c_aA = '<'; break;
    #else
    #ifdef A_ADB
    case A_ADB: c_aA = '<'; break;
    #endif
    #endif
    #ifdef A_PIA
    case A_PIA: c_aA = '}'; break;
    #endif
    #ifdef A_PIB
    case A_PIB: c_aA = '>'; break;
    #endif
  }
  char c_aB = ' ';
  switch(I._aB) {
    #ifdef A_IMM
    case A_IMM: c_aB = '#'; break;
    #endif
    #ifdef A_DIR
    case A_DIR: c_aB = '$'; break;
    #endif
    #ifdef A_INA
    case A_INA: c_aB = '*'; break;
    #endif
    #ifdef A_INB
    case A_INB: c_aB = '@'; break;
    #endif
    #ifdef A_PDA
    case A_PDA: c_aB = '{'; break;
    #endif
    #ifdef A_PDB
    case A_PDB: c_aB = '<'; break;
    #else
    #ifdef A_ADB
    case A_ADB: c_aA = '<'; break;
    #endif
    #endif
    #ifdef A_PIA
    case A_PIA: c_aB = '}'; break;
    #endif
    #ifdef A_PIB
    case A_PIB: c_aB = '>'; break;
    #endif
  }
  printf("%s.%s\t%c%d,\t%c%d\n", s_op, s_mod, c_aA, I._A, c_aB, I._B);
  return;
}

void debug_println2(INSTR2 I) {
  printf("%-*p %d,\t%d\n", (int) sizeof(void*) * 2, I.fn, I.a, I.b);
  return;
}
