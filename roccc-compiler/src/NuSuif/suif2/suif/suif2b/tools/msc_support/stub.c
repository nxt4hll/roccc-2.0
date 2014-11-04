#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <direct.h>
#include <errno.h>
#include <string.h>
#include <direct.h>

int main(int argc, char* argv[]){
    char dll_name[256] = "";
    char dir[256];
    char base_name[256] = "";
    char param[2048];
    char cmd[2048+256+100];
	char* postfixes[] =
		{"\\Debug\\", "\\Debug", "\\.\\Debug",
		"\\Release\\", "\\Release", "\\.\\Release"};
	char *p; int i;
    struct _finddata_t obj_file;
    long hFile;

    if ( argc<2  ) {
        fprintf(stderr, "Usage: stub.exe obj_dir [dll-name]\n\n");
        exit(1);
    }else if(argc>2){
        strcpy(dll_name, argv[2]);
    }

    strcpy(dir, argv[1]);

	if(_chdir(dir)!=0){
        fprintf(stderr, "Can't open %s\n", dir);
        exit(1);
    }

	if(!_getcwd(dir, 255)){
        fprintf(stderr, "cwd failed: %s\n", dir);
        exit(1);
	}

	printf("Using directory %s\n", dir);
	strcpy(param, "");

	if(!strcmp(dll_name, "")){
        for(i=0; i<(sizeof(postfixes))/sizeof p; i++){
			char* postfix = postfixes[i];

			int delta = (strlen(dir)-strlen(postfix));

			p = dir+delta;

			if(!strcmp(postfix, p)){
				strncpy(base_name, dir, delta);
				base_name[delta] = '\0';
				break;
			}
		}

		if(!strcmp(base_name, "")){
			fprintf(stderr, "Couldn't parse the directory name %s\n", dir);
			exit(1);
		}

		if((p = strrchr(base_name, '\\'))==NULL){
			fprintf(stderr, "Can't locate the last component in the path name %s\n", base_name);
			exit(1);
		}

		strcpy(dll_name, p+1);
	}


	hFile = _findfirst("*.obj", &obj_file);

    if(hFile == ENOENT || hFile == EINVAL){
        fprintf(stderr, "No %s\\*.obj files found or the specification was invalid\n", dir);
        exit(1);
    }

	strcpy(param, obj_file.name);
    strcat(param, " ");

    while(_findnext(hFile, &obj_file)==0){
        strcat(param, obj_file.name);
        strcat(param, " ");
    }
    _findclose(hFile);

    sprintf(cmd, "dumpbin.exe %s /symbols > %s.dmp", param, dll_name);
    printf("Creating a .def file for %s: dumping symbols: %s...\n", dll_name, cmd);

    if(system(cmd)==-1){
        fprintf(stderr, "Error running dumpbin: %s\n", strerror(errno));
        exit(1);
	};

    strcpy(cmd, "%NCIHOME%\\bin\\stubber.exe ");
    strcat(cmd, dll_name);
	strcat(cmd, " -quiet");
    printf("Stubbing: %s...\n", cmd);
    if(system(cmd)==-1){
        fprintf(stderr, "Error running the stubber: %s\n", strerror(errno));
        exit(1);
	};
    printf("Done.\n");
	return 0;
}