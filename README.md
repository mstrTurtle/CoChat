# CoChat

Simple TUI chatroom based on C++20 coroutine facilities.

## Project Directories Overview

- src
    - simple-client // Simple client with basic terminal interface
    - complex-client // Client with TUI interface implemented by FTXUI library
    - server // Chat server implemented with customized C==20 coroutine

## Build And Install

To build CoChat, you have CMake installed.

At the root of project as working directory, you type:

```bash
$ cmake -H. -Bbuild
$ cmake --build build
```

Then, you find the static linked binary in `./build/bin/`.