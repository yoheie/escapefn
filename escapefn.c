/*
 * Copyright (c) 2016-2017 Yohei Endo <yoheie@gmail.com>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 *    not claim that you wrote the original software. If you use this
 *    software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must
 *    not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 */
#include <limits.h>
#include <stdio.h>
#include <string.h>

enum {
	ESCAPEFN_ERROR_OK,
	ESCAPEFN_ERROR_OPTION,
	ESCAPEFN_ERROR_OPEN,
	ESCAPEFN_ERROR_READ,
	ESCAPEFN_ERROR_WRITE
};

static int escape(FILE *pfin, FILE *pfout);
static int unescape(FILE *pfin, FILE *pfout);

static const char usage_str[] =
	"Usage: escapefn [options] [file ...]\n"
	"  Read NUL-separated raw filename list from files (or stdin), and\n"
	"  convert to LF-separated escaped filename list.\n"
	"\n"
	"Options:\n"
	"  -m, --minimum     escape LF and backslash only (default)\n"
	"  -c, --cstyle      escape all C0 control chars and DEL in C style\n"
	"  -o, --octal       escape all C0 control chars and DEL in octal\n"
	"  -u, --unescape    unescape\n"
	"  -h, --help        show this help\n"
	;

static enum {
	ESCAPE_MINIMUM,
	ESCAPE_CSTYLE,
	ESCAPE_OCTAL
} escape_opt = ESCAPE_MINIMUM;

int main(int argc, char *argv[])
{
	int r = ESCAPEFN_ERROR_OK, r1;
	int i, j;
	int e = 0;
	int h = 0;
	int u = 0;
	FILE *pfin = NULL;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			/* argv[i] is filename */
			break;
		}
		if (argv[i][1] == '\0') {
			/* argv[i] is "-" (stdin) */
			break;
		}
		if (argv[i][1] == '-') {
			/* argv[i] is long option */
			if (argv[i][2] == '\0') {
				i++;
				break;
			}
			else if (strcmp(&argv[i][2], "minimum") == 0) {
				escape_opt = ESCAPE_MINIMUM;
			}
			else if (strcmp(&argv[i][2], "cstyle") == 0) {
				escape_opt = ESCAPE_CSTYLE;
			}
			else if (strcmp(&argv[i][2], "octal") == 0) {
				escape_opt = ESCAPE_OCTAL;
			}
			else if (strcmp(&argv[i][2], "unescape") == 0) {
				u = 1;
			}
			else if (strcmp(&argv[i][2], "help") == 0) {
				h = 1;
			}
			else {
				e = 1;
			}
		}
		else {
			/* argv[i] is short option(s) */
			for (j = 1; argv[i][j] != '\0'; j++) {
				if (argv[i][j] == 'm') {
					escape_opt = ESCAPE_MINIMUM;
				}
				else if (argv[i][j] == 'c') {
					escape_opt = ESCAPE_CSTYLE;
				}
				else if (argv[i][j] == 'o') {
					escape_opt = ESCAPE_OCTAL;
				}
				else if (argv[i][j] == 'u') {
					u = 1;
				}
				else if (argv[i][j] == 'h') {
					h = 1;
				}
				else {
					e = 1;
					break;
				}
			}
		}
		if (e) {
			fprintf(stderr, "Error: invalid option\n");
			fprintf(stderr, "%s", usage_str);
			return ESCAPEFN_ERROR_OPTION;
		}
	}

	if (h) {
		printf("%s", usage_str);
		return ESCAPEFN_ERROR_OK;
	}

	if (i >= argc) {
		r = (u ? unescape : escape)(stdin, stdout);
	}
	else {
		for (; i < argc; i++) {
			if (strcmp(argv[i], "-") == 0) {
				r1 = (u ? unescape : escape)(stdin, stdout);
			}
			else {
				if (pfin = fopen(argv[i], "rb")) {
					r1 = (u ? unescape : escape)(pfin, stdout);
					fclose(pfin);
				}
				else {
					fprintf(stderr, "Error: cannot open file %s\n", argv[i]);
					r1 = ESCAPEFN_ERROR_OPEN;
				}
			}
			if (r1 != ESCAPEFN_ERROR_OK) {
				if (r == ESCAPEFN_ERROR_OK) {
					r = r1;
				}
			}
		}
	}

	return r;
}

