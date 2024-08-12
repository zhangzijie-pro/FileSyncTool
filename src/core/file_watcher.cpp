#include <thread>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <filesystem>
#include <functional>


namespace fs = std::filesystem;

//! 监听指定的文件夹，检测新增、修改或删除的文件，并触发相应的同步操作
class FileWatcher{
public:
    FileWatcher(const std::string& path_to_watch, std::chrono::duration<int> delay)
    : path_to_watch(path_to_watch),delay(delay){
        for(const auto &file: fs::recursive_directory_iterator(path_to_watch)){
            paths_[file.path().string()] = fs::last_write_time(file);
        }
    }

    void start(const std::function<void(std::string,std::string)> &action){
        while(true){
            std::this_thread::sleep_for(delay);
        
            auto it = paths_.begin();
            if(it!=paths_.end()){
                if(!fs::exists(it->first)){
                    action(it->first,"removed");
                    it=paths_.erase(it);
                }else{
                    it++;
                }
            }

            for (const auto& file : fs::recursive_directory_iterator(path_to_watch)) {
                auto current_file_last_write_time = fs::last_write_time(file);

                if (std::filesystem::last_write_time(file) > paths_.at(file.path().string())) {
                    paths_[file.path().string()] = current_file_last_write_time;
                    action(file.path().string(), "created");
                } else {
                    if (paths_[file.path().string()] != current_file_last_write_time) {
                        paths_[file.path().string()] = current_file_last_write_time;
                        action(file.path().string(), "modified");
                    }
                }
            }
        }
    }
private:
    std::unordered_map<std::string, fs::file_time_type> paths_;
    std::string path_to_watch;
    std::chrono::duration<int> delay;
};