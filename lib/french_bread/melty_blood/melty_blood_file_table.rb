require_relative 'melty_blood_file_entry'

# Melty Blood file table
class MeltyBloodFileTable
  ENCRYPTION_KEY = 0xE3DF59AC

  attr_reader :files

  def initialize(header)
    @header = header
    @files = []
  end

  def read!(arc_file)
    num_files = arc_file.read(4).unpack('L<')[0] ^ ENCRYPTION_KEY
    @files = (1..num_files).map do |i|
      entry = MeltyBloodFileEntry.new
      entry.read!(arc_file, i - 1, @header)
      entry
    end
  end
end
