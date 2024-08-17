#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <filesystem>
#include "Loggerd.cpp"


using boost::asio::ip::tcp;
namespace fs = std::filesystem;


//! 实现文件的增量传输
class FileTransfer{
private:
    tcp::socket socket_;
    std::string filename_;
public:
    FileTransfer(boost::asio::io_context& io_context, const std::string& filename):socket_(io_context),filename_(filename){}

    void send_file(boost::asio::io_context& io_context, const std::string& server_host, unsigned short server_port, const std::string& filepath) {
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(server_host, std::to_string(server_port));
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        // 发送文件名
        std::string filename = fs::path(filepath).filename().string();
        boost::asio::write(socket, boost::asio::buffer(filename + "\n"));

        // 发送文件内容
        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open file for reading.");
        }

        boost::asio::streambuf buf;
        std::ostream output_stream(&buf);
        output_stream << file.rdbuf();

        boost::asio::write(socket, buf.data());
    }


    std::string receive_file(boost::asio::ip::tcp::acceptor& acceptor) {
        // 接受连接
        acceptor.accept(socket_);

        // 先接收文件名
        boost::asio::streambuf buf;
        boost::asio::read_until(socket_, buf, "\n");
        std::istream input_stream(&buf);
        std::getline(input_stream, filename_);

        std::cout << "Receiving file: " << filename_ << std::endl;

        // 接收文件内容
        std::ofstream output_file(filename_, std::ios::binary);
        if (!output_file) {
            throw std::runtime_error("Failed to open file for writing.");
        }

        boost::system::error_code ec;
        while (boost::asio::read(socket_, buf, ec)) {
            if (ec == boost::asio::error::eof) break; // Connection closed cleanly by peer
            if (ec) throw boost::system::system_error(ec); // Some other error
            output_file << &buf;
        }

        return filename_; // 返回接收到的文件名
    }

    void delete_file(const std::string& host, unsigned short port, const std::string& filename) {
        try {
            socket_.connect(tcp::endpoint(boost::asio::ip::address::from_string(host), port));

            std::string delete_command = "DELETE " + filename + "\n";
            boost::asio::write(socket_, boost::asio::buffer(delete_command));
        } catch (std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }

    void handle_delete_command(tcp::acceptor& acceptor, Logger& logger) {
        acceptor.accept(socket_);

        boost::asio::streambuf buf;
        boost::asio::read_until(socket_, buf, "\n");

        std::istream input(&buf);
        std::string command;
        input >> command;

        if (command == "DELETE") {
            std::string filename;
            input >> filename;

            if(fs::remove(filename)){
                logger.log("File deleted: " + filename);
            }else{
                logger.log("Failed to delete file: " + filename);
            }
        }
    }
};