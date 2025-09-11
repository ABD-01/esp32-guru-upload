#!/bin/bash
set -e

# Make sure directory exists
mkdir -p /workspaces/esp

# Clone ESP-IDF if not already present
if [ ! -d "/workspaces/esp/esp-idf" ]; then
    git clone -b v5.5.1 --recursive https://github.com/espressif/esp-idf.git /workspaces/esp/esp-idf --depth 1
fi

# Install ESP-IDF tools
/workspaces/esp/esp-idf/install.sh
. /workspaces/esp/esp-idf/export.sh

# Add alias to user bashrc
grep -qxF "alias get_idf='. /workspaces/esp/esp-idf/export.sh'" ~/.bashrc || \
    echo "alias get_idf='. /workspaces/esp/esp-idf/export.sh'" >> ~/.bashrc

# Install pygments for syntax highlighting
pip3 install pygments --break-system-packages

# Download .gdbinit
wget -P ~ https://git.io/.gdbinit
