# The Vidyut Build System

`Vidyut`, in analogy to self-start feature of modern vehicles, is a self-contained build system
written in C for C projects. You write the build files in C, and don't need any external programs
except your compiler to build your project.

## Basic Info

This directory contains the source code of `Vidyut` build system. This is written specifically
for CT, but anyone can use it. I give the permission to use this (any code in `Vidyut` directory)
in Public Domain (CC0). You can keep your modifications in private, with yourself, no need to expose
it to public.

At present, the build system does not rebuild only required modules, but instead compiles everything.
First it rebuilds itself, then starts a child process with newly built binary to build the project.
The parent process exits quitely.

## Why Use Vidyut?

Do not use it. It's for me. Use it at your own risk if you would. No build system is perfect,
and if you create one, you create one specific for your own use. If you use this, make sure your use
is almost similar to mine (building projects without any external programs, and project with static dependencies).

## How To Use?

- Copy paste the `Vidyut` directory, and `SelfStartMain.c` to your project.
- Change contents of `VidyutMain.c` to build your project, instead of mine ;-)
- You'll need to bootstrap the build system by doing `gcc -o VidyutMain SelfStartMain.c`
- Then for any build, just run `./VidyutMain`, it'll automatically rebuild everything
- Do not change anything int he `Vidyut` directory, all your project build commands must be in your `SelfStartMain.c` at project root.
