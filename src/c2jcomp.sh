#!/usr/bin/bash
gcc `pkg-config --cflags gtk+-3.0` c2j.c -o c2j `pkg-config --libs gtk+-3.0`
