#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

static struct option const long_opts[] = {
        {"recursive", no_argument, NULL, 'R'},
        {"help",      no_argument, NULL, 'h'},
        {"version",   no_argument, NULL, 'v'},
        {NULL, 0,                  NULL, 0}
};

void usage(int status) {
    printf("Usage: cp [OPTION]... SOURCE... DIRECTORY\n");
    fputs("Copy SOURCE(s) to DIRECTORY.\n\n", stdout);
    fputs("Mandatory arguments to long options are mandatory for short options too.\n", stdout);
    fputs("\
  -R, -r, --recursive          copy directories recursively\n\n", stdout);
    fputs("\
  --help     display this help and exit\n", stdout);
    fputs("\
  --version  output version information and exit\n", stdout);

    fputs("cp 0.1\n", stdout);

    exit(status);
}

void _version(int status) {
    printf("cp 0.1\n");
    printf("Copyright (C) 2017 Eugeny Gostkin\n");
    printf("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
    printf("This is free software: you are free to change and redistribute it.\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\n\n");
    printf("Written by Eugeny Gostkin.\n");

    exit(status);
}

int do_copy(int n_files, char** file, char* destination, int recursive) {
    int mode; // 0 - dest = dir, 1 - dest = file
    struct stat s;
    int error = stat(destination, &s);
    if (error == -1) {
        if (errno == ENOENT) {
            mode = 1;
        } else {
            exit(EXIT_FAILURE);
        }
    } else {
        if (S_ISDIR(s.st_mode)) {
            mode = 0;
        } else {
            mode = 1;
        }
    }

    if (n_files > 1 && mode == 1) {
        printf("cp: target \'%s\' is not a directory\n", destination);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n_files; ++i) {
        error = stat(file[i], &s);
        if (error == -1) {
            printf("cp: cannot stat \'%s\': No such file or directory \n", file[i]);
            exit(EXIT_FAILURE);
        }
        if (S_ISDIR(s.st_mode)) {
            if (recursive) {
                // TODO: write for recursive
                /*
                struct dirent *ent;
                while((ent = readdir(dir)) != NULL) {

                }
                */
                printf("cp: omitting directory because of version \'%s\'\n", file[i]);
            } else {
                printf("cp: omitting directory \'%s\'\n", file[i]);
            }
        } else {
            if (mode == 0) {
                size_t len_dest = strlen(destination);
                size_t len_file = strlen(file[i]);
                char* new_dest = (char*) malloc((len_dest + len_file + 4) * sizeof(char));
                for (int k = 0; k < len_dest; ++k) {
                    new_dest[k] = destination[k];
                }

                if (destination[len_dest - 1] != '/') {
                    new_dest[len_dest] = '/';
                    ++len_dest;
                }

                for (int k = 0; k < len_file; ++k) {
                    new_dest[k + len_dest] = file[i][k];
                }

                FILE* old = fopen(file[i], "rb");
                FILE* new = fopen(new_dest, "wb");

                struct stat statRes;
                if(stat(file[i], &statRes) < 0)
                    return 1;

                mode_t bits = statRes.st_mode;

                char str;
                while(fread(&str, sizeof(str), 1, old) == 1){
                    fwrite(&str, sizeof(str), 1, new);
                }

                fclose(old);
                fclose(new);

                chmod(new_dest, bits);
            } else {
                FILE* old = fopen(file[i], "rb");
                FILE* new = fopen(destination, "wb");

                struct stat statRes;
                if(stat(file[i], &statRes) < 0)
                    return 1;

                mode_t bits = statRes.st_mode;

                char str;
                while(fread(&str, sizeof(str), 1, old) == 1){
                    fwrite(&str, sizeof(str), 1, new);
                }

                fclose(old);
                fclose(new);

                chmod(destination, bits);
            }
        }
    }
    return 1;
}

int main(int argc, char** argv) {
    int c;
    int recursive = 0;
    int help = 0;
    int version = 0;
    char* destination = argv[argc - 1];

    while ((c = getopt_long(argc, argv, "r:Rhv", long_opts, NULL)) != -1) {
        switch (c) {
            case 'r':
            case 'R':
                recursive = 1;
                break;

            case 'h':
                help = 1;
                break;

            case 'v':
                version = 1;
                break;

            default:
                usage(EXIT_FAILURE);
        }
    }

    if (help) {
        usage(EXIT_SUCCESS);
    }

    if (version) {
        _version(EXIT_SUCCESS);
    }

    if (argc < 3) {
        printf("At least 2 arguments must be provided.\n");
        exit(EXIT_FAILURE);
    }

    int status = do_copy(argc - optind - 1, argv + optind, destination, recursive);

    exit(status ? EXIT_SUCCESS : EXIT_FAILURE);
}