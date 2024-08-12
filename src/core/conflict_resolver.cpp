#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

//! 检测和解决文件同步过程中的冲突
class ConflictResolver {
public:
    static std::string resolve_conflict(const std::string& file1, const std::string& file2) {
        std::string resolved_filename = "resolved_" + fs::path(file1).filename().string();

        std::ifstream infile1(file1);
        std::ifstream infile2(file2);
        std::ofstream outfile(resolved_filename);

        std::string line1, line2;

        while (std::getline(infile1, line1) && std::getline(infile2, line2)) {
            if (line1 == line2) {
                outfile << line1 << std::endl;
            } else {
                outfile << "<<<<<<< " << file1 << std::endl;
                outfile << line1 << std::endl;
                outfile << "=======" << std::endl;
                outfile << line2 << std::endl;
                outfile << ">>>>>>> " << file2 << std::endl;
            }
        }

        while (std::getline(infile1, line1)) {
            outfile << line1 << std::endl;
        }

        while (std::getline(infile2, line2)) {
            outfile << line2 << std::endl;
        }

        std::cout << "Conflict resolved, output file: " << resolved_filename << std::endl;
        return resolved_filename;
    }
};
