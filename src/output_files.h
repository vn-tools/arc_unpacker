#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "virtual_file.h"

typedef std::unique_ptr<VirtualFile>(*VFPointerFactory)(void *);
typedef std::function<std::unique_ptr<VirtualFile>()> VFLambdaFactory;

class OutputFiles
{
public:
    void save(VFPointerFactory save_proc, void *context) const;
    virtual void save(VFLambdaFactory save_proc) const = 0;
};



class OutputFilesHdd : public OutputFiles
{
public:
    OutputFilesHdd(std::string output_dir);
    ~OutputFilesHdd();

    using OutputFiles::save;
    void save(VFLambdaFactory save_proc) const override;
private:
    struct Internals;
    Internals *internals;
};



class OutputFilesMemory : public OutputFiles
{
public:
    OutputFilesMemory();
    ~OutputFilesMemory();

    using OutputFiles::save;
    void save(VFLambdaFactory save_proc) const override;

    const std::vector<VirtualFile*> get_saved() const;
private:
    struct Internals;
    Internals *internals;
};

#endif
