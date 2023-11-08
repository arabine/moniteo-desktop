#pragma once

#include <string>

struct Tag {
    int64_t id{0};
    uint64_t timestamp{0};
};

class IAppEvent
{
public:
    virtual void Message(const std::string &message) = 0;
};


