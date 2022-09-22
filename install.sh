#!/bin/bash

# change jetson power mode, desktop
sudo nvpmodel -m 8 
# maximize Jetson Xavier performance by setting static max frequency to CPU, GPU, and EMC clocks
sudo jetson_clocks

#allow user to access USB
sudo echo "SUBSYSTEM==\"usb\", MODE=\"0666\", GROUP=\"users\"" > $HOME/openpilot.rules
sudo echo "SUBSYSTEM==\"usb_device\", MODE=\"0666\", GROUP=\"users\"" >> $HOME/openpilot.rules
sudo mv $HOME/openpilot.rules /etc/udev/rules.d
sudo chmod 644 /etc/udev/rules.d/openpilot.rules
sudo chown root /etc/udev/rules.d/openpilot.rules
sudo chgrp root /etc/udev/rules.d/openpilot.rules

sudo apt purge -y libreoffice* thunderbird* rhythmbox* transmission* mutter* *visionworks* pulseaudio ibus* 
sudo apt install -y apt-utils openbox
sudo apt purge -y deluge smplayer* onboard* snapd* vpi1* lxmusic* avahi* yelp* vlc* nfs* ntfs* docker* python-gi samba*
sudo apt autoremove -y && sudo apt clean -y
sudo apt upgrade -y
sudo apt install gir1.2-notify


# install packages in ubuntu_setup.sh
# without clang and qt
sudo apt-get update && sudo apt-get install -y --no-install-recommends \
    autoconf \
    build-essential \
    bzip2 \
    capnproto \
    cppcheck \
    libcapnp-dev \
    cmake \
    curl \
    ffmpeg \
    git \
    libavformat-dev libavcodec-dev libavdevice-dev libavutil-dev libswscale-dev libavresample-dev libavfilter-dev \
    libarchive-dev \
    libbz2-dev \
    libcurl4-openssl-dev \
    libeigen3-dev \
    libffi-dev \
    libglew-dev \
    libgles2-mesa-dev \
    libglfw3-dev \
    libglib2.0-0 \
    liblzma-dev \
    libmysqlclient-dev \
    libomp-dev \
    libopencv-dev \
    libpng16-16 \
    libssl-dev \
    libstdc++-arm-none-eabi-newlib \
    libsqlite3-dev \
    libtool \
    libusb-1.0-0-dev \
    libzmq3-dev \
    libsdl-image1.2-dev libsdl-mixer1.2-dev libsdl-ttf2.0-dev libsmpeg-dev \
    libsdl1.2-dev  libportmidi-dev libswscale-dev libavformat-dev libavcodec-dev libfreetype6-dev \
    libsystemd-dev \
    locales \
    ocl-icd-libopencl1 \
    ocl-icd-opencl-dev \
    opencl-headers \
    python-dev \
    python3-pip \
    screen \
    sudo \
    vim \
    wget \
    gcc-arm-none-eabi

#libcapnp-dev
#capnproto

# install focal version
sudo sed -i -e 's#bionic#focal#g' /etc/apt/sources.list
sudo apt update
sudo apt install -y --no-install-recommends \
	tmux \
	clang \
	qml-module-qtquick2 \
	qt5-default \
	qtmultimedia5-dev \
	qtwebengine5-dev \
  qtdeclarative5-dev \
	qtchooser \
	libqt5x11extras5-dev \
  qtlocation5-dev \
  qtpositioning5-dev \
  libqt5sql5-sqlite \
  libqt5svg5-dev \
	ccache \
	libreadline-dev \
	nvidia-cudnn8 \
	nano

sudo apt install -y libpocl2
sudo sed -i -e 's#focal#bionic#g' /etc/apt/sources.list
sudo apt update

sudo apt purge -y whoopsie apport apparmor rpcbind gpsd isc-dhcp-server

# install jtop
sudo pip3 install setuptools
# jtop
sudo -H pip3 install -U jetson-stats

#install in /data
#make params folder

sudo mkdir /data

sudo chown openpilot /data
sudo chmod ugo+rwx /data

cd /data
mkdir params
git clone -b jetson_dev https://github.com/benjaminmcd/openpilot.git openpilot

rm /data/openpilot/.python-version

cd /data/openpilot

# install git lfs
if ! command -v "git-lfs" > /dev/null 2>&1; then
  curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | sudo bash
  sudo apt-get install git-lfs
fi

# install pyenv
if ! command -v "pyenv" > /dev/null 2>&1; then
  curl -L https://github.com/pyenv/pyenv-installer/raw/master/bin/pyenv-installer | bash
fi

