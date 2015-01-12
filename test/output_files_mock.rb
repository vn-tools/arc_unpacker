# A mock for Archive::OutputFilesMock.
class OutputFilesMock
  attr_reader :files

  def initialize
    @files = []
  end

  def write(&block)
    file_name, data = block.call
    @files.push(file_name: file_name, data: data)
  end
end
