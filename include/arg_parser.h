#ifndef ARGPARSER_H
#define ARGPARSER_H

struct arguments
{
    char *command;
};

void parse_arguments(int argc, char **argv, struct arguments *arguments);

#endif