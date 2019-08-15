#!/bin/bash

if [[ "$#" == "3" ]]; then
    printf "#define INVADER_VERSION_STRING \"%sr%s.%s\"\n" "$1" $(git --git-dir "$2/.git" rev-list --count HEAD) $(git --git-dir "$2/.git" rev-parse --short HEAD) > "$3/version_str.hpp"
elif [[ "$#" == "2" ]]; then
    printf "#define INVADER_VERSION_STRING %s\n" "$1" > "$2/version_str.hpp"
fi
