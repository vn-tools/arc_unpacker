#ifndef FS_H
#define FS_H
#include <vector>
#include <string>

bool is_dir(std::string path);

std::vector<std::string> get_files_recursive(const std::string path);
std::vector<std::string> get_files(const std::string path);

std::string basename(const std::string path);
std::string dirname(const std::string path);

void mkpath(std::string path);

#endif
