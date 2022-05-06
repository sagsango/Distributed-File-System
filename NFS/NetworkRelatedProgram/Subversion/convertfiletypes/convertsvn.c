#include <dirent.h>
#include <stdio.h>
#include <unixlib/local.h>

int __riscosify_control = __RISCOSIFY_STRICT_UNIX_SPECS;

void listdir(char *dirname)
{
	DIR *dir;
        struct dirent *dirent;

	dir = opendir(dirname);

	dirent = readdir(dir);

	while (dirent) {
		if (strcmp(dirent->d_name, ".") &&
		    strcmp(dirent->d_name, "..") &&
		    strcmp(dirent->d_name, ".svn")) {
			char newname[1024];
			snprintf(newname, sizeof(newname), "%s/%s", dirname, dirent->d_name);

			if (dirent->d_type == DT_DIR) {
				listdir(newname);
			} else {
				int len = strlen(newname);
				if (len > 4 && newname[len - 4] == ','
				    && isxdigit(newname[len - 3])
				    && isxdigit(newname[len - 2])
				    && isxdigit(newname[len - 1])) {
					printf("svn ps svn:riscosfiletype %s %s\n",newname + len - 3, newname);
					printf("svn mv --force %s ",newname);
					newname[len - 4] = '\0';
					printf("%s\n",newname);
				}
			}
		}
		dirent = readdir(dir);
	}

	closedir(dir);
}

int main(void)
{
	listdir(".");
	return 0;
}

