#!/bin/sh

# Start all debug tools and have them listening

tmux new-session -s 'debug' -d

tmux split-pane -v socat - unix-connect:qemu-serial-socket,forever
tmux split-pane -h socat -,echo=0,icanon=0 unix-connect:qemu-monitor-socket,forever

tmux select-pane -L
tmux select-pane -U

tmux send -t 'debug' 'gdb -x debug.gdbinit' ENTER

tmux -2 attach-session -d
tmux kill-session 'debug'
