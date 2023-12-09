#!/bin/bash
rm -rf build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_OSX_ARCHITECTURES=arm64 -B build/debug && cmake --build build/debug && ln -s build/debug/compile_commands.json compile_commands.json
