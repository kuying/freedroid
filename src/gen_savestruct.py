#!/usr/bin/env python
#-*- encoding: utf-8 -*-
#
# struct_to_io.py
# Extracts structures from a C header file and generates in/output code.
#
# Copyright (c) 2008 Pierre "delroth" Bourdon <root@delroth.is-a-geek.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import sys, re

# Parts of regexps
c_id = r'[\w\d]+' # Standard C identifier
c_type = r'((?:(?:unsigned|signed|short|long)\s+)*' + c_id + r'\s*\*?)' # C type

# Regexp which search for a structure
find_structure_rxp = re.compile(r'typedef struct .+?'
                                r'\{'
                                r'([^\}]+)'
                                r'\}'
                                r'\s*(' + c_id + ').*?;', re.M | re.S)
# Regexp which search for a field
find_members_rxp = re.compile(r'\s*' + c_type + r'\s+(' + c_id + r')(?:\[(\d+)\])?\s*?;.*')

# Special types replacements
special_types = {
    'char_ptr': 'string',
    'unsigned_int': 'uint',
}

only_dump_structs = [ "gps", "point", "moderately_finepoint", "finepoint", "tux_t", "item", "enemy", "bullet" ]

def main():
    if len(sys.argv) < 3:
        print "Usage: %s <input> <output>" % sys.argv[0]
        sys.exit(1)
    
    # Filenames
    inpfn, outfn = sys.argv[1:]
    inpf = open(inpfn, 'r')
    outf = open(outfn, 'w')

    data = {}

    header = inpf.read()
    structures = find_structure_rxp.findall(header)
    for s in structures:
        # s is a tuple which contains (code, name) with code = the code inside the structure
        code, name = s

	if name not in only_dump_structs:
	    continue

        # Avoid matching things in comments
        lines = []
        in_comment_block = False
        for l in code.split('\n'):
            l = l.split('//')[0]
            if "/*" in l:
                l = l.split('/*')[0]
                lines.append(l)
                in_comment_block = True
            elif "*/" in l:
                in_comment_block = False
                l = l.split('*/')[1]
            if not in_comment_block: lines.append(l)
        
        # This list will contain all the (type, field, size) tuples
        a = []
        for l in lines:
            m = find_members_rxp.findall(l)
            if len(m) == 0: continue
            else: a.append(m[0])

        data[name] = []

        # Transform the raw tuple
        for f in a:
            type, field, size = f
            type = type.lower().strip()
            
            # Pointers
            if '*' in type:
                type = type.replace('*', '').strip() + '_ptr'
            # Spaces
            type = type.replace(' ', '_')
            if type in special_types.keys(): type = special_types[type]
            elif type == 'char' and size: type = "string" # HACK: char foo[SIZE] -> string
            elif size: type += '[%s]' % size
            data[name].append((type, field))
    
    # Writing loop
    for s_name in data.keys():
        str_save = str_read = ''
        str_save += 'int save_%s(%s * target)\n{\nfprintf(zl_savefile, "<%s ");\n' % (s_name,s_name,s_name)
        str_read += 'int read_%s(char* buffer, %s * target)\n{\n' % (s_name,s_name)
        for (type, field) in data[s_name]:
            size = None
            if "[" in type:
                size = int(type.split('[')[1][:-1])
                type = type.split('[')[0] + '_array'
            str_save += 'save_%s("%s", target->%s%s);\n' % (type, field, field, (', %s' % size) if size else '')
            str_read += 'read_%s(buffer, "%s", &(target->%s)%s);\n' % (type, field, field, (', %s' % size) if size else '')
        str_save += 'fprintf(zl_savefile, " >\\n");\nreturn 0;\n}\n\n'
        str_read += 'return 0;\n}\n\n'
        outf.write(str_save)
        outf.write(str_read)

if __name__ == '__main__': main()
