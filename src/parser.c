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
#include <stdarg.h>

struct tPARSOPT {
  int standard; ///< Current standard according to parser settings, or 0 if custom
  int always_use_default; ///< Don't set standard according to ;redcode line
  int default_standard; ///< Default standard for ;redcode without a tag
  int allow_slash_comments; ///< Comments can start with '/' as well as ';'
  int must_start_redcode; ///< File starts at ;redcode, ignore everything before
  int do_first_pass; ///< Perform first pass
  int do_second_pass; ///< Perform second pass
  int allow_pseudo_org; ///< Allow ORG
  int allow_pseudo_equ; ///< Allow EQU
  int allow_pseudo_for; ///< Allow FOR
  int allow_pseudo_pin; ///< Allow PIN
  int allow_pseudo_end; ///< Allow END
  int allow_end_operand; ///< END [label]
  int allow_pseudo_space; ///< Allow SPACE
  int allow_multiline_equ; ///< Allow multiline EQUs
  int predef_constants; ///< Allow predefined constants
  int allow_variables; ///< Allow variables in expressions
  int opers_whitespace; ///< Allow whitespace in expressions
  int opers_plusminus; ///< Operators + and - (unary)
  int opers_addsub; ///< Operators + and - (binary)
  int opers_muldiv; ///< Operators * and /
  int opers_parentheses; ///< Parentheses in expressions
  int opers_modulo; ///< Operand %
  int opers_logical; ///< Operands ! , && and ||
  int opers_comparison; ///< Operands == , != , < , > , <= and >=
  //int opers_asterisk; // ICWS '86 asterisk nullary operand (current line)
  int allow_concatenate; ///< Allow concatenation using the & operator
  int labels_maxlength; ///< Max length of labels, 0 means no limit
  int labels_case_sens; ///< Labels are case-sensitive
  int assert_give_error; ///< Whether ;assert evaluating to 0 should give an error or just a warning
  int allow_modifiers; ///< Allow instruction modifiers (.xx)
  int def_modif_rules; ///< Rules for default modifier if omitted
  int addr_expl_direct; ///< Allow explicit '$' for direct address mode
  int addr_immediate; ///< Immediate, '#'
  int addr_bindirect; ///< B-field indirect, '@'
  int addr_bdecrement; ///< B-field auto-/predecrement, '<'
  int addr_bincrement; ///< B-field postincrement, '>'
  int addr_aindirect; ///< A-field indirect, '*'
  int addr_adecrement; ///< A-field predecrement, '{'
  int addr_aincrement; ///< A-field postincrement, '}'
  int allow_all_addrs; ///< Allow all address modes for all opcodes
  int out_fields_neg; ///< Use negative fields in load file if magnitude is lower
  int field_count_rules; ///< 0 = two on all, 1 = exact number for opcode, 2 = can omit one on some, 3 = can omit one on all
  int field_separator; ///< bit 0 = comma, bit 1 = whitespace
  int exp_funclike_equ; ///< Allow function-like macros (non-standard)
  int exp_label_arith; ///< Allow label arithmetic inside FOR and ;assert (see compute_expression())
  int exp_var_maxlength; ///< Max length of variable names, 0 means no limit
  int exp_for_preprocess; ///< Expand non-concatenated constants in loops before expanding the loop
  int exp_org_expression; ///< Allow an expression after ORG and END
  int exp_pin_expression; ///< Allow an expression (possibly with forward references) after PIN
  int exp_assert_forward; ///< Allow forward references in ;assert expressions
  int exp_allow_omit_both; ///< Allow omitting both fields and loading #0 (independent of #field_count_rules)
} parsopt = {
  .standard = STANDARD,
  #if (STANDARD == 84)
  .allow_slash_comments = 1,
  .do_first_pass = 0,
  .do_second_pass = 0,
  .allow_pseudo_end = 0,
  .opers_plusminus = 0,
  .opers_addsub = 0,
  .addr_bdecrement = 0,
  .field_separator = 2,
  #else
  .allow_slash_comments = 0,
  .do_first_pass = 1,
  .do_second_pass = 1,
  .allow_pseudo_end = 1,
  .opers_plusminus = 1,
  .opers_addsub = 1,
  .addr_bdecrement = 1,
    #if (STANDARD == 94)
    .field_separator = 1,
    #else
    .field_separator = 3,
    #endif
  #endif

  #if (STANDARD <= 86)
  .must_start_redcode = 0,
  .allow_pseudo_equ = 0,
  .predef_constants = 0,
  .labels_maxlength = 8,
  .labels_case_sens = 0,
  .opers_muldiv = 0,
  .allow_end_operand = 0,
  .def_modif_rules = 86,
  .field_count_rules = 1,
  #else
  .must_start_redcode = 1,
  .allow_pseudo_equ = 1,
  .predef_constants = 1,
  .labels_maxlength = 0,
  .labels_case_sens = 1,
  .opers_muldiv = 1,
  .allow_end_operand = 1,
  .def_modif_rules = 88,
    #if (STANDARD == 94)
    .field_count_rules = 3,
    #else
    .field_count_rules = 2,
    #endif
  #endif

  #if (STANDARD <= 88)
  .allow_pseudo_org = 0,
  .opers_whitespace = 0,
  .opers_comparison = 0,
  .opers_parentheses = 0,
    #if (STANDARD == 86)
    .addr_expl_direct = 1,
    #else
    .addr_expl_direct = 0,
    #endif
  #else
  .allow_pseudo_org = 1,
  .opers_whitespace = 1,
  .opers_comparison = 1,
  .opers_parentheses = 1,
  .addr_expl_direct = 1,
  #endif

  #if (STANDARD < 94)
  .allow_pseudo_for = 0,
  .allow_pseudo_pin = 0,
  .allow_multiline_equ = 0,
  .allow_variables = 0,
  .opers_modulo = 0,
  .opers_logical = 0,
  .allow_concatenate = 0,
  .allow_modifiers = 0,
  .addr_bincrement = 0,
  .addr_aindirect = 0,
  .addr_adecrement = 0,
  .addr_aincrement = 0,
  #else
  .allow_pseudo_for = 1,
  .allow_pseudo_pin = 1,
  .allow_multiline_equ = 1,
  .allow_variables = 1,
  .opers_modulo = 1,
  .opers_logical = 1,
  .allow_concatenate = 1,
  .allow_modifiers = 1,
  .addr_bincrement = 1,
  .addr_aindirect = 1,
  .addr_adecrement = 1,
  .addr_aincrement = 1,
  #endif

  #if (STANDARD == 86)
  .allow_pseudo_space = 1,
  #else
  .allow_pseudo_space = 0,
  #endif

  #if (STANDARD == 88)
  .allow_all_addrs = 0,
  #else
  .allow_all_addrs = 1,
  #endif

  #ifdef PARSER_NONSTANDARD
  .exp_funclike_equ = 1,
  .exp_label_arith = 1,
  .exp_var_maxlength = 0,
  .exp_for_preprocess = 1,
  .exp_org_expression = 1,
  .exp_assert_forward = 1,
  .exp_pin_expression = 1,
  .exp_allow_omit_both = 1,
  #else
  .exp_funclike_equ = 0,
  .exp_label_arith = 0,
  .exp_var_maxlength = 1,
  .exp_for_preprocess = 0,
  .exp_org_expression = 0,
  .exp_assert_forward = 0,
  .exp_pin_expression = 0,
  .exp_allow_omit_both = 0,
  #endif

  .always_use_default = 0,
  .default_standard = 89,
  .assert_give_error = 0,
  .addr_immediate = 1,
  .addr_bindirect = 1,
  .out_fields_neg = 1,
};

/** \struct tPARSOPT
    \brief Contains all the configuration options for the parser.

    # Default values of members
    Depend on the selected value of #STANDARD at compile time:
    | Member                | 84 | 86 | 88 | 89 | 94 |
    | --------------------- | -- | -- | -- | -- | -- |
    | #standard             | 84 | 86 | 88 | 89 | 94 |
    | #allow_slash_comments |  1 |  0 |  0 |  0 |  0 |
    | #must_start_redcode   |  0 |  0 |  1 |  1 |  1 |
    | #always_use_default   |  0 |  0 |  0 |  0 |  0 |
    | #default_standard     | 89 | 89 | 89 | 89 | 89 |
    | #do_first_pass        |  0 |  1 |  1 |  1 |  1 |
    | #do_second_pass       |  0 |  1 |  1 |  1 |  1 |
    | #allow_pseudo_org     |  0 |  0 |  0 |  1 |  1 |
    | #allow_pseudo_equ     |  0 |  0 |  1 |  1 |  1 |
    | #allow_pseudo_for     |  0 |  0 |  0 |  0 |  1 |
    | #allow_pseudo_pin     |  0 |  0 |  0 |  0 |  1 |
    | #allow_pseudo_end     |  0 |  1 |  1 |  1 |  1 |
    | #allow_end_operand    |  0 |  0 |  1 |  1 |  1 |
    | #allow_pseudo_space   |  0 |  1 |  0 |  0 |  0 |
    | #allow_multiline_equ  |  0 |  0 |  0 |  0 |  1 |
    | #predef_constants     |  0 |  0 |  1 |  1 |  1 |
    | #allow_variables      |  0 |  0 |  0 |  0 |  1 |
    | #opers_whitespace     |  0 |  0 |  0 |  1 |  1 |
    | #opers_plusminus      |  0 |  1 |  1 |  1 |  1 |
    | #opers_addsub         |  0 |  1 |  1 |  1 |  1 |
    | #opers_muldiv         |  0 |  0 |  1 |  1 |  1 |
    | #opers_parentheses    |  0 |  0 |  0 |  1 |  1 |
    | #opers_modulo         |  0 |  0 |  0 |  0 |  1 |
    | #opers_logical        |  0 |  0 |  0 |  0 |  1 |
    | #opers_comparison     |  0 |  0 |  0 |  1 |  1 |
    | #allow_concatenate    |  0 |  0 |  0 |  0 |  1 |
    | #labels_maxlength     |  8 |  8 |  0 |  0 |  0 |
    | #labels_case_sens     |  0 |  0 |  1 |  1 |  1 |
    | #assert_give_error    |  0 |  0 |  0 |  0 |  0 |
    | #allow_modifiers      |  0 |  0 |  0 |  0 |  1 |
    | #def_modif_rules      | 86 | 86 | 88 | 88 | 88 |
    | #addr_expl_direct     |  0 |  1 |  0 |  1 |  1 |
    | #addr_immediate       |  1 |  1 |  1 |  1 |  1 |
    | #addr_bindirect       |  1 |  1 |  1 |  1 |  1 |
    | #addr_bdecrement      |  0 |  1 |  1 |  1 |  1 |
    | #addr_bincrement      |  0 |  0 |  0 |  0 |  1 |
    | #addr_aindirect       |  0 |  0 |  0 |  0 |  1 |
    | #addr_adecrement      |  0 |  0 |  0 |  0 |  1 |
    | #addr_aincrement      |  0 |  0 |  0 |  0 |  1 |
    | #allow_all_addrs      |  1 |  1 |  0 |  1 |  1 |
    | #out_fields_neg       |  1 |  1 |  1 |  1 |  1 |
    | #field_count_rules    |  1 |  1 |  2 |  2 |  3 |
    | #field_separator      |  2 |  3 |  3 |  1 |  1 |

    Non-standard features, depend on PARSER_NONSTANDARD:
    | Member               | Disabled | Enabled |
    | -------------------- | -------- | ------- |
    | #exp_funclike_equ    |        0 |       1 |
    | #exp_label_arith     |        0 |       1 |
    | #exp_var_maxlength   |        1 |       0 |
    | #exp_for_preprocess  |        0 |       1 |
    | #exp_org_expression  |        0 |       1 |
    | #exp_assert_forward  |        0 |       1 |
    | #exp_pin_expression  |        0 |       1 |
    | #exp_allow_omit_both |        0 |       1 |

    \sa STANDARD, parse()
*/

/** \var parsopt
    \brief Parser configuration options.

    Members are set at compile time to
    default values depending on the
    simulator's STANDARD setting. However,
    they can be set to any value at
    run-time. For information on the
    different members and their default
    values, see tPARSOPT.
    \sa struct tPARSOPT, set_standard_by_tag(), STANDARD
*/

typedef struct tDICTIONARY {
  struct {
    struct tDICTIONARY_ENTRY{
      char* restrict tag;   /**< The tag for this entry. */
      LINE* restrict value; /**< (LINE*) 1 means existing but no value. */
    }* ptr; ///< Pointer to an array of entries with the same hash.
    int length; ///< Number of entries.
  } hasht[64]; ///< Hash table. The hash is calculated like this:
               ///< `hash = (tag[0]&7)|((tag[strlen(tag)-1]&7)<<3);`
} DICTIONARY;

DICTIONARY d_constants = {{{NULL, 0}}};
DICTIONARY d_opcodes = {{{NULL, 0}}};
DICTIONARY d_pseudoops = {{{NULL, 0}}};
DICTIONARY d_signatures = {{{NULL, 0}}}; ///< For each function-like macro, the
                                         ///< comma-separated list of its parameters
DICTIONARY d_parameters = {{{NULL, 0}}}; ///< Updated with the values of the parameters to
                                         ///< the function-like macro currently being expanded
DICTIONARY d_variables = {{{NULL, 0}}}; ///< Its #LINE elements' tLINE.data members contain
                                        ///< pointers to #EXP_VAL structures.
DICTIONARY d_labels = {{{NULL, 0}}}; ///< Its #LINE elements' tLINE.data members contain
                                     ///< pointers to #EXP_VAL structures with .b = 0.
int CURLINE;
int allow_forward_refs; //1 during first pass, 0 on second pass
int org_is_set; //whether or not the start label has been defined
int pin_is_set; //whether or not the PIN number has been defined

//line types
#define LTYPE_OTHER 0
#define LTYPE_NORMAL 1
#define LTYPE_FOR 2
#define LTYPE_ASSERT 3
#define LTYPE_ROF 4
#define LTYPE_END 5
#define LTYPE_ORG 6
#define LTYPE_PIN 7

char* restrict restricted_copy(char* str) {
  /** \brief Makes a new copy of str, for preventing pointer aliasing.
      \param str [in] Null-terminated string to copy.
      \return A new copy of str

      This is because all dictionary tags (and values) must be restrict'ed.
      See tDICTIONARY.
   */
  char* restrict r = (char*) malloc(strlen(str)+1);
  strcpy(r, str);
  return r;
}

LINE* get_dictionary(DICTIONARY* dict, char* tag) {
  /** \brief Gets the value of dictionary entry identified by tag.
      \param dict [in] Pointer to the dictionary to retrieve the value from.
      \param tag [in] The tag identifying a dictionary entry.
      \return The value of the corresponding dictionary entry,
              or NULL if the entry doesn't exist.
              Note that (LINE*) 1 means by convention that
              the entry exists but contains no data.
   */
  //printf("(%p) getting \"%s\": ", dict, tag); //D
  int hash;
  hash = (tag[0]&7)|((tag[strlen(tag)-1]&7)<<3);
  int c; LINE* r = NULL;
  for(c = 0; c < dict->hasht[hash].length; ++c) {
    if(!strcmp(dict->hasht[hash].ptr[c].tag, tag)) {
      r = dict->hasht[hash].ptr[c].value;
      break;
    }
  }
  /*if(r < (LINE*) 2) printf("%p\n", r);
  else printf("\"%s\"\n", (char*) r->data);*/ //D
  return r;
}

void set_dictionary(DICTIONARY* dict, char* tag, LINE* restrict data) { //only call if the entry does not exist
  /** \brief Creates a new dictionary entry identified by tag.
      \param dict [out] Pointer to the dictionary to create the entry in.
      \param tag [in] The tag identifying the new dictionary entry.
      \param data [in] The data that the new dictionary entry will contain.

      Note that this function should only be called if an entry
      identified by tag doesn't already exist. Otherwise, the behavior
      is undefined (but will not produce any degree of memory corruption).
      Also note that the function will create a copy of \a tag, but not
      of \a data.
   */
  //printf("(%p) SETTING \"%s\": ", dict, tag); //D
  /*if(data < (LINE*) 2) printf("%p\n", data);
  else printf("\"%s\"\n", (char*) data->data);*/ //D
  int hash;
  hash = (tag[0]&7)|((tag[strlen(tag)-1]&7)<<3);
  dict->hasht[hash].length++;
  dict->hasht[hash].ptr = (struct tDICTIONARY_ENTRY*) realloc(dict->hasht[hash].ptr, dict->hasht[hash].length*sizeof(struct tDICTIONARY_ENTRY));
  dict->hasht[hash].ptr[dict->hasht[hash].length-1].tag = (char*) malloc(strlen(tag)+1);
  strcpy(dict->hasht[hash].ptr[dict->hasht[hash].length-1].tag, tag);
  dict->hasht[hash].ptr[dict->hasht[hash].length-1].value = data;
  return;
}

