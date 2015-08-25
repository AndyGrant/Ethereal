#!/bin/sh

gprof foo.exe gmon.out > output.txt
notepad output.txt
