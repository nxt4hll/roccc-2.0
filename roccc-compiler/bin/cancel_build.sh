#!/bin/bash

ps aux | grep "compile_" | awk " { print \$2 ; }" | xargs kill -9 2> /dev/null
ps aux | grep "suifdriver " | awk " { print \$2 ; }" | xargs kill -9 2> /dev/null
ps aux | grep "opt " | awk " { print \$2 ; }" | xargs kill -9 2> /dev/null