int update_dictionary(DICTIONARY* dict, char* tag, LINE* restrict new_data) {
  /** \brief Updates an already-existing dictionary entry identified by tag.
      \param dict [out] Pointer to the dictionary to modify.
      \param tag [in] The tag identifying a dictionary entry.
      \param new_data [in] The new data that the dictionary entry will contain.
      \return nonzero if the entry already exists, 0 if it is newly created.
      \sa set_dictionary()

      If the entry doesn't exist, it will be created instead.
      The old text will be freed via freetext().
   */
   int hash;
   hash = (tag[0]&7)|((tag[strlen(tag)-1]&7)<<3);
   int c; int chk = 0;
   for(c = 0; c < dict->hasht[hash].length; ++c) {
     if(!strcmp(dict->hasht[hash].ptr[c].tag, tag)) {
       freetext(dict->hasht[hash].ptr[c].value);
       dict->hasht[hash].ptr[c].value = new_data;
       chk = 1;
       break;
     }
   }
   if(!chk) {
     set_dictionary(dict, tag, new_data);
     return 0;
   }
   return 1;
}

void clear_dictionary(DICTIONARY* dict) {
  int c1;
  for(c1 = 0; c1 < 64; ++c1) {
    if(dict->hasht[c1].ptr != NULL) {
      int c2;
      for(c2 = 0; c2 < dict->hasht[c1].length; ++c2) {
        free(dict->hasht[c1].ptr[c2].tag);
        if(dict->hasht[c1].ptr[c2].value > (LINE*) 1) {
          freetext(dict->hasht[c1].ptr[c2].value);
        }
      }
      free(dict->hasht[c1].ptr);
      dict->hasht[c1].ptr = NULL;
      dict->hasht[c1].length = 0;
    }
  }
  return;
}

void parser_error(LINE* line, const char* format, ...) {
  va_list parms;
  va_start(parms, format);
  printf("Parser error: ");
  vprintf(format, parms);
  puts("");
  va_end(parms);
  if(line != NULL) {
    int c;
    for(c = 0; c < line->nhist; ++c) {
      switch(line->hist[c].type) {
        case THIST_ORGL:
          printf("\tat input line %d\n", line->hist[c].data);
          break;
      }
    }
  }
  return;
}

void parser_warn(LINE* line, const char* format, ...) {
  va_list parms;
  va_start(parms, format);
  printf("Parser warning: ");
  vprintf(format, parms);
  puts("");
  va_end(parms);
  if(line != NULL) {
    int c;
    for(c = 0; c < line->nhist; ++c) {
      switch(line->hist[c].type) {
        case THIST_ORGL:
          printf("\tat input line %d\n", line->hist[c].data);
          break;
      }
    }
  }
  return;
}

void parser_note(LINE* line, const char* format, ...) {
  va_list parms;
  va_start(parms, format);
  printf("Parser note: ");
  vprintf(format, parms);
  puts("");
  va_end(parms);
  if(line != NULL) {
    int c;
    for(c = 0; c < line->nhist; ++c) {
      switch(line->hist[c].type) {
        case THIST_ORGL:
          printf("\tat input line %d\n", line->hist[c].data);
          break;
      }
    }
  }
  return;
}

LINE* copyline(LINE* line) {
  LINE* r = (LINE*) malloc(sizeof(LINE));
  r->prev = NULL;
  r->next = NULL;
  if(line->data != NULL) {
    r->data = (char*) malloc(strlen(line->data)+1);
    strcpy(r->data, line->data);
  }
  else r->data = NULL;
  r->len = line->len;
  if(line->nhist) {
    r->hist = (TEXTHIST*) malloc(line->nhist * sizeof(TEXTHIST));
    int c;
    for(c = 0; c < line->nhist; ++c) {
      r->hist[c].type = line->hist[c].type;
      r->hist[c].data = line->hist[c].data;
    }
  }
  r->nhist = line->nhist;
  return r;
}
void linklines(LINE* first, LINE* second) {
  first->next = second;
  second->prev = first;
}
LINE* copytext(LINE* text) {
  LINE* r = (LINE*) malloc(sizeof(LINE));
  LINE* s = text; LINE* l = r;
  r->prev = NULL;
  for(;;) {
    if(s->len) {
      l->data = (char*) malloc(s->len+1);
      strcpy(l->data, s->data); //invalid read & write
    }
    else l->data = NULL;
    l->len = s->len;
    if(s->nhist) {
      l->hist = (TEXTHIST*) malloc(s->nhist * sizeof(TEXTHIST));
      int c;
      for(c = 0; c < s->nhist; ++c) {
        l->hist[c].type = s->hist[c].type;
        l->hist[c].data = s->hist[c].data;
      }
    }
    else l->hist = NULL;
    l->nhist = s->nhist;

    s = s->next;
    if(s == NULL) {
      l->next = NULL;
      break;
    }
    else {
      l->next = (LINE*) malloc(sizeof(LINE));
      l->next->prev = l;
      l = l->next;
    }
  }
  return r;
}
LINE* delline(LINE* line) {
  line->prev->next = line->next;
  if(line->next != NULL) line->next->prev = line->prev;
  LINE* r = line->prev;
  line->prev = line->next = NULL;
  freetext(line);
  return r;
}

#define FORLINEIN(text, line) for((line) = (text); (line) != NULL; (line) = (line)->next)

LINE* file2text(FILE* file) {
  fseek(file, 0, SEEK_SET);
  LINE* r;
  LINE* rc = NULL;
  int lc, iseof = 0;
  for(lc = 1; !iseof; ++lc) {
    long pos = ftell(file);
    int l, crlf = 0;
    for(l = 0;; ++l) {
      int ch = fgetc(file);
      if(ch == EOF) iseof = 1;
      if(ch == '\r') { //CRLF
        ch = fgetc(file);
        if(ch == '\n') crlf = 1;
        break;
      }
      if(ch == '\n' || ch == '\v' || ch == '\f' || ch == EOF) break;
    }
    LINE* rp = rc;
    rc = (LINE*) malloc(sizeof(LINE));
    rc->data = (char*) malloc(l+1);
    rc->len = l;
    if(rp != NULL) {
      rp->next = rc;
      rc->prev = rp;
    }
    else {
      rc->prev = NULL;
      r = rc;
    }
    fseek(file, pos, SEEK_SET);
    fgets(rc->data, l+1, file);
    if(crlf) fgetc(file); //skip CR in CRLF
    fgetc(file); //advance to next line
    rc->nhist = 1;
    rc->hist = malloc(sizeof(TEXTHIST));
    rc->hist->type = THIST_ORGL;
    rc->hist->data = lc;
  }
  rc->next = NULL;
  return r;
}

void text2file(LINE* text, FILE* file) {
  while(text != NULL) {
    fputs(text->data ?: "", file);
    fputc('\n', file);
    text = text->next;
  }
  return;
}

LINE* string2text(const char* str) {
  LINE* r = (LINE*) malloc(sizeof(LINE));
  r->prev = NULL;
  LINE* l = r;
  int c;
  int start = 0; int end;
  for(c = 0; ; ++c) {
    if(str[c] == '\n' || str[c] == '\0') {
      end = c;
      if(end > start) {
        l->data = (char*) malloc(end-start+1);
        strncpy(l->data, &str[start], end-start);
        l->data[end-start] = '\0';
      }
      else {
        l->data = NULL;
      }
      l->len = end-start;
      l->nhist = 0;
      l->hist = NULL;
      if(str[c] == '\0') {
        l->next = NULL;
        break;
      }
      else {
        l->next = (LINE*) malloc(sizeof(LINE));
        l->next->prev = l;
        l = l->next;
        start = c + 1;
      }
    }
  }
  return r;
}

LINE* format2text(const char* format, ...) {
  va_list args, args2;
  va_start(args, format);
  va_copy(args2, args);
  int len = vsnprintf(NULL, 0, format, args);
  char* str = (char*) malloc(len+1);
  vsprintf(str, format, args2);
  va_end(args);
  va_end(args2);
  LINE* r = string2text(str);
  free(str);
  return r;
}

void append_line_history(LINE* dst, LINE* src, int extra) {
  int len = src->nhist;
  int len2 = 0;
  switch(extra) {
    case THIST_NONE: len2 = len; break;
    case THIST_SSUB: len2 = len + 1; break;
    default:
      error("Unknown 'extra' parameter in append_line_history.");
      HINT_UNREACHABLE();
  }
  len2 += dst->nhist;
  dst->hist = (TEXTHIST*) realloc(dst->hist, len2*sizeof(TEXTHIST));
  if(len > 0) memcpy(&dst->hist[dst->nhist], src->hist, (len)*sizeof(TEXTHIST));
  switch(extra) {
    case THIST_NONE: break;
    case THIST_SSUB:
      if(len > 0) {
        dst->hist[dst->nhist].type = THIST_SSUB;
        dst->hist[dst->nhist+len].type = THIST_ESUB;
        dst->hist[dst->nhist+len].data = 0;
      }
      else {
        dst->hist[dst->nhist].type = THIST_PSUB;
        dst->hist[dst->nhist].data = 0;
      }
      break;
  }
  dst->nhist = len2;
  return;
}

void text_substitute_label(LINE* line, int labstart, int labend, LINE* subs) {
  if(subs->next == NULL) { //single-line constant
    int len = labstart + line->len - labend + subs->len;
    int afterlen = line->len - labend;
    if(len > line->len) line->data = (char*) realloc(line->data, len+1);
    if(afterlen) memmove(&line->data[len-afterlen], &line->data[labend], afterlen);
    if(len < line->len) line->data = (char*) realloc(line->data, len+1);
    if(subs->len) strncpy(&line->data[labstart], subs->data, subs->len);
    line->data[len] = '\0';
    line->len = len;
  }
  else { //multi-line constant
    int afterlen = line->len - labend;
    char* after = (char*) malloc(afterlen+1);
    strcpy(after, &line->data[labend]);
    line->len = labstart + subs->len;
    line->data = (char*) realloc(line->data, line->len+1);
    if(subs->len) strcpy(&line->data[labstart], subs->data);
    append_line_history(line, subs, THIST_SSUB);
    LINE* l = line; LINE* s = subs;
    while(s->next->next != NULL) { //intermediate lines
      s = s->next;
      LINE* tmp = (LINE*) malloc(sizeof(LINE));
      tmp->prev = l;
      tmp->next = l->next;
      l->next->prev = tmp;
      l->next = tmp;
      tmp->len = s->len;
      tmp->data = (char*) malloc(tmp->len+1);
      if(s->len) strcpy(tmp->data, s->data);
      tmp->nhist = 0;
      tmp->hist = NULL;
      append_line_history(tmp, l, THIST_NONE);
      append_line_history(tmp, s, THIST_SSUB);
      l = tmp;
    }
    s = s->next;
    LINE* last = (LINE*) malloc(sizeof(LINE));
    last->prev = l;
    last->next = l->next;
    l->next->prev = last;
    l->next = last;
    last->len = s->len + afterlen;
    if(last->len) {
      last->data = (char*) malloc(last->len+1);
      strcpy(last->data, s->data);
      strcpy(&last->data[s->len], after);
    }
    else last->data = NULL;
    free(after);
  }
  return;
}

void text_substitute_from_dictionary(LINE* text, DICTIONARY* dict) {
  /** \brief Substitute all identifiers within *text* with values from *dict*.
      \param text [in, out] The text on which to perform the operation.
      \param dict [in] Pointer to a dictionary containing all substitution rules.

      For every valid identifier in *text* (every sequence of contiguous
      alphanumeric characters), this function will search it in *dict*,
      and if it gets a corresponding value, it will substitute the
      identifier with that value. No further substitutions will be
      performed in the new text, and thus no recursion is possible.
   */
  LINE* line;
  FORLINEIN(text, line) {
    int c; int labstart = -1; int labend;
    for(c = 0; c <= line->len; ++c) {
      if(isalnum(line->data[c])) {
        if(labstart < 0) labstart = c;
      }
      else {
        if(labstart >= 0) {
          labend = c;
          char* label = (char*) malloc(labend-labstart+1);
          strncpy(label, &line->data[labstart], labend-labstart);
          label[labend-labstart] = '\0';
          LINE* subs = get_dictionary(dict, label);
          free(label);
          if(subs) text_substitute_label(line, labstart, labend, subs);
          labstart = -1;
        }
      }
    }
  }
  return;
}

void freetext(LINE* text) {
  while(text != NULL) {
    LINE* t2 = text;
    text = text->next;
    if(t2->data != NULL) free(t2->data);
    if(t2->hist != NULL) free(t2->hist);
    free(t2);
  }
  return;
}

void clearwarrior(WARRIOR* w) {
  if(w->name != NULL) free(w->name);
  if(w->author != NULL) free(w->author);
  if(w->version != NULL) free(w->version);
  if(w->date != NULL) free(w->date);
  if(w->strategy != NULL) free(w->strategy);
  return;
}

void set_standard_by_tag(LINE* line, char* tag) {
  if(parsopt.always_use_default) return;
  int standard = 0;
  if(tag[0] == '\0') {
    standard = parsopt.default_standard;
  }
  else {
    if(tag[0] != '-') {
      parser_warn(line, "recognized ;redcode switches must begin with '-'");
    }
    switch(tag[1]) {
      case '\0': standard = parsopt.default_standard; break;
      case '9':
        if(tag[2] == '4') standard = 94;
        else {
          parser_warn(line, "unrecognized switch ;redcode%s", tag);
        }
        break;
      case '8':
        switch(tag[2]) {
          case '8':
            if(tag[3] == 'x' || tag[3] == 'X') standard = 89;
            else standard = 88;
            break;
          case '6': standard = 86; break;
          case '4': standard = 84; break;
          default:
            parser_warn(line, "unrecognized switch ;redcode%s", tag);
            break;
        }
        break;
      case 'i': case 'I':
        if(tolower(tag[2]) == 'c'
        && tolower(tag[3]) == 'w'
        && tolower(tag[4]) == 's') standard = 88;
        else parser_warn(line, "unrecognized switch ;redcode%s", tag);
        break;
      default:
        parser_warn(line, "unrecognized switch ;redcode%s", tag);
    }
  }
  if(standard == 0) return;
  if(standard == parsopt.standard) return;
  //now define parser settings
  parser_note(line, "changing settings to %s.",
    (standard == 84)? "pre-ICWS" :
    (standard == 86)? "ICWS '86" :
    (standard == 88)? "ICWS '88" :
    (standard == 89)? "ICWS '88 Extended" :
    (standard == 94)? "ICWS '94" : "unknown");
  parsopt.standard = standard;

  if(standard == 84) {
    parsopt.allow_slash_comments = 1;
    parsopt.do_first_pass = 0;
    parsopt.do_second_pass = 0;
    parsopt.allow_pseudo_end = 0;
    parsopt.opers_plusminus = 0;
    parsopt.opers_addsub = 0;
    parsopt.addr_bdecrement = 0;
    parsopt.field_separator = 2;
  }
  else {
    parsopt.allow_slash_comments = 0;
    parsopt.do_first_pass = 1;
    parsopt.do_second_pass = 1;
    parsopt.allow_pseudo_end = 1;
    parsopt.opers_plusminus = 1;
    parsopt.opers_addsub = 1;
    parsopt.addr_bdecrement = 1;
    if(standard > 88) {
      parsopt.field_separator = 1;
    }
    else{
      parsopt.field_separator = 3;
    }
  }

  if(standard <= 86) {
    parsopt.must_start_redcode = 0;
    parsopt.allow_pseudo_equ = 0;
    parsopt.predef_constants = 0;
    parsopt.labels_maxlength = 8;
    parsopt.labels_case_sens = 0;
    parsopt.opers_muldiv = 0;
    parsopt.allow_end_operand = 0;
    parsopt.def_modif_rules = 86;
    parsopt.field_count_rules = 1;
  }
  else {
    parsopt.must_start_redcode = 1;
    parsopt.allow_pseudo_equ = 1;
    parsopt.predef_constants = 1;
    parsopt.labels_maxlength = 0;
    parsopt.labels_case_sens = 1;
    parsopt.opers_muldiv = 1;
    parsopt.allow_end_operand = 1;
    parsopt.def_modif_rules = 88;
    if(standard == 94) {
      parsopt.field_count_rules = 3;
    }
    else {
      parsopt.field_count_rules = 2;
    }
  }

  if(standard <= 88) {
    parsopt.allow_pseudo_org = 0;
    parsopt.opers_whitespace = 0;
    parsopt.opers_comparison = 0;
    parsopt.opers_parentheses = 0;
    if(standard == 86) {
      parsopt.addr_expl_direct = 1;
    }
    else {
      parsopt.addr_expl_direct = 0;
    }
  }
  else {
    parsopt.allow_pseudo_org = 1;
    parsopt.opers_whitespace = 1;
    parsopt.opers_comparison = 1;
    parsopt.opers_parentheses = 1;
    parsopt.addr_expl_direct = 1;
  }

  if(standard < 94) {
    parsopt.allow_pseudo_for = 0;
    parsopt.allow_pseudo_pin = 0;
    parsopt.allow_multiline_equ = 0;
    parsopt.allow_variables = 0;
    parsopt.opers_modulo = 0;
    parsopt.opers_logical = 0;
    parsopt.allow_concatenate = 0;
    parsopt.allow_modifiers = 0;
    parsopt.addr_bincrement = 0;
    parsopt.addr_aindirect = 0;
    parsopt.addr_adecrement = 0;
    parsopt.addr_aincrement = 0;
  }
  else {
    parsopt.allow_pseudo_for = 1;
    parsopt.allow_pseudo_pin = 1;
    parsopt.allow_multiline_equ = 1;
    parsopt.allow_variables = 1;
    parsopt.opers_modulo = 1;
    parsopt.opers_logical = 1;
    parsopt.allow_concatenate = 1;
    parsopt.allow_modifiers = 1;
    parsopt.addr_bincrement = 1;
    parsopt.addr_aindirect = 1;
    parsopt.addr_adecrement = 1;
    parsopt.addr_aincrement = 1;
  }

  if(standard == 86) {
    parsopt.allow_pseudo_space = 1;
  }
  else {
    parsopt.allow_pseudo_space = 0;
  }

  if(standard == 88) {
    parsopt.allow_all_addrs = 0;
  }
  else {
    parsopt.allow_all_addrs = 1;
  }

  return;
}

