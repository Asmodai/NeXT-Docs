//
// Copyright (C) 1994 by NeXT Computer, Inc.  All rights reserved
// 940413 sam streeper created
//

#if 0

	This protocol allows applications to request that the Terminal
application perform commands on their behalf.  This is useful if
a program wants to open up a shell window for the user to interact
with, and it may be easier than setting up pipes and doing a 
fork/exec of a program yourself.  The commands may be run in a window,
in which case a window handle is returned to allow a window to be
reused for subsequent commands, or they may be performed in the
background.  Additionally, you can pass in an NXData object whose
data will be fed to the command's stdin, and if you pass in a pointer
to an NXData, Terminal can hand you back the output of the commands
stdout and/or stderr.

	Terminal cannot provide services if it is not running.  It may
be necessary to use the workspace request protocol to launch
Terminal if the connection to Terminal fails.

	The following protocol is accessed through distributed objects.
For security reasons, the Terminal service is registered with the
bootstrap server rather than the netname server.

Terminal's distributed service may be disabled as follows:
	dwrite Terminal DisableDOServices YES
		(or)
	touch /NextApps/Terminal.app/.DisableDOServices

You can only get the stdout and stderr of a command if it is run in
the background.  If you pass a pointer to get these back, the data
at the pointer should be nil before invoking the command.  Upon
completion, the pointer will point to an newly created
NXData containing the output.

The following example shows how Terminal may be asked to perform a service:

//------------------------------
#import <appkit/appkit.h>
#import <mach/mach.h>
#import <mach/mach_error.h>
#import <mach/task_special_ports.h>
#import <servers/bootstrap.h>
#import <machkit/NXPort.h>
#import "/NextDeveloper/Headers/apps/Terminal.h"

main()
{
	int i, ret;
	int handle;
	NXData *myOutput=nil, *myEnv=nil;
	id <TSTerminalDOServices> terminal;
	port_t	sport;
	int err = bootstrap_look_up(bootstrap_port, "TerminalDO", &sport);
	NXPort *terminalPort;
	char *cp;

	if (err != KERN_SUCCESS) exit(-1);
	terminalPort = [NXPort newFromMachPort:sport];
	terminal = [NXConnection connectToPort:terminalPort];
	if (!terminal) exit(-1);

	for (i=0; i<8; i++)
	{
		[terminal runCommand:"echo \"Hello world.\""
			windowType: TSWindowDedicated
			windowHandle: &handle
			shellType:TSShellFastC
			windowTitle:"Echo Test"
			returnCode:&ret];
		sleep(1);
	}

	[terminal runCommand:"cd /bin; ls -la"
		windowTitle:"Listing of /bin"];

	// the following is bogus; you should compute the size
	// of the environment if you pass it in.  If you pass
	// NULL, however, the env is inherited.
	myEnv = [[NXData alloc] initWithSize:(8192)];
	cp = [myEnv data];
	strcpy(cp,"MESSAGE=Terminal is my friend!");
	while (*cp++);
	strcpy(cp,"PATH=/usr/local/bin:/usr/ucb:/usr/bin:/bin");
	while (*cp++);
	strcpy(cp,"");

	myOutput=nil;	// we only want data copied back to us
	[terminal runCommand:"ls -la; echo 'the environment:'; printenv"
		inputData:NULL
		outputData:&myOutput
		errorData:NULL
		waitForReturn:YES			// doesn't matter since there is out data
		windowType: TSWindowNone
		windowHandle: NULL
		exitAction: TSCloseOnExit	// doesn't matter, no window...
		shellType:TSShellBourne
		windowTitle:NULL
		directory:"/etc"
		environment:myEnv
		returnCode:&ret];
	printf("length:%d result is:\n%s\n",[myOutput size],[myOutput data]);
}
//------------------------------

#endif

#import <objc/Object.h>

typedef enum {
	TSWindowNone = 0,
	TSWindowIdle = 2,
	TSWindowNew = 4,
	TSWindowDedicated = 8
	} TSWindowType;

typedef enum {
	TSShellNone = 1,
	TSShellBourne = 2,
	TSShellDefault = 4,
	TSShellFastC = 8
	} TSShellType;

typedef enum {
	TSCloseOnExit = 0,
	TSCloseUnlessError = 1,
	TSDontCloseOnExit = 2
	} TSExitAction;

@protocol TSTerminalDOServices

// indicates the version implemented
- (int) protocolVersion;

// run in a new window, using fast c shell
- runCommand:(const char *)cmdString
	windowTitle:(const char *)winTitle;

// run command in a specified window
- runCommand:(const char *)cmdString
	windowType: (TSWindowType)winType
	windowHandle: (inout int *)myHandle
	shellType:(TSShellType)shellType
	windowTitle:(const char *)winTitle
	returnCode:(out int *)retCode;

// run in background using fast c shell, return stdout
- runCommand:(const char *)cmdString
	inputData:inputData					// an NXData object for stdin
	outputData:(id *)outputData			// a pointer to an NXData object
	waitForReturn:(int)shouldWait		// synchronous even if no out data?
	directory:(const char *)workingDir	// where to cd before execution
	returnCode:(out int *)retCode;

// the full form, all others invoke this
- runCommand:(const char *)cmdString
	inputData:(id)inputData
	outputData:(id *)outputData
	errorData:(id *)errorData
	waitForReturn:(int)shouldWait		// synchronous even if no out data?
	windowType: (TSWindowType)winType
	windowHandle: (inout int *)myHandle
	exitAction: (TSExitAction)exitAction
	shellType:(TSShellType)shellType
	windowTitle:(const char *)winTitle
	directory:(const char *)workingDir
	environment:(id)environmentData		// null separated in an NXData
	returnCode:(out int *)retCode;

@end
