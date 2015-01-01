require_relative 'cxdec_key_deriver'

# Decryption routine for CXDEC-protected games
class CxdecFilter
  def initialize(plugin)
    @plugin = plugin
    @key_deriver = CxdecKeyDeriver.new(plugin)
  end

  def filter(data, encryption_key)
    bytes = data.unpack('C*')

    begin
      hash = encryption_key
      key = (hash & @plugin.key[0]) + @plugin.key[1]

      if bytes.length > key
        dec_len = key
      else
        dec_len = data.length
      end

      decrypt_chunk(hash, 0, bytes, dec_len)
      offset = dec_len
      dec_len = data.length - offset

      decrypt_chunk((hash >> 16) ^ hash, offset, bytes, dec_len) if dec_len > 0
    rescue StandardError => e
      puts e.message
      puts e.backtrace
    end

    bytes.pack('C*')
  end

  private

  def decrypt_chunk(hash, base_offset, bytes, length)
    seed = hash & 0x7f
    hash >>= 7
    ret0 = @key_deriver.derive(seed, hash)
    ret1 = @key_deriver.derive(seed, hash ^ 0xffff_ffff)

    xor0 = (ret0 >> 8) & 0xff
    xor1 = (ret0 >> 16) & 0xff
    xor2 = ret0 & 0xff
    xor2 = 1 if xor2 == 0

    offset0 = ret1 >> 16
    offset1 = ret1 & 0xffff

    if offset0.between?(base_offset, base_offset + length)
      bytes[offset0] ^= xor0
    end

    if offset1.between?(base_offset, base_offset + length)
      bytes[offset1] ^= xor1
    end

    (base_offset..(base_offset + length - 1)).each { |i| bytes[i] ^= xor2 }
  end
end
