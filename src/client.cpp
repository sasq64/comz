
#include "comz_generated.h"

#include <coreutils/utils.h>

#include <asio/connect.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <linenoise.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

using asio::ip::tcp;

int main(int argc, char** argv)
{
    asio::io_context io;
    asio::system_error err;
    tcp::resolver resolver(io);
    std::vector<uint8_t> data;

    tcp::socket sock(io);
    auto endpoints = resolver.resolve({argv[1], argv[2]});
    asio::connect(sock, endpoints);

    linenoiseSetCompletionCallback([](const char* text,
                                      linenoiseCompletions* lc) -> void {
        std::vector<std::string> parts = utils::split(std::string(text), " ");
        linenoiseAddCompletion(lc, "apa");
    });
    while (char* buf = linenoise(">> ")) {
        if (strlen(buf) > 0) {
            linenoiseHistoryAdd(buf);
        }
        free(buf); // NOLINT
    }

    flatbuffers::FlatBufferBuilder builder;

    auto mdata = CreateLoginMessage(builder, builder.CreateString("sasq"));

    auto msg = CreateMessage(builder, MsgId::MsgId_Login, 0x0123456789abcdef,
                             MsgData::MsgData_LoginMessage, mdata.Union());
    builder.Finish(msg);
    asio::write(sock,
                asio::buffer(builder.GetBufferPointer(), builder.GetSize()));
    io.run();
    return 0;
}

