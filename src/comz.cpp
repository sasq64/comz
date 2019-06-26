
#include "messageboard.h"

#include "comz_generated.h"

#include <asio/connect.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include <fmt/format.h>

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <unordered_map>
#include <utility>
#include <vector>

using asio::ip::tcp;
using std::byte;

class Session
{
    std::string name_;
    std::unique_ptr<MessageBoard> board_;

public:
    void login_messageboard(sqlite3db::Database& db, std::string const& name)
    {
        board_ = std::make_unique<MessageBoard>(db, 0);
        name_ = name;
    }

    void post(std::string const& topic, std::string const& text)
    {
        board_->post(topic, text);
    }
};

class Server
{
    class client_session : public std::enable_shared_from_this<client_session>
    {
        Server& server_;
        tcp::socket socket_;
        std::vector<byte> data_;

        void handle_message(Message const* msg)
        {
            fmt::print("Hash {:x}\n", msg->session());

            auto& session = server_.get_session(msg->session());
            if (msg->data_type() == MsgData::MsgData_LoginMessage) {
                auto* login = msg->data_as_LoginMessage();
                session.login_messageboard(server_.get_database(),
                                           login->name()->str());
                fmt::print("Login {}\n", login->name()->str());
            } else if (msg->data_type() == MsgData::MsgData_PostMessage) {
                auto* post = msg->data_as_PostMessage();
                session.post(post->topic()->str(), post->text()->str());
            }
        }

        static constexpr int Max_Message_Length = 128;

        void read_data()
        {
            data_.resize(Max_Message_Length);
            socket_.async_read_some(
                asio::buffer(data_),
                [self = shared_from_this()](std::error_code const& error,
                                            size_t count) {
                    if (error) {
                        fmt::print("Error:{}\n", error.message());
                        return;
                    }
                    if (count != 0) {
                        fmt::print("Read {} bytes\n", count);
                        self->handle_message(GetMessage(self->data_.data()));
                    }
                    self->read_data();
                });
        }

    public:
        client_session(tcp::socket socket, Server& server)
            : socket_(std::move(socket)), server_(server)
        {}

        void start() { read_data(); }
    };

    std::unordered_map<uint64_t, Session> sessions;
    sqlite3db::Database db;
    asio::io_context& io_;
    tcp::acceptor acceptor_;

    void accept()
    {
        acceptor_.async_accept([&](std::error_code ec, tcp::socket socket) {
            auto endpoint = socket.remote_endpoint().address().to_string();
            fmt::print("Connect from {}\n", endpoint);
            std::make_shared<client_session>(std::move(socket), *this)->start();
            accept();
        });
    }

public:
    Server(asio::io_context& io, int port)
        : db("comz.db"), io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port))
    {
        accept();
    }

    Session& get_session(uint64_t hash) { 
        auto it = sessions.find(hash);
        if(it == sessions.end()) {
            return sessions.emplace(hash, Session{}).first->second;
        }
        return it->second;
    }
    sqlite3db::Database& get_database() { return db; }
};

int main()
{
    asio::io_context io;
    Server s{io, 12345};
    io.run();
    return 0;
}

