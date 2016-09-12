FROM ubuntu:xenial

MAINTAINER D.E. Goodman-Wilson

RUN dpkg --add-architecture i386 && rm -rf /var/lib/apt/lists/* && apt-get update && apt-get install -y python-dev sudo build-essential clang-3.6 libc++-dev wget git vim libc6-dev-i386 libgmp-dev libmpfr-dev libmpc-dev libc6-dev nasm dh-autoreconf valgrind libcurl4-openssl-dev libmicrohttpd-dev pkg-config libssl-dev libsasl2-dev cmake
# RUN rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Install clang 3.6
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-3.6 10 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-3.6 10 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/clang 50 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 50 && \
    update-alternatives --install /usr/bin/opt opt /usr/bin/opt-3.6 50 && \
    update-alternatives --install /usr/bin/llvm-nm llvm-nm /usr/bin/llvm-nm-3.6 50

#RUN wget https://cmake.org/files/v3.4/cmake-3.4.1-Linux-x86_64.tar.gz --no-check-certificate && tar -xzf cmake-3.4.1-Linux-x86_64.tar.gz && cp -fR cmake-3.4.1-Linux-x86_64/* /usr && rm -rf cmake-3.4.1-Linux-x86_64 && rm cmake-3.4.1-Linux-x86_64.tar.gz

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
RUN ls -alR .
RUN conan install -s compiler=clang -s compiler.version=3.6 -s compiler.libcxx=libstdc++11 --build=missing
WORKDIR /home/conan/app/build
RUN cmake ..
RUN cmake --build .
RUN ls -alR .
CMD ["/home/conan/app/build/bin/statlerbot"]
