#include <iostream>
#include <thread>
#include <condition_variable>
#include <luna/luna.h>
#include <slack/slack.h>
#include <easylogging++.h>
#include "token_storage.h"
#include "event_receiver.h"

#define ELPP_LOGGING_FLAGS_FROM_ARG
#define ELPP_THREAD_SAFE
#define ELPP_FORCE_USE_STD_THREAD
INITIALIZE_EASYLOGGINGPP

void luna_logger(luna::log_level level, const std::string &message)
{
    switch(level)
    {
        case luna::log_level::DEBUG:
            LOG(DEBUG) << message;
            break;
        case luna::log_level::INFO:
            LOG(INFO) << message;
            break;
        case luna::log_level::WARNING:
            LOG(WARNING) << message;
            break;
        case luna::log_level::ERROR:
            LOG(ERROR) << message;
            break;
        case luna::log_level::FATAL:
            LOG(FATAL) << message;
            break;
    }
}

void slack_logger(slack::log_level level, const std::string &message)
{
    switch(level)
    {
        case slack::log_level::DEBUG:
            LOG(DEBUG) << message;
            break;
        case slack::log_level::INFO:
            LOG(INFO) << message;
            break;
        case slack::log_level::WARNING:
            LOG(WARNING) << message;
            break;
        case slack::log_level::ERROR:
            LOG(ERROR) << message;
            break;
        case slack::log_level::FATAL:
            LOG(FATAL) << message;
            break;
    }
}

int main(int argc, char* argv[])
{
    el::Loggers::addFlag(el::LoggingFlag::HierarchicalLogging);


    START_EASYLOGGINGPP(argc, argv);

//    el::Configurations defaultConf;
//    defaultConf.setToDefault();
//    // Values are always std::string
//    defaultConf.set(el::Level::Info,
//                    el::ConfigurationType::Format, "%datetime %level %msg");
//    // default logger uses default configurations
//    el::Loggers::reconfigureLogger("default", defaultConf);

    //http://www.tutorialspoint.com/cplusplus/cpp_signal_handling.htm

    // First, let's check those env variables
    uint16_t port = 8080;
    if (auto port_str = std::getenv("PORT"))
    {
        port = atoi(port_str);
    }



    // Now, let's stand up a storage layer
    token_storage store;

    // Next, let's stand up a webserver
    // Let's not worry about TLS for now, as we'll stand up behind ngrok for now
    luna::server server{luna::server::port{port}};

    if (!server)
    {
        LOG(FATAL) << "Failed to stand up webserver!";
        return -1;
    }

    luna::set_logger(luna_logger);

    slack::set_logger(slack_logger);

    LOG(INFO) << "Server started on port " << std::to_string(server.get_port());

    event_receiver receiver{&server, &store, ""}; //use empty string because beep boop is doing the checking for us.

    //IDLE UNTIL DEAD basically just stop this thread in its tracks
    std::mutex m;
    std::condition_variable cv;
    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, []{return false;});
    }

    return 0;
}