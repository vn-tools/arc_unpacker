require_relative 'generic_factory'

# Converts --fmt string to a corresponding converter module.
class ConverterFactory < GenericFactory
  def self.factory
    {
      'mgd' => lambda do
        require 'lib/formats/gfx/mgd_converter'
        MgdConverter
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
      end
    }
  end
end
