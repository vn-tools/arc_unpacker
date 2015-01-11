require 'lib/binary_io'
require 'digest/md5'

# Converts MSD to text and vice versa.
# Seen in FJSYS archives.
module MsdConverter
  module_function

  MAGIC = 'MSCENARIO FILE'

  # How to find out the key for your game:
  # 1. Search for _wsprintfA("%s%d", ...) instruction.
  # 2. The register used to fill "%s" part should point to the key.
  # 3. Apparently, this is just the main window title, encoded in SJIS.
  COMMON_KEYS = {
    sonohana1: 'その花びらにくちづけを'.encode('sjis'),
    sonohana2: 'その花びらにくちづけを　わたしの王子さま'.encode('sjis'),
    sonohana3: 'その花びらにくちづけを　あなたと恋人つなぎ'.encode('sjis'),
    sonohana4: 'その花びらにくちづけを　愛しさのフォトグラフ'.encode('sjis'),
    sonohana5: 'その花びらにくちづけを　あなたを好きな幸せ'.encode('sjis'),
    sonohana6: 'その花びらにくちづけを　唇とキスで呟いて'.encode('sjis'),
    sonohana7: 'その花びらにくちづけを　あまくてほしくてとろけるちゅう'.encode('sjis'),
    sonohana8: 'その花びらにくちづけを　天使の花びら染め'.encode('sjis'),
    sonohana9: 'その花びらにくちづけを　あまくておとなのとろけるちゅう'.encode('sjis'),
    sonohana10: 'その花びらにくちづけを　リリ・プラチナム'.encode('sjis'),
    sonohana11: 'その花びらにくちづけを ミカエルの乙女たち'.encode('sjis')
  }

  def add_cli_help(arg_parser)
    arg_parser.add_help(
      '--msd-key=KEY',
      'Sets key used for decrypting MSD files.',
      possible_values: COMMON_KEYS.keys)
  end

  def parse_cli_options(arg_parser, options)
    key = arg_parser.switch(['--msd-key'])
    key = MsdConverter::COMMON_KEYS[key.to_sym] unless key.nil?
    options[:msd_key] = key
  end

  def decode(data, options)
    return data if data.start_with?(MAGIC)
    fail 'Must supply a key to decrypt this file.' if options[:msd_key].nil?

    data = data.unpack('C*')
    k = 0
    how_many = (data.length + 31) & (0xff_ff_ff_ff ^ 31)

    catch :done do
      how_many.times do |i|
        md5 = Digest::MD5.hexdigest(options[:msd_key] + i.to_s)
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

  def encode(data, _options)
    data
  end
end
