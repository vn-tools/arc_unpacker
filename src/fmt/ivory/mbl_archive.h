#pragma once

#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace ivory {

    class MblArchive final : public Archive
    {
    public:
        MblArchive();
        ~MblArchive();
        void register_cli_options(ArgParser &) const override;
        void parse_cli_options(const ArgParser &) override;
        void set_plugin(const std::string &name);
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
