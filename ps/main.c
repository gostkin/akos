#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static struct option const long_opts[] = {
        {"all",     no_argument, NULL, 'A'},
        {"help",    no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {NULL, 0,                NULL, 0}
};

static struct option const terms[] = {
        {"mem",         no_argument, NULL, 1},
        {"tty",         no_argument, NULL, 4},
        {"tty",         no_argument, NULL, 5},
        {"lp",          no_argument, NULL, 6},
        {"vcs",         no_argument, NULL, 7},
        {"misc",        no_argument, NULL, 10},
        {"input",       no_argument, NULL, 13},
        {"sg",          no_argument, NULL, 21},
        {"fb",          no_argument, NULL, 29},
        {"video4linux", no_argument, NULL, 81},
        {"i2c",         no_argument, NULL, 89},
        {"ppdev",       no_argument, NULL, 99},
        {"ppp",         no_argument, NULL, 108},
        {"alsa",        no_argument, NULL, 116},
        {"ptm",         no_argument, NULL, 128},
        {"pts",         no_argument, NULL, 136},
        {"usb",         no_argument, NULL, 180},
        {"usb_device",  no_argument, NULL, 189},
        {"rfcomm",      no_argument, NULL, 216},
        {"drm",         no_argument, NULL, 226},
        {"media",       no_argument, NULL, 242},
        {"mei",         no_argument, NULL, 243},
        {"kfd",         no_argument, NULL, 244},
        {"aux",         no_argument, NULL, 245},
        {"ptp",         no_argument, NULL, 246},
        {"pps",         no_argument, NULL, 247},
        {"hidraw",      no_argument, NULL, 248},
        {"bsg",         no_argument, NULL, 249},
        {"watchdog",    no_argument, NULL, 250},
        {"rtc",         no_argument, NULL, 251},
        {"dimmctl",     no_argument, NULL, 252},
        {"ndctl",       no_argument, NULL, 253},
        {"tpm",         no_argument, NULL, 254}
};

void usage(int status) {
    printf("Usage:\n ps [options]\n\nBasic options:\n -A, -e               all processes\n");
    fputs(" --help               display this help and exit\n", stdout);
    fputs(" --version            output version information and exit\n", stdout);
    fputs("ps 0.1\n", stdout);
    exit(status);
}

