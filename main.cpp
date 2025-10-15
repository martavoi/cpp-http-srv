#include <boost/asio.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/impl/read.hpp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/log/trivial.hpp>
#include <thread>

using namespace std;

namespace asio = boost::asio;
namespace beast = boost::beast;

int main() {

    asio::io_context io_ctx;

    // init acceptor socket
    asio::ip::tcp::acceptor acceptor(io_ctx);
    acceptor.open(asio::ip::tcp::v4());
    acceptor.set_option(asio::socket_base::reuse_address(true));
    acceptor.bind(asio::ip::tcp::endpoint(asio::ip::address_v4::any(), 8080));
    acceptor.listen();

    asio::co_spawn(io_ctx, [acceptor = std::move(acceptor)]() mutable -> asio::awaitable<void> {
        for (;;)
        {
            auto [ec, socket] = co_await acceptor.async_accept(asio::as_tuple(asio::use_awaitable));
            if (ec)
            {
                BOOST_LOG_TRIVIAL(error) << "Error accepting connection: " << ec.message();
                continue;
            }

            BOOST_LOG_TRIVIAL(info) << "Accepted connection from " << socket.remote_endpoint();
            asio::co_spawn(socket.get_executor(), [socket = std::move(socket)]() mutable -> asio::awaitable<void> {
                beast::tcp_stream stream(std::move(socket));
                beast::flat_buffer buffer;
                for (;;)
                {
                    stream.expires_after(std::chrono::seconds(30));

                    beast::http::request<beast::http::string_body> req;
                    auto [ec_read, bytes_read] = co_await beast::http::async_read(stream, buffer, req, asio::as_tuple(asio::use_awaitable));
                    if (ec_read)
                    {
                        // End of stream is normal when client closes connection
                        // Operation canceled happens on timeout - also normal
                        if (ec_read != beast::http::error::end_of_stream && 
                            ec_read != asio::error::operation_aborted)
                        {
                            BOOST_LOG_TRIVIAL(error) << "Error reading request: " << ec_read.message();
                        }
                        break;
                    }

                    BOOST_LOG_TRIVIAL(info) << "Request " << req.method() << " " << req.target() << " received from " << stream.socket().remote_endpoint() << " with " << bytes_read << " bytes";
                    
                    // Build response
                    beast::http::response<beast::http::string_body> res;
                    res.keep_alive(req.keep_alive());
                    
                    if (req.method() == beast::http::verb::get && req.target() == "/")
                    {
                        res.result(beast::http::status::ok);
                        res.set(beast::http::field::content_type, "text/plain");
                        res.body() = "Hello, World!\n";
                    }
                    else
                    {
                        res.result(beast::http::status::not_found);
                        res.set(beast::http::field::content_type, "text/plain");
                        res.body() = "Not Found\n";
                    }
                    res.prepare_payload();

                    auto [ec_write, bytes_written] = co_await beast::http::async_write(stream, res, asio::as_tuple(asio::use_awaitable));
                    if (ec_write)
                    {
                        BOOST_LOG_TRIVIAL(error) << "Error writing response: " << ec_write.message();
                        break;
                    }

                    BOOST_LOG_TRIVIAL(info) << "Response sent to " << stream.socket().remote_endpoint() << " with " << bytes_written << " bytes";

                    // Clear buffer for next request
                    buffer.consume(buffer.size());
                    
                    if (!req.keep_alive())
                    {
                        break;
                    }
                }
            }, asio::detached);
        }
    }, asio::detached);
 
    // run io_context in threads (N-1 workers + main thread = N total)
    const auto parallelism = 4;
    auto threads = vector<jthread>(parallelism - 1);
    for (auto& thread : threads)
    {
        thread = jthread([&io_ctx]() {
            io_ctx.run();
        });
    }

    // Run in main thread too (4th worker)
    io_ctx.run();
    
    return 0;
}