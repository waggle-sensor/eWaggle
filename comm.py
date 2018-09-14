boundary = 0x7e
escape = 0x7d


def encode(msg):
    for c in msg:
        if c == boundary or c == escape:
            yield escape
            yield c ^ 0x20
        else:
            yield c


def decode():
    pass

# right...all we have to do is scan for a message boundary
# we can do an initial lock and then start processing.


data = bytes([1, 2, 3, 4, 5, 6])
print(list(encode(data)))

data = bytes([1, 2, 3, escape, 5, 6])
print(list(encode(data)))
