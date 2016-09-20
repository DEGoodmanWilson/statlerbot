//
// Created by D.E. Goodman-Wilson on 8/9/16.
//

#include "event_receiver.h"
#include <slack/slack.h>
#include <random>
#include "logging.h"

#define WALDORF_APP_ID "A2CLSFX98"

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

uint8_t d100_()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    auto i = dis(gen);
    return i;
}

std::deque<std::pair<std::string, std::string>> bot_id_deque_;
std::map<std::string, std::string> bot_id_map_;

bool is_from_us_(slack::http_event_client::message message)
{
    slack::bot_id my_bot_id;
    //first, attempt to retrieve our bot id from cache
    if (bot_id_map_.count(message.token.team_id))
    {
        //cache hit
        my_bot_id = bot_id_map_.at(message.token.team_id);
        //bump this token to the end by first removing it then push it onto the tail.
        for (auto i = bot_id_deque_.begin(); i != bot_id_deque_.end(); ++i)
        {
            if (i->first == message.token.team_id)
            {
                bot_id_deque_.erase(i);
                break;
            }
        }
    }
    else
    {
        //cache miss
        slack::slack c{message.token.bot_token};
        auto user = c.users.info(message.token.bot_id).user;
        my_bot_id = *user.profile.bot_id;
        if (bot_id_deque_.size() > 100)
        {
            bot_id_deque_.pop_front(); //remove least-recently accessed element
        }
    }

    //mark this element as most recently accessed
    bot_id_deque_.push_back(std::make_pair(message.token.team_id, my_bot_id));

    if (message.from_user_id == my_bot_id)
    {
        return true;
    }

    return false;
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
    LOG(WARNING) << "Unknown event: " << event->type << ": " << event->raw_event;

    if (event->type == "bb.team_added")
    {
        //we've just been added to the team. Message the app installer.
        //TODO do this in the background
        slack::slack c{envelope.token.bot_token};

        bool is_companion_installed = false;
        auto user_list = c.users.list().members;
        for (const auto &user : user_list)
        {
            if (user.is_bot && user.profile.api_app_id && (*(user.profile.api_app_id) == WALDORF_APP_ID))
            {
                is_companion_installed = true;
                break;
            }
        }

        c.chat.postMessage(envelope.token.user_id, "Thanks for installing me!");
        if (is_companion_installed)
        {
            c.chat.postMessage(envelope.token.user_id,
                               "Just invite Waldorfbot and me into any channel, and we'll get to heckling.");
        }
        else
        {
            c.chat.postMessage(envelope.token.user_id,
                               "Please also install <https://beepboophq.com/bots/469ae2c9f27a48829bcd28f0f276b00c|my friend Waldorfbot!>, then invite us into any channel to start heckling!");
        }
    }
}

//TODO
// void event_receiver::handle_leave_channel()

void event_receiver::handle_join_channel(std::shared_ptr<slack::event::message_channel_join> event, const slack::http_event_envelope &envelope)
{
    //someone just joined a channel, is it us?
    if(event->user != envelope.token.bot_id) return; //it wasn't us
    //TODO was it Waldorf?

    //see if waldorf is in this channel
    //TODO do this in the background
    slack::slack c{envelope.token.bot_token};

    bool is_companion_installed = false;
    slack::user_id companion_bot_id;
    auto user_list = c.users.list().members;
    for (const auto &user : user_list)
    {
        if (user.is_bot && user.profile.api_app_id && (*(user.profile.api_app_id) == WALDORF_APP_ID))
        {
            is_companion_installed = true;
            companion_bot_id = user.id;
            break;
        }
    }

    bool is_companion_in_channel = false;
    if(is_companion_installed)
    {
        auto channel_members = c.channels.info(event->channel).channel.members;
        for (const auto &user : channel_members)
        {
            if (user == companion_bot_id)
            {
                is_companion_in_channel = true;
                break;
            }
        }
    }

    if(!is_companion_installed)
    {
        c.chat.postMessage(event->channel, "Waldorfbot, where are you? Can someone <https://beepboophq.com/bots/469ae2c9f27a48829bcd28f0f276b00c|install Waldorfbot> into this team?");
    }
    else if(!is_companion_in_channel)
    {
        c.chat.postMessage(event->channel, "Waldorfbot, where are you? Can someone invite Waldorfbot into the channel?");
    }
    else
    {
        c.chat.postMessage(event->channel, "Waldorfbot! There you are, old chum.");
    }
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


    if (d100_() <= 1) //only respond 1% of the time TODO make this configurable
    {
        auto phrase = *select_randomly(phrases.begin(), phrases.end());
        slack::slack c{envelope.token.bot_token};
        c.chat.postMessage(event->channel, phrase);
    }
}

