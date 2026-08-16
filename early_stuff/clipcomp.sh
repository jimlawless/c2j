#!/usr/bin/bash
gcc `pkg-config --cflags gtk+-3.0` clipboard_monitor.c -o clipboard_monitor `pkg-config --libs gtk+-3.0`
