#include "binary/binary.h"

void initFunction(char *argv[]) {

	(*func[0]) (0);		//initialize the FS
	(*func[1]) (argv[1]);
}

int findCMD(char *command) {
	int op = -1;
		if (!strncmp(command, "ls", 2)) 				op = 2;
		else if (!strncmp(command, "cd", 2)) 			op = 3;
		else if (!strncmp(command, "pwd", 3))	 		op = 4;
		else if (!strncmp(command, "mkdir", 5))			op = 5;
		else if (!strncmp(command, "create", 6))		op = 6;
		else if (!strncmp(command, "rmdir", 5))			op = 7;
		else if (!strncmp(command, "link", 4))			op = 8;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "rm", 2))			op = 10;
		else if (!strncmp(command, "symlink", 7))		op = 11;
		else if (!strncmp(command, "chmod", 5))			op = 12;
		else if (!strncmp(command, "chown", 5))			op = 13;
		else if (!strncmp(command, "stat", 4))			op = 14;
		else if (!strncmp(command, "touch", 5))			op = 15;
		else if (!strncmp(command, "quit", 4))			op = 30;
		else if (!strncmp(command, "menu", 4))			op = 31;
		else {
			printf("No command, %s\n", command);
			op = 31;
		}
	return op;
}

int main(int argc, char *argv[], char *env[]) {
	int op;
	char line[128];

	if (argc < 2) {
		fprintf(stderr, "Need to enter disk name\n");
		exit(-1);
	}

	//initialize the table of function pointers
	initFunction(argv);


	while(1) {
		printf("Enter command: ");
		//(*func[31]) (0);
		fgets(line, 128, stdin);
		line[strlen(line) - 1] = 0;
		if (line[0] == 0) {
			printf("No command.\n");
			continue;
		}

		sscanf(line, "%s %s %s", cmd, pathname, parameter);

		op=findCMD(cmd);

		
		(*func[op]) (pathname);
		//printf("\n");
		bzero(cmd, 32);
		bzero(parameter, 64);
		bzero(pathname, 64);

	}

<<<<<<< HEAD
=======

>>>>>>> testing
	return 0;
}