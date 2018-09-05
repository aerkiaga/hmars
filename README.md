# hmars
A fast and feature-rich Memory Array Redcode Simulator for Corewar.

**WARNING:** This is a work-in-progress; the current version should not be considered stable, and still contains many unfinished code.

## Installation
1. Take a look at src/config.h, this contains several useful configuration options.
2. Build one of the following:
   * Type `make hmars` to build the JIT command-line version.
   * Type `make hmars-gui` to build the SDL interactive debugger.
   * Type `make` or `make all` to build both of them.

## Usage
### Command line

    hmars [options] file1 [-l loadfile] file2 ...
    hmars-gui [options] file1 [-l loadfile] file2 ...

Option | Description
------ | -------------
`-r rounds` | Set number of rounds to fight.
`-l loadfile` | Must come after warrior file, specifies optional load file to output.
`-V` | Increase verbosity by one level, up to 2 levels.

### Debugger
The debugger is not currently finished. For now, it only displays the ongoing round on a rectangular area.
