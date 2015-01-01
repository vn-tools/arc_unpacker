# No decryption
class NoopFilter
  def filter(data, _file_entry)
    data
  end
end
