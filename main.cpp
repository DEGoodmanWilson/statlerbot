#include <iostream>
#include <thread>
#include <condition_variable>
#include <luna/luna.h>
#include <slack/slack.h>
#include "logging.h"
#include "event_receiver.h"
#include "beep_boop_persist.h"

INITIALIZE_EASYLOGGINGPP

void error_logger(luna::log_level level, const std::string &message)
{
    switch (level)
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

void access_logger(const luna::request &request)
{
    LOG(INFO) << request.ip_address << ": " << luna::to_string(request.method) << " " << request.path << " "
              << request.http_version << " " << request.headers.at("user-agent");
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
    START_EASYLOGGINGPP(argc, argv);

//    el::Configurations defaultConf;
//    defaultConf.setToDefault();
//    // Values are always std::string
//    defaultConf.set(el::Level::Info,
//                    el::ConfigurationType::Format, "%datetime %level %msg");
//    // default logger uses default configurations
//    el::Loggers::reconfigureLogger("default", defaultConf);

    //http://www.tutorialspoint.com/cplusplus/cpp_signal_handling.htm

    luna::set_access_logger(access_logger);
    luna::set_error_logger(error_logger);

    slack::set_logger(slack_logger);

    // First, let's check those env variables
    uint16_t port = 8080;
    if (auto port_str = std::getenv("PORT"))
    {
        port = atoi(port_str);
    }

    // Create a memory store
    auto beepboop_token_raw = std::getenv("BEEPBOOP_TOKEN");
    auto beepboop_persist_url_raw = std::getenv("BEEPBOOP_PERSIST_URL");
    std::string beepboop_token, beepboop_persist_url;

    if(beepboop_token_raw && beepboop_persist_url_raw)
    {
        beepboop_token = {beepboop_token_raw};
        beepboop_persist_url = {beepboop_persist_url_raw};
    }
    beep_boop_persist store{beepboop_persist_url, beepboop_token};

    // Now, let's stand up a webserver
    // Let's not worry about TLS for now, as we'll stand up behind ngrok for now
    luna::server server{luna::server::port{port}};

    if (!server)
    {
        LOG(FATAL) << "Failed to stand up webserver!";
        return -1;
    }

    LOG(INFO) << "Server started on port " << std::to_string(server.get_port());

    event_receiver receiver{server, store, ""}; //use empty string because beep boop is doing the checking for us.

    //IDLE UNTIL DEAD basically just stop this thread in its tracks
    server.await();

    return 0;
}

