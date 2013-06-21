#include <stdio.h>
#include <stdlib.h>
#include <CoreServices/CoreServices.h> 

/* fswatch.c
 * 
 * usage: ./fswatch /some/directory[:/some/otherdirectory:...] [latency]
 *
 * compile me with something like: gcc fswatch.c -framework CoreServices -o fswatch
 *
 * adapted from the FSEvents api PDF
*/

// print filename when there's any change in watch file
void callback(
	ConstFSEventStreamRef streamRef, 
	void *clientCallBackInfo, 
	size_t numEvents, 
	void *eventPaths, 
	const FSEventStreamEventFlags eventFlags[], 
	const FSEventStreamEventId eventIds[]) 
{ 
	int i;
	char **paths = (char **)eventPaths;
	for ( i = 0; i < (int)numEvents; i++ ) {
		// it would be lovely to be able to tell specifically what file and what operation 
		// is happening here, but the data returned is always a path with a flag of 0
		// maybe someday this will be useful: http://developer.apple.com/library/mac/#documentation/Darwin/Reference/FSEvents_Ref/Reference/reference.html
		// THANKS APPLE. >=(
		printf("UPDATED: %s\n", paths[i]);
	}
	fflush(stdout);
} 
 
//set up fsevents and callback
int main(int argc, char **argv) {
	void *callbackInfo = NULL; 
	FSEventStreamRef stream; 
	CFAbsoluteTime latency = 1.0;

	if (argc < 2) {
		fprintf(stderr, "usage: fswatch /some/directory[:/some/otherdirectory:...] [latency]\n");
		exit(1);
	}
	if (argc > 2) {
		// user specified a latency, overwrite default
		latency = atof(argv[2]);
	}

	CFStringRef mypath = CFStringCreateWithCString(NULL, argv[1], kCFStringEncodingUTF8); 
	CFArrayRef pathsToWatch = CFStringCreateArrayBySeparatingStrings (NULL, mypath, CFSTR(":"));

	stream = FSEventStreamCreate(NULL,
								 &callback,
								 callbackInfo,
								 pathsToWatch,
								 kFSEventStreamEventIdSinceNow,
								 latency,
								 kFSEventStreamCreateFlagNone
								 ); 

	FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode); 
	FSEventStreamStart(stream);
	CFRunLoopRun();
}
