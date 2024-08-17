#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "file_transfer.cpp"
#include "file_watcher.cpp"
#include "conflict_resolver.cpp"


using boost::asio::ip::tcp;

// 服务器端：启动服务器，接收文件并处理潜在的文件冲突
void start_server(boost::asio::io_context& io_context, unsigned short port,Logger& logger) {
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

    while (true) {
        std::cout << "Server: Waiting for incoming files..." << std::endl;

        // 接收文件名
        std::string received_file;
        FileTransfer file_transfer(io_context, "");

        try {
            received_file = file_transfer.receive_file(acceptor);
            std::cout << "Server: Received file: " << received_file << std::endl;
            logger.log("Server: Received file: " + received_file);
        } catch (const std::exception& e) {
            std::cerr << "Error receiving file: " << e.what() << std::endl;
            logger.log("Error receiving file: " + std::string(e.what()));
            continue;
        }

        // 动态生成本地文件路径用于冲突检测
        std::string local_file = "sync_folder_backup/" + received_file;

        // 检查是否存在同名文件，触发冲突解决机制
        if (std::ifstream(local_file)) {
            std::cout << "Server: Conflict detected with " << local_file << std::endl;
            logger.log("Server: Conflict detected with " + local_file);

            // 调用冲突解决模块，生成解决后的文件
            std::string resolved_file = ConflictResolver::resolve_conflict(local_file, received_file);
            std::cout << "Server: Conflict resolved, result saved in " << resolved_file << std::endl;
            logger.log("Server: Conflict resolved, result saved in " + resolved_file);
        } else {
            // 如果没有冲突，直接将接收的文件作为新的本地文件保存
            std::cout << "Server: No conflict detected. File saved as " << received_file << std::endl;
            logger.log("Server: No conflict detected. File saved as " + received_file);
            // 你可以根据需要处理文件的存储逻辑
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

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <server|client>" << std::endl;
        return 1;
    }

    boost::asio::io_context io_context;

    std::string role = argv[1];
    std::string path_to_watch = "./sync_folder";
    std::string server_host = "127.0.0.1";
    unsigned short port = 12345;
    Logger logger("./synctool.log");

    if (role == "server") {
        start_server(io_context, port, logger);
    } else if (role == "client") {
        start_client(io_context, path_to_watch, server_host, port, logger);
    } else {
        std::cerr << "Invalid role: " << role << std::endl;
        return 1;
    }

    return 0;
}
