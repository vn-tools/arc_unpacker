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

    def tell(self):
        return self.file.tell()

    def seek(self, *args):
        return self.file.seek(*args)

    def skip(self, bytes):
        self.file.seek(bytes, io.SEEK_CUR)

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

    def write_u8(self, x):
        self.file.write(struct.pack('B', x))

    def write_u16_le(self, x):
        self.file.write(struct.pack('<H', x))

    def write_u32_le(self, x):
        self.file.write(struct.pack('<I', x))

    def write_u64_le(self, x):
        self.file.write(struct.pack('<Q', x))

    def write_u16_be(self, x):
        self.file.write(struct.pack('>H', x))

    def write_u32_be(self, x):
        self.file.write(struct.pack('>I', x))

    def write_u64_be(self, x):
        self.file.write(struct.pack('>Q', x))

    def write_zero_padded(self, x, size):
        self.file.write(x)
        self.file.write(b'\x00' * (size - len(x)))

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
