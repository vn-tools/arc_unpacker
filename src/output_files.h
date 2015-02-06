#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H
#include <vector>
#include <string>
#include "virtual_file.h"

class OutputFiles
{
public:
    virtual bool save(
        VirtualFile *(*save_proc)(void *),
        void *context) const = 0;
};



class OutputFilesHdd : public OutputFiles
{
public:
    OutputFilesHdd(std::string output_dir);
    ~OutputFilesHdd();
    bool save(VirtualFile *(*save_proc)(void *), void *context) const override;
private:
    struct Internals;
    Internals *internals;
};



class OutputFilesMemory : public OutputFiles
{
public:
    OutputFilesMemory();
    ~OutputFilesMemory();
    bool save(VirtualFile *(*save_proc)(void *), void *context) const override;
    const std::vector<VirtualFile*> get_saved() const;
private:
    struct Internals;
    Internals *internals;
};

#endif
