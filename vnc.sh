#!/bin/sh

Xvfb :1 -screen 0 1024x768x24 &
x11vnc -display :1 -nopw -forever -shared &
disown %1 %2
