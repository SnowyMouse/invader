# SPDX-License-Identifier: GPL-3.0-only

import os
import sys

import definition_parser
import header_generator
import parser_generator

if len(sys.argv) != 4:
    print("Usage: {} <definition_dir> <header_dir> <source_dir>".format(sys.argv[0]))
    sys.exit(1)

definitions = definition_parser.parse_definitions_from_dir(sys.argv[1])
header_generator.generate_headers(definitions, sys.argv[2])
parser_generator.generate_parser(definitions, sys.argv[3])