export PYENV_ROOT="$HOME/.pyenv"
export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init --path)"
# install bashrc
source ~/.bashrc
if [ -z "$OPENPILOT_ENV" ]; then
  echo "export PYENV_ROOT=\"$HOME/.pyenv\"" >> ~/.bashrc
  echo "export PATH=\"$PYENV_ROOT/bin:$PATH\"" >> ~/.bashrc
  echo "eval \"$(pyenv init --path)\"" >> ~/.bashrc
  echo "source /data/openpilot/tools/openpilot_env.sh" >> ~/.bashrc
  source ~/.bashrc
  echo "added openpilot_env to bashrc"
  echo "export PYTHONPATH="/data/openpilot:$PYTHONPATH"" >> ~/.bashrc

fi


# do the rest of the git checkout
git lfs pull
git submodule init
git submodule update

# install python
pyenv install -s 3.8.5
pyenv global 3.8.5
pyenv rehash
eval "$(pyenv init -)"

# **** in python env ****
pip install --upgrade pip==20.2.4
pip install pipenv==2020.8.13

pip install setuptools
pip install wheel
pip install pkgconfig
pip install cython
pip install pycapnp==1.1.0
pip install numpy
pip install pycurl
pip install scons
pip install jinja2
pip install setuptools-cythonize
pip install sympy
pip install cffi
pip install logentries
pip install pyzmq
pip install pyjwt
pip install requests
pip install atomicwrites
pip install setproctitle
pip install psutil
pip install smbus2
pip install libusb1
pip install tqdm
pip install crcmod
pip install raven
pip install pycryptodome
pip install hexdump
pip install onnx
pip install casadi
pip install serial
pip install sentry-sdk
pip install libusb1
pip install future_fstrings


cd $HOME
# install opencv4
sudo apt install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
wget -O opencv.zip https://github.com/opencv/opencv/archive/4.5.2.zip
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/4.5.2.zip
unzip opencv.zip
unzip opencv_contrib.zip
mv opencv-4.5.2 opencv
mv opencv_contrib-4.5.2 opencv_contrib
cd $HOME/opencv/
mkdir build
cd build

cmake -D CMAKE_BUILD_TYPE=RELEASE \
	-D WITH_CUDA=ON \
	-D CUDA_ARCH_PTX="" \
	-D CUDA_ARCH_BIN="7.2" \
  -D WITH_CUDNN=ON \
  -D CUDNN_VERSION="8.0" \
	-D BUILD_opencv_python3=ON \
	-D BUILD_opencv_python2=OFF \
	-D BUILD_opencv_java=OFF \
	-D WITH_GSTREAMER=ON \
	-D WITH_GTK=OFF \
	-D BUILD_TESTS=OFF \
	-D BUILD_PERF_TESTS=OFF \
	-D BUILD_EXAMPLES=OFF \
	-D BUILD_FFMPEG=ON \
	-D OPENCV_DNN_CUDA=ON \
	-D ENABLE_FAST_MATH=ON \
	-D CUDA_FAST_MATH=ON \
	-D WITH_QT=ON \
	-D ENABLE_NEON=ON \
	-D ENABLE_VFPV3=OFF \
	-D BUILD_TESTS=OFF \
  -D INSTALL_PYTHON_EXAMPLES=OFF \
  -D INSTALL_C_EXAMPLES=OFF \
	-D OPENCV_ENABLE_NONFREE=ON \
  -D OPENCV_GENERATE_PKGCONFIG=ON \
  -D PYTHON_EXECUTABLE=/home/`whoami`/.pyenv/versions/3.8.5/bin/python \
  -D PYTHON_DEFAULT_EXECUTABLE=/home/`whoami`/.pyenv/versions/3.8.5/bin/python \
  -D PYTHON_PACKAGES_PATH=/home/`whoami`/.pyenv/versions/3.8.5/lib/python3.8/site-packages/ \
	-D OPENCV_EXTRA_MODULES_PATH=/home/`whoami`/opencv_contrib/modules ..

make -j $(nproc)
sudo make install

#install tera renderer
#https://github.com/acados/tera_renderer

# instrall onnxruntime-gpu
cd $HOME/Downloads
https://nvidia.box.com/shared/static/8xgbee5ghhb92i9rrcr04yymg0n3x3t0.whl
wget https://nvidia.box.com/shared/static/8xgbee5ghhb92i9rrcr04yymg0n3x3t0.whl -O onnxruntime_gpu-1.7.0-cp38-cp38-linux_aarch64.whl 
pip install onnxruntime_gpu-1.7.0-cp38-cp38-linux_aarch64.whl 
rm -rf onnxruntime_gpu-1.7.0-cp38-cp38-linux_aarch64.whl

#add user to the input device so it can read mouse
sudo usermod -aG input openpilot

sudo apt autoremove -y

cd /data/openpilot
USE_WEBCAM=1 scons -j $(nproc)

sudo reboot
