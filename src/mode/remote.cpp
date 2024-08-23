#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "file_check.cpp"

using boost::asio::ip::tcp;

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
        std::string local_file = "server_work_app/" + received_file;

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


        std::thread cli_thread(cli_file_check);
        cli_thread.join();
    }
}