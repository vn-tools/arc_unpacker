# PAK header
class PakHeader
  MAGIC1 = "\x01\x00\x00\x00"
  MAGIC2 = "\x02\x00\x00\x00"
  MAGIC3 = "\x03\x00\x00\x00"
  MAGIC4 = "\x04\x00\x00\x00"

  attr_reader :magic

  def read!(file)
    @magic = file.read(4)

    fail 'Not a PAK archive' \
      unless [MAGIC1, MAGIC2, MAGIC3, MAGIC4].include?(@magic)

    self
  end
end
