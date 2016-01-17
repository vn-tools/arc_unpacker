import io
import sys
import struct

class io_ext(object):
    def __init__(self, file=None):
        if file is None:
            self.file = io.BytesIO()
        else:
            self.file = file

    def __enter__ (self):
        return self

    def __exit__ (self, exc_type, exc_value, traceback):
        self.file.close()

    def size(self):
        pos = self.file.tell()
        self.file.seek(0, io.SEEK_END)
        size = self.file.tell()
        self.file.seek(pos, io.SEEK_SET)
        return size

    def tell(self):
        return self.file.tell()

    def seek(self, *args):
        self.file.seek(*args)
        return self

    def skip(self, bytes):
        self.file.seek(bytes, io.SEEK_CUR)
        return self

    def peek(self, *args):
        return self.PeekObject(self.file, *args)

    def read_to_zero(self):
        out = b''
        byte = self.file.read(1)
        while byte != b"\x00":
            out += byte
            byte = self.file.read(1)
        return out

    def read_to_eof(self):
        return self.file.read()

    def read(self, *args):
        return self.file.read(*args)

    def read_u8(self):
        return struct.unpack('B', self.file.read(1))[0]

    def read_u16_le(self):
        return struct.unpack('<H', self.file.read(2))[0]

    def read_u32_le(self):
        return struct.unpack('<I', self.file.read(4))[0]

    def read_u64_le(self):
        return struct.unpack('<Q', self.file.read(8))[0]

    def read_u16_be(self):
        return struct.unpack('>H', self.file.read(2))[0]

    def read_u32_be(self):
        return struct.unpack('>I', self.file.read(4))[0]

    def read_u64_be(self):
        return struct.unpack('>Q', self.file.read(8))[0]

    def write(self, *args):
        self.file.write(*args)
        return self

    def write_u8(self, x):
        self.file.write(struct.pack('B', x))
        return self

    def write_u16_le(self, x):
        self.file.write(struct.pack('<H', x))
        return self

    def write_u32_le(self, x):
        self.file.write(struct.pack('<I', x))
        return self

    def write_u64_le(self, x):
        self.file.write(struct.pack('<Q', x))
        return self

    def write_u16_be(self, x):
        self.file.write(struct.pack('>H', x))
        return self

    def write_u32_be(self, x):
        self.file.write(struct.pack('>I', x))
        return self

    def write_u64_be(self, x):
        self.file.write(struct.pack('>Q', x))
        return self

    def write_zero_padded(self, x, size):
        self.file.write(x)
        self.file.write(b'\x00' * (size - len(x)))
        return self

    class PeekObject(object):
        def __init__(self, file, *seek_args):
            self.file = file
            self.seek_args = seek_args
        def __enter__(self):
            self.old_pos = self.file.tell()
            self.file.seek(*self.seek_args)
        def __exit__(self, *unused):
            self.file.seek(self.old_pos)

class open_ext(object):
    def __init__ (self, *args):
        if args:
            self.file = io_ext(open(*args))
        else:
            self.file = io_ext(sys.stdout.buffer)

    def __enter__ (self):
        return self.file.__enter__()

    def __exit__ (self, exc_type, exc_value, traceback):
        self.file.__exit__(exc_type, exc_value, traceback)

class BitWriter:
    def __init__(self, f):
        self.accumulator = 0
        self.bcount = 0
        self.out = f

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.flush()

    def write_bit(self, bit):
        if self.bcount == 8:
            self.flush()
        if bit > 0:
            self.accumulator |= (1 << (7 - self.bcount))
        self.bcount += 1

    def write_bits(self, bits, n):
        while n > 0:
            self.write_bit(bits & (1 << (n - 1)))
            n -= 1

    def flush(self):
        self.out.write(bytes([self.accumulator]))
        self.accumulator = 0
        self.bcount = 0

def lzss_compress_bitwise(input):
    WINDOW_BITS = 12
    LENGTH_BITS  = 4
    MIN_MATCH = 3
    WINDOW_SIZE = (1 << WINDOW_BITS)
    MAX_MATCH = MIN_MATCH + (1 << LENGTH_BITS) - 1
    read_ptr = 0
    with io_ext() as out_io:
        with BitWriter(out_io) as bit_writer:
            window = bytearray(b'\x00' * WINDOW_SIZE)
            window_pos = 0xFEE
            while read_ptr < len(input):
                best_match_len, best_match_ptr = 0, 0
                for match_len in range(MAX_MATCH, MIN_MATCH-1, -1):
                    match = input[read_ptr : read_ptr + match_len + 1]
                    if match in window + window:
                        best_match_len = match_len
                        best_match_ptr = (window + window).index(match)
                        break
                if best_match_len >= MIN_MATCH:
                    bit_writer.write_bit(0)
                    bit_writer.write_bits(best_match_ptr, WINDOW_BITS)
                    bit_writer.write_bits(best_match_len - MIN_MATCH, LENGTH_BITS)
                    for c in match:
                        window[window_pos] = c
                        window_pos += 1
                        window_pos %= WINDOW_SIZE
                    read_ptr += best_match_len
                else:
                    bit_writer.write_bit(1)
                    bit_writer.write_bits(input[read_ptr], 8)
                    window[window_pos] = input[read_ptr]
                    window_pos += 1
                    window_pos %= WINDOW_SIZE
                    read_ptr += 1
        return out_io.seek(0).read()

