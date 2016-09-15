//
// Created by D.E. Goodman-Wilson on 8/9/16.
//

#include "event_receiver.h"
#include <slack/slack.h>
#include <random>
//#include <easylogging++.h>


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
//    std::cout << "ERROR: " << message << " " << received << std::endl;
//    std::cout << message << " " << received << std::endl;
}

void
event_receiver::handle_unknown(std::shared_ptr<slack::event::unknown> event, const slack::http_event_envelope &envelope)
{
    std::cout << "WARNING: " << "Unknown event: " << event->type << std::endl;
}

void
event_receiver::handle_message(std::shared_ptr<slack::event::message> event, const slack::http_event_envelope &envelope)
{
    std::cout << "DEBUG: " << "Handling message: " << event->text << std::endl;

    static std::vector<std::string> phrases = {
            "I wonder if there really is life on another planet.",
            "Boo!",
            "Hm. Do you think this channel is educational?",
            "He was doing okay until he left the channel.",
            "I liked that last message.",
            "I'm going to see my lawyer!",
            "You know, the older I get, the more I appreciate good wit.",
            "That really offended me. I'm a student of Shakespeare.",
            "I love it! I love it!",
            "More! More!",
            "I don't believe it! They've managed the impossible! What an achievement! Bravo, bravo!",
            "I wonder if anybody reads this channel besides us?",
            "You know, I think they were trying to make a point with that comment.",
            "You know, that was almost funny.",
            "Are you ready for the end of the world?",
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

    server->handle_request(request_method::POST, "/slack/event", [&](auto req) -> response
    {
        if (req.headers.count("Bb-Slackaccesstoken"))
        {
            token_storage::token_info token{
                    req.headers["Bb-Slackaccesstoken"],
                    req.headers["Bb-Slackbotaccesstoken"],
                    req.headers["Bb-Slackuserid"],
                    req.headers["Bb-Slackbotuserid"],
            };
            store_->set_token(req.headers["Bb-Slackteamid"], token);
        }

        if (!req.body.empty())
        {
            return {handler_.handle_event(req.body)};
        }
        else if (!req.params["event"].empty())
        {
            return {handler_.handle_event(req.params["event"])};
        }
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

    handler_.hears("You know, that's pretty catchy.", [](const auto &message)
    {
        message.reply("So is smallpox.");
    });

    handler_.hears("More! More!", [](const auto &message)
    {
        message.reply("No, not so loud: they may hear you!");
    });

    handler_.hears("Yeah, whadya think?", [](const auto &message)
    {
        message.reply("Beats sitting home watching television.");
    });

    handler_.hears("What did you like about it?", [](const auto &message)
    {
        message.reply("It was the _last_ message!");
    });

    handler_.hears("Have we ever said that this channel is for the birds?", [](const auto &message)
    {
        message.reply("Yes, and we'll keep saying it until it gets a laugh.");
    });

    handler_.hears("Do you think there's life in outer space?", [](const auto &message)
    {
        message.reply("There's certainly none in this channel.");
    });

    handler_.hears("Well, this has been a day to remember.", [](const auto &message)
    {
        message.reply("Why is that?");
    });

    handler_.hears("Why?", [](const auto &message)
    {
        message.reply("I'm going to find out if you can sue a team for breach of taste!");
    });

    handler_.hears(":one:", [](const auto &message)
    {
        message.reply("You gave him a one?");
    });

    handler_.hears("More! More!", [](const auto &message)
    {
        message.reply("Less! Less!");
    });

    handler_.hears("Yeah? What's that got to do with what we just read?", [](const auto &message)
    {
        message.reply("Nothing, just thought I'd mention it.");
    });

    handler_.hears("You know, I'm really going to enjoy today!", [](const auto &message)
    {
        message.reply("You plan to like this channel?");
    });

    handler_.hears(":tv: What's the name of this movie?", [](const auto &message)
    {
        message.reply("\"Beach Blanket Frankenstein\".");
    });
    handler_.hears("Awful.", [](const auto &message)
    {
        message.reply("Terrible film!");
    });
    handler_.hears("Yeah, well, we could read this channel instead.", [](const auto &message)
    {
        message.reply(":eyes:");
    });
    handler_.hears(":eyes:", [](const auto &message)
    {
        message.reply("Wonderful.");
    });

    handler_.hears("How do they do it?", [](const auto &message)
    {
        message.reply("How do _we read_ it?");
    });
    handler_.hears("_Why_ do we read it?", [](const auto &message)
    {
        message.reply("Why do _the rest of you_ read it?");
    });

    handler_.hears("What, you mean you actually like this channel now?", [](const auto &message)
    {
        message.reply("No, they've made the channel even worse!");
    });

    handler_.hears("Eh, this channel is good for what ails me.", [](const auto &message)
    {
        message.reply("Well, what ails ya?");
    });

    handler_.hears("That seemed like something very different.", [](const auto &message)
    {
        message.reply("Did you like it?");
    });
    handler_.hears("No.", [](const auto &message)
    {
        message.reply("Then it wasn't different.");
    });

    handler_.hears(":zzz:", [](const auto &message)
    {
        message.reply("Besides me?");
    });

    handler_.hears("Ohh...", [](const auto &message)
    {
        message.reply("What's wrong with you?");
    });
    handler_.hears("It's either this channel or indigestion. I hope it's indigestion.", [](const auto &message)
    {
        message.reply("Why indigestion?");
    });

    handler_.hears("What's the point?", [](const auto &message)
    {
        message.reply("You're right. Forget it.");
    });

    handler_.hears("That was a funny comment.", [](const auto &message)
    {
        message.reply("Yes, it was. I wonder if they meant it that way.");
    });
}
