#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include "util.h"




int init(char *name) {

}

int mount_root(char *name) {

}

int _ls(char *name) {


}

int _cd(char *name) {

}

int _pwd(char *name) {

}

int _mkdir(char *name) {
}

int _create(char *name) {

}

int _rmdir(char *name) {

}

int _link(char *name) {

}

int _unlink(char *name) {

}

int menu(char *name) {
	printf("[ls][cd][pwd][mkdir][create][rmdir][link][unlink][menu] ");
}

int (*func[10]) (char *name); 