static int escape(FILE *pfin, FILE *pfout)
{
	int r = ESCAPEFN_ERROR_OK;
	int c;
	int o[3];
	int i;
	const char abtnvfr[7] = "abtnvfr";

	while ((c = getc(pfin)) != EOF) {
		if (c == '\0') {
			if (putc('\n', pfout) == EOF) break;
		}
		else if (c == '\\') {
			if (putc('\\', pfout) == EOF) break;
			if (putc('\\', pfout) == EOF) break;
		}
		else if ((escape_opt != ESCAPE_OCTAL)
		      && (c == '\n')
		) {
			if (putc('\\', pfout) == EOF) break;
			if (putc('n', pfout) == EOF) break;
		}
		else if ((escape_opt == ESCAPE_CSTYLE)
		      && ((007 <= c) && (c <= 015))
		) {
			if (putc('\\', pfout) == EOF) break;
			if (putc(abtnvfr[c - 007], pfout) == EOF) break;
		}
		else if ((escape_opt != ESCAPE_MINIMUM)
		      && (((001 <= c) && (c <= 037)) || (c == 0177))
		) {
			if (putc('\\', pfout) == EOF) break;
			for (i = 0; i < 3; i++) {
				o[2 - i] = '0' + c % 8;
				c /= 8;
			}
			for (i = 0; i < 3; i++) {
				if (putc(o[i], pfout) == EOF) break;
			}
			if (i < 3) {
				break;
			}
		}
		else {
			if (putc(c, pfout) == EOF) break;
		}
	}
	if(ferror(pfin)) {
		fprintf(stderr, "Error: read error\n");
		r = ESCAPEFN_ERROR_READ;
	}
	if(ferror(pfout)) {
		fprintf(stderr, "Error: write error\n");
		r = ESCAPEFN_ERROR_WRITE;
	}

	return r;
}

static int unescape(FILE *pfin, FILE *pfout)
{
	int r = ESCAPEFN_ERROR_OK;
	int c;
	int t;
	int i;

	while ((c = getc(pfin)) != EOF) {
		if (c == '\n') {
			if (putc('\0', pfout) == EOF) break;
		}
		else if (c == '\\') {
			if ((c = getc(pfin)) == EOF) {
				fprintf(stderr, "Warning: invalid escape sequence\n");
				if (putc('\\', pfout) == EOF) break;
				break;
			}
			/* Backslash */
			if (c == '\\') {
				if (putc('\\', pfout) == EOF) break;
			}
			/* C style (control characters) */
			else if (c == 'n') {
				if (putc('\n', pfout) == EOF) break;
			}
			else if (c == 't') {
				if (putc('\t', pfout) == EOF) break;
			}
			else if (c == 'v') {
				if (putc('\v', pfout) == EOF) break;
			}
			else if (c == 'b') {
				if (putc('\b', pfout) == EOF) break;
			}
			else if (c == 'r') {
				if (putc('\r', pfout) == EOF) break;
			}
			else if (c == 'f') {
				if (putc('\f', pfout) == EOF) break;
			}
			else if (c == 'a') {
				if (putc('\a', pfout) == EOF) break;
			}
			/* C style (graphic characters) */
			else if (c == '\?') {
				if (putc('\?', pfout) == EOF) break;
			}
			else if (c == '\'') {
				if (putc('\'', pfout) == EOF) break;
			}
			else if (c == '\"') {
				if (putc('\"', pfout) == EOF) break;
			}
			/* From GNU coreutils ls -b */
			else if (c == ' ') {
				if (putc(' ', pfout) == EOF) break;
			}
			else if (c == '=') {
				if (putc('=', pfout) == EOF) break;
			}
			else if (c == '>') {
				if (putc('>', pfout) == EOF) break;
			}
			else if (c == '@') {
				if (putc('@', pfout) == EOF) break;
			}
			else if (c == '|') {
				if (putc('|', pfout) == EOF) break;
			}
			/* Octal */
			else if (('0' <= c) && (c <= '9')) {
				t = c - '0';
				for (i = 1; i < 3; i++) {
					if ((c = getc(pfin)) == EOF) {
						break;
					}
					if ((c < '0') || ('9' < c)) {
						break;
					}
					t = (t * 8) + (c - '0');
					if (t > UCHAR_MAX) {
						fprintf(stderr, "Warning: invalid escape sequence\n");
						t /= 8;
						break;
					}
				}
				if (putc(t, pfout) == EOF) break;
				if (i < 3) {
					if (c == EOF) break;
					if (ungetc(c, pfin) == EOF) break;
				}
			}
			else {
				fprintf(stderr, "Warning: invalid escape sequence\n");
				if (putc('\\', pfout) == EOF) break;
				if (putc(c, pfout) == EOF) break;
			}
		}
		else {
			if (putc(c, pfout) == EOF) break;
		}
	}
	if(ferror(pfin)) {
		fprintf(stderr, "Error: read error\n");
		r = ESCAPEFN_ERROR_READ;
	}
	if(ferror(pfout)) {
		fprintf(stderr, "Error: write error\n");
		r = ESCAPEFN_ERROR_WRITE;
	}

	return r;
}
