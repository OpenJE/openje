// Copyright 2024 OpenJE

#ifndef JE_CLIENT_SERVER_HPP_
#define JE_CLIENT_SERVER_HPP_

#include "types.hpp"
#include "je/net/client/server.hpp"

namespace JE {
	namespace Client {
        class Server : public net::client::server {}; // class Server
    }; // namespace Client
} // namespace JE

#endif // JE_CLIENT::SERVER_HPP_
