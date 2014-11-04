#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <io.h>
#include <windows.h>
#include <time.h>

#undef WRITE_IMPORT_SYMBOLS


#define DEF_FILE "exports.def"
#define SYMBOL_FILE "symbols.dmp"
#define EXPORT_TOKEN "External"
#define IMPORT_TOKEN "Static"
#define PUBLIC_ACCESS "(public:"
#define PRIVATE_ACCESS "(private:"
#define EXPORT_ATTRIBUTE_ONE "SECT"
#define EXPORT_ATTRIBUTE_TWO "External"

/* default to 256K read chunks */
#define READ_BUFFER_SIZE 262144
#define PRINT_BUFFER_SIZE 2048

/* if there is less than RELOAD_SIZE bytes, then reload */
#define RELOAD_SIZE 512

static char file_buffer[READ_BUFFER_SIZE];
static char symbol_buffer[PRINT_BUFFER_SIZE];
static char description_buffer[PRINT_BUFFER_SIZE];
static char *eol_ptr;

/* some marker stuff */
static unsigned int symbol_file_bytes;
static unsigned int symbol_file_bytes_read;
static char *buffer_ptr;

static char *buffer_end;
static FILE *symbol_file;
static FILE *exports_file;
static char *libname;
static char symbolfile[256];
static char defname[256];
static int debug = 0;
static int ordinal, duplicates, vdd;
static unsigned int fileSize, bytesProcessed;
static int fileHandle;
static float percentProcessed;

static int quiet = 0;

void PrintDEFHeader(FILE *exports_file, char *libname);
char *ValidSymbol(char *symbol, char *descriptor);


