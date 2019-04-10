#include <asio/connect.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

using asio::ip::tcp;

class server
{
    class client_session : public std::enable_shared_from_this<client_session>
    {
        tcp::socket socket_;
        std::vector<uint8_t> v_;

        void read_data()
        {
            v_.resize(128);
            socket_.async_read_some(
                asio::buffer(v_),
                [self = shared_from_this()](std::error_code const& error,
                                            size_t count) {
                    if (error) {
                        std::cout << "Error " << error;
                        return;
                    }
                    if (count == 0)
                        return;
                    std::cout << "Read " << count << " bytes\n";
                    std::string s(self->v_.begin(), self->v_.begin() + count);
                    std::cout << s << "\n";
                    self->read_data();
                });
        }

    public:
        client_session(tcp::socket socket) : socket_(std::move(socket)) {}

        void start() { read_data(); }
    };

    asio::io_context& io_;
    tcp::acceptor acceptor_;

    void accept()
    {
        acceptor_.async_accept([&](std::error_code ec, tcp::socket socket) {
            std::cout << "Connect from "
                      << socket.remote_endpoint().address().to_string() << "\n";
            std::make_shared<client_session>(std::move(socket))->start();
            accept();
        });
    }

public:
    server(asio::io_context& io, int port)
        : io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port))
    {
        accept();
    }
};

int main()
{
    asio::io_context io;

    server s{io, 12345};

    /* tcp::resolver resolver(io); */
    /* auto endpoints = resolver.resolve("apone.org", "http"); */

    /* tcp::socket socket(io); */
    /* asio::connect(socket, endpoints); */

    /* auto len = socket.send(asio::buffer("GET / HTTP/1.0\r\n")); */
    /* std::cout << "Wrote " << len << " bytes\n"; */

    /* std::vector<uint8_t> v(128); */
    /* socket.async_read_some(asio::buffer(v), */
    /*                        [&](std::error_code const& error, size_t count) {
     */
    /*                            std::cout << "Read " << count << " bytes\n";
     */
    /*                            std::string s(v.begin(), v.begin() + count);
     */
    /*                            std::cout << s << "\n"; */
    /*                        }); */
    io.run();
    return 0;
}

