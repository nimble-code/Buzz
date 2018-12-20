# Buzz
## a framework for studying logic model checking algorithms

This is an early version of the framework, so you may
encounter rough edges here and there.
If you do, please report them and/or suggest fixes.

## Installation

These installation notes assume a Linux/Unix-like system.

Pick a location for the tool and untar the source code there.
For instance:

    $ mkdir ~/Tools
    $ mv buzz_v01.tar.gz ~/Tools
    $ cd ~/Tools
    $ tar -xzf buzz_v01.tar.gz
    $ cd Buzz
    $ sudo make install
    $ buzz -V  # basic check that it's working
    Buzz Version 1.0 -- 1 January 2019

This should compile all parts of the toolset without warnings (although
on different platforms compilers can decide to pick on different things -
if you see a warning please let me know).
It will also copy the 'buzz' executable to /usr/local/bin on your system.
Which means you'll probably need the sudo at the install.

You'll notice that the installation does not remove the .o files that
were generated. That's because the tool will be using those files to link
to the verification code that is generated for each specification.
So, you should leave the .o files in the various subdirectories of
the main Buzz/ tool directory in place.
Buzz can independently decide when it needs to recompile one of those
files, but it will do so in a separate directory (i.e., in the directory
where buzz is invoked.)

Also make sure that after you compile and install buzz that you don't
rename or move the directory with the source code, and all those .o
files -- or else the tool will not be able to find all these things.
(If you need to rename or move the directory, just recompile and
reinstall the tool from scratch -- it's quick.)

## Principle of operation

When you ask buzz to verify a specification, it will assemble all the
code it needs based on the preferences that you specify, creates a
specialized verification program for your application, and it then
compiles and runs it -- all silently in the background.
If errors are encountered, they are reported at the earliest possible point.

The way you always get the performance of compiled code
and can easily add or modify algorithms written.

A basic call to buzz would look like this:

    $ cd examples	# in the Buzz directory
    $ buzz c_abp	# perform verification run
    search completed
        Dfs search, Closed storage
        8 maxdepth, 8 states (0M), 1 revisits (0M), 32 MB mem used
    
        #errors 0, #procs 2, #chans 2
    unreached:
        (0 states unreached)
        elapsed 0.08 sec, rate 100 states/sec

In this run, the verifier explore eight reachable states
for the alternating bit protocol, revisted one of those states,
and found no errors. The specification contained two active processes
and two message channels.
Buzz first generated the source code for the verifier, plus a script to
compile and run it, and then it cleaned everything up to fool you into
thinking that it did the entire verification by itself.
We'll discuss more detailed usage below, after exploring the structure
of the Buzz distribution itself.

## Directory Structure

In the top-level directory you will find the makefile and the main buzz.c
source file.  This main file provides an interface to the rest of the code.
The main directories are:

    examples, parse, trans, lex, sym, init, main, include,
    hash, search, and store

A description of what is in each of these follows.

## Buzz/examples

This subdirectory contains a set of small examples of buzz specifications.
The framework contains two different parsers, for two different
specification languages. The tool can figure out which of the two languages
is used by scanning the notations used, so this rarely would need to be
specified explicitly in an analysis.

Most of the files use the parse/chan.y parser, which accepts a simple predefined
message passing language, e.g.:

    c_3way  c_abp  c_abp2  c_deadlock  c_multi_label c_philo c_x21

The c_ prefix is for convenience, but not required for buzz to work correctly.
Each statement is either a send statement, a receive statement, or a next state
designation. Labels can be used to identify states. Statements are separated
by semi-colons.
The grammar for this specification language is exceedingly simple -- see the
definition in parse/chan.y (and the lexical analyzer in lex/lex.y).

Four other specifications use a different language, with the parser
specified in parse/parse.y. For convenience, the names of these files
is prefixed with p_, altough that's not necessary for buzz to recognize
the specification format that is used:

    p_anderson p_call p_fct p_scc

This parse.y language is the more capable one, where you define transitions
as general C functions.  When the function returns non-zero, buzz will
understand that the transition was executable and its effect was applied
to the system state.  When the function returns zero, buzz will assume
that the transition is currently not executable and cannot be chosen.
The functions are specified in standard C, following a marker of %% on
a single line. (The presence of the marker will indicate to buzz that
this is the format that is used.) Each function used must return an
integer value, and can have no parameters.
This format also support non-determinism and choice. Transitions
are specified one per line, giving first the source statename,
followed by a column, and then the transition function, followed
by a semi-colon and then a next-state designation, as in:

    S1: f(); next S2;

The example p_scc is a simple example of this format.
There are no variables, so if you need them you have to declare
them as global statevars, and use them inside the C transition functions.
Note also that each transition function is executed atomically.

Note that in both languages the same source state can be used to
specify multiple transitions, one per line (compare for instance the c_philo
example and the p_scc example). This allows us to define if-then-else
or switch/case selections, and non-determinism.

You can add a new language by adding a parser and lexical analyzer in
the appropriate directories, which you can then select on the command
line or require with a tag inside a specification writting in that language.
If you do this, make sure you generate a transition system in the format
that the rest of the framework can accept. (You'll have to study the code
to see the details. There isn't much code, so this can be relatively
straightforward.)

## Buzz/parse

This subdirectory contains the files to define the two parsers

    chan.c   chan.o   chan.y	# first parser
    parse.c  parse.o  parse.y	# 2nd parser
    common.c			# common routines

The idea is of course that you can easily add more parsers for
your own specification language.  The various modules of buzz
are kept as independent as possible, so that a change in one part
of the system does not unnecessarily influence the other parts.

## Buzz/trans

The two parsers use different definitions for the specification
of the transitions, as specified in the two files in this directory:

    chan.c
    trans.c

## Buzz/lex

Both parsers have a single lexical analyzer for tokenization:

    lex.l

## Buzz/sym

A generic set of symbol table routines.

    sym.c

## Buzz/init

Contains a dummy definition of an init.c program that is often
(but not always) replaced with a new version generated by buzz
in parsing a specification.

    init.c

## Buzz/main

Contains the main part of the program that Buzz synthesizes
when it builds a verifier for your application.

    main.c

If you add a new storage mode or a new search algorithm (but not
if you just add a new hash function) you may need to add an entry
at the end of this file to identify it.

## Buzz/include

Contains headerfiles with all type definitions

    buzz.h  functions.h  state.h  state_orig.h

The most important file here is buzz.h, which contains all
interface definitions.  When you add a new module (e.g., a
hash function or a new storage routine) you must include
this file buzz.h, and no others, to get all the data
and interface definitions you need.

## Buzz/hash

Contains a number of sample hash routines to experiment
with and, most importantly, to add to, including:

    bernstein.c  jenkins.c  naive.c  reeds.c  simple.c

## Buzz/search

A basic set of just the three most important search routines.

    bfs.c dfs.c dfs_claim.c
    bfs_common.c
    dfs_common.c

bfs.c defines a breadth-first search algorithm with back-pointers
to allow the generation of counter-examples. dfs.c defines a plain
depth-first search with a stack for counter-example generation, and
dfs_claim.c defines a nested depth-first search.

## Buzz/store

Basic storage routines, for open hashing and closed hashing.
The closed.c file contains the support for back-pointers
that is used in bfs.c.
Also included is an implementation of a closed storage routine
without backpointers (closed_notrace.c) which can of course be
quite a bit simpler than the full version with backpointers.

    open.c closed.c closed_notrace.c
    open_common.c
    closed_common.c

## Usage Detail

Typing

    $ buzz -v c_abp

performs a more verbose verification run, where every step in the search
is printed. This is rarely useful, other then for debugging of course.

Typing

    $ buzz -d c_abp

tells buzz to generate all the files, print on the standard output what
actions it is taking, and produce a script to compile and
run the verification, but it will not actually execute the verification itself.
On my system the output looks something like this:

    $ cd Buzz/examples
    $ buzz -d c_abp
    # required: 'parse=chan'
    # required: 'trans=chan'
    #!/bin/sh
    
    B=/home/gh/Dropbox/Tools/Buzz
    cc -g -O3 -I$B -I$B/include -o ./buzz_e \
        $B/lex/lex.o \
        $B/parse/chan.o \
        $B/sym/sym.o \
        $B/trans/chan.o \
        $B/hash/jenkins.o \
        $B/store/closed.o \
        $B/search/dfs.o \
        $B/main/main.c \
        $B/init/init.o
    # ./buzz_e c_abp
    x=$?
    # rm -f *.o ./buzz_e _buzz_*
    exit $x

After this command completes, you can find the script and the executable
verifier in your work directory:

    $ ls -l *buzz*
    -rwx------ 1 gh gh    305 Feb 18 12:46 _buzz_.sh
    -rwxrwxr-x 1 gh gh 143418 Feb 18 12:46 buzz_e

The _buzz_.sh script contains the compilation commands that are also printed out.
You can now complete the job by performing the verification manually as follows:

    $ ./buzz_e c_abp

(And, if you forget the 'c_abp' argument, the command will tell you what options
it was expecting.)

The runs so far were done with all options defaulting to predefined values.
To see what other options are available in this version of Buzz (i.e.,
given what we included in the hash, search, and store subdirectories), type:

    $ buzz --
    Buzz Version 1.0 -- 1 January 2019
    usage: 'buzz [options | sizes | modules]* filename' options:

        -cN     print the Nth error and then stop
        -d      increase debug level
        -mN     maxdepth = N in steps    default 10000
        -ON     use compiler optimization N  default -O3
        -rN     freq     = N in Kstates  default 1000
        -uN     ncores   = N in cores    default 8
        -V      print version and exit
        -v      increase verbosity level
        -wN     width    = N in 2^N      default 20 (hashtable)
        -tN     maxtry   = N in tries    default 20
    
    sizes (with the default values):

              SV=128    (state vector size in uchar)
         MaxProc=16     (max nr of process declarations)
         MaxChan=16     (when using trans/trans)
            Fmax=32     (max successors per state)
          MaxGen=10     (max size of a sequential bfs gen in thousands)
        MaxDepth=10000  (max search depth)
            Freq=1000   (reporting frequency in thousands)
          Ncores=8      (nr of cores for multicore)
             SGB=24     (nr of GB of shared mem for multicore)
            Hmax=20     (2^20 size of hash-table)
            Tmax=20     (max nr of retries in closed-hash)
    
    modules (the first in each list is the default):

        base=/home/gh/Dropbox/Tools/Buzz        (where the .o files are)
        lex=lex
        parse=parse     | chan
        sym=sym
        trans=trans     | chan
        hash=jenkins    | reeds | naive | simple | bernstein
        store=closed    | closed_notrace | open
        search=dfs      | dfs_claim | bfs

Starting at the bottom, you can see that the default search option is 'search=dfs'.
To repeat the run with a breadth-first search instead, you can override this default
by specifying one of the other options. In this case there's just one alternative:

    $ buzz search=bfs c_abp
    search completed
        Bfs search, Closed_trace storage
        8 maxdepth, 8 states (0M), 1 revisits (0M), 51 MB mem used
    
        #errors 0, #procs 2, #chans 2
    unreached:
        (0 states unreached)
        elapsed 0.11 sec, rate 72.7273 states/sec

Note that memory use went up  bit, and the search was a little slower,
but the result is otherwise the same as before.

If you pick a combination of options that cannot work, buzz will warn you.
For instance, if you try to combine a breadth-first search with closed storage
without backpointers, you'll see this error message:

    $ buzz search=bfs store=closed_notrace c_abp
    error: search=bfs requires store=closed

To do the default depth-first search with a different hash-function
you can try, for instance:

    $ buzz hash=reeds c_abp
    search completed
        Dfs search, Closed storage
        8 maxdepth, 8 states (0M), 1 revisits (0M), 32 MB mem used
    
        #errors 0, #procs 2, #chans 2
    unreached:
        (0 states unreached)
        elapsed 0.07 sec, rate 114.286 states/sec

which doesn't look all that different from the default, but ran a little faster.

Buzz will generally know which parser to use, based on the text of
the specification (e.g., c_abp), so don't worry about having to specify
this as well as a command-line argument.
Let us instead focus on the other arguments you can use to define
how the verification is to be performed.
This is the relevant part of the list from above:

    options:
        -cN     print the Nth error and then stop
        -d      increase debug level
        -mN     maxdepth = N in steps    default 10000
        -ON     use compiler optimization N  default -O3
        -rN     freq     = N in Kstates  default 1000
        -uN     ncores   = N in cores    default 8
        -V      print version and exit
        -v      increase verbosity level
        -wN     width    = N in 2^N      default 20 (hashtable)
        -tN     maxtry   = N in tries    default 20

The options will look familiar, since they're similar to what Spin
and it's generated verifier pan supports.  The -tN option is to limit
the number of retries in closed hashing only, so if you use open
hashing this would not apply and has no effect.
The next set allows you to configure and fine-tune a verification run,
by setting new values for specific limits.
    
    sizes (with the default values):
              SV=128    (max state vector size in uchar)
         MaxProc=16     (max nr of process declarations)
         MaxChan=16     (when using trans/trans)
            Fmax=32     (max successors per state)
          MaxGen=10     (max size of a sequential bfs gen in thousands)
        MaxDepth=10000  (max search depth)
            Freq=1000   (reporting frequency in thousands)
          Ncores=8      (nr of cores for multicore)
             SGB=24     (nr of GB of shared mem for multicore)
            Hmax=20     (2^20 size of hash-table)
            Tmax=20     (max nr of retries in closed-hash)

The Ncores and SGB options and similarly the -u option from above,
aren't very useful yet since they make use of parallel search algorithms
that aren't in this distribution of the tool, but the meaning of the
remaining options will be clear.

## Inline Options

Buzz also allows you to specify search and configuration options
within a buzz specification file, which makes it easier to remember
how each specification can be verified.

We can, for instance, define a prefered verification mode for the
c_abp example we showed earlier by including a comment at the top of
the file:

    // requires   store=closed search=bfs

The keyword in the comment is "requires" followed by a tab and then
a sequence of what would otherwise be commandline options as shown earlier.

Another example (for a very large example, which uses bitstate search
and bitstate search and storage algorithms that are not included in the
current distribution):

    // requires Fmax=1024 search=dfs_bit store=bitstate Hmax=30 SV=8 MaxProc=4

These are the keywords that Buzz currently recognizes in inline comments:

    // requires		cf Buzz/examples/c_multi_label
    // statevar		cf Buzz/examples/p_call Buzz/examples/p_anderson
    // constants		cf Buzz/examples/p_fct
    // init			(see below)

The specification Buzz/examples/p_fct illustrates the use of constants to
introduce symbolic names for integer constants (similar to an mtype
in Promela).

The init keyword provides a way to initialize variables, for instance, if
you declared a user-defined variable 'one' that you want to initialize:

    // statevar	int	one;
    // init		B_state.one = 3;

It is important to specify the initialization this way, because Buzz needs
to be able to reinitialize a variable during the search. Hiding the initial
value in the declaration then would not work well.

December 2018
