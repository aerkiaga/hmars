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

/**TODO: make these tuneable at runtime**/
#define CORESIZE 8000
#define PSPACESIZE 500 ///< 0 disables P-space
#define MAXCYCLES 80000
#define MAXPROCESSES 8000
#define MAXLENGTH 100
#define MINDISTANCE 100
#define VERSION 30 ///< hMARS 0.3.0

/// Native standard setting for simulator.
/** **Either 94, 89, 88, 86 or 84**
 **
 ** 89 is 88 with extended parser options and extension XCH_a enabled by default.
 ** If 89 is chosen but EXT_XCH_a/_b is not, XCH will be translated as MOV.X a, a.
 ** All of the different standards are backward compatible in the simulator,
 ** except for/with 86, as it changes behavior of SPL and adds auto-decrement.
 ** Also, 84 (pre-ICWS) adds instruction DJZ, but it is also available as an
 ** extension, which allows compatibility with 84 by enabling EXT_DJZ. */
#define STANDARD 94

/**Extensions, uncomment to use**/
//#define EXT_XCH_a ///< Exchange A- and B-fields of A-operand
//#define EXT_XCH_b ///< Exchange A- and B-operands
//#define EXT_PCT_a ///< Protect A until attempt to modify
//#define EXT_PCT_b ///< Protect A until executed
//#define EXT_DJZ ///< Decrement B and jump to A if zero
#define EXT_STS ///< Print A to stdout

#define PARSER_NONSTANDARD ///< Enables nonstandard parser features by default

/// Multithreading switch
/**If set, select one multithreading method.**/
#define MULTITHREAD

/// Select one multithreading method.
#define THREAD_USE_POSIX
//#define THREAD_USE_C11
//#define THREAD_USE_c11threads
