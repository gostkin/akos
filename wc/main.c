#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct opt_params {
    int lines;
    int words;
    int chars;
    int bytes;
    int max_line_length;
};

static struct option const long_opts[] = {
        {"lines",           no_argument, NULL, 'l'},
        {"words",           no_argument, NULL, 'w'},
        {"chars",           no_argument, NULL, 'm'},
        {"max-line-length", no_argument, NULL, 'L'},
        {"help",            no_argument, NULL, 'h'},
        {"version",         no_argument, NULL, 'v'},
        {NULL, 0,                        NULL, 0}
};

void usage(int status) {
    printf("Usage: wc [OPTION]... [FILE]...\n");
    fputs("or:  wc [OPTION]... --files0-from=F\n", stdout);
    fputs("Print newline, word, and byte counts for each FILE, and a total line if", stdout);
    fputs("more than one FILE is specified.  A word is a non-zero-length sequence of", stdout);
    fputs("characters delimited by white space.\n\n", stdout);
    fputs("With no FILE, or when FILE is -, read standard input.\n\n", stdout);
    fputs("The options below may be used to select which counts are printed, always in", stdout);
    fputs("the following order: newline, word, character, byte, maximum line length.\n", stdout);
    fputs("\t-m, --chars            print the character counts\n", stdout);
    fputs("\t-l, --lines            print the newline counts\n", stdout);
    fputs("\tIf F is - then read names from standard input\n", stdout);
    fputs("\t-L, --max-line-length  print the maximum display width\n", stdout);
    fputs("\t-w, --words            print the word counts\n", stdout);
    fputs("\t--help     display this help and exit\n", stdout);
    fputs("\t--version  output version information and exit\n", stdout);

    fputs("wc 0.1\n", stdout);

    exit(status);
}

void _version(int status) {
    printf("wc 0.1\n");
    printf("Copyright (C) 2017 Eugeny Gostkin\n");
    printf("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
    printf("This is free software: you are free to change and redistribute it.\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\n\n");
    printf("Written by Eugeny Gostkin.\n");

    exit(status);
}

void default_params(struct opt_params* params) {
    params->lines = 1;
    params->words = 1;
    params->chars = 1;
    params->max_line_length = 0;
}

int do_wc(int n_files, char** file, struct opt_params* params) {
    int need_freopen = 0;
    if (n_files == 0) {
        ++n_files;
        need_freopen = 1;
    }
    int total_lines_count = 0;
    int total_words_count = 0;
    int total_chars_count = 0;
    int total_max_line_length = 0;
    for (int i = 0; i < n_files; ++i) {
        int lines_count = 0;
        int words_count = 0;
        int chars_count = 0;
        int max_line_length = 0;
        int temp_line_length = 0;

        FILE* from;
        if (!need_freopen) {
            from = fopen(file[i], "r");
        } else {
            from = stdin;
        }
        if (!from) {
            return EXIT_FAILURE;
        }

        int was_space = 1;
        char str[80];
        while (fgets(str, sizeof(str), from) != NULL) {
            for (int j = 0; j < strlen(str); ++j) {
                ++chars_count;
                if (str[j] == '\n') {
                    ++lines_count;

                    if (temp_line_length > max_line_length)
                        max_line_length = temp_line_length;
                    temp_line_length = 0;
                    was_space = 1;
                } else {
                    ++temp_line_length;
                }
                if (was_space && !isspace(str[j])) {
                    ++words_count;
                    was_space = 0;
                }
                if (!was_space && isspace(str[j])) {
                    was_space = 1;
                }
            }
        }
        if (!need_freopen) {
            fclose(from);
        }
        if (params->lines) {
            printf("\t%d", lines_count);
        }
        if (params->words) {
            printf("\t%d", words_count);
        }
        if (params->chars) {
            printf("\t%d", chars_count);
        }
        if (params->max_line_length) {
            printf("\t%d", max_line_length);
        }
        printf("\t%s\n", file[i]);
        total_chars_count += chars_count;
        total_lines_count += lines_count;
        total_words_count += words_count;
        if (total_max_line_length < max_line_length)
            total_max_line_length = max_line_length;
    }
    if (n_files > 1) {
        if (params->lines) {
            printf("\t%d", total_lines_count);
        }
        if (params->words) {
            printf("\t%d", total_words_count);
        }
        if (params->chars) {
            printf("\t%d", total_chars_count);
        }
        if (params->max_line_length) {
            printf("\t%d", total_max_line_length);
        }
        printf("\t%s\n", "total");
    }
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    int c;
    int help = 0;
    int version = 0;

    struct opt_params* params = (struct opt_params*) malloc(sizeof(struct opt_params));
    int _default = 1;

    while ((c = getopt_long(argc, argv, "cmlLwhv:", long_opts, NULL)) != -1) {
        switch (c) {
            case 'c':
            case 'm':
                params->chars = 1;
                _default = 0;
                break;

            case 'w':
                params->words = 1;
                _default = 0;
                break;

            case 'L':
                params->max_line_length = 1;
                _default = 0;
                break;

            case 'l':
                params->lines = 1;
                _default = 0;
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

    if (_default) {
        default_params(params);
    }

    int status = do_wc(argc - optind, argv + optind, params);

    exit(status ? EXIT_SUCCESS : EXIT_FAILURE);
}