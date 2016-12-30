#!/bin/bash

# Remove VNC lock (if process already killed)
rm /tmp/.X1-lock /tmp/.X11-unix/X1
# Run VNC server with tail in the foreground
vncserver :1 -randr -geometry 1280x800 -depth 24 && tail -F /root/.vnc/*.log
