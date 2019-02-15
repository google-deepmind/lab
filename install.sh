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

# 6) install zsh
sudo apt-get install zsh

# 7) install oh_my_zsh
sh -c "$(wget https://raw.githubusercontent.com/robbyrussell/oh-my-zsh/master/tools/install.sh -O -)"

# 8) choose avit theme
export ZSH_THEME="avit"

# 9) launch zsh
zsh

# 10) build deepmind_lab.so
bazel build :deepmind_lab.so --define headless=osmesa
