# Compiler Tech

A C library to help me experiment with compilers.

I want to implement something that is all of the following to the highest extent possible.

- Extremely compatible with C. I don't care about CPP, it's getting trash day by day.
- Extremenly fast in compilation, if possible, faster than any available C compilers.
  Compilation speed is really important to me, it helps me write code and test it faster.
  With C, it's alost like scripting because of this, because you can clean build even large
  codebases very fast, and then re-iterate as fast as that.
- Feature complete for comptime programming. I want macros as powerful as those in modern languages
  like Haskell, Zig, etc...
- Compiler, Parser, Optimizers, Translators, etc... itself available as a library for the program
  a runtime. You just link and use.
- A linker from scratch.
- Actually I want this completely independent. It to have cross-platform compilation support
  built in. Have it's own libc, no OS dependent library.

My main goal is only experiment with my ideas using this, write my own analysis tools, write compiler,
use compiler tech in normal programs, etc... This does not mean, I want this project to be an experiment,
I want this production ready, to be available to be used anywhere, where any modern language is used.

## Building

There is one bootstrap step, that is to be exectued only once at the very beginning,
and then everytime the executable "Make" is ran, it'll rebuild itself and use the latest
build then to build the actual project everytime. The name "Make" is not arbitrary, you must
always and only use that name for the binary to build project.

```
# bootstrap
gcc -O3 -o Make BuildCommands.c && ./Make

# any following builds
./Make
```
