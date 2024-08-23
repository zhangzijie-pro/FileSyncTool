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
    }
}