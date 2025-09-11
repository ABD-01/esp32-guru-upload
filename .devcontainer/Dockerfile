FROM mcr.microsoft.com/vscode/devcontainers/base:ubuntu

ENV DEBIAN_FRONTEND=noninteractive

# Update container
RUN apt-get update && apt-get upgrade -y

# Install dependencies
RUN apt-get install git wget flex bison gperf python3 python3-pip python3-setuptools python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 \
				libglib2.0-dev libpixman-1-dev libgcrypt20-dev libslirp-dev pkg-config build-essential -y
RUN apt-get clean && rm -rf /var/lib/apt/lists/*


# Set working directory
WORKDIR /workspaces

# Clone qemu at specific commit
# RUN git clone --recursive https://github.com/espressif/qemu.git && \
#     cd qemu && \
#     git checkout 18a7345 && \
#     git submodule update --init --recursive
RUN git clone --depth 1 https://github.com/espressif/qemu.git && \
    cd qemu && \
    git submodule update --init --depth 1


# Build qemu
# see https://github.com/espressif/esp-toolchain-docs/blob/main/qemu/esp32/README.md
WORKDIR /workspaces/qemu
RUN ./configure --target-list=xtensa-softmmu \
    --enable-gcrypt \
    --enable-slirp \
    --enable-debug \
    --disable-sdl \
    --disable-strip --disable-user \
    --disable-capstone --disable-vnc \
    --disable-gtk && \
    make -j$(nproc) && \
    make install


# Set working directory for ESP-IDF
WORKDIR /workspaces/esp
RUN git clone -b v5.5.1 --recursive https://github.com/espressif/esp-idf.git --depth 1 && \
    cd esp-idf && \
    ./install.sh && \
    . ./export.sh

RUN echo "alias get_idf='. /workspaces/esp/esp-idf/export.sh'" >> ~/.bashrc

# terminal colors with xterm
ENV TERM xterm

CMD [ "/bin/bash" ]