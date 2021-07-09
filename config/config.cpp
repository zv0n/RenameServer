#include "config.hpp"
#include <libconfig.h++>
#include <iostream>

bool Configuration::readConfiguration(const std::string &file) {
    libconfig::Config cfg;
    try {
        cfg.readFile(file);
    } catch (const libconfig::FileIOException &fioex) {
        std::cerr << "Couldn't open configuration file" << std::endl;
        return false;
    } catch (const libconfig::ParseException &pex) {
        std::cerr << "Couldn't parse configuration file" << std::endl;
        return false;
    }
    cfg.lookupValue("source_path", source_path);
    auto &targets = cfg.getRoot()["target_paths"];
    for(auto &target : targets) {
        target_paths.emplace_back(target["path"], target["name"]);
    }
    auto &cfg_libraries = cfg.getRoot()["libraries"];
    for(auto &library : cfg_libraries) {
        libraries.emplace_back(
                library.lookup("path"),
                library.lookup("config"));
    }

    auto &cfg_users = cfg.getRoot()["users"];
    for(auto &user : cfg_users) {
        users.emplace_back(
                user.lookup("user"),
                user.lookup("password"));
    }

    return true;
}

const std::string &Configuration::getSourcePath() {
    return source_path;
}
const std::vector<std::pair<std::string, std::string>> &Configuration::getLibraries() {
    return libraries;
}
const std::vector<std::pair<std::string, std::string>> &Configuration::getTargetPaths() {
    return target_paths;
}
const std::vector<std::pair<std::string,std::string>> &Configuration::getUsers() {
    return users;
}
