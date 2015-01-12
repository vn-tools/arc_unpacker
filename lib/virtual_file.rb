# Generic representation of a file that resides in the program memory.
class VirtualFile
  attr_accessor :name
  attr_accessor :data

  def initialize(name, data)
    @name = name
    @data = data
  end
end
