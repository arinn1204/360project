#include "binary.h"


int main(int argc, char *argv[], char *env[]) {
	int op;
	char line[128];



	//initialize the table of function pointers
	func[0] = mount_root;
	func[1] = _ls;
	func[2] = _cd;
	func[3] = _pwd;
	func[4] = _mkdir;
	func[5] = _create;
	func[6] = _rmdir;
	func[7] = _link;
	func[8] = _unlink;
	func[9] = menu;
	func[10] = init;

	(*func[10]) (0);		//initialize the FS
	//(*func[0]) (0);


	while(1) {
		printf("Enter command: ");
		(*func[9]) (0);
		fgets(line, 128, stdin);
		line[strlen(line) - 1] = 0;

		if (!strncmp(line+1, "ls", 2)) 				op = 1;
		else if (!strncmp(line+1, "cd", 2)) 		op = 2;
		else if (!strncmp(line+1, "pwd", 3)) 		op = 3;
		else if (!strncmp(line+1, "mkdir", 5))		op = 4;
		else if (!strncmp(line+1, "create", 6))		op = 5;
		else if (!strncmp(line+1, "rmdir", 5))		op = 6;
		else if (!strncmp(line+1, "link", 4))		op = 7;
		else if (!strncmp(line+1, "unlink", 6))		op = 8;
		else 										op = 9;

		
		(*func[op]) (line);

	}


	return 0;
}