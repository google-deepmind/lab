# echo "deb http://archive.ubuntu.com/ubuntu/ trusty main restricted universe multiversedeb http://archive.ubuntu.com/ubuntu/ trusty-security main restricted universe multiversedeb http://archive.ubuntu.com/ubuntu/ trusty-updates main restricted universe multiversedeb http://archive.ubuntu.com/ubuntu/ trusty-proposed main restricted universe multiversedeb http://archive.ubuntu.com/ubuntu/ trusty-backports main restricted universe multiverse" >> /etc/apt/sources.list

# 1) Uncomment files from /etc/apt/sources.list (cf text above)

# 2) get updates with new packages
sudo apt-get update

# 3) install pip2
sudo apt install python-pip

# 4) install numpy
pip2 install --user numpy

# 5) install other important stuff (maybe steps 3 and 4 are useless)
sudo apt-get install libffi-dev gettext freeglut3-dev libsdl2-dev libosmesa6-dev python-dev python-numpy python-pil zip