event_receiver::event_receiver(server *server, const std::string &verification_token) :
        route_set{server},
        handler_{verification_token}
{

    server->handle_request(request_method::POST, "/slack/event", [&](auto req) -> response
    {
        if (!req.headers.count("Bb-Slackteamid")) //TOOD make this more robust
        {
            return {500, "Missing Beep Boop Headers"};
        }

        slack::token token{
                req.headers["Bb-Slackteamid"],
                req.headers["Bb-Slackaccesstoken"],
                req.headers["Bb-Slackuserid"],
                req.headers["Bb-Slackbotaccesstoken"],
                req.headers["Bb-Slackbotuserid"],
        };

        if (!req.body.empty())
        {
            return {handler_.handle_event(req.body, token)};
        }
        else if (!req.params["event"].empty())
        {
            return {handler_.handle_event(req.params["event"], token)};
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
    handler_.on<slack::event::message_channel_join>(std::bind(&event_receiver::handle_join_channel,
                                                 this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2));

    //dialog responses

    handler_.hears(std::regex{"^They aren’t half bad.$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Nope, they’re _all_ bad!");
    });


    handler_.hears(std::regex{"^What’s all the commotion about\\?$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Waldorf, the bunny ran away!");
    });
    handler_.hears(std::regex{"^Well, you know what that makes him…$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Smarter than us!");
    });


    handler_.hears(std::regex{"^Boooo!$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("That was the worst thing I’ve ever heard!");
    });
    handler_.hears(std::regex{"^It was terrible!$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Horrendous!");
    });
    handler_.hears(std::regex{"^Well it wasn’t that bad.$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Oh, yeah?");
    });
    handler_.hears(std::regex{"^Well, there were parts of it I liked!$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Well, I liked a lot of it.");
    });
    handler_.hears(std::regex{"^Yeah, it was _good_ actually.$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("It was great!");
    });
    handler_.hears(std::regex{"^It was wonderful!$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Yeah, bravo!");
    });
    handler_.hears(std::regex{"^More!$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("More!");
    });

    handler_.hears(std::regex{"^You know, that's pretty catchy.$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("So is smallpox.");
    });

    handler_.hears(std::regex{"^Yeah, whadya think\\?$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Beats sitting home watching television.");
    });

    handler_.hears(std::regex{"^What did you like about it\\?$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("It was the _last_ message!");
    });

    handler_.hears(std::regex{"^Have we ever said that this channel is for the birds\\?$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Yes, and we'll keep saying it until it gets a laugh.");
    });

    handler_.hears(std::regex{"^Do you think there's life in outer space\\?$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("There's certainly none in this channel.");
    });

    handler_.hears(std::regex{"^Well, this has been a day to remember.$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Why is that?");
    });

    handler_.hears(std::regex{"^Why\\?$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("I'm going to find out if you can sue a team for breach of taste!");
    });

    handler_.hears(std::regex{"^:one:$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("You gave him a one?");
    });

    handler_.hears(std::regex{"^More! More!$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Less! Less!");
    });

    handler_.hears(std::regex{"^Yeah\\? What's that got to do with what we just read\\?$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Nothing, just thought I'd mention it.");
    });

    handler_.hears(std::regex{"^You know, I'm really going to enjoy today!$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("You plan to like this channel?");
    });

    handler_.hears(std::regex{"^:tv: What's the name of this movie\\?$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("\"Beach Blanket Frankenstein\".");
    });
    handler_.hears(std::regex{"^Awful.$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Terrible film!");
    });
    handler_.hears(std::regex{"^Yeah, well, we could read this channel instead.$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply(":eyes:");
    });
    handler_.hears(std::regex{"^:eyes:$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Wonderful.");
    });

    handler_.hears(std::regex{"^How do they do it\\?$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("How do _we read_ it?");
    });
    handler_.hears(std::regex{"^_Why_ do we read it\\?$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Why do _the rest of you_ read it?");
    });

    handler_.hears(std::regex{"^What, you mean you actually like this channel now\\?$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("No, they've made the channel even worse!");
    });

    handler_.hears(std::regex{"^Eh, this channel is good for what ails me.$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Well, what ails ya?");
    });

    handler_.hears(std::regex{"^That seemed like something very different.$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Did you like it?");
    });
    handler_.hears(std::regex{"^No.$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Then it wasn't different.");
    });

    handler_.hears(std::regex{"^:zzz:$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Besides me?");
    });

    handler_.hears(std::regex{"^Ohh...$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("What's wrong with you?");
    });
    handler_.hears(std::regex{"^It's either this channel or indigestion. I hope it's indigestion.$"},
                   [](const auto &message)
                   {
                       if (is_from_us_(message))
                       {
                           return;
                       }

                       message.reply("Why indigestion?");
                   });

    handler_.hears(std::regex{"^What's the point\\?$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("You're right. Forget it.");
    });

    handler_.hears(std::regex{"^That was a funny comment.$"}, [](const auto &message)
    {
        if (is_from_us_(message))
        {
            return;
        }

        message.reply("Yes, it was. I wonder if they meant it that way.");
    });


    handler_.hears(std::regex{"^Well, Statlerbot, it's time to go. Thank goodness!$"}, [](const auto &message)
    {
        message.reply("Wait, don't leave me here all by myself!");
    });

    //// Strangely, this is how we find out if we've been kicked. Fragile, I'm guessing. TOTAL HACK ALERT!
    handler_.hears(std::regex{"^You have been removed from #"}, [](const auto &message)
    {
        if(message.from_user_id != "USLACKBOT") return;

        //extract the channel name from the message
        // "You have been removed from #donbot-testing2 by <@U0JFHT99N|don>"
        std::smatch pieces_match;
        std::regex message_regex{"(#[\\w\\d-]+)"};
        if (std::regex_search(message.text, pieces_match, message_regex))
        {
            //post into that channel
            slack::slack c{message.token.bot_token};
            auto channel_name = pieces_match[1].str();
            c.chat.postMessage(channel_name, "Well, Waldorfbot, it's time to go. Thank goodness!");
        }
    });
}
