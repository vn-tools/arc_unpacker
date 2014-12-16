# No decryption
class NoopDecryptor
  def filter(data, _file_entry)
    data
  end
end
