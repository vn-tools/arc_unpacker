#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H
#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include <vector>
#include "file.h"

class FileSaver
{
public:
    virtual void save(const std::shared_ptr<File> &file) const = 0;
};

class FileSaverHdd : public FileSaver
{
public:
    FileSaverHdd(boost::filesystem::path output_dir);
    ~FileSaverHdd();

    virtual void save(const std::shared_ptr<File> &file) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

class FileSaverMemory : public FileSaver
{
public:
    FileSaverMemory();
    ~FileSaverMemory();

    const std::vector<std::shared_ptr<File>> get_saved() const;

    virtual void save(const std::shared_ptr<File> &file) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
