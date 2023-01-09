/* 
 * Copyright 1993 NeXT Computer Inc.  All rights reserved.
 * Written by Marc Majka

 * You may freely copy, distribute and reuse the code in this example.  
 * NeXT disclaims any warranty of any kind, expressed or implied, as to 
 * its fitness for any particular use.
 */

#import <stdio.h>
#import <pwd.h>
#import <signal.h>
#import <sys/reboot.h>

#define MAXATTEMPTS 3

main(argc, argv)
int argc;
char *argv[];
{
	char *crypt(), *getpass(), salt[3], key[9], target[16];
	struct passwd *pw;
	int trying, attempt;

	/* ignore signals */
	signal(SIGINT,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	if (argc < 2) {					/* no command line option */
		pw = getpwuid(0);			/* lookup root */
		if (pw == NULL) exit(0);		/* root doesn't exist! */
	}

	else {
		pw = getpwnam(argv[1]);			/* lookup given user name */
		if (pw == NULL) {			/* user doesn't exist */
			pw = getpwuid(0);		/* lookup root instead */
			if (pw == NULL) exit(0);	/* root doesn't exist! */
		}
	}

	strcpy(target,pw->pw_passwd);		/* target password */

	salt[0] = target[0];			/* the "salt" */
	salt[1] = target[1];
	salt[2] = '\0';

	trying = 1;
	for (attempt = 0; attempt < MAXATTEMPTS && trying; attempt++) {
		strcpy(key, getpass("Password: "));
		if (key[0] != 0 && !strcmp(target, crypt(key, salt))) 
			trying = 0;
	}

	if (trying) reboot(RB_HALT);		/* user was still trying - halt */
	exit(0);				/* user entered the correct password */
}
		
