# Decryption routine for Fate/Stay Night
class FsnDecryptor
  def filter(data, _file_entry)
    bytes = data.unpack('C*')
    (0...data.length - 1).each do |i|
      bytes[i] ^= 0x03 if i == 0x2ea29
      bytes[i] ^= 0x36
      bytes[i] ^= 0x01 if i == 0x13
    end
    bytes.pack('C*')
  end
end