int validate_label(LINE* line, char* label) {
  /** \brief Validate label according to current rules.
      \param line [in] Line of text containing the label, used for diagnostic.
      \param label [in, out] Label to be validated.
      \return Nonzero if valid, zero if invalid.

      This function will test the validity of a label
      according to the current rules. It will display
      the appropriate error or warning. Then, if the
      label is not a legal identifier, it will return
      0. Otherwise, even if a warning is issued, it
      will return nonzero. If the current rules state
      labels to be case-insensitive, *label* will also
      be modified by turning all letters to uppercase.
   */
   if(!isalpha(label[0])) {
     parser_error(line, "invalid label \"%s\". All labels must begin with a letter.", label);
     return 0;
   }
   if(parsopt.labels_maxlength && strlen(label) > parsopt.labels_maxlength) {
     parser_error(line, "invalid label \"%s\". Exceeds maximum length (%d > %d).", label, strlen(label), parsopt.labels_maxlength);
     return 0;
   }
   if(!parsopt.labels_case_sens) {
     int c2;
     for(c2 = 0; label[c2] != '\0'; ++c2) label[c2] = toupper(label[c2]);
   }
   return 1;
}

int init_parser() {
  /** \brief Initialize parser and test settings.
      \return Nonzero if error, zero if correct.
   */
  //just in case
  clear_dictionary(&d_opcodes);
  clear_dictionary(&d_pseudoops);
  clear_dictionary(&d_constants);
  clear_dictionary(&d_signatures);
  clear_dictionary(&d_parameters);
  clear_dictionary(&d_variables);
  clear_dictionary(&d_labels);

  if((!parsopt.opers_plusminus) && parsopt.opers_addsub) {
    parser_error(NULL, "invalid settings; substraction operator enabled but unary minus disabled.");
    return 1;
  }
  if(parsopt.opers_whitespace && parsopt.opers_plusminus && (parsopt.field_separator & 2)) {
    parser_error(NULL, "invalid settings; whitespace both as separator and inside expressions is incompatible with unary + and -.");
    return 2;
  }

  { //instructions
    #ifdef O_DAT
    set_dictionary(&d_opcodes, restricted_copy("DAT"), (void*)1);
    #endif
    #ifdef O_MOV
    set_dictionary(&d_opcodes, restricted_copy("MOV"), (void*)1);
    #endif
    #ifdef O_ADD
    set_dictionary(&d_opcodes, restricted_copy("ADD"), (void*)1);
    #endif
    #ifdef O_SUB
    set_dictionary(&d_opcodes, restricted_copy("SUB"), (void*)1);
    #endif
    #ifdef O_MUL
    set_dictionary(&d_opcodes, restricted_copy("MUL"), (void*)1);
    #endif
    #ifdef O_DIV
    set_dictionary(&d_opcodes, restricted_copy("DIV"), (void*)1);
    #endif
    #ifdef O_MOD
    set_dictionary(&d_opcodes, restricted_copy("MOD"), (void*)1);
    #endif
    #ifdef O_JMP
    set_dictionary(&d_opcodes, restricted_copy("JMP"), (void*)1);
    #endif
    #ifdef O_JMZ
    set_dictionary(&d_opcodes, restricted_copy("JMZ"), (void*)1);
    #endif
    #ifdef O_JMN
    set_dictionary(&d_opcodes, restricted_copy("JMN"), (void*)1);
    #endif
    #ifdef O_DJZ
    set_dictionary(&d_opcodes, restricted_copy("DJZ"), (void*)1);
    #endif
    #ifdef O_DJN
    set_dictionary(&d_opcodes, restricted_copy("DJN"), (void*)1);
    #endif
    #ifdef O_CMP
    set_dictionary(&d_opcodes, restricted_copy("CMP"), (void*)1);
    #endif
    #ifdef O_SEQ
    set_dictionary(&d_opcodes, restricted_copy("SEQ"), (void*)1);
    #endif
    #ifdef O_SNE
    set_dictionary(&d_opcodes, restricted_copy("SNE"), (void*)1);
    #endif
    #ifdef O_SLT
    set_dictionary(&d_opcodes, restricted_copy("SLT"), (void*)1);
    #endif
    #ifdef O_SPL
    set_dictionary(&d_opcodes, restricted_copy("SPL"), (void*)1);
    #endif
    #ifdef O_NOP
    set_dictionary(&d_opcodes, restricted_copy("NOP"), (void*)1);
    #endif
    #ifdef O_LDP
    set_dictionary(&d_opcodes, restricted_copy("LDP"), (void*)1);
    #endif
    #ifdef O_STP
    set_dictionary(&d_opcodes, restricted_copy("STP"), (void*)1);
    #endif
    #if defined(O_XCH) || STANDARD == 89
    set_dictionary(&d_opcodes, restricted_copy("XCH"), (void*)1);
    #endif
    #ifdef O_PCT
    set_dictionary(&d_opcodes, restricted_copy("PCT"), (void*)1);
    #endif
    #ifdef O_STS
    set_dictionary(&d_opcodes, restricted_copy("STS"), (void*)1);
    #endif
  }

  set_dictionary(&d_opcodes, restricted_copy("XXX"), (void*)1);

  if(parsopt.allow_pseudo_org) set_dictionary(&d_pseudoops, restricted_copy("ORG"), (void*)1);
  if(parsopt.allow_pseudo_equ) set_dictionary(&d_pseudoops, restricted_copy("EQU"), (void*)1);
  if(parsopt.allow_pseudo_for) set_dictionary(&d_pseudoops, restricted_copy("FOR"), (void*)1);
  if(parsopt.allow_pseudo_for) set_dictionary(&d_pseudoops, restricted_copy("ROF"), (void*)1);
  if(parsopt.allow_pseudo_end) set_dictionary(&d_pseudoops, restricted_copy("END"), (void*)1);
  if(parsopt.allow_pseudo_pin) set_dictionary(&d_pseudoops, restricted_copy("PIN"), (void*)1);
  if(parsopt.allow_pseudo_space) set_dictionary(&d_pseudoops, restricted_copy("SPACE"), (void*)1);

  if(parsopt.predef_constants) {
    set_dictionary(&d_constants, restricted_copy("CORESIZE"), format2text("%d", CORESIZE));
    set_dictionary(&d_constants, restricted_copy("WARRIORS"), format2text("%d", WARRIORS));
    set_dictionary(&d_constants, restricted_copy("PSPACESIZE"), format2text("%d", PSPACESIZE));
    set_dictionary(&d_constants, restricted_copy("MAXCYCLES"), format2text("%d", MAXCYCLES));
    set_dictionary(&d_constants, restricted_copy("MAXPROCESSES"), format2text("%d", MAXPROCESSES));
    set_dictionary(&d_constants, restricted_copy("MAXLENGTH"), format2text("%d", MAXLENGTH));
    set_dictionary(&d_constants, restricted_copy("MINDISTANCE"), format2text("%d", MINDISTANCE));
    set_dictionary(&d_constants, restricted_copy("VERSION"), format2text("%d", VERSION));
  }

  org_is_set = pin_is_set = 0;
  return 0;
}

/// An expression value of the form `a - b*CURLINE`.
typedef struct tEXP_VAL {
  int a; ///< Value. Positive or negative.
  int b; ///< Coefficient. Can be negative, at the end of evaluation must be zero
  ///Some values for *b* have special meaning
  #define VAL_B_INVALID ((~0) ^ ((unsigned)(~0) >> 1)) //minimum negative number, means expression is invalid
  #define VAL_B_LATER (VAL_B_INVALID + 1) //means expression could not be expanded yet (leave for second pass)
} EXP_VAL;

int expand_constants(LINE* line) {
  /** \brief Expand all non-concatenated constants in a line.
      \param line [in, out] The line to operate on.
      \return Zero if no errors, nonzero if an error occurred.
   */
  int c; int labstart = -1; int labend = -1;
  for(c = 0; c < line->len; ++c) {
    if(labstart < 0) { //no label found yet
      if(isalpha(line->data[c])) { //start label
        labstart = c;
      }
    }
    else { //we are inside a label
      if(!isalnum(line->data[c])) { //end of label
        labend = c;
      }
      if(labend < 0 && c == line->len-1) { //end of line
        labend = c+1;
      }
      if(labend >= 0) { //substitute label
        char* label = (char*) malloc(labend - labstart + 1);
        strncpy(label, &line->data[labstart], labend - labstart);
        label[labend - labstart] = '\0';
        char* lAbel = (char*) malloc(labend - labstart + 1); //case-insensitive label
        int c2;
        for(c2 = 0; label[c2] != '\0'; ++c2) lAbel[c2] = toupper(label[c2]);
        lAbel[labend - labstart] = '\0';
        if(labstart > 0 && isdigit(line->data[labstart-1])) { //begins with a digit
          int k;
          for(k = labstart; k > 0 && isalnum(line->data[k]); --k) ;
          ++k;
          char* full = (char*) malloc(labend-k+1);
          strncpy(full, &line->data[k], labend-k+1);
          full[labend-k] = '\0';
          parser_error(line, "labels must not start with a digit (%s)", full);
          free(full);
          free(label);
          free(lAbel);
          //freetext(txt);
          return 1;
        }
        if(get_dictionary(&d_opcodes, lAbel)) { //is an opcode, skip
          if(line->data[c] == '.') { //also skip modifier
            do{ ++c; } while(isalpha(line->data[c]));
          }
        }
        else if(get_dictionary(&d_pseudoops, lAbel)) { //pseudo-opcode
        }
        else if(labstart == 1 && line->data[0] == ';' && !strcmp(label, "assert")) { //;assert
        }
        else { //actual label
          if(!parsopt.labels_case_sens) { //EXCHANGE label AND lAbel
            char* tmp = label;
            label = lAbel;
            lAbel = tmp;
          }
          LINE* subs = get_dictionary(&d_constants, label);
          int mustfreesubs = 0;
          LINE* signature = NULL;
          if((signature = get_dictionary(&d_signatures, label)) != NULL) { //function-like macro
            for( ; isblank(line->data[c]); ++c);
            if(line->data[c] != '(') {
              parser_error(line, "%s is a function-like macro, expected '(' after occurrence of it.", label);
              free(label);
              free(lAbel);
              //freetext(txt);
              return 1;
            }
            ++c;
            int s = 0;
            int narg;
            for(narg = 0;; ++narg) { //now get all parameters into d_params
              int sfinished = 0; int cfinished = 0; int emptyparam = 0;

              int sstart, send;
              if(signature->data != NULL) {
                for( ; isblank(signature->data[s]); ++s);
                sstart = s;
                for( ; isalnum(signature->data[s]); ++s);
                send = s;
                if(send <= sstart) { //takes no parameters
                  sfinished = 1;
                }
              }
              else sfinished = 1; //takes no parameters
              //now s is after end of parameter name
              //sfinished is 1 if no parameter was found

              for( ; isblank(line->data[c]); ++c);
              int cstart = c; int level = 1;
              if(line->data[c] == ',') {
                emptyparam = 1;
              }
              else {
                for( ; line->data[c] != ','; ++c) {
                  if(line->data[c] == '(') ++level;
                  else if(line->data[c] == ')') {
                    --level;
                    if(!level) { //end of passed parameter list
                      cfinished = 0;
                      break; //from inner loop
                    }
                  }
                  else if(line->data[c] == '\0') {
                    parser_error(line, "expected ')', found end of line. "
                    "Note that currently all parameters to a function-like macro must lie within a single line.");
                    free(label);
                    free(lAbel);
                    //freetext(txt);
                    return 1;
                  }
                }
                --c;
                for( ; isblank(line->data[c]); --c);
                ++c;
              }
              int cend = c;
              if(cend <= cstart && (!emptyparam)) { //empty parameter at the end
                cfinished = 1;
                if(narg == 0) { //no parameters/empty parameter passed
                  if(!sfinished) { //parameters expected (then it must be empty)
                    emptyparam = 1;
                  }
                  else { //no parameters required
                    for( ; isblank(line->data[c]); ++c);
                    ++c;
                    break;
                  }
                }
                else { //the last parameter is empty
                  emptyparam = 1;
                  cfinished = 0;
                }
              }
              //now c is after end of parameter value
              //cfinished is 1 if no more parameters were found

              if(sfinished) {
                parser_error(line, "%s() didn't expect any parameters.", label, signature->data);
                free(label);
                free(lAbel);
                //freetext(txt);
                return 1;
              }

              char* pname = (char*) malloc(send-sstart+1); //tag
              strncpy(pname, &signature->data[sstart], send-sstart);
              pname[send-sstart] = '\0';
              char* pvalue;
              if(emptyparam) {
                pvalue = restricted_copy("");
                emptyparam = 0;
              }
              else {
                pvalue = (char*) malloc(cend-cstart+1); //value
                strncpy(pvalue, &line->data[cstart], cend-cstart);
                pvalue[cend-cstart] = '\0';
              }
              set_dictionary(&d_parameters, pname, string2text(pvalue)); //set it
              free(pname);
              free(pvalue);

              //now advance to next parameter
              for( ; isblank(signature->data[s]); ++s);
              if(signature->data[s] == '\0') sfinished = 1; //end of signature
              else ++s; //we have arrived at the comma

              for( ; isblank(line->data[c]); ++c);
              if(line->data[c] == ')') cfinished = 1; //end of passed parameters
              else if(line->data[c] != ',') {
                parser_error(line, "expected '%c', '%c' found.", (sfinished)? ')' : ',', signature->data[c]);
                free(label);
                free(lAbel);
                //freetext(txt);
                return 1;
              }
              ++c;
              if(sfinished && cfinished) break; //finished
              else if(sfinished || cfinished) {
                parser_error(line, "%s(%s) was passed %s parameters than expected.", label, signature->data, (sfinished)? "more" : "less");
                free(label);
                free(lAbel);
                //freetext(txt);
                return 1;
              }
            } //for
            //all parameters obtained
            labend = c; //just after the closing parenthesis
            subs = copytext(subs);
            text_substitute_from_dictionary(subs, &d_parameters);
            mustfreesubs = 1; //now it's just a copy
            clear_dictionary(&d_parameters);
          }
          if(subs != NULL) { //perform substitution
            text_substitute_label(line, labstart, labend, subs);
            c = -1; //reset to start of line
          }
          if(mustfreesubs) freetext(subs);
        }
        labstart = labend = -1; //no label now
        free(label);
        free(lAbel);
      }
    }
  }
  return 0;
}

