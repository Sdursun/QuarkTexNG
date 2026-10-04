# Host tool chain: native g++ for the code generator, MinGW-w64 for the
# Windows side (QuarkTex.alib) and CMake/Ninja.
FROM debian:bookworm-slim
RUN apt-get update \
 && apt-get install -y --no-install-recommends g++ make cmake ninja-build mingw-w64 \
 && rm -rf /var/lib/apt/lists/*
