//
// Created by D.E. Goodman-Wilson on 8/9/16.
//

#include "event_receiver.h"
#include <random>
#include <iterator>

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
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
    std::cout << i << std::endl;
    return i;
}

void event_receiver::handle_error(std::string message, std::string received)
{
    std::cout << message << " " << received << std::endl;
}

void
event_receiver::handle_unknown(std::shared_ptr<slack::event::unknown> event, const slack::http_event_envelope &envelope)
{
    std::cout << event->type << std::endl;
}

void
event_receiver::handle_message(std::shared_ptr<slack::event::message> event, const slack::http_event_envelope &envelope)
{
    static std::vector<std::string> phrases = {
            "I wonder if there really is life on another planet.",
            "Boo!",
    };

    std::cout << event->type << ": " << event->text << std::endl;

//TODO this can be highly optimized
    token_storage::token_info token;
    if (store_->get_token_for_team(envelope.team_id, token))
    {
        if(d100() <= 1) //only respond 1% of the time
        {
            auto phrase = *select_randomly(phrases.begin(), phrases.end());
            slack::slack c{token.bot_token};
            c.chat.postMessage(event->channel, phrase);
        }
    }

    router_.route(*event);
}

event_receiver::event_receiver(server *server, token_storage *store, const std::string &verification_token) :
        route_set{server},
        handler_{verification_token},
        router_{[=](const slack::team_id team_id) -> std::string
                    {
                        token_storage::token_info token;
                        if (store->get_token_for_team(team_id, token))
                        {
                            return token.bot_token;
                        }
                        return "";
                    }},
        store_{store}
{
    handler_.register_error_handler(std::bind(&event_receiver::handle_error,
                                              this,
                                              std::placeholders::_1,
                                              std::placeholders::_2));
    handler_.register_event_handler<slack::event::unknown>(std::bind(&event_receiver::handle_unknown,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    handler_.register_event_handler<slack::event::message>(std::bind(&event_receiver::handle_message,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));

    server->handle_request(request_method::POST, "/events", [&](auto req) -> response
        {
            return {handler_.handle_event(req.body)};
        });


    //dialog responses
    router_.hears(std::regex{"They aren’t half bad."}, [](const auto &message)
        {
            message.reply("Nope, they’re _all_ bad!");
        });

    router_.hears(std::regex{"What’s all the commotion about?"}, [](const auto &message)
        {
            message.reply("Waldorf, the bunny ran away!");
        });
    router_.hears(std::regex{"Well, you know what that makes him ---"}, [](const auto &message)
        {
            message.reply("Smarter than us!");
        });

    router_.hears(std::regex{"Boooo!"}, [](const auto &message)
        {
            message.reply("That was the worst thing I’ve ever heard!");
        });
    router_.hears(std::regex{"It was terrible!"}, [](const auto &message)
        {
            message.reply("Horrendous!");
        });
    router_.hears(std::regex{"Well it wasn’t that bad."}, [](const auto &message)
        {
            message.reply("Oh, yeah?");
        });
    router_.hears(std::regex{"Well, there were parts of it I liked!"}, [](const auto &message)
        {
            message.reply("Well, I liked alot of it.");
        });
    router_.hears(std::regex{"Yeah, it was _good_ actually."}, [](const auto &message)
        {
            message.reply("It was great!");
        });
    router_.hears(std::regex{"It was wonderful!"}, [](const auto &message)
        {
            message.reply("Yeah, bravo!");
        });
    router_.hears(std::regex{"More!"}, [](const auto &message)
        {
            message.reply("More!");
        });
}
