require_relative 'melty_blood_file_entry'

# Melty Blood file table
class MeltyBloodFileTable
  ENCRYPTION_KEY = 0xE3DF59AC

  attr_reader :files

  def initialize(header)
    @header = header
  end

  def read!(file)
    num_files = file.read(4).unpack('L<')[0] ^ ENCRYPTION_KEY
    @files = []
    (1..num_files).each do |i|
      @files.push(MeltyBloodFileEntry.new(i - 1, @header).read!(file))
    end
    self
  end
end
