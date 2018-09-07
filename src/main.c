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

unsigned int ROUNDS = 1;
unsigned int WARRIORS = 0;

void error(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  char str[strlen(msg) + 14];
  sprintf(str, "Fatal Error: %s\n", msg);
  vprintf(str, args);
  va_end(args);
  exit(0);
}

void _Noreturn display_help_exit() {
  puts(
    "hMARS  v0.3-pre, Copyright (C) 2018  Aritz Erkiaga\n"
    "Usage:\n"
    #ifdef _COREVIEW_
    "hmars-gui [options] file1 [-l loadfile] file2 ...\n"
    #else
    "hmars [options] file1 [-l loadfile] file2 ...\n"
    #endif
    "\n"
    "Options:\n"
    "  -r num   Number of rounds to fight [1]\n"
    "  -l file  Output loadfile, must come after warrior file\n"
    "  -V       Increase verbosity one level, up to two\n"
    "\n"
    "Example:\n"
    #ifdef _COREVIEW_
    "hmars-gui -r 200 -V -V some.red -l out.red other.red\n"
    #else
    "hmars -r 200 -V -V some.red -l out.red other.red\n"
    #endif
  );
  exit(0);
}

int opt_VERBOSE = 0;

int main(int argc, char* argv[]) {
  #if __STDC_VERSION__ >= 201112L
  _Static_assert(sizeof(INSTR1) == 8, "Compiler didn't align fields as expected.");
  #else
  if(sizeof(INSTR1) != 8) error("Compiler didn't align fields as expected.");
  #endif

  int c;
  char** redfn = NULL;
  char** lfn = NULL;
  int redfnc = 0;
  for(c = 1; c < argc; ++c) {
    if(argv[c][0] == '-') {
      if(argv[c][2] != '\0') error("Unknown option %s", argv[c]);
      switch(argv[c][1]) {
        case 'V':
          opt_VERBOSE += 1;
          break;
        case 'l':
          if(redfnc == 0) error("Option -l must be preceded by filename.");
          ++c;
          if(c == argc) error("Option -l must be followed by filename.");
          lfn[redfnc-1] = argv[c];
          break;
        case 'r':
          ++c;
          if(c == argc) error("Option -r must be followed by number of rounds.");
          sscanf(argv[c], "%d", &ROUNDS);
          break;
        default:
          error("Unknown option %s", argv[c]);
      }
    }
    else {
      ++redfnc;
      redfn = realloc(redfn, redfnc * sizeof(char*));
      lfn = realloc(lfn, redfnc * sizeof(char*));
      redfn[redfnc-1] = argv[c];
      lfn[redfnc-1] = NULL;
    }
  }
  WARRIORS = redfnc;

  if(!WARRIORS) display_help_exit();

  initialize();
  if(parse_load(redfn, lfn, argv[0])) {
    free(redfn);
    free(lfn);
    finalize();
    return 1;
  }
  free(redfn);
  free(lfn);

  if(opt_VERBOSE) puts("Running...");

  #ifdef _COREVIEW_
  LOCAL_CORE* local_core = battle1_async(ROUNDS);
  COREVIEW* cv = new_coreview();
  set_coreview_pos(cv, 0, 0);
  set_coreview_core(cv, local_core);
  set_coreview_target(cv, TG_PW_AM | TG_PW_AL | TG_PH_AM | TG_PH_AL | TG_CSIZE,
                          500,       100,       400,       100,       4);

  int quit = 0;
  SDL_Event e;
  while(!quit) {
    //draw window
    SDL_SetRenderDrawColor(renderer, 0x8F, 0x8F, 0x8F, 0xFF);
    SDL_RenderClear(renderer);
    draw_coreview(cv);
    SDL_RenderPresent(renderer);
    //process events
    while(SDL_PollEvent(&e)) {
      switch(e.type) {
        case SDL_QUIT:
          signal_terminate(); //signal thread to stop immediately
          quit = 1; //go on to join thread
          break;
        case SDL_WINDOWEVENT:
          switch(e.window.event) {
            case SDL_WINDOWEVENT_RESIZED: {
              int width = e.window.data1;
              int height = e.window.data2;
              set_coreview_target(cv, TG_PW_AM | TG_PW_AL | TG_PH_AM | TG_PH_AL,
                                      width,     width/2,   height,    height/2);
              break; }
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          puts("Received");
          set_core_runmode(cv, RUN_CLOCK, 1);
          break;
        default:
          break;
      }
    }
  }
  wait_for_core(local_core);
  free(local_core);

  #else
  battle1_single(ROUNDS);
  #endif

  puts("Result:\n Wins  Ties Loses Score  Name and author");
  for(c = 0; c < WARRIORS; ++c)
    printf("%5d %5d %5d %5d  %s by %s\n",
      warriors[c].wins, warriors[c].ties, warriors[c].losses, warriors[c].score, warriors[c].name, warriors[c].author);

  unload_all();
  finalize();
  return 0;
}
