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

    void send_file(const std::string& host, unsigned short port){
        try{
            socket_.connect(tcp::endpoint(boost::asio::ip::address::from_string(host),port));

            std::ifstream infile(filename_,std::ios::binary);
            if (!infile) {
                std::cerr << "File not found: " << filename_ << std::endl;
                return;
            }
            boost::asio::streambuf buf;
            std::ostream output(&buf);
        
            output << infile.rdbuf();
            boost::asio::write(socket_,buf);

            std::cout << "File sent successfully." << std::endl;
        } catch (std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }

    void receive_file(tcp::acceptor& acceptor){
        try
        {
            acceptor.accept(socket_);
            std::ofstream outfile("received_" + filename_, std::ios::binary);

            boost::asio::streambuf buf;
            boost::system::error_code error;

            while (boost::asio::read(socket_, buf, boost::asio::transfer_at_least(1), error)) {
                outfile << &buf;
            }

            if (error == boost::asio::error::eof) {
                Logger log("File received successfully: " + filename_);
            } else {
                Logger log("Error receiving file: " + error.message());
            }
            
        } catch (std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
        
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