void _version(int status) {
    printf("ps 0.1\n");
    printf("Copyright (C) 2017 Eugeny Gostkin\n");
    printf("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
    printf("This is free software: you are free to change and redistribute it.\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\n\n");
    printf("Written by Eugeny Gostkin.\n");

    exit(status);
}

void get_tty(char* path, char* out) {
    char fdpath[PATH_MAX];
    fdpath[0] = '\0';
    strcpy(fdpath, path);
    strcat(fdpath, "/fd/0");

    if (readlink(fdpath, out, PATH_MAX) == -1) {
        strcpy(out, "?");
    } else {
        strcpy(out, out + 5);
    }
}

int get_tty_(int major, char* to) {
    for (int i = 0; i < 33; ++i) {
        if (terms[i].val == major) {
            strcpy(to, terms[i].name);
            return 1;
        }
    }
    return 0;
}

void get_cur_tty(char* start, char* cur_tty) {
    int cur_pid = getpid();
    char cur_path[PATH_MAX];
    strcpy(cur_path, start);

    char t_path[PATH_MAX];
    t_path[0] = '\0';

    cur_tty[0] = '\0';

    while (cur_pid > 0) {
        char* s = malloc(2 * sizeof(char));
        s[0] = (char) ('0' + cur_pid % 10);
        s[1] = '\0';
        strcat(t_path, s);
        cur_pid /= 10;
        free(s);
    }
    int iter = 0;

    int length = (int) strlen(t_path);
    while (iter < length - iter - 1) {
        char t = t_path[iter];
        t_path[iter] = t_path[length - iter - 1];
        t_path[length - iter - 1] = t;
        ++iter;
    }
    strcat(cur_path, t_path);
    get_tty(cur_path, cur_tty);
}

int process(int all) {
    printf("PID\t TTY\t TIME\t CMD\t\n");
    char* start = "/proc/";
    char* add = "/stat";

    char cur_tty[PATH_MAX];
    char p_path[PATH_MAX];
    char p_tty[PATH_MAX];

    get_cur_tty(start, cur_tty);
    DIR* dir = opendir(start);
    if (dir) {
        struct dirent* ent;
        while ((ent = readdir(dir)) != NULL) {
            p_tty[0] = '\0';
            p_path[0] = '\0';

            char* dest = ent->d_name;
            int num = 0;
            for (int i = 0; i < strlen(dest); ++i) {
                if ((int) (dest[i]) - '0' > 9 || (int) (dest[i]) - '0' < 0) {
                    num = -1;
                    break;
                }
                num = num * 10 + ((int) dest[i] - '0');
            }

            if (num == -1) {
                continue;
            }

            size_t len_start = strlen(start);
            size_t len_file = strlen(dest);
            char* new_dest = (char*) malloc((len_start + len_file + 11) * sizeof(char));

            for (int k = 0; k < len_start; ++k) {
                new_dest[k] = start[k];
            }
            for (int k = 0; k < len_file; ++k) {
                new_dest[k + len_start] = dest[k];
            }
            for (int k = 0; k < 5; ++k) {
                new_dest[k + len_file + len_start] = add[k];
            }

            FILE* stats = fopen(new_dest, "r");
            if (!stat) {
                printf("%d\t ?\t 0:00:00:00\t ?\n", num);
            } else {
                int pid = num;
                char comm[NAME_MAX];
                long long utime = 0;
                long long stime = 0;
                long long tty = 0;
                char trash[3 * NAME_MAX];
                for (int i = 0; i < 14; ++i) {
                    if (i == 0) {
                        fscanf(stats, "%d", &pid);
                    } else if (i == 1) {
                        fscanf(stats, "%s", comm);
                    } else if (i == 6) {
                        fscanf(stats, "%lld", &tty);
                    } else if (i == 13) {
                        fscanf(stats, "%lld %lld", &utime, &stime);
                    } else {
                        fscanf(stats, "%s", trash);
                    }
                }

                comm[strlen(comm) - 1] = '\0';
                long long days = (utime + stime) / 3600 / 24 / sysconf(_SC_CLK_TCK);
                long long hours = ((utime + stime) / 3600 / sysconf(_SC_CLK_TCK)) % 24;
                long long minutes = ((utime + stime) / 60 / sysconf(_SC_CLK_TCK)) % 60;
                long long seconds = ((utime + stime) / sysconf(_SC_CLK_TCK)) % 60;

                struct stat session;
                stat(new_dest, &session);

                if (session.st_uid != getuid() && !all) {
                    fclose(stats);
                    free(new_dest);
                    continue;
                }

                p_tty[0] = '\0';
                strcpy(p_path, start);
                strcat(p_path, dest);
                get_tty(p_path, p_tty);
                p_tty[strlen(p_tty) - (strlen(p_tty) - strlen(cur_tty))] = '\0';

                if (strcmp(cur_tty, p_tty) != 0 && !all) {
                    fclose(stats);
                    free(new_dest);
                    continue;
                }

                char ttt[PATH_MAX];
                int ret = get_tty_(major((unsigned long long) tty), ttt);

                if (major((unsigned long long) tty) == 0) {
                    printf("%d\t ?\t %lld:%lld%lld:%lld%lld:%lld%lld\t %s\n", pid, days, hours / 10,
                           hours % 10, minutes / 10, minutes % 10, seconds / 10, seconds % 10, comm + 1);
                } else {
                    if (ret) {
                        printf("%d\t %s/%d\t %lld:%lld%lld:%lld%lld:%lld%lld\t %s\n", pid, ttt,
                               minor((unsigned long long) tty), days, hours / 10,
                               hours % 10,
                               minutes / 10, minutes % 10, seconds / 10, seconds % 10, comm + 1);
                    } else {
                        printf("%d\t %s\t %lld:%lld%lld:%lld%lld:%lld%lld\t %s\n", pid, p_tty, days, hours / 10,
                               hours % 10,
                               minutes / 10, minutes % 10, seconds / 10, seconds % 10, comm + 1);
                    }
                }
                fclose(stats);
            }
            free(new_dest);
        }
        closedir(dir);
    } else {
        printf("Unknown error.");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int main(int argc, char** argv) {
    int c;
    int all = 0;
    int help = 0;
    int version = 0;

    while ((c = getopt_long(argc, argv, "hveA", long_opts, NULL)) != -1) {
        switch (c) {
            case 'A':
            case 'e':
                all = 1;
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

    int status = process(all);

    exit(status ? EXIT_SUCCESS : EXIT_FAILURE);
}