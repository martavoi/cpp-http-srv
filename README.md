# C++ HTTP Server

A low-level HTTP web server implementation using Boost.Asio and Boost.Beast, demonstrating modern C++23 features and asynchronous I/O patterns.

## Overview

This project showcases how to build a production-ready HTTP server from scratch using:
- **Boost.Asio** for asynchronous networking and I/O
- **Boost.Beast** for HTTP protocol handling
- **C++20 Coroutines** for clean async code flow
- **Multi-threading** for concurrent request handling

## Features

- ✅ Async I/O using Boost.Asio coroutines (`co_await`)
- ✅ HTTP/1.1 with keep-alive support
- ✅ Multi-threaded request processing
- ✅ Graceful error handling
- ✅ Docker support with multi-stage builds
- ✅ Modern C++23 standards

## Tech Stack

- **Language**: C++23
- **Compiler**: Clang 20+ (development) / GCC 13+ (Docker)
- **Build System**: CMake 3.28+
- **Libraries**:
  - Boost 1.84+ (Asio, Beast, Log)
  - libc++ (Clang) / libstdc++ (GCC)

## Building

### Prerequisites

**Local Development:**
- CMake 3.28+
- Clang 20+ or GCC 13+
- Boost 1.84+

**Docker:**
- Docker Engine

### Local Build

```bash
# Configure with CMake
cmake -S . -B build \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_CXX_FLAGS="-stdlib=libc++"

# Build
cmake --build build

# Run
./build/bin/cpp-http-srv
```

If you have Boost installed in a custom location:
```bash
cmake -S . -B build \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_CXX_FLAGS="-stdlib=libc++" \
  -DBOOST_ROOT=/path/to/boost
```

### Docker Build

```bash
# Build image
docker build -t cpp-http-srv:latest .

# Run container
docker run -p 8080:8080 cpp-http-srv:latest

# Or use a different port if 8080 is reserved
docker run -p 8081:8080 cpp-http-srv:latest
```

## Usage

The server starts on port 8080 by default and accepts HTTP requests:

```bash
# Using curl
curl http://localhost:8080/

# Using wget
wget -O- http://localhost:8080/

# Using browser
# Navigate to http://localhost:8080/
```

**Response:**
```
Hello, World!
```

## Architecture

### Request Flow

1. **Acceptor** listens on port 8080 for incoming connections
2. Each connection spawns a **coroutine** that handles the socket lifecycle
3. Requests are parsed by **Beast HTTP parser**
4. Responses are constructed and sent back
5. Connection supports **HTTP keep-alive** for multiple requests
6. **4 worker threads** process requests concurrently

### Key Components

```
io_context (event loop)
    ├── acceptor coroutine (accepts connections)
    └── connection coroutines (handle requests)
            ├── async_read (parse HTTP request)
            ├── process & build response
            └── async_write (send HTTP response)
```

### Multi-threading

The server uses a thread pool pattern:
- 4 threads run the `io_context` event loop
- Work is distributed automatically by Boost.Asio
- Each connection is handled independently
- Thread-safe logging with Boost.Log

## Implementation Details

### HTTP Keep-Alive
- Respects `Connection: keep-alive` header from clients
- Reuses TCP connections for multiple requests
- Reduces connection overhead

### Buffer Management
- Uses `flat_buffer` for efficient memory usage
- Buffer is cleared between requests on persistent connections
- Prevents memory leaks on long-lived connections

### Error Handling
- Graceful handling of client disconnections
- Timeout on idle connections (30 seconds)
- Non-blocking operations with proper error codes

### Socket Options
- `SO_REUSEADDR` enabled for quick server restarts
- Binds to `0.0.0.0` for accessibility from all interfaces

## Development

### VS Code Setup

The repository includes VS Code configuration for:
- **Build task**: Ctrl+Shift+B to compile
- **IntelliSense**: Proper C++23 and Boost.Asio support
- **Clang with libc++**: Matches development environment

### Adding Features

To extend the server:
1. Modify request handling in the connection coroutine (line 60-66 in `main.cpp`)
2. Add routing logic based on `req.target()`
3. Implement custom response handlers
4. Add middleware for logging, auth, etc.

## Performance Considerations

- **Async I/O**: Non-blocking operations maximize throughput
- **Thread pool**: Scales with CPU cores
- **Zero-copy**: Beast minimizes memory allocations
- **Connection pooling**: Keep-alive reduces TCP overhead

## License

MIT License - Feel free to use this as a learning resource or starting point for your own projects.

## Acknowledgments

Built with:
- [Boost.Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html)
- [Boost.Beast](https://www.boost.org/doc/libs/release/libs/beast/doc/html/index.html)
- Modern C++ coroutines

