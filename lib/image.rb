require 'lib/common'
require 'base64'
silence_warnings { require 'rmagick' }

# Image manipulation helper.
class Image
  FORMAT = 'PNG'

  attr_accessor :width
  attr_accessor :height
  attr_accessor :pixels
  attr_accessor :format
  attr_accessor :meta

  def self.from_pixels(width, height, pixels, format, meta = {})
    image = new
    image.width = width
    image.height = height
    image.pixels = pixels
    image.format = format
    image.meta = meta
    image
  end

  def self.from_boxed(raw_data, storage_format)
    rmagick_image = Magick::Image.from_blob(raw_data)[0]
    image = new
    image.meta = decode_meta(rmagick_image['comment'])
    image.width = rmagick_image.columns
    image.height = rmagick_image.rows
    image.format = storage_format
    unless storage_format.nil?
      image.pixels = rmagick_image.export_pixels_to_str(
        0,
        0,
        rmagick_image.columns,
        rmagick_image.rows,
        storage_format)
    end
    image
  end

  def to_boxed
    image = Magick::Image.new(@width, @height)
    image.import_pixels(0, 0, @width, @height, @format, @pixels)
    image['comment'] = Image.encode_meta(@meta)
    image.to_blob do
      self.quality = 10
      self.format = FORMAT
    end
  ensure
    !image.nil? && image.destroy!
  end

  def update_file(file)
    file.data = to_boxed
    file.change_extension('png')
  end

  def self.encode_meta(meta)
    Base64.encode64(Marshal.dump(meta))
  end

  def self.decode_meta(serialized)
    return {} if serialized.nil?
    Marshal.load(Base64.decode64(serialized))
  end
end
