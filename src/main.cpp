#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "file_transfer.cpp"
#include "file_watcher.cpp"
#include "conflict_resolver.cpp"

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

using boost::asio::ip::tcp;

// 本地服务器版本：监听本地电脑的文件变化
// 服务器端：启动服务器，接收文件并处理潜在的文件冲突


void start_local_server(boost::asio::io_context& io_context, unsigned short port, Logger& logger) {
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

    while (true) {
        std::cout << "Local Server: Waiting for incoming files..." << std::endl;

        // 接收文件
        std::string received_file;
        FileTransfer file_transfer(io_context, "");

        try {
            received_file = file_transfer.receive_file(acceptor);
            std::cout << "Local Server: Received file: " << received_file << std::endl;
            logger.log("Local Server: Received file: " + received_file);
        } catch (const std::exception& e) {
            std::cerr << "Error receiving file: " << e.what() << std::endl;
            logger.log("Error receiving file: " + std::string(e.what()));
            continue;
        }

        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream timestamp;
        timestamp << std::put_time(std::localtime(&now_time_t), "%Y%m%d_%H%M%S");

        std::string time_received_file = received_file+ "_" + timestamp.str();

        // 动态生成本地文件路径用于冲突检测
        std::string local_file = "sync_folder_backup/" + time_received_file;

        // 检查是否存在同名文件，触发冲突解决机制
        if (std::ifstream(local_file)) {
            std::cout << "Local Server: Conflict detected with " << local_file << std::endl;
            logger.log("Local Server: Conflict detected with " + local_file);

            // 调用冲突解决模块，生成解决后的文件
            std::string resolved_file = ConflictResolver::resolve_conflict(local_file, time_received_file);
            std::cout << "Local Server: Conflict resolved, result saved in " << time_received_file << std::endl;
            logger.log("Local Server: Conflict resolved, result saved in " + time_received_file);
        } else {
            std::cout << "Local Server: No conflict detected. File saved as " << received_file << std::endl;
            logger.log("Local Server: No conflict detected. File saved as " + received_file);
        }
    }
}

// 远程服务器版本：监听来自客户端的文件变化
void start_remote_server(boost::asio::io_context& io_context, unsigned short port, Logger& logger) {
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

    while (true) {
        std::cout << "Remote Server: Waiting for incoming files..." << std::endl;

        // 接收文件
        std::string received_file;
        FileTransfer file_transfer(io_context, "");

        try {
            received_file = file_transfer.receive_file(acceptor);
            std::cout << "Remote Server: Received file: " << received_file << std::endl;
            logger.log("Remote Server: Received file: " + received_file);
        } catch (const std::exception& e) {
            std::cerr << "Error receiving file: " << e.what() << std::endl;
            logger.log("Error receiving file: " + std::string(e.what()));
            continue;
        }

        // 动态生成本地文件路径用于冲突检测
        std::string local_file = "server_storage/" + received_file;

        // 检查是否存在同名文件，触发冲突解决机制
        if (std::ifstream(local_file)) {
            std::cout << "Remote Server: Conflict detected with " << local_file << std::endl;
            logger.log("Remote Server: Conflict detected with " + local_file);

            std::string resolved_file = ConflictResolver::resolve_conflict(local_file, received_file);
            std::cout << "Remote Server: Conflict resolved, result saved in " << resolved_file << std::endl;
            logger.log("Remote Server: Conflict resolved, result saved in " + resolved_file);
        } else {
            std::cout << "Remote Server: No conflict detected. File saved as " << received_file << std::endl;
            logger.log("Remote Server: No conflict detected. File saved as " + received_file);
        }

    }
}

// 客户端：监控文件夹，检测文件变化并发送文件到服务器
void start_client(boost::asio::io_context& io_context, const std::string& path_to_watch, const std::string& server_host, unsigned short server_port, Logger& logger) {
    FileWatcher file_watcher(path_to_watch, std::chrono::seconds(2));

    auto on_change = [&](const std::string& path, const std::string& action) {
        if (action == "created" || action == "modified") {
            std::cout << "Client: File " << action << ": " << path << std::endl;
            logger.log("File " + action + ": " + path);


            std::string backup_path = "sync_folder_backup/"+std::filesystem::relative(path,"sync_folder").string();
            if(std::filesystem::exists(backup_path)){
                std::string diff = file_watcher.compare_files(backup_path,path);
                logger.log(diff);
            }else{
                logger.log("New backup File Created on: "+path);
            }

            // 发送文件到服务器
            FileTransfer file_transfer(io_context, path);
            file_transfer.send_file(io_context,server_host, server_port,path);
            file_watcher.update_backfolder(path);
        } else if (action == "removed") {
            logger.log("File removed: " + path);

            // 创建一个 FileTransfer 对象用于删除文件同步
            FileTransfer file_transfer(io_context, path);
            file_transfer.delete_file(server_host, server_port, path);
        }
    };

    // 开始监控
    file_watcher.start(on_change);
}

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

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 5) {
        std::cerr << "Usage: " << argv[0] << " <mode> <role> [server_ip] [server_port]" << std::endl;
        std::cerr << "mode: local or remote" << std::endl;
        std::cerr << "role: server or client" << std::endl;
        std::cerr << "server_ip: IP address of the remote server (for client only)" << std::endl;
        std::cerr << "server_port: Port number of the remote server (for client only)" << std::endl;
        return 1;
    }

    boost::asio::io_context io_context;

    std::string mode = argv[1];
    std::string role = argv[2];
    std::string path_to_watch = "./sync_folder";
    std::string server_host;
    unsigned short port = 12345;
    Logger logger("./synctool.log");

    if (mode == "remote") {
        if (argc != 5) {
            std::cerr << "For remote mode, you must specify server_ip and server_port." << std::endl;
            return 1;
        }
        server_host = argv[3];
        port = static_cast<unsigned short>(std::stoi(argv[4]));
    } else if (mode == "local") {
        if (argc != 3) {
            std::cerr << "For local mode, no additional arguments are needed." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Invalid mode: " << mode << std::endl;
        return 1;
    }

    if (mode == "local") {
        if(role=="server"){
            start_local_server(io_context,port,logger);
        }else if(role=="client"){
            server_host="127.0.0.1";
            start_client(io_context, path_to_watch, server_host, port, logger);
        }else{
            std::cerr << "Invalid role: " << role << std::endl;
            return 1;
        }
    } else if (mode == "remote") {
        if(role=="server"){
            start_remote_server(io_context,port,logger);
        }else if(role=="client"){
            start_client(io_context, path_to_watch, server_host, port, logger);
        }else{
            std::cerr << "Invalid role: " << role << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Invalid mode: " << role << std::endl;
        return 1;
    }

    std::thread cli_thread(cli_file_check);
    cli_thread.join();

    return 0;
}
