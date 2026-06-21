FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive


RUN apt-get update && apt-get install -y \
    build-essential\
    cmake\
    git\
    libjson-c-dev\
    pkg-config\
    sudo\
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -s /bin/bash appuser && \
    echo "appuser ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

WORKDIR /opt

RUN git clone https://git.openwrt.org/project/libubox.git && \
        cd libubox && \
    cmake -DBUILD_LUA=OFF -DCMAKE_INSTALL_PREFIX=/usr/local . && \
    make -j$(nproc) && \
    make install && \
    ldconfig

RUN git clone https://git.openwrt.org/project/uci.git && \
    cd uci && \
    cmake -DBUILD_LUA=OFF -DCMAKE_INSTALL_PREFIX=/usr/local . && \
    make -j$(nproc) && \
    make install && \
    ldconfig

COPY --chown=appuser:appuser . /opt/ftelecom
WORKDIR /opt/ftelecom

RUN make clean && make all

RUN mkdir -p /etc/config 

COPY /src/config/uci_cfg /etc/config/ 

USER appuser

COPY --chown=appuser:appuser docker-entrypoint.sh /opt/ftelecom/
RUN chmod +x /opt/ftelecom/docker-entrypoint.sh

ENTRYPOINT ["/opt/ftelecom/docker-entrypoint.sh"]
