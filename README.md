AZ2Tonez changes: Print 'UPDATED: <path>' to stdout (or a log file) instead of running arbitrary command. Also, allow the latency to be specified via the command line. Add a fun comment.

## fswatch.c

This is a small program using the Mac OS X FSEvents API to monitor a directory.
When an event about any change to that directory is received, the specified
shell command is executed by `/bin/bash`.

If you're on GNU/Linux,
[inotifywatch](http://linux.die.net/man/1/inotifywatch) (part of the
`inotify-tools` package on most distributions) provides similar
functionality.

### Compile

You need to be on Mac OS X 10.5 or higher with Developer Tools
installed.  Then, run `make`.  Copy the resulting `fswatch` binary to
a directory on your `$PATH`.

### Basic Usage

    ./fswatch /some/dir

This would monitor `/some/dir` for any change, and print the path names
to stdout every second when a modification event is received.

In the case you want to watch multiple directories, just separate them
with colons like:

    ./fswatch /some/dir:/some/otherdir 

In the case you want to wait longer before printing the queued up events
(this will print events after waiting for 3 seconds from the first event received):

    ./fswatch /some/dir:/some/otherdir 3.0

In the case you want to log events to a log file, instead of stdout, specify a log file path:

    ./fswatch /some/dir:/some/otherdir 1.0 /path/to/log.txt

### About

This code was adapted from the example program in the
[FSEvents API documentation](https://developer.apple.com/library/mac/documentation/Darwin/Conceptual/FSEvents_ProgGuide/FSEvents_ProgGuide.pdf).
