import textwrap
import numpy
import os

print(os.getcwd())

COLOURS_NUM = 2
PIECES_NUM = 6
SQUARES_NUM = 64

rng = numpy.random.default_rng(seed=None)

# 64 bit unsigned int keys indexed in a table as [colour][piece][square]
zobrist_table = rng.integers(low=0, high=2**64, size=(COLOURS_NUM, PIECES_NUM, SQUARES_NUM), dtype=numpy.uint64)

zobrist_castling = rng.integers(low=0, high=2**64, size=4, dtype=numpy.uint64)
zobrist_en_passant = rng.integers(low=0, high=2**64, size=8, dtype=numpy.uint64)
zobrist_player_turn = rng.integers(low=0, high=2**64, size=1, dtype=numpy.uint64)

# Converts the table to a C++ style strings
def format_zobrist_table(table, var_name):
    lines = [f"constexpr uint64_t {var_name}[{COLOURS_NUM}][{PIECES_NUM}][{SQUARES_NUM}] = {{"]
    
    for colour in range(table.shape[0]):
        lines.append("    {")
        for piece in range(table.shape[1]):
            # Comma separated upper case hexadecimal values padded with 16 whitespace
            row = ", ".join(f"0x{value:016X}ULL" for value in table[colour, piece])
            lines.append(f"    {{{row}}},")
            
        lines.append("    },")
        
    lines.append("};")
    return "\n".join(lines)

def format_zobrist_array(array, var_name):
    values = ", ".join(f"0x{value:016X}ULL" for value in array)
    return f"inline constexpr uint64_t {var_name}[{len(array)}] = {{{values}}};"

def format_zobrist_scalar(array, var_name):
    value = array[0]
    return f"inline constexpr uint64_t {var_name} = 0x{value:016X}ULL;"

def create_header_file(filename):
    castling_str = format_zobrist_array(zobrist_castling, "zobristCastling")
    en_passant_str = format_zobrist_array(zobrist_en_passant, "zobristEnPassant")
    player_turn_str = format_zobrist_scalar(zobrist_player_turn, "zobristPlayerTurn")

    header_code = textwrap.dedent(f"""\
    #ifndef ZOBRIST_KEYS_H
    #define ZOBRIST_KEYS_H

    #include <cstdint>

    extern const uint64_t zobristTable[{COLOURS_NUM}][{PIECES_NUM}][{SQUARES_NUM}];

    {castling_str}
    {en_passant_str}
    {player_turn_str}

    #endif // ZOBRIST_KEYS_H

    """)

    with open(filename, "w") as file:
        file.write(header_code)

    print(f"Zobrist keys have been written to {filename}")

def create_cpp_file(filename):
    cpp_code = textwrap.dedent("""\
    #include <cstdint>
    #include "zobrist_keys.h"

    """)

    cpp_code += format_zobrist_table(zobrist_table, "zobristTable")

    with open(filename, "w") as file:
        file.write(cpp_code)

    print(f"Zobrist keys have been written to {filename}")

header_file_path = "backend/include/zobrist_keys.h"
create_header_file(header_file_path)

cpp_relative_path = "backend/src/zobrist_keys.cpp"
create_cpp_file(cpp_relative_path)