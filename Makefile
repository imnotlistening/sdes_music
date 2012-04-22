#
# (C) Copyright 2012
# Alex Waterman <imNotListening@gmail.com>
#
# MUSIC is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# MUSIC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with MUSIC.  If not, see <http://www.gnu.org/licenses/>.
#
# Top level makefile. Build any required sub modules.
#

include config.mk

export BUILD := $(PWD)

SUBDIRS	= streaming tests

all:
	@for DIR in $(SUBDIRS); do \
		make --no-print-directory -C $$DIR ; \
	done

clean: clean-bin
	@for DIR in $(SUBDIRS); do \
		make --no-print-directory -C $$DIR clean; \
	done

clean-bin:
	$(MSG) Cleaning \`bin\'
	@rm -rf bin/libmstream.so* bin/multi_stream_test bin/stream_client \
		bin/stream_server bin/test_plist