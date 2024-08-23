#include <iostream>
#include <thread>
#include <boost/asio.hpp>

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