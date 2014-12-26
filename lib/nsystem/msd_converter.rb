require_relative '../binary_io'
require 'digest/md5'

# Converts MSD to text and vice versa.
class MsdConverter
  MAGIC = 'MSCENARIO FILE'

  def self.decode(data, key)
    return data if data.start_with?(MAGIC)
    fail 'Must supply a key to decrypt this file.' if key.nil?

    data = data.unpack('C*')
    k = 0
    how_many = (data.length + 31) & (0xff_ff_ff_ff ^ 31)

    catch :done do
      how_many.times do |i|
        md5 = Digest::MD5.hexdigest(key + i.to_s)
        md5 = md5.split('').map(&:ord)
        (0..31).each do |j|
          data[k] ^= md5[j]
          k += 1
          throw :done if k >= data.length
        end
      end
    end

    data = data.pack('C*')
    fail 'Supplied key can\'t decrypt this file.' unless data.start_with?(MAGIC)
    data
  end

  def self.encode(data, _key = nil)
    data
  end
end
