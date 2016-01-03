#pragma once

#include <memory>
#include "flow/ifile_saver.h"

namespace au {
namespace flow {

    using FileSaveCallback = std::function<void(std::shared_ptr<io::File>)>;

    class FileSaverCallback final : public IFileSaver
    {
    public:
        FileSaverCallback();
        FileSaverCallback(FileSaveCallback callback);
        ~FileSaverCallback();

        void set_callback(FileSaveCallback callback);
        io::path save(std::shared_ptr<io::File> file) const override;
        size_t get_saved_file_count() const override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
