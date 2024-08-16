from dataclasses import dataclass

@dataclass
class LZ77Token:
    offset: int
    length: int
    value: bytes

def lz77_compress_binary(data: bytes, window_size: int = 32) -> list[LZ77Token]:
    """
    Compresses binary data using the LZ77 algorithm.

    Args:
        data: The input binary data to compress.
        window_size: The size of the sliding window.

    Returns:
        A list of LZ77Token representing the compressed data.
    """

    window = b""
    output = []

    while data:
        match_length: int = 0
        match_offset: int = 0
        for i in range(min(len(window), window_size)):
            if data.startswith(window[-i:]):
                match_length = i
                match_offset = len(window) - i
                break

        if match_length > 0:
            offset = match_offset
            length = match_length
            value = data[match_length:match_length + 1]
        else:
            offset = 0
            length = 0
            value = data[0:1]

        output.append(LZ77Token(offset, length, value))
        window += data[:length + 1]
        data = data[length + 1:]
        window = window[max(0, len(window) - window_size):]

    return output



def lz77_decompress(compressed_data: list[LZ77Token]) -> bytes:
    """
    Decompresses LZ77 encoded data.

    Args:
        compressed_data: A list of LZ77Token representing the compressed data.

    Returns:
        The decompressed data as a bytes object.
    """

    decompressed_data = b""
    window = b""

    for token in compressed_data:
        if token.offset == 0:
            # Write only the token
            decompressed_data += token.value
            window += token.value
        else:
            # Copy part of the window into the buffer
            copied_data = window[token.offset:token.offset + token.length]
            decompressed_data += copied_data + token.value
            window += copied_data + token.value

    return decompressed_data


# incoming_data = "ABCDBCDE".encode()
# compressed_data = lz77_compress_binary(incoming_data, 4)
# print(compressed_data)

# restored_data = lz77_decompress(compressed_data)
# print(restored_data.decode())
