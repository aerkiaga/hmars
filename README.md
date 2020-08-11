[![Stuff](https://img.shields.io/static/v1.svg?label=test&message=passed&color=success)](https://github.com/aerkiaga/hmars/)
<!---
[//]: # (![Stuff](https://img.shields.io/static/v1.svg?label=test&message=failed&color=critical))
[//]: # (![Stuff](https://img.shields.io/static/v1.svg?label=test&message=untested&color=important))
--->

# hmars
A fast and feature-rich Memory Array Redcode Simulator for Corewar.

**WARNING:** This is a work-in-progress; the current version should not be considered stable, and still contains much unfinished code.

## Installation (Linux-only)
First, install the required dependencies. The latest GNU LibJIT is required at
the moment, and it must be built from source. GNU Autotools are required; on an
Ubuntu-based distro they can be obtained with:

    sudo apt-get install automake autotools-dev libtool

Then, build and install LibJIT:

    git clone https://git.savannah.gnu.org/git/libjit.git
    cd libjit
    ./bootstrap
    ./configure --prefix=/usr/local
    make
    sudo make install

If the optional SDL debugger is desired, then install SDL 2 too:

    sudo apt-get install libsdl2-dev

Take a look at src/config.h, this contains several useful configuration options.
Also inspect the Makefile, for changing the default compiler and flags.

Build one of the following:
   * Type `make hmars` to build the command-line version.
   * Type `make hmars-gui` to build the SDL interactive debugger.
   * Type `make` or `make all` to build both of them.

Optionally, run `./hmars --test` to check that everything is right. This depends
on the warriors found in `./test` to work. If a pMARS executable is present in
the current directory as `./pmars`, hMARS will check itself against it.

## Usage
### Command line

    hmars [options] file1 [-l loadfile] file2 ...
    hmars --test
    hmars-gui [options] file1 [-l loadfile] file2 ...

Option | Description | Default
------ | ------------|---------
`-r <rounds>` | Set number of rounds to fight. | 1
`-s <size>` | Size of core in instructions. | 8000
`-c <cycles>` | Maximum number of cycles per round. | 80000
`-p <procs>` | Maximum number of processes per warrior. | 8000
`-l <length>` | Maximum warrior length. | 100
`-d <dist>` | Minimum distance between warriors. | 100
`-F <position>` | Fixed position of second warrior (needs exactly 2 warriors). | None
`-S <size>` | Size of P-space in cells. | CORESIZE/16
`-L <loadfile>` | Must come after warrior file, specifies optional load file to output. | None
`-V` | Increase verbosity by one level, up to 2 levels. | -
`--test` | Ignores remaining options and performs self-test. | -

### Debugger
The debugger is not currently finished. For now, it only displays the ongoing
round in a rectangular area, which can be resized.

Symbols | Meaning
------ | -------
**⠀⠀** &nbsp; **⠀⠀** &nbsp; **⠀** &nbsp; &nbsp; **⠀** | None
**⠐⠀** &nbsp; **⠁⠀** &nbsp; **⠁** &nbsp; &nbsp; **⠀** | Read from
**⠕⠅** &nbsp; **⠑⠀** &nbsp; **⠑** &nbsp; &nbsp; **⠁** | Written to
**⠺⠂** &nbsp; **⠃⠀** &nbsp; **⠃** &nbsp; &nbsp; **⠁** | Incremented
**⠒⠂** &nbsp; **⠉⠀** &nbsp; **⠉** &nbsp; &nbsp; **⠁** | Decremented
**⠿⠇** &nbsp; **⠛⠀** &nbsp; **⠛** &nbsp; &nbsp; **⠁** | Executed
**⣿⣿** &nbsp; **⠿⠇** &nbsp; **⠛** &nbsp; &nbsp; **⠁** | In execution
