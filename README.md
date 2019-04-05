[![Stuff](https://img.shields.io/static/v1.svg?label=test&message=passed&color=success)](https://github.com/aerkiaga/hmars/)
<!---
[//]: # (![Stuff](https://img.shields.io/static/v1.svg?label=test&message=failed&color=critical))
[//]: # (![Stuff](https://img.shields.io/static/v1.svg?label=test&message=untested&color=important))
--->

# hmars
A fast and feature-rich Memory Array Redcode Simulator for Corewar.

**WARNING:** This is a work-in-progress; the current version should not be considered stable, and still contains much unfinished code.

## Installation
1. Take a look at src/config.h, this contains several useful configuration options.
2. Build one of the following:
   * Type `make hmars` to build the command-line version.
   * Type `make hmars-gui` to build the SDL interactive debugger.
   * Type `make` or `make all` to build both of them.

## Usage
### Command line

    hmars [options] file1 [-l loadfile] file2 ...
    hmars --test
    hmars-gui [options] file1 [-l loadfile] file2 ...
    hmars-gui --test

Option | Description
------ | -------------
`-r rounds` | Set number of rounds to fight.
`-l loadfile` | Must come after warrior file, specifies optional load file to output.
`-V` | Increase verbosity by one level, up to 2 levels.
`--test` | Ignores other options and performs self-test.

### Debugger
The debugger is not currently finished. For now, it only displays the ongoing round in a rectangular area. It starts paused, and clicking on it begins the simulation.
