#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "virtual_file.h"

typedef std::function<std::unique_ptr<VirtualFile>()> VFFactory;
typedef std::function<std::vector<std::unique_ptr<VirtualFile>>()> VFsFactory;

class OutputFiles
{
public:
    void save(VFFactory save_proc) const;
    virtual void save(VFsFactory save_proc) const = 0;
};



class OutputFilesHdd : public OutputFiles
{
public:
    OutputFilesHdd(std::string output_dir);
    ~OutputFilesHdd();

    using OutputFiles::save;
    void save(VFsFactory save_proc) const override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};



class OutputFilesMemory : public OutputFiles
{
public:
    OutputFilesMemory();
    ~OutputFilesMemory();

    using OutputFiles::save;
    void save(VFsFactory save_proc) const override;

    const std::vector<VirtualFile*> get_saved() const;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
