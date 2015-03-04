#!/bin/bash
###########################################################################
#	Copyright (c) 2015 Scott Furry
#
#	This file is part of Freedroid
#
#	Freedroid is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	Freedroid is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with Freedroid; see the file COPYING. If not, write to the
#	Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#	MA  02111-1307  USA
###########################################################################
cp -v ../../AUTHORS .
cp -v ../../COPYING .
touch ChangeLog
touch NEWS
touch README

rm -rf autom4te.cache

autoreconf --symlink --install --verbose --force || exit $?

echo ""
echo "------------------------------------------------------------------"
echo "autogen_fddnm.sh ran successfully. Execute ./configure to proceed."
