// terminal.m
// this program opens a terminal window in the current directory
// author: sam streeper

/*
**	terminal [-shell] [-dir <dir>] [-noclose] [-env] [command]
*/

#import <appkit/appkit.h>
#import <mach/mach.h>
#import <mach/mach_error.h>
#import <mach/task_special_ports.h>
#import <servers/bootstrap.h>
#import <machkit/NXPort.h>
// #import "/NextDeveloper/Headers/apps/Terminal.h"
#import "TerminalDOProtocol.h"
#import <sys/param.h>
#import <string.h>

#define BLOCKSIZE (2048)
#define streq !strcmp

char pathArray[MAXPATHLEN];
char command[2048];

usage()
{
	fprintf(stderr,"terminal: open a new Terminal.app window\n");
	fprintf(stderr,"usage: terminal [-shell] [-dir <dir>] [-noclose] [-env] [command]\n");
	fprintf(stderr,"	-shell   - run command from default shell\n");
	fprintf(stderr,"	-dir     - specify working directory\n");
	fprintf(stderr,"	-noclose - retain window upon exit\n");
	fprintf(stderr,"	-env     - export current environment\n");
	exit(-1);
}

main(int argc, char *argv[])
{
	int i, ret, firstCommand = 1;
	NXData *myEnv=nil;
	id <TSTerminalDOServices> terminal;
	port_t	sport;
	int err = bootstrap_look_up(bootstrap_port, "TerminalDO", &sport);
	NXPort *terminalPort;
	char *cp;
	char *path = NULL;
	BOOL useShell = NO;
	BOOL close = YES;
	BOOL exportEnv = NO;

	if (err != KERN_SUCCESS)
	{
		// should really ask workspace to open terminal
		// and try again...
		fprintf(stderr,"Can't connect to Terminal\n");
		exit(-1);
	}

	terminalPort = [NXPort newFromMachPort:sport];
	terminal = [NXConnection connectToPort:terminalPort];
	if (!terminal)
	{
		fprintf(stderr,"Can't connect to Terminal\n");
		exit(-2);
	}

	if ([terminal protocolVersion] < 0x10002)
	{
		fprintf(stderr,"Incompatible terminal protocol version\n");
		exit(-4);
	}

	if (argc == 1)
	{
		useShell = YES;
	}

	while ((firstCommand < argc) && (*argv[firstCommand]=='-'))
	{
		if (streq("-shell", argv[firstCommand]))
		{
			useShell = YES;
		}
		else if (streq("-dir", argv[firstCommand]))
		{
			if (++firstCommand >= argc) usage();
			path = argv[firstCommand];
		}
		else if ((streq("-noclose", argv[firstCommand])))
		{
			close = NO;
		}
		else if ((streq("-env", argv[firstCommand])))
		{
			exportEnv = YES;
		}
		else
		{
			usage();
		}
		firstCommand++;
	}

	if (!path)
	{
		path = pathArray;
		if(!getwd(path))
		{
			fprintf(stderr,"Can't get current directory\n");
			exit(-3);
		}
	}

	// build command from args
	command[0] = '\0';
	for (i=firstCommand; i<argc; i++)
	{
		strcat(command, argv[i]);
		strcat(command, " ");
	}

	if (exportEnv)
	{
		//build passed env
		ret = 1;
		for(i=0; environ[i] != NULL; i++)
		{
			ret = ret + strlen(environ[i]) + 1;
		}

		ret = ((ret / BLOCKSIZE) + 1) * BLOCKSIZE;

		myEnv = [[NXData alloc] initWithSize:ret];

		if (myEnv)
		{
			cp = [myEnv data];

			for(i=0; environ[i] != NULL; i++)
			{
				strcpy(cp,environ[i]);
				while (*cp++);
			}
			strcpy(cp,"");
		}
		else exportEnv = NO;
	}

	[terminal runCommand: command[0] ? command : NULL
		inputData: NULL
		outputData: NULL
		errorData: NULL
		waitForReturn: NO
		windowType: TSWindowNew
		windowHandle: NULL
		exitAction: close ? TSCloseUnlessError : TSDontCloseOnExit
		shellType: useShell ? TSShellDefault : TSShellNone
		windowTitle: command[0] ? command : NULL
		directory: path
		environment: exportEnv ? myEnv : NULL
		returnCode: &ret];

	return ret;
}
