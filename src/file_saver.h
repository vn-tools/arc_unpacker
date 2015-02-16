#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H
#include <memory>
#include <string>
#include <vector>
#include "file.h"

class FileSaver
{
public:
    void save(std::vector<std::unique_ptr<File>> files) const;
    virtual void save(std::unique_ptr<File> file) const = 0;
};

class FileSaverHdd : public FileSaver
{
public:
    FileSaverHdd(std::string output_dir);
    ~FileSaverHdd();

    using FileSaver::save;
    virtual void save(std::unique_ptr<File> file) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

class FileSaverMemory : public FileSaver
{
public:
    FileSaverMemory();
    ~FileSaverMemory();

    const std::vector<File*> get_saved() const;

    using FileSaver::save;
    virtual void save(std::unique_ptr<File> file) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
