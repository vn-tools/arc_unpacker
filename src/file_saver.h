#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "file.h"

typedef std::function<std::unique_ptr<File>()> VFFactory;
typedef std::function<std::vector<std::unique_ptr<File>>()> VFsFactory;

class FileSaver
{
public:
    void save(VFFactory save_proc) const;
    virtual void save(VFsFactory save_proc) const = 0;
};



class FileSaverHdd : public FileSaver
{
public:
    FileSaverHdd(std::string output_dir);
    ~FileSaverHdd();

    using FileSaver::save;
    void save(VFsFactory save_proc) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};



class FileSaverMemory : public FileSaver
{
public:
    FileSaverMemory();
    ~FileSaverMemory();

    using FileSaver::save;
    void save(VFsFactory save_proc) const override;

    const std::vector<File*> get_saved() const;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
