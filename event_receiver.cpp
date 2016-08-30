//
// Created by D.E. Goodman-Wilson on 8/9/16.
//

#include "event_receiver.h"
#include <slack/slack.h>
#include <random>
#include <easylogging++.h>


template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator &g)
{
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

uint8_t d100()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    auto i = dis(gen);
    return i;
}

void event_receiver::handle_error(std::string message, std::string received)
{
    // we don't have to log, because it will be logged for us.
//    LOG(ERROR) << message << " " << received;
//    std::cout << message << " " << received << std::endl;
}

void
event_receiver::handle_unknown(std::shared_ptr<slack::event::unknown> event, const slack::http_event_envelope &envelope)
{
    LOG(WARNING) << "Unknown event: " << event->type;
}

void
event_receiver::handle_message(std::shared_ptr<slack::event::message> event, const slack::http_event_envelope &envelope)
{
    LOG(DEBUG) << "Handling message: " << event->text;

    static std::vector<std::string> phrases = {
            "I wonder if there really is life on another planet.",
            "Boo!",
    };


//TODO this can be highly optimized
    token_storage::token_info token;
    if (store_->get_token_for_team(envelope.team_id, token))
    {
        if (d100() <= 1) //only respond 1% of the time
        {
            auto phrase = *select_randomly(phrases.begin(), phrases.end());
            slack::slack c{token.bot_token};
            c.chat.postMessage(event->channel, phrase);
        }
    }
}

event_receiver::event_receiver(server *server, token_storage *store, const std::string &verification_token) :
        route_set{server},
        handler_{[=](const slack::team_id team_id) -> std::string
                     {
                         token_storage::token_info token;
                         if (store->get_token_for_team(team_id, token))
                         {
                             return token.bot_token;
                         }
                         return "";
                     },
                 verification_token},
        store_{store}
{

    server->handle_request(request_method::POST, "/event", [&](auto req) -> response
        {
            if(!req.body.empty())
                return {handler_.handle_event(req.body)};
            else if(!req.params["event"].empty())
                return {handler_.handle_event(req.params["event"])};
            return {404};
        });


    //event handlers
    handler_.on_error(std::bind(&event_receiver::handle_error,
                                this,
                                std::placeholders::_1,
                                std::placeholders::_2));
    handler_.on<slack::event::unknown>(std::bind(&event_receiver::handle_unknown,
                                                 this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2));
    handler_.on<slack::event::message>(std::bind(&event_receiver::handle_message,
                                                 this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2));

    //dialog responses
    handler_.hears("They aren’t half bad.", [](const auto &message)
        {
            message.reply("Nope, they’re _all_ bad!");
        });


    handler_.hears("What’s all the commotion about?", [](const auto &message)
        {
            message.reply("Waldorf, the bunny ran away!");
        });
    handler_.hears("Well, you know what that makes him ---", [](const auto &message)
        {
            message.reply("Smarter than us!");
        });


    handler_.hears("Boooo!", [](const auto &message)
        {
            message.reply("That was the worst thing I’ve ever heard!");
        });
    handler_.hears("It was terrible!", [](const auto &message)
        {
            message.reply("Horrendous!");
        });
    handler_.hears("Well it wasn’t that bad.", [](const auto &message)
        {
            message.reply("Oh, yeah?");
        });
    handler_.hears("Well, there were parts of it I liked!", [](const auto &message)
        {
            message.reply("Well, I liked alot of it.");
        });
    handler_.hears("Yeah, it was _good_ actually.", [](const auto &message)
        {
            message.reply("It was great!");
        });
    handler_.hears("It was wonderful!", [](const auto &message)
        {
            message.reply("Yeah, bravo!");
        });
    handler_.hears("More!", [](const auto &message)
        {
            message.reply("More!");
        });
}
