#ifndef TS_PROTO_H
#define TS_PROTO_H

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <vector>
#include <arpa/inet.h>

constexpr char SEPARATOR = '|';
constexpr char SPACE = ' ';

constexpr int SYNCPACKET_SIZE = 4;

/*
  a.1
  There are 2 types of response kinds:
      * text - represent string message, such as help message, welcome message and etc.
      * vecfloat - vector of float numbers translated into raw bytes to send via sockets
*/
enum class rsp_kind : char {
    text = 'T',
    vecfloat = 'F',
};

inline bool is_kind(char c)
{
    return c == 'T' || c == 'F';
}

struct Message {
    rsp_kind kind = rsp_kind::text;
    size_t count = 0;
    std::string body;

    // build response with fixed structure
    // Structure: <Kind of response (a.1)>|<Length of the body>|<Body (elements)>
    inline std::string build() const
    {
        std::ostringstream oss;
        oss << (char)kind << SEPARATOR << count << SEPARATOR << body;
        return oss.str();
    }

    static bool parse(const std::string& raw, Message& m)
    {
        if (is_kind(raw[0]) == false) return false;
        size_t sep = raw.find(SEPARATOR);
        if (sep != 1) return false;
        size_t sep_second = raw.find(SEPARATOR, sep + 1);
        if (sep_second == std::string::npos) return false;
        m.kind = (rsp_kind)raw[0];
        m.count = std::stoul(raw.substr(sep + 1, sep_second - sep - 1));
        m.body = raw.substr(sep_second + 1);
        return true;
    }

    // returns builded text response
    static Message text(std::string t)
    {
        return {rsp_kind::text, 0, std::move(t)};
    }

    // returns builded text response
    template<typename E>
    static Message vec(const std::vector<E>& vector, char type)
    {
        rsp_kind kind;
        std::ostringstream oss;
        oss << std::setprecision(9);
        for (size_t i = 0; i < vector.size(); ++i) {
            if (i != 0) oss << SPACE;
            oss << vector[i];
        }

        return {kind, vector.size(), oss.str()};
    }

    template<typename E>
    std::vector<E> as_vec() const
    {
        std::vector<E> result;
        E e;
        std::istringstream iss(body);
        while(iss >> e) result.push_back(e);
        return result;
    }
};

using Response = Message;
using Request = Message;

inline static bool send_ueof(int fd, const void* buffer, size_t n)
{
    auto *b = (const char*)buffer;
    while (n) {
        ssize_t sent;
        if ((sent = send(fd, b, n, 0)) < 0 && errno == EINTR) continue;
        if (sent <= 0) return false;
        b += sent;
        n -= (size_t)sent;
    }
    return true;
}

inline bool send_msg(int fd, const Message& m)
{
    auto packet = m.build();
    uint32_t len = (uint32_t)packet.size();
    return send_ueof(fd, &len, SYNCPACKET_SIZE)
        && send_ueof(fd, packet.data(), packet.size());
}

inline static bool recv_ueof(int fd, void* buffer, size_t n)
{
    auto *b = (char*)buffer;
    while (n) {
        ssize_t sent;
        if ((sent = recv(fd, b, n, 0)) <= 0) return false;
        b += sent;
        n -= (size_t)sent;
    }
    return true;
}

inline bool recv_msg(int fd, Message& m)
{
    uint32_t len;
    if (!recv_ueof(fd, &len, SYNCPACKET_SIZE)) return false;
    len = ntohl(len);
    std::string recieved(len, '\0');
    return recv_ueof(fd, recieved.data(), len) && Message::parse(recieved, m);
}

#endif
