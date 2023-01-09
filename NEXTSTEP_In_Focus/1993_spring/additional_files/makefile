# Makefile for password check program

install: pw_check root_password /usr/local/bin rc.boot
	@echo "Installing pw_check in /usr/local/bin"
	@strip pw_check
	@cp pw_check /usr/local/bin/pw_check
	@echo "Coping /.profile to /.profile.orig"
	@cp /.profile /.profile.orig
	@echo "Coping sample .profile to /.profile"
	@cp sample_profile /.profile

pw_check: pw_check.c
	@echo "Compiling pw_check"
	@cc -o pw_check pw_check.c

root_password:
	@echo "************************************************************"
	@echo "* Setting root password in /etc/passwd file                *"
	@echo "* Password will be the same as local NetInfo root password *"
	@echo "************************************************************"
	@nidump passwd . | grep '^root:' > /tmp/new_passwd
	@sed '/^root:/d' < /etc/passwd >> /tmp/new_passwd
	@echo "Copying /etc/passwd to /etc/passwd.orig"
	@cp /etc/passwd /etc/passwd.orig
	@cp /tmp/new_passwd /etc/passwd

/usr/local/bin:
	@echo "Creating /usr/local/bin"
	@mkdirs /usr/local/bin

rc.boot:
	@echo "Protecting /etc/rc.boot"
	@head -1 /etc/rc.boot > /tmp/rc.boot
	@echo trap \"\" 2 3 >> /tmp/rc.boot
	@tail +2 /etc/rc.boot >> /tmp/rc.boot
	@echo "Copying /etc/rc.boot to /etc/rc.boot.orig"
	@cp /etc/rc.boot /etc/rc.boot.orig
	@cp /tmp/rc.boot /etc/rc.boot

undo: clean
	@echo "Restoring original /etc/passwd"
	@cp /etc/passwd.orig /etc/passwd
	@echo "Restoring original /.profile"
	@cp /.profile.orig /.profile
	@echo "Restoring original /etc/rc.boot"
	@cp /etc/rc.boot.orig /etc/rc.boot

clean:
	@echo "Cleaning up"
	@rm -f core *.o pw_check /usr/local/bin/pw_check
