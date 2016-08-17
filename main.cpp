#include <iostream>
#include <luna/luna.h>
#include "token_storage.h"
#include "oauth.h"
#include "event_receiver.h"

int main()
{
//http://www.tutorialspoint.com/cplusplus/cpp_signal_handling.htm

    // First, let's check those env variables
    uint16_t port = 8080;
    if (auto port_str = std::getenv("PORT"))
    {
        port = atoi(port_str);
    }

    auto client_id = std::getenv("CLIENT_ID");
    if (!client_id)
    {
        std::cerr << "Please specify CLIENT_ID in the environment!" << std::endl;
        exit(-1);
    }
    auto client_secret = std::getenv("CLIENT_SECRET");
    if (!client_secret)
    {
        std::cerr << "Please specify CLIENT_SECRET in the environment!" << std::endl;
        exit(-1);
    }
    auto verification_token = std::getenv("VERIFICATION_TOKEN");
    if(!verification_token)
    {
        std::cerr << "Please specify VERIFICATION_TOKEN in the environment!" << std::endl;
        exit(-1);
    }

    // Now, let's stand up a storage layer
    token_storage store;

    // Next, let's stand up a webserver
    // Let's not worry about TLS for now, as we'll stand up behind ngrok for now
    luna::server server{luna::server::port{port}};

    if (!server)
    {
        std::cerr << "Failed to stand up webserver!" << std::endl;
        return -1;
    }


    std::cout << "Server started on port " << server.get_port() << std::endl;

    // Now, we need a couple of things: An OAuth server, and a bot server. Let's stand those up now
    oauth oauth{&server, &store, "bot", client_id, client_secret};

    event_receiver receiver{&server, &store, verification_token};

    //We should do something more interesting with this thread, than just idle.
    while (1);

    return 0;
}