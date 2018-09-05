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

#include <ctype.h>

typedef struct tTEXTHIST{
  int type;
  int data;
} TEXTHIST;

#define THIST_NONE 0 //special value
#define THIST_ORGL 1 //priginal line number in redcode file (1-based)
#define THIST_SSUB 2 //start history of substituted text
#define THIST_ESUB 3 //end history of substituted text
#define THIST_PSUB 4 //substituted predefined constant
/**Any history begins with ORGL
 **SSUB also represents ORGL of appended history
 **ESUB and PSUB data are ignored, should be 0*/

typedef struct tLINE {
  struct tLINE* prev;
  struct tLINE* next;
  char* data; //null-terminated
  unsigned int len; //not including null
  int nhist; //number of history entries in array
  TEXTHIST* hist; //history of line processing, NULL if none
} LINE;

extern LINE* file2text(FILE*);
extern void text2file(LINE*, FILE*);
extern LINE* string2text(const char*);
extern LINE* format2text(const char* format, ...);
extern void append_line_history(LINE*, LINE*, int);
extern void text_substitute_label(LINE*, int, int, LINE*);
extern LINE* parse(LINE*, WARRIOR*);
extern void freetext(LINE*);
