#pragma once

#include <memory>
#include "flow/ifile_saver.h"

namespace au {
namespace flow {

    class FileSaverHdd final : public IFileSaver
    {
    public:
        FileSaverHdd(const io::path &output_dir, const bool overwrite);
        ~FileSaverHdd();

        io::path save(std::shared_ptr<io::File> file) const override;
        size_t get_saved_file_count() const override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
