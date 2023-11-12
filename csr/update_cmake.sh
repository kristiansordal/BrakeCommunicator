!#/bin/bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_OSX_ARCHITECTURES=arm64 -B build/debug
rm compile_commands.json
ln -s build/debug/compile_commands.json compile_commands.json
cmake --build build/debug
