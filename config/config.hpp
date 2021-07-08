#ifndef RS_CONFIG_HPP
#define RS_CONFIG_HPP

#include <string>
#include <vector>

class Configuration {
public:
    bool readConfiguration(const std::string &file);
    const std::string &getSourcePath();
    const std::vector<std::tuple<std::string, std::string, std::string>> &getLibraries();
    const std::vector<std::pair<std::string,std::string>> &getTargetPaths();
    const std::vector<std::pair<std::string,std::string>> &getUsers();
private:
    std::vector<std::tuple<std::string,std::string,std::string>> libraries;
    std::string source_path;
    std::vector<std::pair<std::string, std::string>> target_paths;
    std::vector<std::pair<std::string, std::string>> users;
};

#endif
