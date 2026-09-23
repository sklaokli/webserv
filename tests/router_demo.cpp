#include "config/Config.hpp"
#include "http/Router.hpp"
#include "utils/Logger.hpp"
#include <iostream>
#include <sstream>

// Component harness, not a network server or an HTTP request parser.
int main(int argc, char** argv) {
    if (argc != 4 && argc != 5) {
        std::cerr << "Usage: router_demo CONFIG METHOD TARGET [REPEAT]\n";
        return 2;
    }
    Logger::setLogLevel(ERROR);
    try {
        Config config(argv[1]);
        config.parse();
        unsigned int repeat = 1;
        if (argc == 5) {
            std::istringstream input(argv[4]);
            if (!(input >> repeat) || !input.eof() || repeat < 1 || repeat > 10000)
                return 2;
        }
        RouteResult result;
        for (unsigned int i = 0; i < repeat; ++i)
            result = Router::route(argv[2], argv[3], config.getServers().at(0));
        if (result.action != RouteResult::RESPONSE) {
            std::cout << "DELEGATE "
                << (result.action == RouteResult::CGI_HANDLER ? "CGI" : "METHOD")
                << "\n" << result.base << "\n" << result.relativePath << "\n";
        } else {
            std::cout << result.response.serialize(std::string(argv[2]) == "HEAD");
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 2;
    }
    return 0;
}
