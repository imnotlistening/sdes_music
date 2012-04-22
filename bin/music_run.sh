#!/bin/bash
# Run a music command (for testing purposes). This expects a valid $MUSIC_HOME
# environment variable to be set.
#

# Export the directory holding out music shared lib.
export LD_LIBRARY_PATH=$MUSIC_HOME/bin

# The command to run...
COMMAND=$1
shift

# Execute the command and pass the remaining passed arguments to the command.
#echo $MUSIC_HOME/bin/$COMMAND $*
$MUSIC_HOME/bin/$COMMAND $*
