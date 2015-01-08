require 'lib/warning_silencer'
require 'base64'
silence_warnings { require 'rmagick' }

# Image manipulation helper.
module Image
  module_function

  FORMAT = 'PNG'

  # Converts raw pixels to chosen readable format, such as PNG.
  def raw_to_boxed(width, height, raw_data, format, meta = {})
    image = Magick::Image.new(width, height)
    image.import_pixels(0, 0, width, height, format, raw_data)
    image['comment'] = encode_meta(meta)
    image.to_blob { self.format = FORMAT }
  ensure
    !image.nil? && image.destroy!
  end

  # Opposite of raw_to_boxed.
  def boxed_to_raw(blob, format)
    image = Magick::Image.from_blob(blob)[0]
    meta = rmagick_to_meta(image)
    raw_data = image.export_pixels_to_str(
      0,
      0,
      meta[:width],
      meta[:height],
      format)
    [raw_data, meta]
  ensure
    !image.nil? && image.destroy!
  end

  # Retrieves meta data associated with boxed file type such as PNG.
  def self.read_meta_from_boxed(blob)
    image = Magick::Image.from_blob(blob)[0]
    rmagick_to_meta(image)
  ensure
    !image.nil? && image.destroy!
  end

  def self.add_meta_to_boxed(blob, meta)
    image = Magick::Image.from_blob(blob)[0]
    image['comment'] = encode_meta(meta)
    image.to_blob { self.format = FORMAT }
  ensure
    !image.nil? && image.destroy!
  end

  # Retrieves meta data associated with RMagick handle.
  def self.rmagick_to_meta(image)
    meta = decode_meta(image['comment'])
    meta[:width] = image.columns
    meta[:height] = image.rows
    meta
  end

  def self.encode_meta(meta)
    Base64.encode64(Marshal.dump(meta))
  end

  def self.decode_meta(serialized)
    return {} if serialized.nil?
    Marshal.load(Base64.decode64(serialized))
  end
end
