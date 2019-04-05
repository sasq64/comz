
#include <asio/connect.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>

using asio::ip::tcp;

int main()
{
    asio::io_context io;
    tcp::resolver resolver(io);
    auto endpoints = resolver.resolve("apone.org", "80");

    tcp::socket socket(io);
    asio::connect(socket, endpoints);

    std::vector<uint8_t> v(128);

    socket.async_read_some(asio::buffer(v),
                           [](std::error_code const& error, size_t count) {
                               std::cout << "Read some";
                           });

    io.run();

    return 0;
}
