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
# Build the server responsible for sending out RTP streams.
#

OBJECTS	= gst_rtp.o
PROGS	= stream_server stream_client

# Build a list of dependency files.
DEPS    := $(shell find | grep \\.d$)

# Dynamically figure out the local CFLAGS we will need to build GST/GLib code.
LOCAL_CFLAGS	= $(shell pkg-config --cflags gstreamer-0.10)
PROJ_LIB_FLAGS	= $(shell pkg-config --libs gstreamer-0.10)

include $(BUILD)/config.mk

all: $(OBJECTS) $(PROGS)

.PHONY: clean info

info:
	$(MSG) "LOCAL_CFLAGS   = $(LOCAL_CFLAGS)"
	$(MSG) "PROJ_LIB_FLAGS = $(PROJ_LIB_FLAGS)"

# Some special targets to make the build commands look pretty.
$(PROGS): %: %.c $(OBJECTS)
	$(MSG) " [CCLD] " $@
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) $(CPPFLAGS) $(OBJECTS) -o $@ $@.c \
		$(PROJ_LIB_FLAGS)

# Pull in dynamically generated dependencies. For some reason this has to
# be *below* the $(PROGS) target.
-include $(DEPS)
.c.o:
	$(MSG) " [CC]   " $@
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) $(CPPFLAGS) -c $< -o $@
	$(CC) -MM $(CFLAGS) $(LOCAL_CFLAGS) $(CPPFLAGS) -c $< > \
		 $(dir $(@)).$(notdir $(@:.o=.d))

clean:
	$(MSG) "Cleaning \`streaming'"
	@rm -f $(OBJECTS) $(PROGS)
	@rm -f $(DEPS)