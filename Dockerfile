# Use Arch Linux as base
FROM archlinux:latest

# Update and install required packages
RUN pacman -Syu --noconfirm \
    base-devel \
    git \
    cmake \
    qt6-base \
    qt6-svg \
    qt6-tools \
    curl \
    && rm -rf /var/cache/pacman/pkg/*

# Download and install Stockfish (official static binary)
RUN curl -L -o /tmp/stockfish.tar https://github.com/official-stockfish/Stockfish/releases/download/sf_18/stockfish-ubuntu-x86-64-avx2.tar \
    && tar -xvf /tmp/stockfish.tar -C /tmp \
    && cp /tmp/stockfish/stockfish-ubuntu-x86-64-avx2 /usr/bin/stockfish \
    && chmod +x /usr/bin/stockfish \
    && rm -rf /tmp/stockfish /tmp/stockfish.tar

# Build CuteChess from source
RUN git clone https://github.com/cutechess/cutechess.git && \
    cd cutechess && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr && \
    make -j$(nproc) && \
    make install && \
    cd / && rm -rf cutechess

# Set working directory
WORKDIR /app

# Copy engine source code and compile
COPY . /app
RUN make
RUN chmod +x /app/my_engine

# Create match script (no -debug to avoid warning)
RUN printf '#!/bin/bash\ncutechess-cli -engine cmd=/app/my_engine proto=uci -engine cmd=/usr/bin/stockfish proto=uci -each tc=40/60 -games 1 -pgnout /app/game.pgn\n' > /app/run_match.sh \
    && chmod +x /app/run_match.sh

CMD ["/app/run_match.sh"]
