#!/usr/bin/env sh

# Pheniqs : PHilology ENcoder wIth Quality Statistics
# Copyright (C) 2018  Lior Galanti
# NYU Center for Genetics and System Biology

# Author: Lior Galanti <lior.galanti@nyu.edu>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.

# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# dump configuration.json into a byte array in configuration.h
# NOTICE: this method is no longer in use. please see serialize_configuration.py

printf "/*
    Pheniqs : PHilology ENcoder wIth Quality Statistics
    Copyright (C) 2018  Lior Galanti
    NYU Center for Genetics and System Biology

    Author: Lior Galanti <lior.galanti@nyu.edu>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    This file is auto generated from configuration.json

    Generated on: $(date)
*/

#ifndef PHENIQS_CONFIGURATION_H
#define PHENIQS_CONFIGURATION_H

" > configuration.h;

printf "size_t configuration_json_len = \
$(cat configuration.json | sed -E 's/^ +//g' | hexdump -v -e '1/1 "%02x "' | wc -w | sed 's/ //g');\n\n" >> configuration.h;

printf "const char configuration_json[] = \
{\n    $(cat configuration.json | sed -E 's/^ +//g' | hexdump -v -e '12/1 "0x%02x, " "\n" "    "' | sed -E 's/( 0x  ,)*$//' | grep -vE "^\s+$")\n};\n" >> configuration.h;

printf "\n#endif /* PHENIQS_CONFIGURATION_H */\n" >> configuration.h;
