#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "file_transfer.cpp"
#include "file_watcher.cpp"
#include "conflict_resolver.cpp"
#include "local.cpp"
#include "remote.cpp"

using boost::asio::ip::tcp;

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

void start_local_mode(boost::asio::io_context& io_context, const std::string& path_to_watch, unsigned short server_port, Logger& logger){
    std::thread server_local([&](){
        start_local_server(io_context,server_port,logger);
    });

    std::thread client_local([&](){
        const std::string &server_host = "127.0.0.1";
        start_client(io_context,path_to_watch,server_host,server_port,logger);
    });

    server_local.join();
    client_local.join();
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 5) {
        std::cerr << "Usage: " << argv[0] << " <mode> [server_ip] [server_port]" << std::endl;
        std::cerr << "mode: local or remote" << std::endl;
        std::cerr << "role: server or client" << std::endl;
        std::cerr << "server_ip: IP address of the remote server (for client only)" << std::endl;
        std::cerr << "server_port: Port number of the remote server (for client only)" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    std::string path_to_watch = "./sync_folder";
    std::string server_host;
    unsigned short port = 12345; // Default port number
    Logger logger("./synctool.log");

    if (mode == "remote") {
        if (argc != 5) {
            std::cerr << "For remote mode, you must specify server_ip and server_port." << std::endl;
            return 1;
        }
        server_host = argv[3];
        port = static_cast<unsigned short>(std::stoi(argv[4]));
    }

    boost::asio::io_context io_context;

    if (mode == "local") {
        // 启动本地模式（同时启动服务器和客户端）
        start_local_mode(io_context, path_to_watch, port, logger);
    } else if (mode == "remote") {
        // 分别启动远程服务器或客户端
        std::string role = argv[2];
        if (role == "server") {
            start_remote_server(io_context, port, logger);
        } else if (role == "client") {
            start_client(io_context, path_to_watch, server_host, port, logger);
        } else {
            std::cerr << "Invalid role: " << role << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Invalid mode: " << mode << std::endl;
        return 1;
    }

    return 0;
}
