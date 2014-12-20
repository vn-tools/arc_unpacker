# Melty blood header
class MeltyBloodHeader
  MAGIC1 = "\x00\x00\x00\x00" # encrypted
  MAGIC2 = "\x01\x00\x00\x00" # not encrypted

  attr_reader :magic
  attr_reader :encrypted

  def read!(arc_file)
    @magic = arc_file.read(4)
    @encrypted = @magic == MAGIC1
    fail 'Not a Melty Blood archive' unless @magic == MAGIC1 || @magic == MAGIC2
  end
end