int main(int argc, char **argv){
	unsigned int bytes_read, bytes_requested;
    char *posn, *symbol_ptr;
	SYSTEMTIME stStart, stEnd;
	long time1;
	long seconds, minutes, hours;

    GetSystemTime(&stStart);

	if (argc < 2){
		fprintf(stderr, "usage:\n\tstubber <library name> [-quiet]\n");
		exit(0);
	}else
	if (argc == 3 && !strcmp(argv[2], "-quiet")){
		quiet = 1;
	}else{
		fprintf(stderr, "usage:\n\tstubber <library name> [-quiet]\n");
		exit(0);
	}

    /* get the name of the library to export */
    libname = argv[1];
    sprintf(symbolfile, "%s.dmp", libname);
    sprintf(defname, "%s.def", libname);

	symbol_file = fopen(symbolfile, "r");
    if (symbol_file == 0)
    {
		fprintf(stderr, "Warning:  file '%s' not found\n", symbolfile);
		PrintDEFHeader(exports_file, libname);
        exit(0);
    }

	exports_file = fopen(defname, "w");
    if (exports_file == 0){
		fprintf(stderr, "File '%s' could not be opened\n", defname);
        exit(0);
    }


	GetSystemTime(&stStart);
	fileHandle = _fileno(symbol_file);
	fileSize = _filelength(fileHandle);
	bytesProcessed = 0;
	ordinal = 256;
	duplicates = 0;
	vdd = 0;

        PrintDEFHeader(exports_file, libname);

        /* Build up the list of exports */
        fprintf(exports_file, "EXPORTS\n");
        buffer_ptr = &file_buffer[0];
        while (!feof(symbol_file))
        {
                /* printf("%d\n", buffer_ptr - file_buffer); */
                /* bytes_requested = &file_buffer[READ_BUFFER_SIZE - 1] - buffer_ptr; */
                bytes_requested = READ_BUFFER_SIZE - (buffer_ptr - file_buffer + 1);
                assert(buffer_ptr + bytes_requested == &file_buffer[READ_BUFFER_SIZE - 1]);

                if (debug)
                        printf("buffer_ptr = file_buffer + %d\n", buffer_ptr - &file_buffer[0]);

                bytes_read = fread(buffer_ptr, 1, bytes_requested, symbol_file);
    bytesProcessed += bytes_read;
    percentProcessed = ((float) bytesProcessed) / ((float) fileSize) * 100.0;

	if(!quiet){
		printf("%.2f%c processed\r", percentProcessed, '%');
	}

                if (debug)
                        printf("read %d bytes (+ %d = %d bytes)\n", bytes_read, buffer_ptr - &file_buffer[0],
                                bytes_read + (buffer_ptr - &file_buffer[0]));

                if (bytes_read == bytes_requested)
                {
                        buffer_end = buffer_ptr + bytes_read + 1;
                        assert(buffer_end == &file_buffer[READ_BUFFER_SIZE]);
                        *buffer_end = '\0';

                        buffer_ptr = &file_buffer[0];

                        if (debug)
                        {
                                int len = strlen(buffer_ptr);
                                assert(len == READ_BUFFER_SIZE - 1);
                        }

                        /* keep looping through until our buffer size gets too small...then
                         * swallow another 64k of the file
                         */
                        while ((buffer_end - buffer_ptr) > RELOAD_SIZE)
                        {
                                /* find the EXPORT_ATTRIBUTE identifier */
                                if (posn = strstr(buffer_ptr, EXPORT_ATTRIBUTE_ONE))
                                        buffer_ptr = posn + 4;
                                else
                                        break;

                                /* get to the end of the line */
                                eol_ptr = strstr(buffer_ptr, "\n");

                                /* search for the other attribute...must be on the same line */
                                posn = strstr(buffer_ptr, EXPORT_ATTRIBUTE_TWO);

                                if ((posn == 0) || (posn > eol_ptr))
                                        continue;

                                /* skip over the column format */
                                buffer_ptr = strstr(buffer_ptr, "| ");
                                buffer_ptr += 2;

                                /* get our exported symbol */
                                sscanf(buffer_ptr, "%s", &symbol_buffer[0]);

                                /* skip over this symbol */
                                buffer_ptr += strlen(&symbol_buffer[0]);

                                if ((eol_ptr == 0) || (eol_ptr == buffer_ptr))
                                        description_buffer[0] = '\0';
                                else
                                {
                                        memcpy(&description_buffer[0], buffer_ptr, eol_ptr - buffer_ptr);
                                        /* null terminate */
                                        description_buffer[eol_ptr - buffer_ptr + 1] = '\0';
                                }


                                /* check to see if it's something we want to export */
                                symbol_ptr = ValidSymbol(symbol_buffer, description_buffer);

                                if (symbol_ptr != 0/* && strncmp(symbol_ptr, "?empty", 6) */&& strncmp(symbol_ptr, "_real", 5))
                                {
                                        /* ok, go ahead and export this baby */
                                        if(!strncmp(symbol_ptr, "init_", 5))
                                            fprintf(exports_file, " %s @ %d\n", symbol_ptr, ordinal++);
                                        else
                                            fprintf(exports_file, " %s @ %d NONAME\n", symbol_ptr, ordinal++);
                                }
                        }

                        /* move the end stuff over to the front...don't copy over the
                         * null terminator, because we're concatenating another read
                         * onto the end of what we have left.  This char copy is
                         * overlap-safe.  At the end of this copy, buffer_ptr will
                         * point to the end, which is where we want to concantenate to
                         * remember, the string is null-terminated.
                         */

                        /* Start copying from buffer_ptr...store this off.  minus 1
                         * because buffer_end points to the null terminator
                         */
                        if (debug)
                        {
                                int len = strlen(buffer_ptr);
                                printf("moving %d chars to beginning\n", len);
                                assert((buffer_end - buffer_ptr - 1) == len);
                        }
                        buffer_end = buffer_ptr;
                        buffer_ptr = &file_buffer[0];

                        strcpy(buffer_ptr, buffer_end);


                        buffer_ptr += strlen(&file_buffer[0]);
/*
                        buffer_end = buffer_ptr;
                        buffer_ptr = &file_buffer[0];
                        while (*buffer_ptr++ = *buffer_end++);

                        if (*buffer_ptr == '\0')
                                buffer_ptr--;
*/
                }
                else
                {
                        assert(bytes_read < bytes_requested);
                        /* we've read in less bytes, so this should be the last... */
                        /* Need to readjust the end pointer, since we shifted everything
                         * back to the start of our array buffer
                         */

                        /* should be +1, but in the debugger, I'm seeing something
                         * else being read in, maybe the EOF token?
                         */
                        buffer_end = buffer_ptr + bytes_read;
                        *buffer_end = '\0';
                        buffer_ptr = &file_buffer[0];

                        /* Now we follow a similar loop, but this one we can break
                         * out of because this will be the last chunck < RELOAD_SIZE
                         */

                        /* find the EXPORT_ATTRIBUTE identifier */
                        while ((buffer_ptr = strstr(buffer_ptr, EXPORT_ATTRIBUTE_ONE)) != 0)
                        {
                                buffer_ptr += 4;
                                /* get to the end of the line */
                                eol_ptr = strstr(buffer_ptr, "\n");

                                /* search for the other attribute...must be on the same line */
                                posn = strstr(buffer_ptr, EXPORT_ATTRIBUTE_TWO);

                                if ((posn == 0) || (posn > eol_ptr))
                                        continue;

                                /* skip over the column format */
                                buffer_ptr = strstr(buffer_ptr, "| ");
                                buffer_ptr += 2;

                                /* get our exported symbol */
                                sscanf(buffer_ptr, "%s", &symbol_buffer[0]);

                                /* skip over this symbol */
                                buffer_ptr += strlen(&symbol_buffer[0]);

                                /* get to the end of the line */
                                eol_ptr = strstr(buffer_ptr, "\n");

                                if ((eol_ptr == 0) || (eol_ptr == buffer_ptr))
                                        description_buffer[0] = '\0';
                                else
                                {
                                        memcpy(&description_buffer[0], buffer_ptr, eol_ptr - buffer_ptr);
                                        /* null terminate */
                                        description_buffer[eol_ptr - buffer_ptr + 1] = '\0';
                                }

                                /* check to see if it's something we want to export */
                                symbol_ptr = ValidSymbol(symbol_buffer, description_buffer);

                                if (symbol_ptr != 0 /*&& strncmp(symbol_ptr, "?empty", 6)*/&& strncmp(symbol_ptr, "_real", 5))
                                {
                                        /* ok, go ahead and export this baby */
                                        if(!strncmp(symbol_ptr, "init_", 5))
                                            fprintf(exports_file, " %s @ %d\n", symbol_ptr, ordinal++);
                                        else
                                            fprintf(exports_file, " %s @ %d NONAME\n", symbol_ptr, ordinal++);
                                }
                        }
                }
        }

#ifdef WRITE_IMPORT_SYMBOLS
        fprintf(exports_file, "IMPORTS\n");
        /* Build up the list of imports */
        buffer_ptr = &file_buffer[0];
        rewind(symbol_file);
        while (!feof(symbol_file))
        {
                bytes_requested = &file_buffer[READ_BUFFER_SIZE - 1] - buffer_ptr;
                bytes_read = fread(buffer_ptr, 1, bytes_requested, symbol_file);

                if (bytes_read == bytes_requested)
                {
                        buffer_end = buffer_ptr + bytes_read + 1;
                        *buffer_end = '\0';

                        /* keep looping through until our buffer size gets too small...then
                         * swallow another 64k of the file
                         */
                        while ((buffer_end - buffer_ptr) > RELOAD_SIZE)
                        {
                                /* find the IMPORT_TOKEN identifier */
                                if (posn = strstr(buffer_ptr, IMPORT_TOKEN))
                                        buffer_ptr = posn;
                                else
                                        break;

                                /* skip over the column format...slightly smaller for IMPORT_TOKEN */
                                buffer_ptr = strstr(buffer_ptr, "| ");
                                buffer_ptr += 2;

                                /* get our exported symbol */
                                sscanf(buffer_ptr, "%s", &symbol_buffer[0]);

                                /* skip over this symbol */
                                buffer_ptr += strlen(&symbol_buffer[0]);

                                /* get to the end of the line */
                                eol_ptr = strstr(buffer_ptr, "\n");

                                if ((eol_ptr == 0) || (eol_ptr == buffer_ptr))
                                        description_buffer[0] = '\0';
                                else
                                {
                                        memcpy(&description_buffer[0], buffer_ptr, eol_ptr - buffer_ptr);
                                        /* null terminate */
                                        description_buffer[eol_ptr - buffer_ptr + 1] = '\0';
                                }

                                /* check to see if it's something we want to import */
                                symbol_ptr = ValidSymbol(symbol_buffer, description_buffer);

                                if (symbol_ptr != 0)
                                {
                                        /* ok, go ahead and import this baby */
                                        fprintf(exports_file, " %s @ %d NONAME\n", symbol_ptr, ordinal++);
                                }
                        }

                        /* move the end stuff over to the front...don't copy over the
                         * null terminator, because we're concatenating another read
                         * onto the end of what we have left.  This char copy is
                         * overlap-safe.  At the end of this copy, buffer_ptr will
                         * point to the end, which is where we want to concantenate to
                         * remember, the string is null-terminated.
                         */

                        /* start copying from buffer_ptr...store this off */
                        buffer_end = buffer_ptr;
                        buffer_ptr = &file_buffer[0];
                        while (*buffer_ptr++ = *buffer_end++);
                }
                else
                {
                        assert(bytes_read < bytes_requested);
                        /* we've read in less bytes, so this should be the last... */
                        /* Need to readjust the end pointer, since we shifted everything
                         * back to the start of our array buffer
                         */
                        buffer_end = buffer_ptr + bytes_read + 1;
                        *buffer_end = '\0';
                        buffer_ptr = &file_buffer[0];

                        /* Now we follow a similar loop, but this one we can break
                         * out of because this will be the last chunck < RELOAD_SIZE
                         */

                        /* find the EXPORT_TOKEN identifier */
                        while ((buffer_ptr = strstr(buffer_ptr, IMPORT_TOKEN)) != 0)
                        {
                                /* skip over the column format...slightly smaller for IMPORT_TOKEN */
                                buffer_ptr = strstr(buffer_ptr, "| ");
                                buffer_ptr += 2;

                                /* get our exported symbol */
                                sscanf(buffer_ptr, "%s", &symbol_buffer[0]);

                                /* skip over this symbol */
                                buffer_ptr += strlen(&symbol_buffer[0]);

                                /* get to the end of the line */
                                eol_ptr = strstr(buffer_ptr, "\n");

                                if ((eol_ptr == 0) || (eol_ptr == buffer_ptr))
                                        description_buffer[0] = '\0';
                                else
                                {
                                        memcpy(&description_buffer[0], buffer_ptr, eol_ptr - buffer_ptr);
                                        /* null terminate */
                                        description_buffer[eol_ptr - buffer_ptr + 1] = '\0';
                                }

                                /* check to see if it's something we want to import */
                                symbol_ptr = ValidSymbol(symbol_buffer, description_buffer);

                                if (symbol_ptr != 0)
                                {
                                        /* ok, go ahead and import this baby */
                                        fprintf(exports_file, " %s @ %d NONAME\n", symbol_ptr, ordinal++);
                                }
                        }
                }
        }
#endif

        _fcloseall();

	if(!quiet){
	  printf("\n\nSummary Statistics\n");
	  printf("\t%d symbols exported\n", ordinal - 256);
	  printf("\t%d duplicate symbols removed\n", duplicates);
	  printf("\t%d scalar/vector deleting destructors removed\n", vdd);
	}

    GetSystemTime(&stEnd);

  if ((&stEnd)->wHour < (&stStart)->wHour)
  {
    /* flip the clock */
    time1 = (12 - (&stStart)->wHour + (&stEnd)->wHour) * (60 * 60 * 1000);
  }
  else
    time1 = ((&stEnd)->wHour - (&stStart)->wHour) * (60 * 60 * 1000);

        time1 += (((&stEnd)->wMinute * 60 * 1000) +
                        ((&stEnd)->wSecond * 1000) +
                        (&stEnd)->wMilliseconds) -

                        (((&stStart)->wMinute * 60 * 1000) +
                        ((&stStart)->wSecond * 1000) +
                        (&stStart)->wMilliseconds);

  hours = time1 / (60 * 60 * 1000);
  time1 = time1 % (60 * 60 * 1000);

  minutes = time1 / (60 * 1000);
  time1 = time1 % (60 * 1000);

  seconds = time1 / 1000;
  if(!quiet){
	printf("Run Time:  %d hours, %d minutes, %d seconds\n", hours, minutes, seconds);
  }

  return 0;
}




