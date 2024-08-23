#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <filesystem>

namespace fs = std::filesystem;

void check_file_or_directory(const std::string& path) {
    if (fs::exists(path)) {
        if (fs::is_directory(path)) {
            std::cout << "Path exists and it is a directory: " << path << std::endl;
        } else if (fs::is_regular_file(path)) {
            std::cout << "Path exists and it is a file: " << path << std::endl;
        } else {
            std::cout << "Path exists but it is neither a regular file nor a directory: " << path << std::endl;
        }
    } else {
        std::cout << "Path does not exist: " << path << std::endl;
    }
}

// remote cli to check file exist
void cli_file_check(){
    std::string command;

    while(true){
        std::cout << "Enter command: ";
        std::getline(std::cin, command);

        // Parse the command and take action
        if (command.rfind("file check ", 0) == 0) {
            std::string path = command.substr(11);
            check_file_or_directory(path);
        } else if (command == "exit") {
            std::cout << "Exiting command line interface..." << std::endl;
            break;
        } else {
            std::cout << "Unknown command. Available commands: file check <path>, exit" << std::endl;
        }
    }
}