EXP_VAL compute_expression_rec(char* expr, int start, int end, LINE* line, int linetype) {
  int c;
  int level = 0;
  int position = -1; //position of the operator
  int precedence = 0; //precedence (1 is highest, 7 is lowest)
  int parstart = -1; int parend = -1; //in case everything is enclosed in parentheses
  int can_unary = 1; //whether the following operand must be unary
  for(c = start; c < end; ++c) { //find lowest precedence operator outside any parentheses
    for(; isblank(expr[c]); ++c) ; //ignore whitespace
    if(c >= end) break;
    char ch = expr[c];
    if(ch == '(') {
      if(!level) parstart = c+1;
      ++level;
      can_unary = 1;
    }
    else if(ch == ')') {
      --level;
      if(!level) parend = c;
      can_unary = 0;
    }
    else if(isalnum(ch)) {
      can_unary = 0;
    }
    else if(!level) { //operators/operands outside parentheses
      switch(precedence) {
        case 0:
          if(can_unary && ((ch == '!' && ((c+1 == end) || expr[c+1] != '=')) || ch == '+' || ch == '-')) { //precedence 1, right-associative
            position = c;
            precedence = 1;
            break;
          }
          else if((!can_unary) && (ch == '=' && ((c+1 == end) || expr[c+1] != '='))) { //precedence 7, right-associative, precedence 0 to the left
            position = c;
            precedence = 7;
            break;
          }
        case 1:
        case 2:
          if((!can_unary) && (ch == '*' || ch == '/' || ch == '%')) { //precedence 2, left-associative
            position = c;
            precedence = 2;
            break;
          }
        case 3:
          if((!can_unary) && (ch == '+' || ch == '-')) { //precedence 3, left-associative
            position = c;
            precedence = 3;
            break;
          }
        case 4:
          if((!can_unary) && (((c+1 < end) && expr[c+1] == '=') || ch == '<' || ch == '>')) { //precedence 4, left-associative
            position = c;
            precedence = 4;
            break;
          }
        case 5:
          if((!can_unary) && (ch == '&')) { //precedence 5, left-associative
            position = c;
            precedence = 5;
            break;
          }
        case 6:
          if((!can_unary) && (ch == '&')) { //precedence 6, left-associative
            position = c;
            precedence = 6;
            break;
          }
      } //switch
      can_unary = 1;
      if(precedence >= 7) break; //we have a winner (maximum precedence, right associative)
    }
  }
  if(position >= 0) { //operator present
    if(expr[position] == '=' && expr[position+1] != '=') { //assignment to variable
      if(!parsopt.allow_variables) {
        parser_error(line, "variables are disabled on current parser settings, enable them to use assignments.");
        EXP_VAL r = {0, VAL_B_INVALID};
        return r;
      }
      //check left side
      int k; int varstart = -1; int varend = -1;
      for(k = position-1; k >= start; --k) {
        char ch = expr[k];
        if(isalnum(ch)) {
          if(varend < 0) varend = k+1;
          else if(varstart >= 0) {
            parser_error(line, "variable names cannot consist of multiple words.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
        }
        else if(isblank(ch)) {
          if(varend >= 0 && varstart < 0) {
            varstart = k+1;
          }
        }
        else {
          parser_error(line, "stray '%c' near variable name.", ch);
          EXP_VAL r = {0, VAL_B_INVALID};
          return r;
        }
      }
      if(varstart < 0 && varend >= 0) varstart = start;
      if(varstart < 0) {
        parser_error(line, "assignment operator = must be preceded by variable name.");
        EXP_VAL r = {0, VAL_B_INVALID};
        return r;
      }
      char* varname = (char*) malloc(varend-varstart+1);
      char* vArname;
      strncpy(varname, &expr[varstart], varend-varstart);
      varname[varend-varstart] = '\0';
      //check name
      if(parsopt.exp_var_maxlength && (strlen(varname) > parsopt.exp_var_maxlength)) {
        parser_error(line, "invalid variable name \"%s\". Exceeds maximum variable length (%d > %d).", varname, strlen(varname), parsopt.exp_var_maxlength);
        free(varname);
        EXP_VAL r = {0, VAL_B_INVALID};
        return r;
      }
      if(!validate_label(line, varname)) {
        free(varname);
        EXP_VAL r = {0, VAL_B_INVALID};
        return r;
      }
      if(get_dictionary(&d_constants, varname) != NULL) {
        parser_error(line, "variable \"%s\" has the same name as a constant.", varname);
        parser_note(get_dictionary(&d_constants, varname), "previously defined:");
        free(varname);
        EXP_VAL r = {0, VAL_B_INVALID};
        return r;
      }
      if(get_dictionary(&d_labels, varname) != NULL) {
        parser_error(line, "variable \"%s\" has the same name as a label.", varname);
        free(varname);
        EXP_VAL r = {0, VAL_B_INVALID};
        return r;
      }
      if(parsopt.labels_case_sens) { //for checking against (pseudo-)opcodes
        vArname = (char*) malloc(varend-varstart+1);
        int c2; for(c2 = 0; varname[c2] != '\0'; ++c2) vArname[c2] = toupper(varname[c2]);
        vArname[varend-varstart] = '\0';
      }
      else vArname = varname;
      if(get_dictionary(&d_opcodes, vArname) != NULL) {
        parser_error(line, "variable \"%s\" has the same name as an opcode.", varname);
        free(varname);
        if(parsopt.labels_case_sens) free(vArname);
        EXP_VAL r = {0, VAL_B_INVALID};
        return r;
      }
      if(get_dictionary(&d_pseudoops, vArname) != NULL) {
        parser_error(line, "variable \"%s\" has the same name as a pseudo-opcode.", varname);
        free(varname);
        if(parsopt.labels_case_sens) free(vArname);
        EXP_VAL r = {0, VAL_B_INVALID};
        return r;
      }

      EXP_VAL v = compute_expression_rec(expr, position+1, end, line, linetype);
      if((v.b == VAL_B_INVALID) || (v.b == VAL_B_LATER)) return v;
      if(v.b != 0) {
        parser_error(line, "relative labels cannot be assigned to variables in non-field expressions.");
        free(varname);
        EXP_VAL r = {0, VAL_B_INVALID};
        return r;
      }

      LINE* lv = (LINE*) malloc(sizeof(LINE));
      lv->prev = lv->next = NULL; lv->hist = NULL;
      lv->nhist = 0;
      lv->data = (char*) malloc(sizeof(EXP_VAL));
      *((EXP_VAL*) lv->data) = v;
      lv->len = sizeof(LINE);
      update_dictionary(&d_variables, varname, lv);
      free(varname);
      return v;
    }
    else if(precedence == 1) { //unary operators
      EXP_VAL v = compute_expression_rec(expr, position+1, end, line, linetype);
      if((v.b == VAL_B_INVALID) || (v.b == VAL_B_LATER)) return v;
      switch(expr[position]) {
        case '!':
          if(!parsopt.opers_logical) {
            parser_error(line, "logical operands (!, && and ||) are not enabled on current parser settings.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          if(v.b) {
            parser_error(line, "relative labels cannot be applied the ! operator in non-field expressions.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          v.a = (v.a)? 0 : 1;
          break;
        case '+':
          if(!parsopt.opers_plusminus) {
            parser_error(line, "unary + and - operators are not enabled on current parser settings.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          break;
        case '-':
          if(!parsopt.opers_plusminus) {
            parser_error(line, "unary + and - operators are not enabled on current parser settings.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          v.a = -v.a;
          v.b = -v.b; //the coefficient is not subject to modulo math
          break;
      }
      return v;
    }
    else { //binary
      EXP_VAL v1 = compute_expression_rec(expr, start, position, line, linetype);
      if((v1.b == VAL_B_INVALID) || (v1.b == VAL_B_LATER)) return v1;
      int opchars = 1; //characters that the operator takes
      switch(expr[position]) {
        case '+': case '-': case '*': case '/': case '%':
          opchars = 1;
          break;
        case '&': case '|': case '=': case '!':
          opchars = 2;
          break;
        case '<': case '>': // 2 options
          if(expr[position+1] == '=') opchars = 2;
          else opchars = 1;
          break;
      }
      EXP_VAL v2 = compute_expression_rec(expr, position+opchars, end, line, linetype);
      if((v2.b == VAL_B_INVALID) || (v2.b == VAL_B_LATER)) return v2;
      switch(expr[position]) {
        case '+':
          if(!parsopt.opers_addsub) {
            parser_error(line, "binary + and - operators are not enabled on current parser settings.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          v1.a += v2.a;
          v1.b += v2.b;
          break;
        case '-':
          if(!parsopt.opers_addsub) {
            parser_error(line, "binary + and - operators are not enabled on current parser settings.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          v1.a -= v2.a;
          v1.b -= v2.b; //the coefficient is not subject to modulo math
          break;
        case '*':
          if(!parsopt.opers_muldiv) {
            parser_error(line, "* and / operators are not enabled on current parser settings.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          if(v1.b && v2.b) {
            parser_error(line, "two relative labels cannot be multiplied by each other in non-field expressions.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          v1.a *= v2.a;
          v1.b = v1.a * v2.b + v1.b * v2.a;
          break;
        case '/':
          if(!parsopt.opers_muldiv) {
            parser_error(line, "* and / operators are not enabled on current parser settings.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          if(v2.b) { //divisor is a label
            if(v1.a*v2.b == v1.b*v2.a) { //proportional to dividend
              v1.a /= v2.a; //(pa + pbx)/(qa + qbx) = p/q
              v1.b = 0;
            }
            else {
              parser_error(line, "relative labels cannot divide another value in non-field expressions, unless they are proportional to the dividend.");
              EXP_VAL r = {0, VAL_B_INVALID};
              return r;
            }
          }
          else {
            if(!v2.a) {
              parser_error(line, "division by zero in expression.");
              EXP_VAL r = {0, VAL_B_INVALID};
              return r;
            }
            v1.a /= v2.a; //(a + bx)/c = a/c + (b/c)x
            v1.b /= v2.a;
          }
          break;
        case '%':
          if(!parsopt.opers_modulo) {
            parser_error(line, "%% operator is not enabled on current parser settings.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          if(v1.b || v2.b) {
            parser_error(line, "relative labels cannot be used with the %% operator in non-field expressions.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          if(!v2.a) {
            parser_error(line, "modulo by zero in expression.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          v1.a %= v2.a;
          break;
        case '&': // &&
          if(!parsopt.opers_logical) {
            parser_error(line, "logical operands (!, && and ||) are not enabled on current parser settings.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          if(v1.b || v2.b) {
            parser_error(line, "relative labels cannot be used with the && operator in non-field expressions.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          v1.a = (v1.a && v2.a)? 1 : 0; //ternary operator Just In Case
          break;
        case '|': // ||
          if(!parsopt.opers_logical) {
            parser_error(line, "logical operands (!, && and ||) are not enabled on current parser settings.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          if(v1.b || v2.b) {
            parser_error(line, "relative labels cannot be used with the || operator in non-field expressions.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          v1.a = (v1.a || v2.a)? 1 : 0; //ternary operator Just In Case
          break;
        case '=': // ==
          if(!parsopt.opers_comparison) {
            parser_error(line, "comparison operands (== , != , < , > , <= and >=) are not enabled on current parser settings.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          if(v1.b != v2.b) {
            parser_error(line, "relative labels may only be compared if their coefficients are the same in non-field expressions.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          v1.a = (v1.a == v2.a)? 1 : 0; //ternary operator Just In Case
          break;
        case '!': // !=
          if(!parsopt.opers_comparison) {
            parser_error(line, "comparison operands (== , != , < , > , <= and >=) are not enabled on current parser settings.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          if(v1.b != v2.b) {
            parser_error(line, "relative labels may only be compared if their coefficients are the same in non-field expressions.");
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          v1.a = (v1.a != v2.a)? 1 : 0; //ternary operator Just In Case
          break;
        case '<': // 2 options
          if(expr[position+1] == '=') { // <=
            if(!parsopt.opers_comparison) {
              parser_error(line, "comparison operands (== , != , < , > , <= and >=) are not enabled on current parser settings.");
              EXP_VAL r = {0, VAL_B_INVALID};
              return r;
            }
            if(v1.b != v2.b) {
              parser_error(line, "relative labels may only be compared if their coefficients are the same in non-field expressions.");
              EXP_VAL r = {0, VAL_B_INVALID};
              return r;
            }
            v1.a = (v1.a <= v2.a)? 1 : 0; //ternary operator Just In Case
          }
          else { // <
            if(!parsopt.opers_comparison) {
              parser_error(line, "comparison operands (== , != , < , > , <= and >=) are not enabled on current parser settings.");
              EXP_VAL r = {0, VAL_B_INVALID};
              return r;
            }
            if(v1.b != v2.b) {
              parser_error(line, "relative labels may only be compared if their coefficients are the same in non-field expressions.");
              EXP_VAL r = {0, VAL_B_INVALID};
              return r;
            }
            v1.a = (v1.a < v2.a)? 1 : 0; //ternary operator Just In Case
          }
          break;
        case '>': // 2 options
          if(expr[position+1] == '=') { // >=
            if(!parsopt.opers_comparison) {
              parser_error(line, "comparison operands (== , != , < , > , <= and >=) are not enabled on current parser settings.");
              EXP_VAL r = {0, VAL_B_INVALID};
              return r;
            }
            if(v1.b != v2.b) {
              parser_error(line, "relative labels may only be compared if their coefficients are the same in non-field expressions.");
              EXP_VAL r = {0, VAL_B_INVALID};
              return r;
            }
            v1.a = (v1.a >= v2.a)? 1 : 0; //ternary operator Just In Case
          }
          else { // >
            if(!parsopt.opers_comparison) {
              parser_error(line, "comparison operands (== , != , < , > , <= and >=) are not enabled on current parser settings.");
              EXP_VAL r = {0, VAL_B_INVALID};
              return r;
            }
            if(v1.b != v2.b) {
              parser_error(line, "relative labels may only be compared if their coefficients are the same in non-field expressions.");
              EXP_VAL r = {0, VAL_B_INVALID};
              return r;
            }
            v1.a = (v1.a > v2.a)? 1 : 0; //ternary operator Just In Case
          }
          break;
      }
      return v1;
    }
  }
  else { //either everything is enclosed, or this is a simple operand
    if(parstart >= 0) { //enclosed
      return compute_expression_rec(expr, parstart, parend, line, linetype);
    }
    else { //operand
      int cs, ce;
      for(cs = start; (cs < end) && (isblank(expr[cs])); ++cs) ; //discard leading whitespace
      for(ce = end-1; (ce >= start) && (isblank(expr[ce])); --ce) ; //discard trailing whitespace
      ++ce;
      int num = 0; //in case it's a number
      int c;
      for(c = cs; c < ce; ++c) {
        char ch = expr[c];
        if(isdigit(ch)) {
          num *= 10;
          num += ch - '0';
        }
        else {
          num = -1;
          break;
        }
      }
      if(num < 0) { //not a number
        char* oper = (char*) malloc(ce-cs+1);
        strncpy(oper, &expr[cs], ce-cs);
        oper[ce-cs] = '\0';
        //can be CURLINE, variable or relative label
        if(!validate_label(line, oper)) { //this may turn it uppercase
          free(oper);
          EXP_VAL r = {0, VAL_B_INVALID};
          return r;
        }
        if(!strcmp(oper, "CURLINE")) {
          free(oper);
          EXP_VAL r = {CURLINE, 0};
          return r;
        }
        LINE* res = get_dictionary(&d_variables, oper);
        if(res != NULL) { //is a variable
          free(oper);
          EXP_VAL r = *(EXP_VAL*)res->data;
          return r;
        }
        res = get_dictionary(&d_labels, oper);
        //free(oper);
        if(res != NULL) { //is a relative label
          if(linetype == LTYPE_NORMAL) { //normal statement
            free(oper);
            EXP_VAL r = {((EXP_VAL*)res->data)->a - CURLINE, 0};
            return r;
          }
          else {
            free(oper);
            if(!parsopt.exp_label_arith) {
              parser_error(line, "using relative labels inside non-field expressions is not enabled on current parser settings.");
              EXP_VAL r = {0, VAL_B_INVALID};
              return r;
            }
            EXP_VAL r = {((EXP_VAL*)res->data)->a, 1};
            return r;
          }
        }
        else { //undefined label, cannot expand yet. Leave it for second pass, error if second pass
          if(!allow_forward_refs) {
            parser_error(line, "undefined identifier: %s.", oper);
            free(oper);
            EXP_VAL r = {0, VAL_B_INVALID};
            return r;
          }
          else {
            free(oper);
            EXP_VAL r = {0, VAL_B_LATER};
            return r;
          }
        }
      }
      else { //number
        EXP_VAL r = {num, 0};
        return r;
      }
    }
  }
  EXP_VAL rr = {0, VAL_B_LATER}; //return value. By default means the expression can't be expanded yet
  return rr;
}

char* restrict compute_expression(char* expr, LINE* line, int linetype) {
  /** \brief Tries to evaluate an expression.
      \param expr [in] Null-terminated string containing a valid expression.
      \param line [in] Line containing the expression, for diagnostic purposes.
      \param linetype [in] Line type. Possible types are:
      * LTYPE_NORMAL: free use of relative labels.
      * LTYPE_FOR: very restrictive use of relative labels.
      * LTYPE_ASSERT: as LTYPE_FOR.
      * LTYPE_PIN: as LTYPE_FOR.
      \return If the expression can be evaluated, returns a new string
      containing the result. This can be a copy of the original, e.g. if the
      original expression only consists of a number. If the expression is
      illegal, a static empty string is returned. If the expression can't be
      evaluated, NULL is returned. Additionally, if the expression can't
      be evaluated and *linetype* is 2, an error is produced ("" is returned).

      Evaluates an expression as stated by ICWS rules. It will substitute
      any variables and occurrences of CURLINE. If the line is a normal
      statement, relative labels will be substituted by their position minus
      the current line's. Otherwise, relative labels will only be allowed if
      the whole expression is not dependent on the value used as the current
      line, i.e. `label1-label2` is correct, as well as `-label1-2*label2+label3*3`.
      Note that this behavior requires tPARSOPT.exp_label_arith to be enabled
      in #parsopt. Otherwise, the expression is considered illegal.
      The string returned by this function will have at least one
      trailing whitespace character (unless it's the special value "").
   */
  //printf("(%d) Expression: %s\n", linetype, expr); //D
  if((!parsopt.opers_whitespace) && (linetype == LTYPE_NORMAL)) { //check for whitespace inside the expression
    int state = 0; int level = 0;
    int c;
    for(c = 0; expr[c] != '\0'; ++c) {
      char ch = expr[c];
      if(ch == '(') ++level;
      else if(ch == ')') --level;
      switch(state) {
        case 0: if(!isblank(ch)) state = 1; break;
        case 1: if(isblank(ch) && (!level)) state = 2; break;
        case 2:
          if(!isblank(ch)) {
            parser_error(line, "whitespace in expressions is not enabled on current parser settings.");
            return "";
          }
          break;
      }
    }
  }
  EXP_VAL ev = compute_expression_rec(expr, 0, strlen(expr), line, linetype);
  if(ev.b == VAL_B_INVALID) { //illegal expression
    return "";
  }
  if(ev.b == VAL_B_LATER) { //could not expand (yet)
    return NULL;
  }
  if(ev.b && ((linetype == LTYPE_FOR) || (linetype == LTYPE_ASSERT) || (linetype == LTYPE_PIN))) { //evaluates to label
    parser_error(line, "non-field expressions must evaluate to position-independent values. This expression evaluates to (%d - (%d)*CURLINE).", ev.a, ev.b);
    return "";
  }
  char* r = (char*) malloc(snprintf(NULL, 0, "%d ", ev.a)+1);
  sprintf(r, "%d ", ev.a);
  //printf("(%d) Result: %s\n", linetype, r); //D
  return r;
}

int identify_line_type(LINE* line, int* exp_base) {
  if(line->data[0] == ';') { //;assert
    *exp_base = 8;
    return LTYPE_ASSERT;
  }
  else { //FOR, normal and other
    int c; int wstart = -1; int wend;
    for(c = 0; c <= line->len; ++c) {
      char ch = line->data[c];
      if(isalnum(ch)) {
        if(wstart < 0) { //begin word
          wstart = c;
        }
      }
      else {
        if(wstart >= 0) { //end word
          wend = c;
          char* word = (char*) malloc(wend-wstart+1);
          strncpy(word, &line->data[wstart], wend-wstart);
          word[wend-wstart] = '\0';
          int c2;
          for(c2 = 0; word[c2] != '\0'; ++c2) word[c2] = toupper(word[c2]);
          if(!strcmp(word, "FOR")) {
            free(word);
            *exp_base = c+1; //followed by expression
            return LTYPE_FOR;
          }
          if(!strcmp(word, "END")) {
            free(word);
            *exp_base = c+1; //followed by label, but possibly support expression
            return LTYPE_END;
          }
          if(!strcmp(word, "ORG")) {
            free(word);
            *exp_base = c+1; //followed by label, but possibly support expression
            return LTYPE_ORG;
          }
          if(!strcmp(word, "PIN")) {
            free(word);
            *exp_base = c+1; //followed by number, but possibly support expression
            return LTYPE_PIN;
          }
          if(get_dictionary(&d_opcodes, word)) {
            free(word);
            for(; (line->data[c] != '\0') && (!isblank(line->data[c])); ++c) ; //skip modifier
            *exp_base = c+1;
            return LTYPE_NORMAL;
          }
          free(word);
          wstart = -1;
        }
      }
    } //for
  }
  return LTYPE_OTHER;
}

int expand_expressions(LINE* line, int line_is, int exp_base) {
  /** \brief Expand all possible expressions in *line*.
      \param line [in, out] The line on which to operate.
      \param line_is [in] Line type.
      \param exp_base [in] The position to start looking for expressions at.
      \return Zero if no errors, nonzero if an error occurred.
   */
  int c; int state = 0; //finite-state machine
  int expstart = -1; int expend = -1;
  int level = 0;
  for(c = exp_base; c <= line->len; ++c) { //compute all expressions
    int skipped = 0;
    for(; isblank(line->data[c]); ++c) skipped = 1; //ignore whitespace
    if(skipped && (!parsopt.opers_whitespace) && state && (!level) && (line_is == LTYPE_NORMAL)) { //whitespace as separator
      switch(state) {
        case 3: case 4:
          state = 0; //expr
          expend = c;
          break;
        case 1:
          parser_error(line, "expected '(', number or label after unary operator, got separating whitespace.");
          return 1;
        case 2:
          parser_error(line, "expected unary operator, '(', number or label at start of nested subexpression, got separating whitespace.");
          return 1;
        case 5:
          parser_error(line, "expected unary operator, '(', number or label after binary operator, got separating whitespace.");
          return 1;
        case 6:
          parser_error(line, "expected unary operator, '(', number or label after binary operator, got separating whitespace.");
          return 1;
        case 7:
          parser_error(line, "unfinished comparison operator. Expected '=', got separating whitespace.");
          return 1;
        case 8:
          parser_error(line, "unfinished '&&' operator. Expected '&', got separating whitespace.");
          return 1;
        case 9:
          parser_error(line, "unfinished '||' operator. Expected '|', got separating whitespace.");
          return 1;
      }
    }
    char ch = line->data[c];
    if(expend < 0) switch(state) {
      case 0: //expr
        if(ch == '+' || ch == '-' || ch == '!') {
          expstart = c; expend = -1;
          state = 1; //unary
        }
        else if(ch == '(') {
          expstart = c; expend = -1;
          state = 2; //open
        }
        else if(isalnum(ch)) {
          expstart = c; expend = -1;
          state = 3; //number/label
        }
        //the following code does not work well with whitespace separators
        /*else if(expstart >= 0) {
          parser_error(line, "expected unary operator, '(', number or label at start of expression, got '%c'.", ch);
          return 1;
        }*/
        break;
      case 1: //unary
        if(ch == '(') {
          state = 2; //open
        }
        else if(isalnum(ch)) {
          state = 3; //number/label
        }
        else {
          parser_error(line, "expected '(', number or label after unary operator, got '%c'.", ch);
          return 1;
        }
        break;
      case 2: //open
        ++level;
        if(ch == '+' || ch == '-' || ch == '!') {
          state = 1; //unary
        }
        else if(ch == '(') {
          state = 2; //open
        }
        else if(isalnum(ch)) {
          state = 3; //number/label
        }
        else {
          parser_error(line, "expected unary operator, '(', number or label at start of nested subexpression, got '%c'.", ch);
          return 1;
        }
        break;
      case 3: //number/label
        if(isalnum(ch)) {
          if(skipped) { //space after number/label and before another
            state = 0; //expr
            expend = c;
          }
          else state = 3; //number/label
        }
        else if(ch == ')') {
          state = 4; //close
        }
        else if(ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%') {
          state = 5; //binary
        }
        else if(ch == '=') {
          state = 6; //=(=)
        }
        else if(ch == '<' || ch == '>' || ch == '!' || ch == '=') {
          state = 7; //*=
        }
        else if(ch == '&') {
          state = 8; //*&
        }
        else if(ch == '|') {
          state = 9; //*|
        }
        else if(level == 0) {
          state = 0; //expr
          expend = c;
        }
        else {
          parser_error(line, "unmatched '(' in expression.");
          return 1;
        }
        break;
      case 4: //close
        --level;
        if(level < 0) {
          parser_error(line, "unmatched ')' in expression.");
          return 1;
        }
        if(ch == ')') {
          state = 4; //close
        }
        else if(ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%') {
          state = 5; //binary
        }
        else if(ch == '=') {
          state = 6; //=(=)
        }
        else if(ch == '<' || ch == '>' || ch == '!' || ch == '=') {
          state = 7; //*=
        }
        else if(ch == '&') {
          state = 8; //*&
        }
        else if(ch == '|') {
          state = 9; //*|
        }
        else if(level == 0) {
          state = 0; //expr
          expend = c;
        }
        else {
          parser_error(line, "unmatched '(' in expression.");
          return 1;
        }
        break;
      case 5: //binary
        if(ch == '+' || ch == '-' || ch == '!') {
          state = 1; //unary
        }
        else if(ch == '(') {
          state = 2; //open
        }
        else if(isalnum(ch)) {
          state = 3; //number/label
        }
        else if(expstart >= 0) {
          parser_error(line, "expected unary operator, '(', number or label after binary operator, got '%c'.", ch);
          return 1;
        }
        break;
      case 6: //=(=)
        if(ch == '=') { //==
          if(skipped) {
            parser_error(line, "two-character comparison operators must not be split with empty space.");
            return 1;
          }
          state = 5; //binary
        }
        else { //=
          if(ch == '+' || ch == '-' || ch == '!') {
            state = 1; //unary
          }
          else if(ch == '(') {
            state = 2; //open
          }
          else if(isalnum(ch)) {
            state = 3; //number/label
          }
          else if(expstart >= 0) {
            parser_error(line, "expected unary operator, '(', number or label after binary operator, got '%c'.", ch);
            return 1;
          }
        }
        break;
      case 7: //*=
        if(ch == '=') {
          if(skipped) {
            parser_error(line, "two-character comparison operators must not be split with empty space.");
            return 1;
          }
          state = 5; //binary
        }
        else {
          parser_error(line, "unfinished comparison operator. Expected '=', got '%c'.", ch);
          return 1;
        }
        break;
      case 8: //*&
        if(ch == '&') {
          if(skipped) {
            parser_error(line, "operator '&&' must not be split with empty space.");
            return 1;
          }
          state = 5; //binary
        }
        else {
          parser_error(line, "unfinished '&&' operator. Expected '&', got '%c'.", ch);
          return 1;
        }
        break;
      case 9: //*|
        if(ch == '|') {
          if(skipped) {
            parser_error(line, "operator '||' must not be split with empty space.");
            return 1;
          }
          state = 5; //binary
        }
        else {
          parser_error(line, "unfinished '||' operator. Expected '|', got '%c'.", ch);
          return 1;
        }
        break;
    } //switch
    if(expend >= 0) { //expression found
      char* expr = (char*) malloc(expend-expstart+1);
      strncpy(expr, &line->data[expstart], expend-expstart);
      expr[expend-expstart] = '\0';
      char* newexp = compute_expression(expr, line, line_is);
      if(newexp != NULL) { //if it has been evaluated
        free(expr);
        expr = newexp;
        if(expr[0] == '\0') { //returned static "" (error)
          return 1;
        }
        LINE* exprl = string2text(expr);
        text_substitute_label(line, expstart, expend, exprl);
        freetext(exprl);
        c = strlen(expr) + expstart; //position at end of expression
        free(expr);
      }
      --c; //continue here on next iteration
      expstart = expend = -1;
    }
  } //for
  return 0;
}

LINE* parse(LINE* red, WARRIOR* w) { //returns load file
  /**
      \brief Parses a redcode file and outputs load file and metadata.
      \param red [in] The raw redcode file to parse.
      \param w [out] The warrior to write metadata to.
      \return The parsed load file in pMARS format.
      \sa parsopt
  */
  w->org = w->len = w->haspin = w->pin = 0;
  w->name = w->author = w->version = w->date = w->strategy = NULL;
  LINE* txt = NULL;
  { //make copy of whole text, without blank lines
    LINE* line;
    LINE* prev;
    FORLINEIN(red, line) {
      if(line->len > 0) {
        LINE* copy = copyline(line);
        if(txt == NULL) txt = copy;
        else linklines(prev, copy);
        prev = copy;
      }
    }
    if(txt == NULL) {
      parser_error(NULL, "file is empty or contains only whitespace");
      return NULL;
    }
  }
  { //remove leading whitespace
    LINE* line;
    FORLINEIN(txt, line) {
      int c;
      for(c = 0; c < line->len; ++c) {
        if(!isblank(line->data[c])) break;
      }
      //c contains index of first non-blank character
      line->len -= c;
      memmove(line->data, &line->data[c], line->len+1);
      line->data = (char*) realloc(line->data, line->len+1);
    }
  }
  { //remove comments and trailing whitespace, process special lines
    w->name = w->author = w->version = w->date = w->strategy = NULL;
    LINE* line;
    int redcode_line_found = 0;
    FORLINEIN(txt, line) {
      int c, l;
      for(c = l = 0; c < line->len; ++c) {
        int bbrk = 0;
        switch(line->data[c]) {
          case ';': bbrk = 1; break;
          case '/':
            if(parsopt.allow_slash_comments) bbrk = 1;
            else l = c+1;
            break;
          case ' ': case '\t': break;
          default: l = c+1;
        }
        if(bbrk) break;
      }
      //now l contains length of line without comments and trailing whitespace
      if(line == txt && line->prev != NULL) { //free previously discarded line
        //check pointers just in case free() is weird...
        if(line->prev->data) free(line->prev->data);
        if(line->prev->hist) free(line->prev->hist);
        free(line->prev);
        line->prev = NULL;
      }
      if(line == txt && parsopt.must_start_redcode && !redcode_line_found) { //first non-blank line, ;redcode
        if(strncmp(line->data, ";redcode", 8)) { //discard lines before ;redcode
          txt = line->next;
          continue;
        }
        redcode_line_found = 1;
        set_standard_by_tag(txt, &line->data[8]);
        txt = line->next; //discard
      }
      else if(l == 0) { //other special/full line comments
        if(!strncmp(line->data, ";assert ", 7)) { //;assert lines will not be removed yet
          //no code inside
        }
        else {
          if(!strncmp(line->data, ";name ", 6)) {
            int len2 = line->len - 6;
            if(w->name != NULL) {
              parser_error(line, "multiple ;name lines present");
              freetext(txt);
              clearwarrior(w);
              return NULL;
            }
            w->name = (char*) malloc(len2+1);
            strcpy(w->name, &line->data[6]);
          }
          else if(!strncmp(line->data, ";author ", 7)) {
            int len2 = line->len - 7;
            if(w->author != NULL) {
              parser_error(line, "multiple ;author lines present");
              freetext(txt);
              clearwarrior(w);
              return NULL;
            }
            w->author = (char*) malloc(len2+1);
            strcpy(w->author, &line->data[7]);
          }
          else if(!strncmp(line->data, ";version ", 8)) {
            int len2 = line->len - 8;
            if(w->version != NULL) {
              parser_error(line, "multiple ;version lines present");
              freetext(txt);
              clearwarrior(w);
              return NULL;
            }
            w->version = (char*) malloc(len2+1);
            strcpy(w->version, &line->data[8]);
          }
          else if(!strncmp(line->data, ";date ", 5)) {
            int len2 = line->len - 5;
            if(w->date != NULL) {
              parser_error(line, "multiple ;date lines present");
              freetext(txt);
              clearwarrior(w);
              return NULL;
            }
            w->date = (char*) malloc(len2+1);
            strcpy(w->date, &line->data[5]);
          }
          else if(!strncmp(line->data, ";strategy ", 10)) {
            int len2 = line->len - 9;
            if(w->strategy != NULL) {
              int len3 = strlen(w->strategy);
              w->strategy = (char*) realloc(w->strategy, len3 + len2 + 2);
              w->strategy[len3] = '\n';
              strcpy(&w->strategy[len3+1], &line->data[10]);
            }
            else {
              w->strategy = (char*) malloc(len2+1);
              strcpy(w->strategy, &line->data[10]);
            }
          }
          if(txt == line) txt = line->next; //mark for later removal
          else { //remove now
            line = delline(line);
          }
        }
      }
      else { //normal lines
        line->data = (char*) realloc(line->data, l+1);
        line->data[l] = '\0';
        line->len = l;
      }
    }
    if(txt == NULL) {
      if(parsopt.must_start_redcode && !redcode_line_found)
        parser_error(NULL, "no ;redcode line found");
      else parser_error(NULL, "file consists entirely of comments");
      freetext(txt);
      clearwarrior(w);
      return NULL;
    }

    LINE* first = (LINE*) malloc(sizeof(LINE)); //add blank line at start
    first->len = 0;
    first->data = NULL;
    first->nhist = 0;
    first->hist = NULL;
    first->prev = NULL;
    first->next = txt;
    txt->prev = first;
    txt = first;
  }
  if(init_parser()) { //initialize dictionaries, check settings
    freetext(txt);
    return NULL;
  }
  { //first pass parsing
    /**
    FIRST PASS
    ==========

    ##Each line:
    1. Expand constants (except CURLINE)
    2. Process EQUs
    3. Evaluate expressions (+ variables, CURLINE and labels)
    4. Get labels
    5. Process FORs

    ##Each line inside FOR block:
    - Expand constants (preprocessing).

    Then for each line in each iteration of the loop,
    stringify the loop counter and place the line
    appropriately.
    Finally, return to the first expanded
    line of the loop, and keep processing as usual.

    + After an EQU, check if multiline.
    + If the line is an instruction, increment CURLINE.
    - If the line is an ;assert, check it.
    - Also detect PIN and ORG. Finish at END.
    */
    if(parsopt.do_first_pass) {
      allow_forward_refs = 1;
      CURLINE = 0;
      LINE* line;
      FORLINEIN(txt, line) {
        if(!line->len) continue; //skip empty lines
        { //Expand constants
          if(expand_constants(line)) {
            freetext(txt);
            return NULL;
          }
        }
        { //Process EQUs
          if(parsopt.allow_pseudo_equ && line->len && line->data[0] != ';') {
            int c;
            int labstart = -1; int labend = -1; int equstart = -1; int parmstart = -1; int parmend = -1;
            for(c = 0; c <= line->len; ++c) {
              char ch = line->data[c];
              if(isalnum(ch)) {
                if(c == 0) labstart = 0; //label starts at the beginning of the line
                else if(labstart == 0 && labend < 0) { //part of the label
                  continue;
                }
                else if(equstart >= 0) { //part of the possible EQU
                  continue;
                }
                else if(labend >= 0 && equstart < 0) { //start of possible EQU
                  equstart = c;
                }
                else { //this is not an EQU line
                  equstart = -1;
                  break;
                }
              }
              else {
                if(labstart == 0 && labend < 0) { //end of label
                  labend = c;
                }
                else if(equstart >= 0) { //end of possible EQU
                  if(c == equstart + 3 && toupper(line->data[c-3]) == 'E'
                                       && toupper(line->data[c-2]) == 'Q'
                                       && toupper(line->data[c-1]) == 'U') { //EQU!
                    break;
                  }
                  else { //this is not an EQU line
                    equstart = -1;
                    break;
                  }
                }
                if(isblank(ch)) { //just plain space
                  continue;
                }
                else if(ch == '(') { //parameters
                  parmstart = c;
                  for(; ; ++c) {
                    ch = line->data[c];
                    if(ch == ')') break;
                    else if(ch == '\0') {
                      parser_error(line, "currently, a function-like macro's parameter list must lie within a single line.");
                      freetext(txt);
                      return NULL;
                    }
                  }
                  ++c;
                  parmend = c;
                }
                else if(ch == '\0') { //end of line
                  equstart = -1;
                  break;
                }
                else { //other characters
                  equstart = -1;
                  break;
                }
              }
            }
            //now if EQU, equstart >= 0, labstart/end are set, and parmstart/end may be set
            if(equstart >= 0) { //is an EQU
              //c is at equstart + 3
              char* label = (char*) malloc(labend-labstart+1);
              char* lAbel;
              strncpy(label, &line->data[labstart], labend-labstart);
              label[labend-labstart] = '\0';
              if(!validate_label(line, label)) { //will give error/warning as appropriate, may turn label to uppercase
                free(label);
                freetext(txt);
                return NULL;
              }
              if(get_dictionary(&d_constants, label) != NULL) {
                parser_error(line, "redefinition of constant \"%s\".", label);
                parser_note(get_dictionary(&d_constants, label), "previus definition:");
                free(label);
                freetext(txt);
                return NULL;
              }
              if(get_dictionary(&d_variables, label) != NULL) {
                parser_error(line, "constant \"%s\" has same name as a variable.", label);
                free(label);
                freetext(txt);
                return NULL;
              }
              if(get_dictionary(&d_labels, label) != NULL) {
                parser_error(line, "constant \"%s\" has same name as a label.", label);
                free(label);
                freetext(txt);
                return NULL;
              }
              if(parsopt.labels_case_sens) { //for checking against (pseudo-)opcodes
                lAbel = (char*) malloc(labend-labstart+1);
                int c2; for(c2 = 0; label[c2] != '\0'; ++c2) lAbel[c2] = toupper(label[c2]);
                lAbel[labend-labstart] = '\0';
              }
              if(get_dictionary(&d_opcodes, label) != NULL) {
                parser_error(line, "constant \"%s\" has same name as an opcode.", label);
                free(label);
                if(parsopt.labels_case_sens) free(lAbel);
                freetext(txt);
                return NULL;
              }
              if(get_dictionary(&d_pseudoops, label) != NULL) {
                parser_error(line, "constant \"%s\" has same name as a pseudo-opcode.", label);
                free(label);
                if(parsopt.labels_case_sens) free(lAbel);
                freetext(txt);
                return NULL;
              }
              if(parmstart >= 0) { //function-like
                if(!parsopt.exp_funclike_equ) {
                  parser_error(line, "function-like macros are not enabled on current parser settings.");
                  free(label);
                  if(parsopt.labels_case_sens) free(lAbel);
                  freetext(txt);
                  return NULL;
                }
                int k; int namstart = -1; int namend = -1;
                ++parmstart; --parmend; //don't include parentheses
                for(k = parmstart; k < parmend; ++k) { //check parameter list syntax
                  char ch = line->data[k];
                  if(isalnum(ch)) { //part of a parameter name
                    if(namstart < 0) { //start of name
                      namstart = k;
                    }
                    else if(namend < 0) { //part of name
                      continue;
                    }
                    else {
                      parser_error(line, "expected ',' or ')' after parameter name, but identifier found.");
                      free(label);
                      if(parsopt.labels_case_sens) free(lAbel);
                      freetext(txt);
                      return NULL;
                    }
                  }
                  else { //not part
                    if(namstart >= 0) { //end of name
                      namend = k;
                      char* name = (char*) malloc(namend-namstart+1);
                      strncpy(name, &line->data[namstart], namend-namstart);
                      name[namend-namstart] = '\0';
                      if(!validate_label(line, name)) {
                        free(label);
                        if(parsopt.labels_case_sens) free(lAbel);
                        free(name);
                        freetext(txt);
                        return NULL;
                      }
                      free(name);
                    }
                    if(ch == ',') {
                      if(namend >= 0) { //comma after parameter name
                        namstart = namend = -1;
                      }
                      else {
                        parser_error(line, "expected parameter name, but ',' found.");
                        free(label);
                        if(parsopt.labels_case_sens) free(lAbel);
                        freetext(txt);
                        return NULL;
                      }
                    }
                    else if(isblank(ch)) {
                      continue;
                    }
                    else { //another character
                      parser_error(line, "stray '%c' in parameter list.", ch);
                      free(label);
                      if(parsopt.labels_case_sens) free(lAbel);
                      freetext(txt);
                      return NULL;
                    }
                  }
                } //for
                char* parms = (char*) malloc(parmend-parmstart+1);
                strncpy(parms, &line->data[parmstart], parmend-parmstart);
                parms[parmend-parmstart] = '\0';
                LINE* parmsline = (LINE*) malloc(sizeof(LINE));
                parmsline->data = parms;
                parmsline->len = strlen(parms);
                parmsline->hist = NULL;
                parmsline->nhist = 0;
                parmsline->prev = NULL;
                parmsline->next = NULL;
                set_dictionary(&d_signatures, restricted_copy(label), parmsline);
              } //if function-like
              //start retrieving value
              for(; isblank(line->data[c]); ++c) ; //remove leading whitespace
              int valstart = c; int valend = c;
              for( ; c < line->len; ++c) { //remove trailing whitespace
                if(!isblank(line->data[c])) valend = c;
              }
              ++valend;
              //now the EQU line will be the value for dictionary
              line->len = valend-valstart;
              memmove(line->data, &line->data[valstart], valend-valstart);
              line->data = (char*) realloc(line->data, valend-valstart+1);
              line->data[valend-valstart] = '\0';
              //remove EQU line
              LINE* prev = line->prev; //the line before all the block
              prev->next = line->next;
              line->next->prev = prev;
              line->prev = NULL;
              LINE* cur = line->next; //the current line
              LINE* value = line; //alias for the value
              LINE* lastv = line; //last line in value
              line->next = NULL;
              for(; cur != NULL; cur = cur->next) { //multiline EQUs
                if(!cur->len) { //empty line (for some reason), end of EQU block
                  break;
                }
                if(cur->data[0] == ';') { //;assert lines are just comments, ignore them
                  continue;
                }
                else if(toupper(cur->data[0]) == 'E'
                     && toupper(cur->data[1]) == 'Q'
                     && toupper(cur->data[2]) == 'U') { //one more EQU line
                  if(!parsopt.allow_multiline_equ) {
                    parser_error(line, "multiline EQUs are disabled on current parser settings.");
                    freetext(txt);
                    freetext(value);
                    free(label);
                    return NULL;
                  }
                  //now this line will become a new value line
                  int k; int end = -1;
                  for(k = 3; isblank(cur->data[k]); ++k) ; //discard leading whitespace
                  int start = k;
                  for(k = cur->len-1; isblank(cur->data[k]); --k) ; //discard trailing whitespace
                  ++k;
                  end = k;
                  cur->len = end-start;
                  memmove(cur->data, &cur->data[start], end-start);
                  cur->data = (char*) realloc(cur->data, end-start+1);
                  cur->data[end-start] = '\0';
                  //remove line and append to value
                  LINE* bef = cur->prev;
                  bef->next = cur->next;
                  if(cur->next) cur->next->prev = cur->prev;
                  cur->prev = lastv;
                  cur->next = NULL;
                  lastv->next = cur;
                  lastv = cur;
                  cur = bef;
                }
                else { //end of EQU block
                  break;
                }
              }
              set_dictionary(&d_constants, label, value);
              free(label);
              if(parsopt.labels_case_sens) free(lAbel);
              line = prev; //continue on first line after the line before the block
              continue;
            }
          }
        }
        int line_is = LTYPE_OTHER; //1 = normal, 2 = FOR, 3 = ;assert, 0 = other (no expressions)
        { //Evaluate expressions (+variables and labels)
          int exp_base = 0; //the minimum position for an expression
          { //Identify line type
            line_is = identify_line_type(line, &exp_base);
          } //identify line type
          if((line_is == LTYPE_NORMAL) || (line_is == LTYPE_FOR) || (line_is == LTYPE_ASSERT)) { //line types which can contain expressions
            if(expand_expressions(line, line_is, exp_base)) { //an error occurred
              freetext(txt);
              return NULL;
            }
          }
        }
        { //Get labels
          if(line_is != 2) { //exclude FOR counters
            int c; int labstart = -1; int labend = -1; int colon = 0;
            for(c = 0; c <= line->len; ++c) { //get all labels
              char ch = line->data[c];
              if(isalnum(ch)) {
                if(labstart < 0) {
                  colon = 0;
                  labstart = c;
                }
              }
              else {
                if(ch == ':') {
                  if(labstart >= 0 && labend < 0) {
                    colon = 1;
                    labend = c;
                  }
                  else break;
                }
                else if(isblank(ch) || ch == '\0') {
                  if(labstart >= 0 && labend < 0) {
                    labend = c;
                  }
                }
                else break;
              }
              if(labend >= 0) { //label found
                char* label = (char*) malloc(labend-labstart+1);
                char* lAbel;
                strncpy(label, &line->data[labstart], labend-labstart);
                label[labend-labstart] = '\0';
                if(!validate_label(line, label)) {
                  free(label);
                  if(parsopt.labels_case_sens) free(lAbel);
                  freetext(txt);
                  return NULL;
                }
                if(get_dictionary(&d_constants, label) != NULL) {
                  parser_error(line, "label \"%s\" has the same name as a constant.", label);
                  parser_note(get_dictionary(&d_constants, label), "previously defined:");
                  free(label);
                  freetext(txt);
                  return NULL;
                }
                if(get_dictionary(&d_variables, label) != NULL) {
                  parser_error(line, "label \"%s\" has the same name as a variable.", label);
                  free(label);
                  freetext(txt);
                  return NULL;
                }
                if(get_dictionary(&d_labels, label) != NULL) {
                  parser_error(line, "label \"%s\" has the same name as another label.", label);
                  free(label);
                  freetext(txt);
                  return NULL;
                }
                if(parsopt.labels_case_sens) { //for checking against (pseudo-)opcodes
                  lAbel = (char*) malloc(labend-labstart+1);
                  int c2; for(c2 = 0; label[c2] != '\0'; ++c2) lAbel[c2] = toupper(label[c2]);
                  lAbel[labend-labstart] = '\0';
                }
                else lAbel = label;
                if(get_dictionary(&d_opcodes, lAbel) != NULL) {
                  free(label);
                  if(parsopt.labels_case_sens) free(lAbel);
                  break;
                }
                if(get_dictionary(&d_pseudoops, lAbel) != NULL) {
                  free(label);
                  if(parsopt.labels_case_sens) free(lAbel);
                  break;
                }
                LINE* value = (LINE*) malloc(sizeof(LINE));
                value->prev = value->next = NULL;
                value->hist = NULL;
                value->nhist = 0;
                value->data = (char*) malloc(sizeof(EXP_VAL));
                ((EXP_VAL*)value->data)->a = CURLINE;
                ((EXP_VAL*)value->data)->b = 0;
                value->len = sizeof(EXP_VAL);
                set_dictionary(&d_labels, label, value);
                c -= strlen(label) + ((colon)? 1 : 0);
                free(label);
                if(parsopt.labels_case_sens) free(lAbel);
                LINE* empty = (LINE*) malloc(sizeof(LINE));
                empty->prev = empty->next = NULL;
                empty->hist = NULL;
                empty->data = NULL;
                empty->nhist = empty->len = 0;
                text_substitute_label(line, labstart, labend + ((colon)? 1 : 0), empty);
                labstart = labend = -1;
              }
            }
          }
        }
        { //Process FORs
          if(line_is == 2) { //is a FOR
            int c;
            for(c = 0; isblank(line->data[c]); ++c) ; //skip leading whitespace
            int countstart = c; //counter label/FOR keyword
            for(; isalnum(line->data[c]); ++c) ;
            int countend = c;
            if(countstart >= countend) {
              parser_error(line, "expected counter or 'FOR' at start of line, but '%c' found.", line->data[c]);
              freetext(txt);
              return NULL;
            }
            char* word = malloc(countend-countstart+1);
            strncpy(word, &line->data[countstart], countend-countstart);
            word[countend-countstart] = '\0';
            if(!validate_label(line, word)) {; //may turn it to uppercase
              free(word);
              freetext(txt);
              return NULL;
            }

            char* wOrd;
            if(parsopt.labels_case_sens) {
              wOrd = (char*) malloc(countend-countstart+1);
              int c2; for(c2 = 0; word[c2] != '\0'; ++c2) wOrd[c2] = toupper(word[c2]);
              wOrd[countend-countstart] = '\0';
            }
            else wOrd = word;
            char* counter = NULL; //counter, if it is present
            if(strcmp(wOrd, "FOR")) { //this is a counter
              //make sure it's followed by FOR
              for(; isblank(line->data[c]); ++c) ; //skip whitespace
              if(toupper(line->data[c]) != 'F'
              || toupper(line->data[++c]) != 'O'
              || toupper(line->data[++c]) != 'R'
              || (!isblank(line->data[++c]))) {
                if(line->data[c] == '\0') parser_error(line, "FOR keyword must be followed by count, but end of line was found.");
                else parser_error(line, "multiple counters are not allowed in a FOR statement.");
                free(word);
                if(parsopt.labels_case_sens) free(wOrd);
                return NULL;
              }
              counter = restricted_copy(word);

              if(get_dictionary(&d_constants, word) != NULL) {
                parser_error(line, "counter \"%s\" has the same name as a constant.", word);
                parser_note(get_dictionary(&d_constants, word), "previously defined:");
                free(word);
                if(parsopt.labels_case_sens) free(wOrd);
                return NULL;
              }
              if(get_dictionary(&d_labels, word) != NULL) {
                parser_error(line, "counter \"%s\" has the same name as a label.", word);
                free(word);
                if(parsopt.labels_case_sens) free(wOrd);
                return NULL;
              }
              if(get_dictionary(&d_opcodes, wOrd) != NULL) {
                parser_error(line, "counter \"%s\" has the same name as an opcode.", word);
                free(word);
                if(parsopt.labels_case_sens) free(wOrd);
                return NULL;
              }
              if(get_dictionary(&d_pseudoops, wOrd) != NULL) {
                parser_error(line, "counter \"%s\" has the same name as a pseudo-opcode.", word);
                free(word);
                if(parsopt.labels_case_sens) free(wOrd);
                return NULL;
              }
            }
            free(word);
            if(parsopt.labels_case_sens) free(wOrd);
            //now c is positioned just after the FOR keyword
            for(; isblank(line->data[c]); ++c) ; //skip whitespace
            int number = 0;
            int state = 0;
            for(;; ++c) {
              char ch = line->data[c];
              if(isblank(ch)) {
                if(state == 1) state = 2;
              }
              else {
                if(isdigit(ch)) {
                  number *= 10;
                  number += ch - '0';
                }
                else if(ch == '\0') break;
                else {
                  parser_error(line, "FOR expression must contain no forward references and evaluate to a single positive number.");
                  if(counter) free(counter);
                  freetext(txt);
                  return NULL;
                }
                if(state == 0) state = 1;
                if(state == 2) {
                  parser_error(line, "FOR expression must contain no forward references and evaluate to a single positive number.");
                  if(counter) free(counter);
                  freetext(txt);
                  return NULL;
                }
              }
            }
            if(!state) {
              parser_error(line, "expected expression after FOR keyword, found end of line.");
              if(counter) free(counter);
              freetext(txt);
              return NULL;
            }

            //we have the number of iterations and possibly a counter
            //fetch the lines to expand
            LINE* prev = line->prev; //the line before all the block
            LINE* loop = NULL; //the (preprocessed) lines to expand
            LINE* cur = line->next; //current line
            LINE* cloop = NULL; //current line in loop
            line = delline(line); //processing will continue at the start of the block
            int nesting = 1; //FOR blocks can be nested
            for(;;) { //fetch block
              //preprocess it
              if(parsopt.exp_for_preprocess) {
                if(expand_constants(line)) {
                  free(word);
                  if(parsopt.labels_case_sens) free(wOrd);
                  if(loop != NULL) freetext(loop);
                  freetext(txt);
                  return NULL;
                }
              }
              //look for ROF
              int k; int wstart = -1; int wend = -1;
              int ltype = LTYPE_OTHER; //line type
              int preceded = 0; //word is preceded by nonspace
              for(k = 0; k <= cur->len; ++k) {
                char ch = cur->data[k];
                if(isalnum(ch)) {
                  if(wstart < 0) wstart = k;
                }
                else {
                  if(wstart >= 0) {
                    wend = k;
                    char* word = (char*) malloc(wend-wstart+1);
                    strncpy(word, &cur->data[wstart], wend-wstart);
                    word[wend-wstart] = '\0';
                    int c2; for(c2 = 0; word[c2] != '\0'; ++c2) word[c2] = toupper(word[c2]);
                    if(!strcmp(word, "ROF")) {
                      if(ltype != LTYPE_OTHER) {
                        parser_error(cur, "two or more pseudo-opcodes in the same line.");
                        free(word);
                        if(loop != NULL) freetext(loop);
                        freetext(txt);
                        return NULL;
                      }
                      if(preceded) {
                        parser_error(cur, "ROF keyword can only be preceded by whitespace.");
                        free(word);
                        if(loop != NULL) freetext(loop);
                        freetext(txt);
                        return NULL;
                      }
                      ltype = LTYPE_ROF;
                    }
                    else if(!strcmp(word, "FOR")) {
                      if(ltype != LTYPE_OTHER) {
                        parser_error(cur, "two or more pseudo-opcodes in the same line.");
                        free(word);
                        if(loop != NULL) freetext(loop);
                        freetext(txt);
                        return NULL;
                      }
                      ltype = LTYPE_FOR;
                    }
                    else if(!strcmp(word, "END")) {
                      if(ltype != LTYPE_OTHER) {
                        parser_error(cur, "two or more pseudo-opcodes in the same line.");
                        free(word);
                        if(loop != NULL) freetext(loop);
                        freetext(txt);
                        return NULL;
                      }
                      //the following code would detect an END inside the FOR block
                      //which is not the correct behavior and breaks some warriors
                      /*parser_error(cur, "unterminated FOR block, expected ROF but found END.");
                      free(word);
                      if(loop != NULL) freetext(loop);
                      freetext(txt);
                      return NULL;*/
                    }
                    else {
                      preceded = 1;
                    }
                    free(word);
                    wstart = wend = -1;
                  }
                  if(ch == '\0') break;
                  if(isblank(ch)) {
                    //ignore it
                  }
                  else {
                    preceded = 1;
                  }
                }
              }
              switch(ltype) {
                case LTYPE_FOR:
                  ++nesting;
                  break;
                case LTYPE_ROF:
                  --nesting;
                  break;
              }
              //manage linked list
              cur->prev->next = cur->next;
              if(cur->next != NULL) cur->next->prev = cur->prev;
              if(nesting) { //not the last one
                cur->prev = cloop;
                if(cloop != NULL) cloop->next = cur;
                else loop = cur;
                LINE* tmp = cur->next;
                cur->next = NULL;
                cloop = cur;
                cur = tmp;
                if(cur == NULL) {
                  parser_error(cur, "unterminated FOR block, expected FOR but found end of file.");
                  if(loop != NULL) freetext(loop);
                  freetext(txt);
                  return NULL;
                }
              }
              else { //last ROF
                cur->prev = NULL;
                cur->next = NULL;
                freetext(cur);
                break;
              }
            }

            int iteration; //the actual counter
            //will add all lines after prev
            for(iteration = 1; iteration <= number; ++iteration) { //expand the loop
              LINE* sl;
              FORLINEIN(loop, sl) { //if loop == NULL, will do nothing
                //copy line in place
                LINE* dl = copyline(sl);
                dl->prev = prev;
                dl->next = prev->next;
                prev->next = dl;
                if(dl->next != NULL) dl->next->prev = dl;
                prev = dl;
                //stringify & concatenate counter
                if(counter != NULL) {
                  char* val = (char*) malloc(snprintf(NULL, 0, "%02d", iteration)+1);
                  sprintf(val, "%02d", iteration);
                  LINE* lval = (LINE*) malloc(sizeof(LINE));
                  lval->data = val;
                  lval->len = strlen(val);
                  lval->hist = NULL;
                  lval->nhist = 0;
                  lval->prev = lval->next = NULL;

                  int c; int labstart = -1; int labend;
                  for(c = 0; c <= dl->len; ++c) {
                    if(isalnum(dl->data[c])) {
                      if(labstart < 0) labstart = c;
                    }
                    else {
                      if(labstart >= 0) {
                        labend = c;
                        if(!strncmp(&dl->data[labstart], counter, labend-labstart)) {
                          //concatenate by overwriting the & token
                          if(parsopt.allow_concatenate && (labstart >= 2) && (dl->data[labstart-1] == '&') && isalnum(dl->data[labstart-2])) {
                            --labstart;
                          }
                          //substitute counter
                          text_substitute_label(dl, labstart, labend, lval);
                        }
                        labstart = -1;
                      }
                    }
                  } //for
                }
              }
            }
            if(loop != NULL) freetext(loop);
            continue; //straight to parsing start of block
          }
        }
        { //Other line types
          switch(line_is) {
            case LTYPE_END: {
              //get label
              int c;
              for(c = 0; isblank(line->data[c]); ++c) ;
              if(toupper(line->data[c++]) != 'E'
              || toupper(line->data[c++]) != 'N'
              || toupper(line->data[c++]) != 'D') {
                parser_error(line, "END keyword must not be preceded by label.");
                freetext(txt);
                return NULL;
              }
              for(; isblank(line->data[c]); ++c) ;
              if(line->data[c] != '\0') {
                if(!parsopt.allow_end_operand) {
                  parser_error(line, "label operand after END currently not enabled.");
                  freetext(txt);
                  return NULL;
                }
                int opstart = c;
                for(; isalnum(line->data[c]); ++c) ; //to end of label
                int opend = c;
                for(; isblank(line->data[c]); ++c) ;
                if(line->data[c] == '\0') { //single label
                  char* oper = (char*) malloc(opend-opstart+1);
                  strncpy(oper, &line->data[opstart], opend-opstart);
                  oper[opend-opstart] = '\0';
                  validate_label(line, oper); //may turn it to uppercase
                  LINE* org = get_dictionary(&d_labels, oper);
                  if(org == NULL) {
                    parser_error(line, "undefined label \"%s\".", oper);
                    free(oper);
                    freetext(txt);
                    return NULL;
                  }
                  if(org_is_set && (w->org != ((EXP_VAL*)org->data)->a)) {
                    parser_error(line, "conflicting start labels.");
                    free(oper);
                    freetext(txt);
                    return NULL;
                  }
                  w->org = ((EXP_VAL*)org->data)->a;
                  org_is_set = 1;
                  free(oper);
                }
                else { //expression
                  if(!parsopt.exp_org_expression) {
                    parser_error(line, "expressions after ORG and END are currently not enabled.");
                    freetext(txt);
                    return NULL;
                  }
                  opend = line->len;
                  int tmp = allow_forward_refs;
                  allow_forward_refs = 0;
                  EXP_VAL v = compute_expression_rec(line->data, opstart, opend, line, line_is);
                  allow_forward_refs = tmp;
                  if(v.b == VAL_B_INVALID) {
                    freetext(txt);
                    return NULL;
                  }
                  else if(v.b != 1) {
                    parser_error(line, "start label expressions must evaluate to valid labels. This expression evaluates to (%d - (%d)*CURLINE).", v.a, v.b);
                    freetext(txt);
                    return NULL;
                  }
                  if(org_is_set && (w->org != v.a)) {
                    parser_error(line, "conflicting start labels.");
                    freetext(txt);
                  }
                  w->org = v.a;
                  org_is_set = 1;
                }
              }
              //delete this line and all following lines
              LINE* prev = line->prev;
              line->prev = NULL;
              freetext(line);
              prev->next = NULL;
              line = prev;
              break; }
            case LTYPE_ASSERT: {
              int c;
              for(c = 8; isblank(line->data[c]); ++c) ; //skip whitespace
              int valcheck = -1;
              for(; isdigit(line->data[c]); ++c) {
                if(line->data[c] != '0') valcheck = 1;
                else if(valcheck < 0) valcheck = 0;
              }
              for(; isblank(line->data[c]); ++c) ; //skip whitespace
              if(line->data[c] != '\0') { //does not evaluate to number
                if(parsopt.exp_assert_forward) { //leave it for later
                  break;
                }
                parser_error(line, "expression does not evaluate to a number, or contains forward references.");
                freetext(txt);
                return NULL;
              }
              if(valcheck < 0) { //no expression
                parser_error(line, ";assert must be followed by an expression.");
                freetext(txt);
                return NULL;
              }
              if(!valcheck) { //evaluates to 0
                if(parsopt.assert_give_error) {
                  parser_error(line, ";assert evaluates to 0.");
                  freetext(txt);
                  return NULL;
                }
                parser_warn(line, ";assert evaluates to 0.");
              }
              //remove this line
              line = delline(line); //continue on the next line
              continue; //outer loop
            }
            case LTYPE_PIN: {
              int c;
              for(c = 0; isblank(line->data[c]); ++c) ;
              if(toupper(line->data[c++]) != 'P'
              || toupper(line->data[c++]) != 'I'
              || toupper(line->data[c++]) != 'N') {
                parser_error(line, "PIN keyword must not be preceded by label.");
                freetext(txt);
                return NULL;
              }
              for(; isblank(line->data[c]); ++c) ; //skip whitespace
              if(line->data[c] == '\0') {
                parser_error(line, "PIN keyword must be followed by number%s.", (parsopt.exp_pin_expression)? " or expression" : "");
                freetext(txt);
                return NULL;
              }
              int val = 0;
              for(; isdigit(line->data[c]); ++c) {
                val *= 10;
                val += line->data[c] - '0';
              }
              for(; isblank(line->data[c]); ++c) ; //skip whitespace
              if(line->data[c] != '\0') {
                if(parsopt.exp_pin_expression) { //leave it for later
                  break;
                }
                parser_error(line, "expressions after PIN keyword are not enabled on current parser settings.");
                freetext(txt);
                return NULL;
              }
              if(pin_is_set && (w->pin != val)) {
                parser_error(line, "conflicting P-space identification numbers.");
                freetext(txt);
                return NULL;
              }
              pin_is_set = 1;
              w->haspin = 1;
              w->pin = val;
              //delete the PIN line
              line = delline(line);
              continue; //outer loop
            }
          } //switch
        }
        if(line_is == 1) ++CURLINE;
      }
    }
    else { //only detect END
      LINE* line;
      FORLINEIN(txt, line) {
        if(!line->len) continue; //skip empty lines
        int exp_base, line_type;
        line_type = identify_line_type(line, &exp_base);
        if(line_type == LTYPE_END) {
          line->prev->next = NULL;
          line->prev = NULL;
          freetext(line);
          break;
        }
      }
    }
  }
  { //second pass label replacement
    if(parsopt.do_second_pass) {
      allow_forward_refs = 0;
      CURLINE = 0;
      LINE* line;
      FORLINEIN(txt, line) {
        int line_is = LTYPE_OTHER;
        if(line->len) { //Expressions and constants
          if(expand_constants(line)) { //forward EQUs
            freetext(txt);
            return NULL;
          }
          int exp_base = 0; //the minimum position for an expression
          { //Identify line type
            line_is = identify_line_type(line, &exp_base);
          } //identify line type
          if((line_is == LTYPE_NORMAL) || (line_is == LTYPE_ASSERT) || (line_is == LTYPE_PIN)) { //forward labels
            if(expand_expressions(line, line_is, exp_base)) { //an error occurred
              freetext(txt);
              return NULL;
            }
          }
        }
        { //Other line types
          switch(line_is) {
            case LTYPE_ORG: {
              //get label
              int c;
              for(c = 0; isblank(line->data[c]); ++c) ;
              if(toupper(line->data[c++]) != 'O'
              || toupper(line->data[c++]) != 'R'
              || toupper(line->data[c++]) != 'G') {
                parser_error(line, "ORG keyword must not be preceded by label.");
                freetext(txt);
                return NULL;
              }
              for(; isblank(line->data[c]); ++c) ;
              if(line->data[c] != '\0') {
                int opstart = c;
                for(; isalnum(line->data[c]); ++c) ; //to end of label
                int opend = c;
                for(; isblank(line->data[c]); ++c) ;
                if(line->data[c] == '\0') { //single label
                  char* oper = (char*) malloc(opend-opstart+1);
                  strncpy(oper, &line->data[opstart], opend-opstart);
                  oper[opend-opstart] = '\0';
                  validate_label(line, oper); //may turn it to uppercase
                  LINE* org = get_dictionary(&d_labels, oper);
                  if(org == NULL) {
                    parser_error(line, "undefined label \"%s\".", oper);
                    free(oper);
                    freetext(txt);
                    return NULL;
                  }
                  if(org_is_set && (w->org != ((EXP_VAL*)org->data)->a)) {
                    parser_error(line, "conflicting start labels.");
                    free(oper);
                    freetext(txt);
                    return NULL;
                  }
                  w->org = ((EXP_VAL*)org->data)->a;
                  org_is_set = 1;
                  free(oper);
                }
                else { //expression
                  if(!parsopt.exp_org_expression) {
                    parser_error(line, "expressions after ORG and END are currently not enabled.");
                    freetext(txt);
                    return NULL;
                  }
                  opend = line->len;
                  int tmp = allow_forward_refs;
                  allow_forward_refs = 0;
                  EXP_VAL v = compute_expression_rec(line->data, opstart, opend, line, line_is);
                  allow_forward_refs = tmp;
                  if(v.b == VAL_B_INVALID) {
                    freetext(txt);
                    return NULL;
                  }
                  else if(v.b != 1) {
                    parser_error(line, "start label expressions must evaluate to valid labels. This expression evaluates to (%d - (%d)*CURLINE).", v.a, v.b);
                    freetext(txt);
                    return NULL;
                  }
                  if(org_is_set && (w->org != v.a)) {
                    parser_error(line, "conflicting start labels.");
                    freetext(txt);
                  }
                  w->org = v.a;
                  org_is_set = 1;
                }
              }
              else { //no operand
                parser_error(line, "ORG keyword must be followed by a label%s.", (parsopt.exp_org_expression)? " or expression" : "");
                freetext(txt);
                return NULL;
              }
              //delete this line
              line = delline(line);
              continue; //outer loop
            }
            case LTYPE_ASSERT: {
              int c;
              for(c = 8; isblank(line->data[c]); ++c) ; //skip whitespace
              int valcheck = -1;
              for(; isdigit(line->data[c]); ++c) {
                if(line->data[c] != '0') valcheck = 1;
                else if(valcheck < 0) valcheck = 0;
              }
              for(; isblank(line->data[c]); ++c) ; //skip whitespace
              if(line->data[c] != '\0') { //does not evaluate to number
                parser_error(line, "expression does not evaluate to a number.");
                freetext(txt);
                return NULL;
              }
              if(valcheck < 0) { //no expression
                parser_error(line, ";assert must be followed by an expression.");
                freetext(txt);
                return NULL;
              }
              if(!valcheck) { //evaluates to 0
                if(parsopt.assert_give_error) {
                  parser_error(line, ";assert evaluates to 0.");
                  freetext(txt);
                  return NULL;
                }
                parser_warn(line, ";assert evaluates to 0.");
              }
              //remove this line
              line = delline(line); //continue on the next line
              continue; //outer loop
            }
            case LTYPE_PIN: {
              int c;
              for(c = 0; isblank(line->data[c]); ++c) ;
              if(toupper(line->data[c++]) != 'P'
              || toupper(line->data[c++]) != 'I'
              || toupper(line->data[c++]) != 'N') {
                parser_error(line, "PIN keyword must not be preceded by label.");
                freetext(txt);
                return NULL;
              }
              for(; isblank(line->data[c]); ++c) ; //skip whitespace
              if(line->data[c] == '\0') {
                parser_error(line, "PIN keyword must be followed by number%s.", (parsopt.exp_pin_expression)? " or expression" : "");
                freetext(txt);
                return NULL;
              }
              int val = 0;
              for(; isdigit(line->data[c]); ++c) {
                val *= 10;
                val += line->data[c] - '0';
              }
              for(; isblank(line->data[c]); ++c) ; //skip whitespace
              if(line->data[c] != '\0') {
                parser_error(line, "expressions after PIN keyword are not enabled on current parser settings.");
                freetext(txt);
                return NULL;
              }
              if(pin_is_set && (w->pin != val)) {
                parser_error(line, "conflicting P-space identification numbers.");
                freetext(txt);
                return NULL;
              }
              pin_is_set = 1;
              w->haspin = 1;
              w->pin = val;
              //delete the PIN line
              line = delline(line);
              continue; //outer loop
            }
          }
        }
        if(line_is == 1) ++CURLINE;
      } //for
    }
  }
  { //check syntax, format load file
    LINE* line;
    LINE* last = NULL;
    CURLINE = 0;
    FORLINEIN(txt, line) {
      if(!line->len) { //blank line
        if(line->prev != NULL) line = delline(line); //if in the middle, delete line
        continue;
      }
      int c;
      for(c = 0; isblank(line->data[c]); ++c) ; //skip whitespace
      if((line->data[c] == '\0') && (line->prev != NULL)) { //blank line in the middle
        line = delline(line); //delete line and continue on previous
        continue;
      }
      if(!isalpha(line->data[c])) {
        parser_error(line, "unrecognized or missing opcode.");
        freetext(txt);
        return NULL;
      }
      uint32_t s_op = toupper(line->data[c++]); s_op <<= 8;
      s_op |= toupper(line->data[c++]); s_op <<= 8;
      s_op |= toupper(line->data[c++]);

      int def_nops = 0;
      switch(s_op) { //number of operands used by the instruction
        #ifdef O_DAT
        case 0x444154:
        #endif
        #ifdef O_JMP
        case 0x4a4d50:
        #endif
        #ifdef O_SPL
        case 0x53504c:
        #endif
        #ifdef O_PCT
        case 0x504354:
        #endif
        #ifdef O_STS
        case 0x535453:
        #endif
        #if (defined(EXT_XCH_a) || (STANDARD == 89))
        case 0x584348:
        #endif
          def_nops = 1;
          break;

        #ifdef O_MOV
        case 0x4d4f56:
        #endif
        #ifdef O_ADD
        case 0x414444:
        #endif
        #ifdef O_SUB
        case 0x535542:
        #endif
        #ifdef O_JMZ
        case 0x4a4d5a:
        #endif
        #ifdef O_CMP
        case 0x434d50:
        #endif
        #ifdef O_JMN
        case 0x4a4d4e:
        #endif
        #ifdef O_DJN
        case 0x444a4e:
        #endif
        #ifdef O_SLT
        case 0x534c54:
        #endif
        #ifdef O_MUL
        case 0x4d554c:
        #endif
        #ifdef O_DIV
        case 0x444956:
        #endif
        #ifdef O_MOD
        case 0x4d4f44:
        #endif
        #ifdef O_SEQ
        case 0x534551:
        #endif
        #ifdef O_SNE
        case 0x534e45:
        #endif
        #ifdef O_NOP
        case 0x4e4f50:
        #endif
        #ifdef O_LDP
        case 0x4c4450:
        #endif
        #ifdef O_STP
        case 0x535450:
        #endif
        #ifdef O_DJZ
        case 0x444a5a:
        #endif
        #if defined(EXT_XCH_b)
        case 0x584348:
        #endif
        case 0x585858: //XXX
          def_nops = 2;
          break;
        default:
          parser_error(line, "unknown opcode %c%c%c.", (s_op >> 16) & 0xFF, (s_op >> 8) & 0xFF, s_op & 0xFF);
          freetext(txt);
          return NULL;
      }

      uint16_t s_md = 0;
      if(line->data[c] == '.') {
        if(!parsopt.allow_modifiers) {
          parser_error(line, "instruction modifiers are not enabled on current parser settings.");
          freetext(txt);
          return NULL;
        }
        ++c;
        s_md |= toupper(line->data[c++]);
        if(isalpha(line->data[c])) {
          s_md <<= 8;
          s_md |= toupper(line->data[c++]);
        }
      }
      for(; isblank(line->data[c]); ++c) ; //skip whitespace
      char s_aA = 0, s_aB = 0;
      int v_A, v_B;
      if(line->data[c] == '\0') { //no operands
        if(!parsopt.exp_allow_omit_both) {
          parser_error(line, "omitting both fields of an instruction is not enabled on current parser settings.");
          freetext(txt);
          return NULL;
        }
        //both operands set to #0
        s_aA = s_aB = '#';
        v_A = v_B = 0;
      }
      else { //1 or 2 operands
        s_aA = line->data[c++];
        switch(s_aA) {
          case '#':
            if(!parsopt.addr_immediate) {
              parser_error(line, "immediate address mode not enabled on current parser settings.");
              freetext(txt);
              return NULL;
            }
            break;
          case '$':
            if(!parsopt.addr_expl_direct) {
              parser_error(line, "explicit '$' for direct address mode not enabled on current parser settings.");
              freetext(txt);
              return NULL;
            }
            break;
          case '@':
            if(!parsopt.addr_bindirect) {
              parser_error(line, "B-field indirect address mode not enabled on current parser settings.");
              freetext(txt);
              return NULL;
            }
            break;
          case '*':
            if(!parsopt.addr_aindirect) {
              parser_error(line, "A-field indirect address mode not enabled on current parser settings.");
              freetext(txt);
              return NULL;
            }
            break;
          case '<':
            if(!parsopt.addr_bdecrement) {
              parser_error(line, "B-field auto-/predecrement address mode not enabled on current parser settings.");
              freetext(txt);
              return NULL;
            }
            break;
          case '>':
            if(!parsopt.addr_bincrement) {
              parser_error(line, "B-field postincrement address mode not enabled on current parser settings.");
              freetext(txt);
              return NULL;
            }
            break;
          case '{':
            if(!parsopt.addr_adecrement) {
              parser_error(line, "A-field predecrement address mode not enabled on current parser settings.");
              freetext(txt);
              return NULL;
            }
            break;
          case '}':
            if(!parsopt.addr_aincrement) {
              parser_error(line, "A-field postincrement address mode not enabled on current parser settings.");
              freetext(txt);
              return NULL;
            }
            break;
          default:
            if(!isdigit(s_aA)) {
              parser_error(line, "expected address mode, digit or '-', got %c.", s_aA);
              freetext(txt);
              return NULL;
            }
            //fall through
          case '-': case '+':
            s_aA = '$';
            --c;
            break;
        }
        for(; isblank(line->data[c]); ++c) ; //skip whitespace
        int neg; //negative
        if(line->data[c] == '-') {
          neg = 1;
          ++c;
          for(; isblank(line->data[c]); ++c) ; //skip whitespace
        }
        else if(line->data[c] == '+') {
          neg = 0;
          ++c;
          for(; isblank(line->data[c]); ++c) ; //skip whitespace
        }
        else neg = 0;
        v_A = 0;
        for(; isdigit(line->data[c]); ++c) {
          v_A *= 10;
          v_A += line->data[c] - '0';
        }
        v_A %= CORESIZE;
        if(neg) v_A = CORESIZE - v_A;
        v_A %= CORESIZE; //in case neg = 1 and v_A = CORESIZE - 0
        if(parsopt.out_fields_neg) {
          if(v_A > CORESIZE/2) v_A -= CORESIZE;
        }

        int skipped = 0;
        for(; isblank(line->data[c]); ++c) skipped = 1; //skip whitespace

        if(line->data[c] == '\0') { //one operand
          if(parsopt.field_count_rules == 0) {
            parser_error(line, "current parser settings require two%s fields on every instruction.", (parsopt.exp_allow_omit_both)? " or zero" : "");
            freetext(txt);
            return NULL;
          }
          else if(parsopt.field_count_rules == 1) { //exact number
            if(def_nops != 1) {
              parser_error(line, "current parser settings require two fields on %c%c%c.", (s_op >> 16) & 0xFF, (s_op >> 8) & 0xFF, s_op & 0xFF);
              freetext(txt);
              return NULL;
            }
          }
          else if(parsopt.field_count_rules == 2) {
            if(def_nops != 1) {
              parser_error(line, "current parser settings don't allow omitting a field on %c%c%c.", (s_op >> 16) & 0xFF, (s_op >> 8) & 0xFF, s_op & 0xFF);
              freetext(txt);
              return NULL;
            }
          }
          //default field
          #ifdef opt_SPL86
          if((s_op == 0x444154) || (s_op == 0x53504c)) {
          #else
          if(s_op == 0x444154) {
          #endif
            s_aB = s_aA;
            v_B = v_A;
            s_aA = '#';
            v_A = 0;
          }
          else {
            s_aB = '#';
            v_B = 0;
          }
        }
        else { //two operands
          if(parsopt.field_count_rules == 1) {
            if(def_nops != 2) {
              parser_error(line, "current parser settings require a single field on %c%c%c.", (s_op >> 16) & 0xFF, (s_op >> 8) & 0xFF, s_op & 0xFF);
              freetext(txt);
              return NULL;
            }
          }
          if(line->data[c] == ',') { //comma
            if(!(parsopt.field_separator & 1)) {
              parser_error(line, "current parser settings don't allow comma as field separator.");
              freetext(txt);
              return NULL;
            }
            for(++c; isblank(line->data[c]); ++c) ; //skip whitespace
          }
          else { //whitespace
            if(!skipped) {
              parser_warn(line, "no separator between operands, interprested as whitespace.");
            }
            if(!(parsopt.field_separator & 2)) {
              parser_error(line, "current parser settings don't allow whitespace as a field separator.");
              freetext(txt);
              return NULL;
            }
          }

          s_aB = line->data[c++];
          switch(s_aB) {
            case '#':
              if(!parsopt.addr_immediate) {
                parser_error(line, "immediate address mode not enabled on current parser settings.");
                freetext(txt);
                return NULL;
              }
              break;
            case '$':
              if(!parsopt.addr_expl_direct) {
                parser_error(line, "explicit '$' for direct address mode not enabled on current parser settings.");
                freetext(txt);
                return NULL;
              }
              break;
            case '@':
              if(!parsopt.addr_bindirect) {
                parser_error(line, "B-field indirect address mode not enabled on current parser settings.");
                freetext(txt);
                return NULL;
              }
              break;
            case '*':
              if(!parsopt.addr_aindirect) {
                parser_error(line, "A-field indirect address mode not enabled on current parser settings.");
                freetext(txt);
                return NULL;
              }
              break;
            case '<':
              if(!parsopt.addr_bdecrement) {
                parser_error(line, "B-field auto-/predecrement address mode not enabled on current parser settings.");
                freetext(txt);
                return NULL;
              }
              break;
            case '>':
              if(!parsopt.addr_bincrement) {
                parser_error(line, "B-field postincrement address mode not enabled on current parser settings.");
                freetext(txt);
                return NULL;
              }
              break;
            case '{':
              if(!parsopt.addr_adecrement) {
                parser_error(line, "A-field predecrement address mode not enabled on current parser settings.");
                freetext(txt);
                return NULL;
              }
              break;
            case '}':
              if(!parsopt.addr_aincrement) {
                parser_error(line, "A-field postincrement address mode not enabled on current parser settings.");
                freetext(txt);
                return NULL;
              }
              break;
            default:
              if(!isdigit(s_aB)) {
                parser_error(line, "expected address mode, digit or '-', got %c.", s_aB);
                freetext(txt);
                return NULL;
              }
              //fall through
            case '-': case '+':
              s_aB = '$';
              --c;
              break;
          }
          for(; isblank(line->data[c]); ++c) ; //skip whitespace

          if(line->data[c] == '-') {
            neg = 1;
            ++c;
            for(; isblank(line->data[c]); ++c) ; //skip whitespace
          }
          else if(line->data[c] == '+') {
            neg = 0;
            ++c;
            for(; isblank(line->data[c]); ++c) ; //skip whitespace
          }
          else neg = 0;
          v_B = 0;
          for(; isdigit(line->data[c]); ++c) {
            v_B *= 10;
            v_B += line->data[c] - '0';
          }
          v_B %= CORESIZE;
          if(neg) v_B = CORESIZE - v_B;
          v_B %= CORESIZE; //in case neg = 1 and v_B = CORESIZE - 0
          if(parsopt.out_fields_neg) {
            if(v_B > CORESIZE/2) v_B -= CORESIZE;
          }

          for(; isblank(line->data[c]); ++c) ; //skip whitespace
          if(line->data[c] != '\0') {
            parser_warn(line, "non-whitespace characters after second field, ignoring.");
          }
        }
      }

      if(!parsopt.allow_all_addrs) {
        #ifdef O_DAT
        if(s_op == 0x444154) {
          if(((s_aA != '#') && (s_aA != '<'))
          || ((s_aB != '#') && (s_aB != '<'))) {
            parser_error(line, "current parser settings require DAT to use only # and < addressing modes.");
            freetext(txt);
            return NULL;
          }
        }
        #endif
        else if(s_aA == '#') {
          switch(s_op) {
            #ifdef O_JMP
            case 0x4a4d50:
            #endif
            #ifdef O_JMZ
            case 0x4a4d5a:
            #endif
            #ifdef O_JMN
            case 0x4a4d4e:
            #endif
            #ifdef O_DJN
            case 0x444a4e:
            #endif
            #ifdef O_DJZ
            case 0x444a5a:
            #endif
            #ifdef O_SPL
            case 0x53504c:
            #endif
            #if (defined(EXT_XCH_a) || (STANDARD == 89))
            case 0x584348:
            #endif
            #ifdef O_PCT
            case 0x504354:
            #endif
              parser_error(line, "current parser settings forbid # addressing mode on %c%c%c's A-field.", (s_op >> 16) & 0xFF, (s_op >> 8) & 0xFF, s_op & 0xFF);
              freetext(txt);
              return NULL;
          }
        }
        else if(s_aB == '#') {
          switch(s_op) {
            #ifdef O_MOV
            case 0x4d4f56:
            #endif
            #ifdef O_ADD
            case 0x414444:
            #endif
            #ifdef O_SUB
            case 0x535542:
            #endif
            #ifdef O_MUL
            case 0x4d554c:
            #endif
            #ifdef O_DIV
            case 0x444956:
            #endif
            #ifdef O_MOD
            case 0x4d4f44:
            #endif
            #ifdef O_CMP
            case 0x434d50:
            #endif
            #ifdef O_SEQ
            case 0x534551:
            #endif
            #ifdef O_SNE
            case 0x534e45:
            #endif
            #ifdef O_SLT
            case 0x534c54:
            #endif
            #if defined(EXT_XCH_b)
            case 0x584348:
            #endif
            #ifdef O_LDP
            case 0x4c4450:
            #endif
              parser_error(line, "current parser settings forbid # addressing mode on %c%c%c's B-field.", (s_op >> 16) & 0xFF, (s_op >> 8) & 0xFF, s_op & 0xFF);
              freetext(txt);
              return NULL;
          }
        }
      }

      #if ((!defined(O_XCH)) && (STANDARD == 89))
      if(s_op == 0x584348) { //emulate XCH as MOV.X
        s_op = 0x4d4f56;
        s_md = 'X';
        switch(s_aA) {
          case '#': //preserve field values
            s_aB = '#';
            break;
          case '$': case '@': case '*':
            v_B = v_A;
            s_aB = s_aA;
            break;
          case '<':
            v_B = v_A;
            s_aB = '@';
            break;
          case '>':
            v_B = v_A;
            s_aA = '@';
            s_aB = '>';
            break;
          case '{':
            v_B = v_A;
            s_aB = '*';
            break;
          case '}':
            v_B = v_A;
            s_aA = '*';
            s_aB = '}';
            break;
        }
      }
      #endif

      if(!s_md) { //default modifier
        switch(s_op) {
          #ifdef O_DAT
          case 0x444154:
          #endif
          #ifdef O_NOP
          case 0x4e4f50:
          #endif
          #if defined(EXT_XCH_a)
          case 0x584348:
          #endif
          case 0x585858: //XXX
            s_md = 'F';
            break;

          #ifdef O_MOV
          case 0x4d4f56:
          #endif
          #ifdef O_CMP
          case 0x434d50:
          #endif
          #ifdef O_SEQ
          case 0x534551:
          #endif
          #ifdef O_SNE
          case 0x534e45:
          #endif
          #if defined(EXT_XCH_b)
          case 0x584348:
          #endif
            if(s_aA == '#') s_md = 0x4142;
            else if(s_aB == '#') s_md = 'B';
            else s_md = 'I';
            break;

          #ifdef O_ADD
          case 0x414444:
          #endif
          #ifdef O_SUB
          case 0x535542:
          #endif
          #ifdef O_MUL
          case 0x4d554c:
          #endif
          #ifdef O_DIV
          case 0x444956:
          #endif
          #ifdef O_MOD
          case 0x4d4f44:
          #endif
            if(s_aA == '#') s_md = 0x4142;
            else if((s_aB == '#') || (parsopt.def_modif_rules == 86)) s_md = 'B';
            else s_md = 'F';
            break;

          #ifdef O_SLT
          case 0x534c54:
          #endif
          #ifdef O_LDP
          case 0x4c4450:
          #endif
          #ifdef O_STP
          case 0x535450:
          #endif
            if(s_aA == '#') s_md = 0x4142;
            else s_md = 'B';
            break;

          #ifdef O_JMP
          case 0x4a4d50:
          #endif
          #ifdef O_JMZ
          case 0x4a4d5a:
          #endif
          #ifdef O_JMN
          case 0x4a4d4e:
          #endif
          #ifdef O_DJN
          case 0x444a4e:
          #endif
          #ifdef O_SPL
          case 0x53504c:
          #endif
          #ifdef O_DJZ
          case 0x444a5a:
          #endif
          #ifdef O_PCT
          case 0x504354:
          #endif
          #ifdef O_STS
          case 0x535453:
          #endif
            s_md = 'B';
            break;
        }
      }

      int linlen = snprintf(NULL, 0, "       XXX.XX $%6d, $%6d     ", v_A, v_B);
      char* str = (char*) malloc(linlen+1);
      snprintf(str, linlen+1, "%5s  %c%c%c.%c%c %c%6d, %c%6d     ",
        (CURLINE == w->org)? "START" : "",
        (s_op >> 16) & 0xFF,
        (s_op >> 8) & 0xFF,
        s_op & 0xFF,
        (s_md & 0xFF00)? ((s_md >> 8) & 0xFF) : s_md,
        (s_md & 0xFF00)? (s_md & 0xFF) : ' ',
        s_aA,
        v_A,
        s_aB,
        v_B
      );
      free(line->data);
      line->data = str;
      line->len = linlen;

      last = line;
      ++CURLINE;
    }

    last->len -= 5; //remove trailing whitespace of last line
    last->data = (char*) realloc(last->data, last->len+1);
    last->data[last->len] = '\0';

    w->len = CURLINE;

    if(txt->data != NULL) free(txt->data);
    txt->data = restricted_copy("       ORG      START");
    txt->len = strlen(txt->data);
  }
  clear_dictionary(&d_opcodes);
  clear_dictionary(&d_pseudoops);
  clear_dictionary(&d_constants);
  clear_dictionary(&d_variables);
  clear_dictionary(&d_labels);
  return txt;
}
