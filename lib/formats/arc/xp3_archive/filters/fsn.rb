# Decryption routine for Fate/Stay Night
class FsnFilter
  def filter(data, _adlr_chunk)
    bytes = data.unpack('C*')
    bytes[0x2ea29] ^= 0x03 if data.length > 0x2ea29
    bytes[0x13] ^= 0x01 if data.length > 0x13
    (0...data.length - 1).each { |i| bytes[i] ^= 0x36 }
    bytes.pack('C*')
  end
end
