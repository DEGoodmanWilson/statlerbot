FROM ubuntu:xenial

MAINTAINER D.E. Goodman-Wilson

RUN dpkg --add-architecture i386 && rm -rf /var/lib/apt/lists/* && apt-get update && apt-get install -y python-dev sudo build-essential libc++-dev wget git vim libc6-dev-i386 libgmp-dev libmpfr-dev libmpc-dev libc6-dev dh-autoreconf libcurl4-openssl-dev libmicrohttpd-dev libssl-dev libsasl2-dev cmake

RUN wget https://bootstrap.pypa.io/get-pip.py --no-check-certificate && python get-pip.py && pip install -U pip
RUN pip install conan
RUN groupadd 1001 -g 1001
RUN groupadd 1000 -g 1000
RUN useradd -ms /bin/bash conan -g 1001 -G 1000 && echo "conan:conan" | chpasswd && adduser conan sudo
RUN echo "conan ALL= NOPASSWD: ALL\n" >> /etc/sudoers
RUN mkdir  -p /home/conan/.conan
RUN mkdir -p /home/conan/app
ADD . /home/conan/app
RUN mkdir -p /home/conan/app/build
RUN chown -R conan /home/conan
USER conan
RUN export LD_LIBRARY_PATH="/usr/local/lib"
RUN sudo ldconfig
WORKDIR /home/conan
WORKDIR /home/conan/app
RUN conan install -s compiler=gcc -s compiler.version=5.4 -s compiler.libcxx=libstdc++11 --build=missing
WORKDIR /home/conan/app/build
RUN cmake ..
RUN cmake --build .
CMD ["/home/conan/app/build/bin/statlerbot", "-v"]
