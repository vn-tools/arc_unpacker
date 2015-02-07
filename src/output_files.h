#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H
#include <memory>
#include <string>
#include <vector>
#include "virtual_file.h"

typedef std::unique_ptr<VirtualFile>(*VirtualFileFactory)(void *);

class OutputFiles
{
public:
    virtual void save(
        VirtualFileFactory save_proc,
        void *context) const = 0;
};



class OutputFilesHdd : public OutputFiles
{
public:
    OutputFilesHdd(std::string output_dir);
    ~OutputFilesHdd();
    void save(VirtualFileFactory save_proc, void *context) const override;
private:
    struct Internals;
    Internals *internals;
};



class OutputFilesMemory : public OutputFiles
{
public:
    OutputFilesMemory();
    ~OutputFilesMemory();
    void save(VirtualFileFactory save_proc, void *context) const override;
    const std::vector<VirtualFile*> get_saved() const;
private:
    struct Internals;
    Internals *internals;
};

#endif
