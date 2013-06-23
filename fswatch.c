#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <CoreServices/CoreServices.h> 

/* fswatch.c
 * 
 * usage: ./fswatch /some/directory[:/some/otherdirectory:...] [latency] [logfilepath]
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
	time_t rawtime;
	struct tm * timeinfo;
	char timeStr[80];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime (timeStr, 80, "[%Y-%m-%d|%H:%M:%S]", timeinfo);
	
	FILE *pLog = NULL;
	if ( clientCallBackInfo != NULL ) {
		int i = 0;
		while ( pLog == NULL && i < 3 ) {
			// there's a log file, open it
			pLog = fopen((char *)clientCallBackInfo, "at");
			if ( pLog == NULL && i < 2 ) { 
				printf("Unable to open log file, trying again in 3 seconds...\n");
				sleep(3000);
			}
			i++;
		}
		
		if ( pLog == NULL ) fprintf(stderr, "Unable to open log file %s for writing - event will be written to stdout instead!\n", (char *)clientCallBackInfo);
	}
  
  	int i;
	char **paths = (char **)eventPaths;
	for ( i = 0; i < (int)numEvents; i++ ) {
		// it would be lovely to be able to tell specifically what file and what operation 
		// is happening here, but the data returned is always a path with a flag of 0
		// maybe someday this will be useful: http://developer.apple.com/library/mac/#documentation/Darwin/Reference/FSEvents_Ref/Reference/reference.html
		// THANKS APPLE. >=(
		if ( pLog != NULL ) fprintf(pLog, "%s - UPDATED: %s\n", timeStr, paths[i]);
		else printf("%s - UPDATED: %s\n", timeStr, paths[i]);
	}
	fflush(stdout);
	if ( pLog != NULL ) { fflush(pLog); fclose(pLog); }
} 
 
//set up fsevents and callback
int main(int argc, char **argv) {
	FSEventStreamRef stream; 
	CFAbsoluteTime latency = 1.0;
	FILE *pLog = NULL;

	if (argc < 2) {
		fprintf(stderr, "usage: fswatch /some/directory[:/some/otherdirectory:...] [latency] [logfilepath]\n");
		exit(1);
	}
	if (argc > 2) {
		// user specified a latency, overwrite default
		latency = atof(argv[2]);
	}
	if (argc > 3) {
		// user specified a log file path, open/lock the file
		
		// make sure directory exists
		char dir[256];
		int len = 0;
		char *pDir;
		
		// figure out dir name
		strncpy(dir, argv[3], 256);
		len = strlen(dir);
		pDir = &dir[len-1];
		while (*pDir != '/') pDir--;
		*pDir = '\0';
		mkdir(dir, S_IREAD | S_IWRITE | S_IEXEC | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

		// open file for writing
		pLog = fopen(argv[3], "wt");
		if ( !pLog ) { fprintf(stderr, "Failed to open log file - %s!\n", argv[3]); exit(1); }
		fclose(pLog);
	}

	CFStringRef mypath = CFStringCreateWithCString(NULL, argv[1], kCFStringEncodingUTF8); 
	CFArrayRef pathsToWatch = CFStringCreateArrayBySeparatingStrings (NULL, mypath, CFSTR(":"));
	FSEventStreamContext ctx;
	ctx.version = 0; ctx.retain = NULL; ctx.release = NULL; ctx.copyDescription = NULL;
	ctx.info = argv[3];

	stream = FSEventStreamCreate(NULL,
								 &callback,
								 &ctx,
								 pathsToWatch,
								 kFSEventStreamEventIdSinceNow,
								 latency,
								 kFSEventStreamCreateFlagNone
								 ); 

	FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode); 
	FSEventStreamStart(stream);
	CFRunLoopRun();
}
