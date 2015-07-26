#ifndef AU_FMT_CONVERTER_H
#define AU_FMT_CONVERTER_H
#include "transformer.h"

namespace au {
namespace fmt {

    class Converter : public Transformer
    {
    public:
        virtual void add_cli_help(ArgParser &) const override;
        virtual void parse_cli_options(const ArgParser &) override;
        virtual FileNamingStrategy get_file_naming_strategy() const override;
        virtual ~Converter();

        virtual void unpack(File &file, FileSaver &file_saver) const override;
        std::unique_ptr<File> decode(File &) const;
    protected:
        virtual std::unique_ptr<File> decode_internal(File &) const = 0;
    };

} }

#endif
