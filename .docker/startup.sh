#!/bin/bash

# Remove VNC lock (if process already killed)
rm -f /tmp/.X1-lock /tmp/.X11-unix/X1

# Set the DISPLAY that will be used by default. Makes lab show up in our xvfb display
export DISPLAY=:1

# Start the X virtual frame buffer which uses memory instead of an actual device.
# Allows lab to be run headless.
Xvfb "$DISPLAY" -screen 0 "$XVFB_RESOLUTION" &

# Run a lightweight Window Manager (fluxbox is smaller than lxdm, gnome, unity, kde, etc)
# (pretty sure this is required.)
fluxbox &

# Run the x11vnc server
# Explanation of options:
#   -display : This needs to match the xvfb display number
#   -passwd : Use the password from an ENV var here instead of using ~/.vnc/passwd
#   -o : Specifies where to send the log output.
#   -noipv6 : Skip trying to serve vnc on ipv6 (avoids warnings)
#   -bg : Run in the background
#   -forever : Don't die when the first user disconnects from vnc (which is the default)
#   -N : Makes it use port 5900+N where N is the display.. so 5901. (default is 5900)
x11vnc \
    -display "$DISPLAY" \
    -passwd "$VNC_PASSWORD" \
    -o /dev/stderr \
    -noipv6 \
    -bg \
    -forever \
    -N
