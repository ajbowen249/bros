from audioop import add
import sys
import re

def build_symbol_map(lines):
    symbol_map = {}
    address_map = {}
    for line in lines:
        match = re.match(r"^al (?P<address>[\dA-F]{6}) (?P<name>.*)$", line)
        if not match:
            continue
        groups = match.groups()
        symbol_map[groups[1]] = groups[0]
        address_map[groups[0]] = groups[1]

    return (symbol_map, address_map)

def symbol_map_to_ld65_symbols(symbol_map):
    lines = []
    for symbol in symbol_map:
        # strip the leading dot
        name = symbol[1:]

        # strip the leading two zeros
        address = symbol_map[symbol][2:]
        lines.append('\t{name}: type = weak, value = ${address};'.format(name=name, address=address))

    return lines

def read_file_lines(file_name):
    with open(file_name, 'r') as input_file:
        file_text = input_file.read()
        return file_text.split('\n')

def main():
    args = sys.argv[1:]
    symbol_file = read_file_lines(args[0])
    source_config = read_file_lines(args[1])

    (symbol_map, _) = build_symbol_map(symbol_file)
    insert_lines = symbol_map_to_ld65_symbols(symbol_map)

    start_insert_line = source_config.index('; INSERT_SYMBOLS_HERE')
    source_config.remove('; INSERT_SYMBOLS_HERE')
    for i in range(len(insert_lines)):
        source_config.insert(start_insert_line + i, insert_lines[i])

    with open(args[2], 'w') as output_file:
        output_file.write("\n".join(source_config))

if __name__ == '__main__':
    main()
