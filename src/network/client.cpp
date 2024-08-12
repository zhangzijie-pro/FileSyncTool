#include <boost/asio.hpp>
#include <iostream>
#include <fstream>

using boost::asio::ip::tcp;

void send_file(const std::string &host, unsigned short port, const std::string &filename){
    try{
        boost::asio::io_context io_context;

        //? 连接到服务器
        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(boost::asio::ip::address::from_string(host),port));
        
        std::cout << "Server Connected" << std::endl;

        //? 发送文件名
        boost::asio::write(socket,boost::asio::buffer(filename+"\n"));

        //? 发送文件内容
        std::ifstream infile(filename, std::ios::binary);
        if (!infile) {
            std::cerr << "File not found: " << filename << std::endl;
            return;
        }

        boost::asio::streambuf buf;
        std::ostream output(&buf);

        output << infile.rdbuf();
        boost::asio::write(socket, buf);

        std::cout << "File sent successfully." << std::endl;
    }catch(std::exception &e){
        std::cout << "Exception: " << e.what() << std::endl;
    }   
}

int main(){
    send_file("127.0.0.1",12345,"example.txt");
    return 0;
}