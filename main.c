/**
 * fannc
 * Copyright (c) 2014, Claudi Martínez <claudix.kernel@gmail.com>, All rights reserved.
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmd.h"

static void usage() {
    banner();
    puts("Usage: fannc COMMAND ARGS...");
    puts("To see a list of all commands, type: fannc help");
    puts("To see the help of a particular command, type: fannc COMMAND --help");
    exit(1);
}
/*
 * Usage: fannc COMMAND ARGS...
 */
int main(int argc, char** argv) {
    if (argc <= 1) usage();
    return runCommand(argc-1, argv+1);    
}

