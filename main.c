#include "binary.h"

void initFunction(char *argv[]) {
/*	//Part one
	func[0] = init;
	func[1] = mount_root;
	func[2] = _ls;
	func[3] = _cd;
	func[4] = _pwd;
	func[5] = _mkdir;
	func[6] = _create;
	func[7] = _rmdir;
	func[8] = _link;
	func[9] = _unlink;
	func[10] = _rm;
	func[11] = _symlink;
	func[12] = _chmod;
	func[13] = _chown;
	func[14] = _stat;
	func[15] = _touch;
	func[30] = quit;
	func[31] = menu;
	//Part Two

	/*func[16] = _open;
	func[17] = _close;
	func[18] = _pfd;
	func[19] = _lseek;
	func[20] = _access;
	func[21] = _read;
	func[22] = _write;
	func[23] = _cat;
	func[24] = _cp;
	func[25] = _mv;

	//Part Three	
	func[26] = _mount;
	func[27] = _umount;
	func[28] = cs;
	func[29] = sync;
	func[30] = quit;
	/*/
	
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
		else if (!strncmp(command, "touch", 5))			op = 15; /*
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;
		else if (!strncmp(command, "unlink", 6))		op = 9;*/
	return op;
}



char pathname[64], parameter[64], cmd[32];

int main(int argc, char *argv[], char *env[]) {
	int op;
	char line[128];

	P0 = (PROC *)calloc(sizeof(PROC *) + 1, 1);
	P1 = (PROC *)calloc(sizeof(PROC *) + 1, 1);
	running = (PROC *)calloc(sizeof(PROC *) + 1, 1);

	if (argc < 2) {
		fprintf(stderr, "Need to enter disk name\n");
		exit(-1);
	}

	//initialize the table of function pointers
	//initFunction(argv);


	while(1) {
		printf("Enter command: ");
		(*func[9]) (0);
		fgets(line, 128, stdin);
		line[strlen(line) - 1] = 0;
		if (line[0] == 0) {
			printf("No command.\n");
			continue;
		}

		sscanf(line, "%s %s %64c", cmd, pathname, parameter);



		
		(*func[op]) (line);
		printf("\n");

	}

	free(P0); free(P1); free(running);


	return 0;
}