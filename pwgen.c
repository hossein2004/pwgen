/*
 * pwgen.c --- generate secure passwords
 *
 * Copyright (C) 2001 by Theodore Ts'o
 * 
 * This file may be distributed under the terms of the GNU Public
 * License.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "pwgen.h"

struct pwgen_func generators[] = {
	{ "phonemes",	pw_phonemes,	0 },
	{ "rand",	pw_rand,	0 },
	{ 0,		0,		0 }
};

/* Program parameters set via getopt */

int	pw_length = 8;
int	num_pw = -1;
int	case_flag = 0;
int	numeric_flag = 0;
int	do_columns = 0;

#ifdef HAVE_GETOPT_LONG
struct option pwgen_options[] = {
	{ "alt-phonics", no_argument, 0, 'a' },
	{ "capitalize", no_argument, 0, 'c' },
	{ "numerals", no_argument, 0, 'n'},
	{ "num-passwords", required_argument, 0, 'N'},
	{ "secure", no_argument, 0, 's' },
	{ "help", no_argument, 0, 'h'},
	{ "no-numerals", no_argument, &numeric_flag, 0 },
	{ "no-capitalize", no_argument, &case_flag, 0 },
	{ 0, 0, 0, 0}
};
#endif

int main(int argc, char **argv)
{
	int	term_width = 80;
	int	c, i, num_cols;
	char	*buf, *tmp;
	void	(*pwgen)(char *buf, int size, int pw_flags);

	pwgen = pw_phonemes;
	if (isatty(1)) {
		do_columns = 1;
		case_flag = PW_ONE_CASE;
		numeric_flag = PW_ONE_NUMBER;
	}

	while (1) {
#ifdef HAVE_GETOPT_LONG
		c = getopt_long(argc, argv, "1aCcnN:sh", pwgen_options, 0);
#else
		c = getopt(argc, argv, "1aCcnN:sh");
#endif
		if (c == -1)
			break;
		switch (c) {
		case 'a':
			break;
		case 'c':
			case_flag = PW_ONE_CASE;
			break;
		case 'n':
			numeric_flag = PW_ONE_NUMBER;
			break;
		case 'N':
			num_pw = strtol(optarg, &tmp, 0);
			if (*tmp) {
				fprintf(stderr,
					"Invalid number of passwords: %s\n",
					optarg);
				exit(1);
			}
			break;
		case 's':
			pwgen = pw_rand;
			break;
		case 'C':
			do_columns = 1;
			break;
		case '1':
			do_columns = 0;
			break;
		}
	}
	if (optind < argc) {
		pw_length = strtol(argv[optind], &tmp, 0);
		if (*tmp) {
			fprintf(stderr, "Invalid password length: %s\n",
				argv[optind]);
			exit(1);
		}
		optind++;
	}

	if (optind < argc) {
		num_pw = strtol(argv[optind], &tmp, 0);
		if (*tmp) {
			fprintf(stderr, "Invalid number of passwords: %s\n",
				argv[optind]);
			exit(1);
		}
	}
	
	if (do_columns) 
		num_cols = term_width / (pw_length+1);
	if (num_pw < 0)
		num_pw = do_columns ? num_cols * 20 : 1;
	
	buf = malloc(pw_length+1);
	if (!buf) {
		fprintf(stderr, "Couldn't malloc password buffer.\n");
		exit(1);
	}
	for (i=0; i < num_pw; i++) {
		pwgen(buf, pw_length, case_flag | numeric_flag);
		if (!do_columns || ((i % num_cols) == (num_cols-1)))
			printf("%s\n", buf);
		else
			printf("%s ", buf);
	}
	if ((i % num_cols) != 0 && (num_cols > 1))
		fputc('\n', stdout);
	free(buf);
}
