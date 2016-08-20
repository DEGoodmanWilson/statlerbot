//
// Created by D.E. Goodman-Wilson on 8/19/16.
//

#pragma once

#include <regex>
#include <vector>
#include <utility>
#include <slack/slack.h>

class message_router
{
public:
    message_router() = default;
    ~message_router() = default;

    bool route(const slack::event::message &message);

    //TODO we want to eventually move to slack::message, really.
    //TODO we want to be able to limit matches against mentions and direct mentions as well! Which means we need to know our user_id!
    using hears_cb = std::function<void(const slack::event::message &message)>;

    template<typename T>
    void hears(T&& message, hears_cb callback)
    {
        callbacks_.emplace_back(std::regex{std::forward<T>(message)}, callback);
    }

private:
    //let's just brute force this for now
    std::vector<std::pair<std::regex, hears_cb>> callbacks_;
};