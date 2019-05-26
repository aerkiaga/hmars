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
#include <signal.h>

#define PATH_TEST_TEST "./test/test.red"
#define PATH_TEST_DUMMY "./test/dummy.red"
#define PATH_TEST_PMARS "./pmars"
#define PATH_TEST_IN "./__in__"
#define PATH_TEST_OUT "./__out__"
#define CHECK_CMD "2>__out__"
#define STDOUT_CMD ">__out__"

unsigned int ROUNDS = 1;
unsigned int WARRIORS = 0;
unsigned int CORESIZE = 8000;
unsigned int MAXCYCLES = 80000;
unsigned int MAXPROCESSES = 8000;
unsigned int MAXLENGTH = 100;
unsigned int MINDISTANCE = 100;
#ifdef PSPACE
unsigned int PSPACESIZE = 500;
#endif
int start_order_random = 1;

void _Noreturn error(const char* msg, ...) {
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
    "hmars-gui [options] (<file> [-l <loadfile>]) ...\n"
    "hmars-gui --test\n"
    #else
    "hmars [options] (<file1> [-l <loadfile1>]) ...\n"
    "hmars --test\n"
    #endif
    "\n"
    "Options:\n"
    "  -r <rounds>  Number of rounds to fight [1]\n"
    "  -s <size>    Size of core in instructions [8000]\n"
    "  -c <cycles>  Maximum number of cycles per round [80000]\n"
    "  -p <procs>   Maximum number of processes per warrior [8000]\n"
    "  -l <length>  Maximum warrior length [100]\n"
    "  -d <dist>    Minimum distance between warriors [100]\n"
    #ifdef PSPACE
    "  -S <size>    Size of P-space in cells [CORESIZE/16]\n"
    #endif
    "  -L <file>    Output loadfile, must come after warrior file\n"
    "  -V           Increase verbosity one level, up to two\n"
    "  --test       Perform self test, ignores other arguments\n"
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

void print_offending_code() {
  puts("Loaded code of the offending warrior:");
  int c;
  for(c = 0; c < warriors[0].len; ++c) {
    if(warriors[0].code2 != NULL) {
      printf("%-*ld ", (int) sizeof(jitind_t) * 2, warriors[0].code2[c].in);
    }
    debug_println1(warriors[0].code1[c]._I, stdout);
  }
  return;
}

void output_disassembled_code(FILE* stream, WARRIOR* w) {
  fprintf(stream, ";redcode-94\n;name %s\n;author %s\n;assert 1\n       ORG      START\n", w->name, w->author);
  int c;
  for(c = 0; c < w->len; ++c) {
    if(c == w->org) fprintf(stream, "START  ");
    else fprintf(stream, "       ");
    debug_println1(w->code1[c]._I, stream);
  }
  fputs("END\n", stream);
  return;
}

int test_crash_loaded = 0; //whether the offending warrior has been loaded
void test_crash_handler(int sig) {
  printf("Test code crashed: ");
  switch(sig) {
    case SIGSEGV: puts("SIGSEGV"); break;
    case SIGILL: puts("SIGILL"); break;
    case SIGFPE: puts("SIGFPE"); break;
  }
  if(test_crash_loaded) print_offending_code();
  else puts("No warrior was loaded.");
  puts("");
  puts("FAILED!");
  raise(SIGABRT); //abort
  return;
}

int self_test() {
  puts("Performing self test...");
  /*TEST 1*/
  puts("TEST 1: classic vs JIT");
  initialize();
  set_nwarriors(1);
  FILE* red = fopen(PATH_TEST_TEST, "rt");
  if(red == NULL) {
    error("Could not open %s.", PATH_TEST_TEST);
  }
  LINE* redt = file2text(red);
  fclose(red);
  LINE* loadt = parse(redt, &warriors[0]);
  freetext(redt);
  if(loadt == NULL) {
    free(warriors);
    return -1;
  }
  signal(SIGSEGV, test_crash_handler);
  signal(SIGILL, test_crash_handler);
  signal(SIGFPE, test_crash_handler);

  init_warrior(&warriors[0]);

  int n;
  int wins = 0;
  for(n = 0; n < 10000; ++n) {
    if(n % 1000 == 0) printf("Round %d (%f%% wins)\n", n, 100.0*wins/(n?:1)); //D
    load1(&warriors[0], loadt);
    test_crash_loaded = 1;
    add_hdat();
    load2(&warriors[0], loadt);

    int w;
    reset_warrior(&warriors[0]);
    battle1_single(1);
    w = warriors[0].wins;
    reset_warrior(&warriors[0]);
    battle2_single(1);
    if(w != warriors[0].wins) {
      puts("The two simulators produced different results.");
      if(w) puts("Classic wins, JIT loses.");
      else puts("JIT wins, classic loses.");
      print_offending_code();

      free(warriors);
      puts("");
      puts("FAILED!");
      return 1;
    }
    jit_clear(); //get rid of all compiled code and translation data
    wins += w;
    test_crash_loaded = 0;
    free(warriors[0].code1);
    warriors[0].code1 = NULL;
    free(warriors[0].code2);
    warriors[0].code2 = NULL;
  }

  printf("wins: %d / 10000\n", wins);
  puts("PASSED!");

  /*TEST 2*/
  puts("TEST 2: classic vs pMARS");
  for(;;) { //dummy loop, acts like 'try' block
    if(!system(NULL)) {
      puts("There is no usable command processor.");
      break;
    }
    char str[200];
    snprintf(str, 200, "%s %s", PATH_TEST_PMARS, CHECK_CMD);
    system(str);
    FILE* out = fopen(PATH_TEST_OUT, "rt");
    int res = fscanf(out, "pMARS %99s", str);
    fclose(out);
    if(res < 1) {
      remove(PATH_TEST_OUT);
      puts("There is no pMARS in the current directory.");
      break;
    }
    *(strchr(str, ',') ?: &str[199]) = '\0';
    printf("Found pMARS %s\n", str);

    set_nwarriors(2);
    init_warrior(&warriors[1]);
    red = fopen(PATH_TEST_DUMMY, "rt");
    if(red == NULL) {
      error("Could not open %s.", PATH_TEST_DUMMY);
    }
    redt = file2text(red);
    fclose(red);
    LINE* loadt2 = parse(redt, &warriors[1]);
    freetext(redt);
    load1(&warriors[1], loadt2);

    start_order_random = 0; //it's not random in pMARS
    wins = 0;
    for(n = 0; n < 10000; ++n) {
      if(n % 1000 == 0) printf("Round %d (%f%% wins)\n", n, 100.0*wins/(n?:1)); //D
      load1(&warriors[0], loadt);
      test_crash_loaded = 1;

      int w;
      reset_warrior(&warriors[0]);
      battle1_single(1);
      w = warriors[0].ties || warriors[0].wins;

      FILE* in = fopen(PATH_TEST_IN, "wt");
      output_disassembled_code(in, &warriors[0]);
      fclose(in);
      snprintf(str, 200, "%s -r 1 -s %d -c %d -p %d -l %d -d %d -S %d -F %ld -b -k %s %s %s",
        PATH_TEST_PMARS, CORESIZE, MAXCYCLES, MAXPROCESSES, MAXLENGTH, MINDISTANCE, PSPACESIZE, warriors[1].pos, PATH_TEST_IN, PATH_TEST_DUMMY, STDOUT_CMD);
      system(str);

      out = fopen(PATH_TEST_OUT, "rt");
      char ch1, ch2;
      res = fscanf(out, "%c %c", &ch1, &ch2);
      fclose(out);

      if(res < 1) {
        puts("Could not parse pMARS output.");
        free(warriors);
        puts("");
        puts("FAILED!");
        return 1;
      }

      if(w != (ch1 - '0' || ch2 - '0')) {
        puts("The two simulators produced different results.");
        if(w) puts("Classic wins, pMARS loses.");
        else puts("pMARS wins, classic loses.");
        printf("Dummy warrior placed at %ld\n", warriors[1].pos);
        print_offending_code();

        free(warriors);
        puts("");
        puts("FAILED!");
        return 1;
      }

      wins += w;
      test_crash_loaded = 0;
      free(warriors[0].code1);
      warriors[0].code1 = NULL;
    }
    remove(PATH_TEST_IN);
    remove(PATH_TEST_OUT);
    start_order_random = 1;
    break;
  }

  printf("wins: %d / 10000\n", wins);
  puts("PASSED!");

  #ifdef PSPACE
  free(warriors[0].pspace);
  warriors[0].pspace = NULL;
  free(warriors[1].pspace);
  warriors[1].pspace = NULL;
  #endif
  mdestroy(warriors[0].mutex);
  mdestroy(warriors[1].mutex);

  unload_all();
  return 0;
}

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
      if((argv[c][2] != '\0') && (argv[c][1] != '-')) error("Unknown option %s", argv[c]);
      switch(argv[c][1]) {
        case 'V':
          opt_VERBOSE += 1;
          break;
        case 'L':
          if(redfnc == 0) error("Option -L must be preceded by filename.");
          ++c;
          if(c == argc) error("Option -L must be followed by filename.");
          lfn[redfnc-1] = argv[c];
          break;
        case 'r':
          ++c;
          if(c == argc) error("Option -r must be followed by a number.");
          sscanf(argv[c], "%d", &ROUNDS);
          break;
        case 's':
          ++c;
          if(c == argc) error("Option -s must be followed by a number.");
          sscanf(argv[c], "%d", &CORESIZE);
          break;
        case 'c':
          ++c;
          if(c == argc) error("Option -c must be followed by a number.");
          sscanf(argv[c], "%d", &MAXCYCLES);
          break;
        case 'p':
          ++c;
          if(c == argc) error("Option -p must be followed by a number.");
          sscanf(argv[c], "%d", &MAXPROCESSES);
          break;
        case 'l':
          ++c;
          if(c == argc) error("Option -l must be followed by a number.");
          sscanf(argv[c], "%d", &MAXLENGTH);
          break;
        case 'd':
          ++c;
          if(c == argc) error("Option -d must be followed by a number.");
          sscanf(argv[c], "%d", &MINDISTANCE);
          break;
        #ifdef PSPACE
        case 'S':
          ++c;
          if(c == argc) error("Option -S must be followed by a number.");
          sscanf(argv[c], "%d", &PSPACESIZE);
          break;
        #endif
        case '-':
          if(!strcmp(&(argv[c][2]), "test")) {
            ++c;
            return self_test();
          }
          else {
            error("Unknown option %s", argv[c]);
          }
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

  if(CORESIZE <= 2) error("CORESIZE must be at least 3");
  else if(CORESIZE > 32768) error("CORESIZE must be less than or equal to 32768");
  #ifdef PSPACE
    if(CORESIZE % PSPACESIZE) puts("Warning: PSPACESIZE should be a factor of CORESIZE");
  #endif
  if(CORESIZE < MINDISTANCE*2) puts("CORESIZE should be at least equal to MINDISTANCE*2");

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
  init_coreview();
  LOCAL_CORE* local_core = battle1_async(ROUNDS);
  COREVIEW* cv = new_coreview();
  set_coreview_pos(cv, 0, 0);
  set_coreview_core(cv, local_core);
  set_coreview_target(cv, TG_PW_AM | TG_PW_AL | TG_PH_AM | TG_PH_AL | TG_CSIZE,
                          500,       100,       400,       100,       4);

  int quit = 0;
  SDL_Event e;
  set_coreview_runmode(cv, RUN_CLOCK, 1);
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
          /*puts("Received");
          set_core_runmode(cv, RUN_CLOCK, 1);*/
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
  /*puts("Result:\n Wins  Ties Loses Score  Name and author");
  for(c = 0; c < WARRIORS; ++c)
    printf("%5d %5d %5d %5d  %s by %s\n",
      warriors[c].wins, warriors[c].ties, warriors[c].losses, warriors[c].score, warriors[c].name, warriors[c].author);

  warriors[0].wins = warriors[0].losses = 0;
  memset(warriors[0].pspace, 0, PSPACESIZE * sizeof(pcell_t));
  warriors[0].psp0 = CORESIZE - 1;
  battle2_single(ROUNDS);*/ //D
  #endif

  puts("Result:\n Wins  Ties Loses Score  Name and author");
  for(c = 0; c < WARRIORS; ++c)
    printf("%5d %5d %5d %5d  %s by %s\n",
      warriors[c].wins, warriors[c].ties, warriors[c].losses, warriors[c].score, warriors[c].name, warriors[c].author);

  unload_all();
  finalize();
  return 0;
}
