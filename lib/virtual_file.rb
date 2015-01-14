# Generic representation of a file that resides in the program memory.
class VirtualFile
  attr_accessor :name
  attr_accessor :data

  def initialize(name, data)
    @name = name
    @data = data
  end

  def change_extension(new_ext)
    return if @name.nil?
    @name = @name.gsub(/\.[a-zA-Z0-9]+\Z/, '') + '.' + new_ext.gsub(/\A\.+/, '')
  end
end
