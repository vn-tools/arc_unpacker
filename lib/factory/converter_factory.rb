require_relative 'generic_factory'

# Converts --fmt string to a corresponding converter module.
class ConverterFactory < GenericFactory
  def self.factory
    {
      'mgd' => lambda do
        require 'lib/formats/gfx/mgd_converter'
        MgdConverter
      end,

      'cbg' => lambda do
        require 'lib/formats/gfx/cbg_converter'
        CbgConverter
      end,

      'prs' => lambda do
        require 'lib/formats/gfx/prs_converter'
        PrsConverter
      end,

      'spb' => lambda do
        require 'lib/formats/gfx/spb_converter'
        SpbConverter
      end,

      'ykg' => lambda do
        require 'lib/formats/gfx/ykg_converter'
        YkgConverter
      end,

      'msd' => lambda do
        require 'lib/formats/script/msd_converter'
        MsdConverter
      end,

      'yks' => lambda do
        require 'lib/formats/script/yks_converter'
        YksConverter
      end,

      'g00' => lambda do
        require 'lib/formats/gfx/g00_converter'
        G00Converter
      end,

      'tlg' => lambda do
        require 'lib/formats/gfx/tlg_converter'
        TlgConverter
      end,

      'xyz' => lambda do
        require 'lib/formats/gfx/xyz_converter'
        XyzConverter
      end,

      'nwa' => lambda do
        require 'lib/formats/sfx/nwa_converter'
        NwaConverter
      end,

      'sotes' => lambda do
        require 'lib/formats/gfx/sotes_converter'
        SotesConverter
      end
    }
  end
end
