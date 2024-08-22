FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
    build-essential \
    cmake \
    python3 \
    python3-pip \
    python3-tk \
    git \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install --no-cache-dir \
    && rm -rf /root/.cache/pip

WORKDIR /workspace

COPY . /workspace

RUN cd build && cmake .. && make

CMD ["/bin/bash"]