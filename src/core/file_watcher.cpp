#include <thread>
#include <iostream>
#include <chrono>
#include <fstream>
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
        if (!fs::exists(backup_folder)) {
            fs::create_directory(backup_folder);
        }

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

    void update_backfolder(const std::string& path){
        std::string back_path = backup_folder+'/'+fs::relative(path,path_to_watch).string();
        fs::create_directories(fs::path(back_path).parent_path());
        fs::copy_file(path,back_path,fs::copy_options::overwrite_existing);
    }

    std::string compare_files(const std::string& old_file, const std::string& new_file){
        std::vector<std::string> old_lines;
        std::vector<std::string> new_lines;

        std::ifstream old_f(old_file);
        std::ifstream new_f(new_file);
        std::string line;

        while(std::getline(old_f,line)) old_lines.push_back(line);
        while(std::getline(new_f,line)) new_lines.push_back(line);

        std::stringstream diff;
        diff << "Changes in file: " << new_file << std::endl;

        for(size_t i=0;i<std::max(old_lines.size(),new_lines.size());++i){
            if(i<old_lines.size() && i<new_lines.size()){
                if(old_lines[i]!=new_lines[i]){
                    diff << "Line " << i+1 << " modified: " << std::endl;
                    diff << "Old: " << old_lines[i] << std::endl;
                    diff << "New: " << new_lines[i] << std::endl;
                }
            }else if(i<old_lines.size()){
                diff << "Line " << i+1 << " removed: " << std::endl;
            }else{
                diff << "Line " << i+1 << "added: " << std::endl;
            }
        }

        return diff.str();
    }

private:
    std::unordered_map<std::string, fs::file_time_type> paths_;
    std::string path_to_watch;
    std::string backup_folder = "sync_folder_backup";
    std::chrono::duration<int> delay;
};