def lzss_compress_bytewise(input):
    class Lzss(object):
        def __init__(self):
            window_bits = 12
            length_bits  = 4
            self.window_size = 1 << window_bits
            self.min_match = 3
            self.max_match = self.min_match + (1 << length_bits) - 1
            self.nil = self.window_size

            self.text_buf = bytearray(b'\x00' * (self.window_size + self.max_match - 1))
            self.match_position = 0
            self.match_length = 0
            self.lson = [self.nil for i in range(self.window_size + 1)]
            self.rson = [self.nil for i in range(self.window_size + 257)]
            self.dad = [self.nil for i in range(self.window_size + 1)]

        def insert_node(self, r):
            cmp = 1
            p = self.window_size + 1 + self.text_buf[r]
            self.rson[r] = self.lson[r] = self.nil
            self.match_length = 0
            while True:
                if cmp >= 0:
                    if self.rson[p] != self.nil:
                        p = self.rson[p]
                    else:
                        self.rson[p] = r
                        self.dad[r] = p
                        return
                else:
                    if self.lson[p] != self.nil:
                        p = self.lson[p]
                    else:
                        self.lson[p] = r
                        self.dad[r] = p
                        return
                i = 1
                while i < self.max_match:
                    cmp = self.text_buf[r + i] - self.text_buf[p + i]
                    if cmp != 0:
                        break
                    i += 1
                if i > self.match_length:
                    self.match_position = p
                    self.match_length = i
                    if self.match_length >= self.max_match:
                        break
            self.dad[r] = self.dad[p]
            self.lson[r] = self.lson[p]
            self.rson[r] = self.rson[p]
            self.dad[self.lson[p]] = r
            self.dad[self.rson[p]] = r
            if self.rson[self.dad[p]] == p:
                self.rson[self.dad[p]] = r
            else:
                self.lson[self.dad[p]] = r
            self.dad[p] = self.nil

        def delete_node(self, p):
            if self.dad[p] == self.nil:
                return
            if self.rson[p] == self.nil:
                q = self.lson[p]
            elif self.lson[p] == self.nil:
                q = self.rson[p]
            else:
                q = self.lson[p]
                if self.rson[q] != self.nil:
                    q = self.rson[q]
                    while self.rson[q] != self.nil:
                        q = self.rson[q]
                    self.rson[self.dad[q]] = self.lson[q]
                    self.dad[self.lson[q]] = self.dad[q]
                    self.lson[q] = self.lson[p]
                    self.dad[self.lson[p]] = q
                self.rson[q] = self.rson[p]
                self.dad[self.rson[p]] = q
            self.dad[q] = self.dad[p]
            if self.rson[self.dad[p]] == p:
                self.rson[self.dad[p]] = q
            else:
                self.lson[self.dad[p]] = q
            self.dad[p] = self.nil

        def encode(self, input):
            code_buf = bytearray(b'\x00' * 17)
            code_buf[0] = 0
            code_buf_ptr = mask = 1
            s = 0
            r = self.window_size - self.max_match

            output = bytearray()
            with io_ext(io.BytesIO(input)) as input_io:
                for i in range(s, r):
                    self.text_buf[i] = 0
                length = 0
                while length < self.max_match:
                    if input_io.tell() == input_io.size():
                        break
                    self.text_buf[r + length] = input_io.read_u8()
                    length += 1
                if length == 0:
                    return b''

                for i in range(1, self.max_match + 1):
                    self.insert_node(r - i)
                self.insert_node(r)
                while length > 0:
                    if self.match_length > length:
                        self.match_length = length
                    if self.match_length < self.min_match:
                        self.match_length = 1
                        code_buf[0] |= mask
                        code_buf[code_buf_ptr] = self.text_buf[r]
                        code_buf_ptr += 1
                    else:
                        self.match_position = (self.match_position - 0) & 0xFFF
                        code_buf[code_buf_ptr] = self.match_position & 0xFF
                        code_buf_ptr += 1
                        code_buf[code_buf_ptr] = (
                            (((self.match_position & 0xF00) >> 4)
                             | ((self.match_length - self.min_match) & 0xF)))
                        code_buf_ptr += 1
                    mask <<= 1
                    mask &= 0xFF
                    if mask == 0:
                        for i in range(code_buf_ptr):
                            output.append(code_buf[i])
                        code_buf[0] = 0
                        code_buf_ptr = mask = 1
                    last_match_length = self.match_length
                    i = 0
                    while i < last_match_length:
                        if input_io.tell() == input_io.size():
                            break
                        c = input_io.read_u8()
                        self.delete_node(s)
                        self.text_buf[s] = c
                        if s < self.max_match - 1:
                            self.text_buf[s + self.window_size] = c
                        s = (s + 1) & (self.window_size - 1)
                        r = (r + 1) & (self.window_size - 1)
                        self.insert_node(r)
                        i += 1

                    while i < last_match_length:
                        self.delete_node(s)
                        s = (s + 1) & (self.window_size - 1)
                        r = (r + 1) & (self.window_size - 1)
                        length -= 1
                        if length > 0:
                            self.insert_node(r)
                        i += 1

                if code_buf_ptr > 1:
                    for i in range(code_buf_ptr):
                        output.append(code_buf[i])
            return output

    return bytes(Lzss().encode(input))
