//
// Created by D.E. Goodman-Wilson on 8/9/16.
//

#include "event_receiver.h"
#include <slack/slack.h>
#include <random>
#include "logging.h"

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
    handler_.hears(std::regex{"^They aren’t half bad.$"}, [](const auto &message)
    {
        message.reply("Nope, they’re _all_ bad!");
    });


    handler_.hears(std::regex{"^What’s all the commotion about?$"}, [](const auto &message)
    {
        message.reply("Waldorf, the bunny ran away!");
    });
    handler_.hears(std::regex{"^Well, you know what that makes him…$"}, [](const auto &message)
    {
        message.reply("Smarter than us!");
    });


    handler_.hears(std::regex{"^Boooo!$"}, [](const auto &message)
    {
        message.reply("That was the worst thing I’ve ever heard!");
    });
    handler_.hears(std::regex{"^It was terrible!$"}, [](const auto &message)
    {
        message.reply("Horrendous!");
    });
    handler_.hears(std::regex{"^Well it wasn’t that bad.$"}, [](const auto &message)
    {
        message.reply("Oh, yeah?");
    });
    handler_.hears(std::regex{"^Well, there were parts of it I liked!$"}, [](const auto &message)
    {
        message.reply("Well, I liked alot of it.");
    });
    handler_.hears(std::regex{"^Yeah, it was _good_ actually.$"}, [](const auto &message)
    {
        message.reply("It was great!");
    });
    handler_.hears(std::regex{"^It was wonderful!$"}, [](const auto &message)
    {
        message.reply("Yeah, bravo!");
    });
    handler_.hears(std::regex{"^More!$"}, [](const auto &message)
    {
        message.reply("More!");
    });

    handler_.hears(std::regex{"^You know, that's pretty catchy.$"}, [](const auto &message)
    {
        message.reply("So is smallpox.");
    });

    handler_.hears(std::regex{"^More! More!$"}, [](const auto &message)
    {
        message.reply("No, not so loud: they may hear you!");
    });

    handler_.hears(std::regex{"^Yeah, whadya think?$"}, [](const auto &message)
    {
        message.reply("Beats sitting home watching television.");
    });

    handler_.hears(std::regex{"^What did you like about it?$"}, [](const auto &message)
    {
        message.reply("It was the _last_ message!");
    });

    handler_.hears(std::regex{"^Have we ever said that this channel is for the birds?$"}, [](const auto &message)
    {
        message.reply("Yes, and we'll keep saying it until it gets a laugh.");
    });

    handler_.hears(std::regex{"^Do you think there's life in outer space?$"}, [](const auto &message)
    {
        message.reply("There's certainly none in this channel.");
    });

    handler_.hears(std::regex{"^Well, this has been a day to remember.$"}, [](const auto &message)
    {
        message.reply("Why is that?");
    });

    handler_.hears(std::regex{"^Why?$"}, [](const auto &message)
    {
        message.reply("I'm going to find out if you can sue a team for breach of taste!");
    });

    handler_.hears(std::regex{"^:one:$"}, [](const auto &message)
    {
        message.reply("You gave him a one?");
    });

    handler_.hears(std::regex{"^More! More!$"}, [](const auto &message)
    {
        message.reply("Less! Less!");
    });

    handler_.hears(std::regex{"^Yeah? What's that got to do with what we just read?$"}, [](const auto &message)
    {
        message.reply("Nothing, just thought I'd mention it.");
    });

    handler_.hears(std::regex{"^You know, I'm really going to enjoy today!$"}, [](const auto &message)
    {
        message.reply("You plan to like this channel?");
    });

    handler_.hears(std::regex{"^:tv: What's the name of this movie?$"}, [](const auto &message)
    {
        message.reply("\"Beach Blanket Frankenstein\".");
    });
    handler_.hears(std::regex{"^Awful.$"}, [](const auto &message)
    {
        message.reply("Terrible film!");
    });
    handler_.hears(std::regex{"^Yeah, well, we could read this channel instead.$"}, [](const auto &message)
    {
        message.reply(":eyes:");
    });
    handler_.hears(std::regex{"^:eyes:$"}, [](const auto &message)
    {
        message.reply("Wonderful.");
    });

    handler_.hears(std::regex{"^How do they do it?$"}, [](const auto &message)
    {
        message.reply("How do _we read_ it?");
    });
    handler_.hears(std::regex{"^_Why_ do we read it?$"}, [](const auto &message)
    {
        message.reply("Why do _the rest of you_ read it?");
    });

    handler_.hears(std::regex{"^What, you mean you actually like this channel now?$"}, [](const auto &message)
    {
        message.reply("No, they've made the channel even worse!");
    });

    handler_.hears(std::regex{"^Eh, this channel is good for what ails me.$"}, [](const auto &message)
    {
        message.reply("Well, what ails ya?");
    });

    handler_.hears(std::regex{"^That seemed like something very different.$"}, [](const auto &message)
    {
        message.reply("Did you like it?");
    });
    handler_.hears(std::regex{"^No.$"}, [](const auto &message)
    {
        message.reply("Then it wasn't different.");
    });

    handler_.hears(std::regex{"^:zzz:$"}, [](const auto &message)
    {
        message.reply("Besides me?");
    });

    handler_.hears(std::regex{"^Ohh...$"}, [](const auto &message)
    {
        message.reply("What's wrong with you?");
    });
    handler_.hears(std::regex{"^It's either this channel or indigestion. I hope it's indigestion.$"}, [](const auto &message)
    {
        message.reply("Why indigestion?");
    });

    handler_.hears(std::regex{"^What's the point?$"}, [](const auto &message)
    {
        message.reply("You're right. Forget it.");
    });

    handler_.hears(std::regex{"^That was a funny comment.$"}, [](const auto &message)
    {
        message.reply("Yes, it was. I wonder if they meant it that way.");
    });
}
