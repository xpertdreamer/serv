#ifndef TS_PROTO_H
#define TS_PROTO_H

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

constexpr char SEPARATOR = '|';
constexpr char SPACE = ' ';

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

struct Response {
    rsp_kind kind = rsp_kind::text;
    size_t count;
    std::string body;
    // build response with fixed structure
    // Structure: <Kind of response (a.1)>|<Length of the body>|<Body (elements)>
    inline std::string build() const
    {
        std::ostringstream oss;
        oss << (char)kind << SEPARATOR << count << SEPARATOR << body;
        return oss.str();
    }

    // returns builded text response
    static Response text(std::string t)   { return {rsp_kind::text, 0, std::move(t)}; }

    // returns builded text response
    template<typename E>
    static Response vec(const std::vector<E>& vector, char type) {
        rsp_kind kind;
        switch (type) {
            case 'F': kind = rsp_kind::vecfloat; break;
            default: kind = rsp_kind::text; break;
        }

        std::ostringstream oss;
        for (size_t i = 0; i < vector.size(); ++i) {
            if (i != 0) oss << SPACE;
            oss << vector[i];
        }

        return {kind, vector.size(), oss.str()};
    }
};

#endif
