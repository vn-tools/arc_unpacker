# Melty blood header
class MeltyBloodHeader
  MAGIC1 = "\x00\x00\x00\x00" # encrypted
  MAGIC2 = "\x01\x00\x00\x00" # not encrypted

  attr_reader :magic
  attr_reader :encrypted

  def read!(file)
    @magic = file.read(4)
    @encrypted = @magic == MAGIC1
    fail 'Not a Melty Blood archive' unless @magic == MAGIC1 || @magic == MAGIC2
    self
  end
end
