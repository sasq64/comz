
#include "comz_generated.h"

#include <coreutils/file.h>
#include <coreutils/utils.h>

#include <asio/connect.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <fmt/format.h>
#include <linenoise.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

using asio::ip::tcp;

void add_command(std::string const& cmd,
                 std::function<void(std::string)> const& fn)
{}

std::string edit_text()
{
    int lno = 0;
    bool quit = false;
    std::vector<std::string> lines;
    while (!quit) {
        std::string prompt = fmt::format("{:02}: ", lno + 1);
        if (lines.size() > lno)
            linenoisePreloadBuffer(lines[lno].c_str());
        prompt = fmt::format("{:02}: ", lno + 1);
        if (char* buf = linenoise(prompt.c_str())) {
            if (*buf == 0) {
                return utils::join(lines.begin(), lines.end(), "\n");
            }
            if (*buf == '!' && std::isalpha(buf[1])) {
                if (buf[1] == 'e') {
                    lno--;
                    continue;
                }
            }
            if (lno >= lines.size())
                lines.emplace_back(buf);
            else
                lines[lno] = buf;
            lno++;
            free(buf); // NOLINT
        } else {
            if (linenoiseKeyType() == 2) {
                fmt::print("[s]ave [e]dit >");
                fflush(stdout);
                int c = linenoiseGetKey();
                if (c == 'e') {
                    utils::File f{".temp", utils::File::Mode::Write};
                    f.writeString(
                        utils::join(lines.begin(), lines.end(), "\n"));
                    f.close();
                    system("nvim .temp");
                    utils::File f2{".temp", utils::File::Mode::Read};
                    lines.clear();
                    for (auto const& l : f2.lines()) {
                        lines.push_back(l);
                    }
                }

            } else
                return "";
        }
    }
}

int main(int argc, char** argv)
{
    asio::io_context io;
    asio::system_error err;
    tcp::resolver resolver(io);
    std::vector<uint8_t> data;

    fmt::print("{}", edit_text());

    tcp::socket sock(io);
    auto endpoints = resolver.resolve({argv[1], argv[2]});
    asio::connect(sock, endpoints);

    add_command("post", [](std::string const& x) {});

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

