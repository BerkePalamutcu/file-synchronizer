#include "arg_parser.h"
#include <argp.h>
#include <string.h>

/* Program documentation. */
static char doc[] = "File Synchronizer -- A program to synchronize files and folders";

/* A description of the arguments we accept. */
static char args_doc[] = "COMMAND";

/* The options we understand. */
static struct argp_option options[] = {
    {0}};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch (key)
    {
    case ARGP_KEY_ARG:
        if (state->arg_num >= 1)
        {
            /* Too many arguments. */
            argp_usage(state);
        }
        arguments->command = arg;
        break;

    case ARGP_KEY_END:
        if (state->arg_num < 1)
        {
            /* Not enough arguments. */
            argp_usage(state);
        }
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc};

void parse_arguments(int argc, char **argv, struct arguments *arguments)
{
    /* Default values. */
    arguments->command = NULL;

    /* Parse our arguments; every option seen by parse_opt will be reflected in arguments. */
    argp_parse(&argp, argc, argv, 0, 0, arguments);
}
