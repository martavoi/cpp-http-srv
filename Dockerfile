# syntax=docker/dockerfile:1

# ==============================================================================
# Builder Stage - Compile the application
# ==============================================================================
FROM alpine:3 AS builder

# Accept version as build argument
ARG VERSION=0.1.0

# Install build dependencies
# - g++ for C++23 support (Alpine 3.21+ has GCC 13.2+)
# - cmake for build system
# - boost-dev for Boost libraries (ASIO, Beast, Log)
# - make for build process
RUN apk add --no-cache \
    g++ \
    cmake \
    make \
    boost-dev \
    && rm -rf /var/cache/apk/*

# Set working directory for build
WORKDIR /build

# Copy only dependency files first for better layer caching
COPY CMakeLists.txt ./

# Copy source code
COPY main.cpp ./

# Configure and build in Release mode with optimizations
# Use g++ with libstdc++ (Alpine default)
# Use all available cores for compilation
RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    && cmake --build build --parallel $(nproc) \
    && strip build/bin/cpp-http-srv

# ==============================================================================
# Runtime Stage - Minimal production image
# ==============================================================================
FROM alpine:3 AS runtime

# Install only runtime dependencies
# - libstdc++ for C++ standard library
# - boost-* runtime libraries (system and log, beast/asio are header-only)
RUN apk add --no-cache \
    libstdc++ \
    boost-system \
    boost-log \
    ca-certificates \
    && rm -rf /var/cache/apk/*

# Create non-root user for security
RUN addgroup -g 1000 httpserver \
    && adduser -D -u 1000 -G httpserver httpserver

# Set working directory
WORKDIR /app

# Copy binary from builder stage
COPY --from=builder --chown=httpserver:httpserver /build/build/bin/cpp-http-srv /app/cpp-http-srv

# Switch to non-root user
USER httpserver

# Expose default port
EXPOSE 8080

# Add healthcheck
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD wget --no-verbose --tries=1 --spider http://localhost:8080/ || exit 1

# Run the application
ENTRYPOINT ["/app/cpp-http-srv"]
CMD []

