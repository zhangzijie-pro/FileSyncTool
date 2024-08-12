#include <boost/asio.hpp>
#include <iostream>
#include <fstream>

using boost::asio::ip::tcp;

void start_server(unsigned short port){
    try{
        boost::asio::io_context io_context;

        //? 监听特定的端口
        tcp::acceptor acceptor(io_context,tcp::endpoint(tcp::v4(),port));
        std::cout << "Server Listening on port:" << port << "..." << std::endl; 
        
        while(true){
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            std::cout << "Client Connected" << std::endl;

            //? 接受文件名
            boost::asio::streambuf buf;
            boost::asio::read_until(socket,buf,"\n");
            std::istream input(&buf);
            std::string filename;
            std::getline(input,filename);

            std::ofstream outfile("receive_"+filename,std::ios::binary);

            //? 接受文件内容
            boost::system::error_code error;
            while (boost::asio::read(socket, buf, boost::asio::transfer_at_least(1), error)) {
                    outfile << &buf;
            }

            if (error == boost::asio::error::eof) {
                std::cout << "File received successfully." << std::endl;
            } else {
                std::cerr << "Error receiving file: " << error.message() << std::endl;
            }
        }
    }catch(std::exception &e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main(){
    start_server(12345);
    return 0;
}