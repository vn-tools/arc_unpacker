# Converts --fmt string to instance of given archive.
class ArchiveFactory
  def self.get(fmt)
    factory[fmt].call
  end

  def self.format_strings
    factory.keys
  end

  def self.each(&block)
    factory.each { |k, v| block.call(k, v.call) }
  end

  def self.factory
    {
      'xp3' => lambda do
        require_relative 'kirikiri/xp3_archive'
        Xp3Archive
      end,

      'ykc' => lambda do
        require_relative 'yuka/ykc_archive'
        YkcArchive
      end,

      'sar' => lambda do
        require_relative 'nscripter/sar_archive'
        SarArchive
      end,

      'nsa' => lambda do
        require_relative 'nscripter/nsa_archive'
        NsaArchive
      end,

      'melty_blood' => lambda do
        require_relative 'french_bread/melty_blood_archive'
        MeltyBloodArchive
      end,

      'nitroplus/pak2' => lambda do
        require_relative 'nitroplus/pak2_archive'
        Pak2Archive
      end,

      'fjsys' => lambda do
        require_relative 'nsystem/fjsys_archive.rb'
        FjsysArchive
      end,

      'rpa' => lambda do
        require_relative 'renpy/rpa_archive.rb'
        RpaArchive
      end,

      'mbl' => lambda do
        require_relative 'ivory/mbl_archive.rb'
        MblArchive
      end,

      'exe' => lambda do
        require_relative 'misc/exe_reader.rb'
        ExeReader
      end
    }
  end
end
