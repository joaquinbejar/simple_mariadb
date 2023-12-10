# Start from the latest Ubuntu image
FROM ubuntu:latest

ENV LANG=C.UTF-8 LC_ALL=C.UTF-8
ENV PATH /usr/local/bin:/usr/local/sbin:$PATH
ENV GIT_SSL_NO_VERIFY=true

RUN apt update && apt --no-install-recommends install -y build-essential ninja-build cmake clang gdb git libssl-dev zlib1g-dev && \
        apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /app/bin
WORKDIR /app/lib
WORKDIR /app/src

COPY . /app/src
WORKDIR /app/src/build
RUN cmake .. -S /app/src -B /app/src/build
RUN make
RUN make install


# Change owner of app folder
RUN chown -R 1001 /app

# Set user
USER 1001


CMD ["sleep", "infinity"]

