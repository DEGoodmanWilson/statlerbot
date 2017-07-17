FROM degoodmanwilson/conan-luna

MAINTAINER D.E. Goodman-Wilson

WORKDIR /app
ADD . /app
RUN conan --version
RUN conan install --build=missing
RUN cmake .
RUN cmake --build .
ENV PORT 8080
EXPOSE 8080
