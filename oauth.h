//
// Created by D.E. Goodman-Wilson on 8/8/16.
//

#pragma once

#include <luna/luna.h>
#include "token_storage.h"
#include "route_set.h"

using namespace luna;

class oauth : public route_set
{
public:
    oauth(server* server, token_storage* store, const std::string &scope, const std::string &client_id, const std::string &client_secret); //: server_{server}
};