void PrintDEFHeader(FILE *exports_file, char *libname)
{
        static char *suif_header[] =
        {
                ";",
                ";      Copyright (c) 1996, 1997 Stanford University",
                ";",
                ";      All rights reserved.",
                ";",
                ";      This software is provided under the terms described in",
                ";      the \"suif_copyright.h\" include file.",
                ";"
        };

        static char *description = "your description goes here";
        int i;

        for (i = 0; i < 8; i++)
                fprintf(exports_file, "%s\n", suif_header[i]);

        fprintf(exports_file, "\nLIBRARY %s\nDESCRIPTION \"%s\"\n\n",
                libname, description);
}



char *ValidSymbol(char *symbol, char *descriptor)
{
        static char symbol_array[70000][512];

  /* don't export "vector deleting destructor" and scalar...*/

  if (strstr(descriptor, "deleting destructor") != 0)
  {
    if (debug)
      printf("found vdd\n");
    vdd++;
    return 0;
  }

  /* appears that we can get rid of this one also, since it doesn't look right */
  if (strstr(symbol, "??_C@") == symbol)
    return 0;

        if ((symbol[0] == '?') || (symbol[0] == '_'))
        {
                int i;

    if (symbol[0] == '_')
      symbol++;

                /* this loop will eliminate duplicates */
                for (i = 0; i < (ordinal - 256); i++)
                {
                        if (strcmp(symbol, symbol_array[i]) == 0)
                        {
        if (debug)
                                printf("duplicate symbol\n");
        duplicates++;
                                return 0;
                        }
                }
                strcpy(symbol_array[ordinal - 256], symbol);

                return symbol;
        }
        return 0;
}