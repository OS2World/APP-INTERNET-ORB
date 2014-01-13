// orb - HTML preprocessor
// orb.cc - main program source
//
// Copyright (C) 1996-1998  Craig Berry
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
// USA.
//
// ---
// Usage:
//  See printHelp() function, below.


#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errrpt.h"
#include "orbstate.hpp"
#include "orbver.h"
#include "util.hpp"
#include "varstr.hpp"

// Local function prototypes.

void  loadEnv   ();
void  processOpt(int& n, int argc, char* argv[],
                 StringAssoc& dict);
void  genFiles  (VarStr& inName, char*  outBuf,
                 FILE*&  inFile, FILE*& outFile);
void  printHelp ();

// Functions.

int main(int argc, char* argv[])
{
  caseMode = CASEMODE;

  if (argc < 2 || strcmp(argv[1], "-b")) {
    perr(Always, "%s - Copyright (C) 1996-1998 by Craig Berry", ORB_ID);
  }

  if (argc < 2) {
    fprintf(stderr, "No arguments found.\n");
    fprintf(stderr, "Usage: orb -q -v -ofullname -xext -Cc -Dvar=val "
                    "-Ipath file...\n"
                    "       (-h for more info)\n");
    exit(1);
  }
  else {
    loadEnv();

    StringAssoc cmdLineVars;  // Storage for -D vars from cmd line.
    VarStr      inName;
    unsigned    fileCount = 0;
    FILE*       inFile;
    FILE*       outFile;

    for (int n = 1; n < argc; n++) {
      if (argv[n][0] == '-' && strlen(argv[n]) > 1) {
        processOpt(n, argc, argv, cmdLineVars);  // May increment n.
      }
      else {
        char outName[MAXPATH];

        inName.set(argv[n]);
        strFixCase(inName);

        genFiles(inName, outName, inFile, outFile);

        perr(Info, "Compiling %s to %s", (const char*) inName, outName);

        OrbState state(inFile, outFile, inName);

        state += cmdLineVars;
        state.value("base",   inName);
        state.value("target", outName);

        if (debugFlag) {
          puts("*** Parser contents just before compilation:");
          state.dump();
        }

        state.compile();

        if (debugFlag) {
          puts("*** Parser contents just after compilation:");
          state.dump();
        }

        if (inFile  != stdin)  fclose(inFile);
        if (outFile != stdout) fclose(outFile);
        fileCount++;
      }
    }

    if (debugFlag) {
      puts("Include file list after all compilations:");

      for (size_t x = 0; x < inclPathCount; x++) {
        printf("  %s\n", inclPath[x]);
      }
    }

    if (fileCount > 0) {
      perr(Detail, "Processed %d modules(s).", fileCount);
    }
    else {
      perr(Die, "No files processed.");
    }
  }

  return 0;   // Was EXIT_SUCCESS, but this caused probs on SunOS g++.
}

void loadEnv()
{
  const char* const incl = getenv("ORBPATH");

  if (incl) {
    const char* path;
    char*       buf = new char[strlen(incl) + 1];
    assert(buf != NULL);

    strcpy(buf, incl);

    for (unsigned toks = 0;
         (path = strtok(toks ? (char*) NULL : buf, ARGSEP)) != NULL;
         toks++) {
      if (inclPathCount >= maxInclPath) {
        perr(Die, "Cannot define more than %d include paths (including "
                  "those from ORBPATH environment variable).",
                  maxInclPath - 1);
      }

      strncpy(inclPath[inclPathCount++], path, MAXPATH);
    }

    delete [] buf;
  }
}

void processOpt(int& n, int argc, char* argv[], StringAssoc& dict)
{
  const char* const opt    = argv[n] + 1;
  const size_t      optLen = strlen(opt);
  const char        optc   = optLen > 0 ? opt[0] : ' ';

  if (debugFlag) printf("*** Opt /%s/\n", opt);

  if (optLen == 1 && optc == 'b') {
    // Ignore...'bashful' mode handled directly at top of main.
  }
  else if (optLen == 1 && optc == 'v') {
    setMinLevel(Detail);
  }
  else if (optLen == 1 && optc == 'q') {
    setMinLevel(Err);
    bashfulFlag = 1;
  }
  else if (optLen >= 1 && optLen <= maxExtUsed && optc == 'x') {
    strcpy(outExt, ".");
    strcat(outExt, opt + 1);
  }
  else if (optLen > 1 && optLen <= MAXPATH && optc == 'o') {
    strcpy(outPath, opt + 1);
  }
  else if (optLen == 2 && optc == 'C') {
    const char c = toupper(opt[1]);

    switch (c) {
      case 'L': caseMode = Lower; break;
      case 'U': caseMode = Upper; break;
      case 'M': caseMode = Mixed; break;
      default:
        perr(Die, "Opt -C must be followed by L, U, or M (lower, upper, "
                  "or mixed case).");
        break;
    }
  }
  else if (optLen >= 2 && optc == 'D') {
    VarStr buf(opt + 1);

    char* const eqp = strchr(buf, '=');

    if (eqp) {
      *eqp = '\0';
      dict.value(buf, eqp + 1);
    }
    else if (++n < argc) {
      dict.value(buf, argv[n]);
    }
    else {
      perr(Die, "-Dvar specified with no value argument following.");
    }
  }
  else if (optLen >= 2 && optc == 'I') {
    const char* path;
    char*       buf = new char[strlen(opt + 1) + 1];
    assert(buf != NULL);

    strcpy(buf, opt + 1);

    for (unsigned toks = 0;
         (path = strtok(toks ? (char*) NULL : buf, ARGSEP)) != NULL;
         toks++) {
      if (inclPathCount >= maxInclPath) {
        perr(Die, "Cannot define more than %d include paths (including "
                  "those from ORBPATH environment variable).",
                  maxInclPath - 1);
      }

      const size_t slot = 1 + inclPathCountCmd;

      memmove(inclPath[slot + 1], inclPath[slot],
              (inclPathCount - slot) * MAXPATH);
      
      strncpy(inclPath[slot], path, MAXPATH);
      ++inclPathCount;
      ++inclPathCountCmd;
    }

    delete [] buf;
  }
  else if (optc == 's') {
    lineSpliceFlag = 0;
  }
  else if (optc == 'h' || optc == '?') {
    printHelp();
    exit(0);
  }
  else if (optLen == 1 && optc == ';') {   // Hidden debug mode switch.
    debugFlag = 1;
    puts("*** Debug mode activated.");
  }
  else {
    perr(Die, "Cannot interpret option '-%s'.", opt);
  }
}

void genFiles(VarStr& inName,
              char*   outBuf,
              FILE*&  inFile, 
              FILE*&  outFile)
{
  inFile  = strcmp(inName, "-") ? fopen(inName, "r") : stdin;
  outFile = NULL;

  if (! inFile) {
    perr(Die, "Cannot open %s for reading.", (const char*) inName);
  }

  if (! strcmp(inName, "-")) inName.set("stdin");

  if (*outPath) {
    // If the output file has been specified by the user on the command
    // line, just use that, then zero it out so the name won't be reused
    // for the next file.

    if (*outPath == '+') {
      outFile = stdout;
      strcpy(outBuf, outPath + 1);

      if (*outBuf == '\0') strcpy(outBuf, "stdout");
    }
    else {
      strcpy(outBuf, outPath);
    }

    *outPath = 0;
  }
  else {
    // Change the file's extension to create the output file name.
    // There are two ways to implement this; the easy way is used
    // under DOS, where Borland C provides special functions to
    // split and merge filenames.  The hard way is used on all other
    // platforms, calling only ANSI functions; this may possibly be
    // confused by input filenames containing no extension, but with
    // periods further back in the path spec (e.g., in a dir name).

    #if 0    // def __MSDOS__
      char drive[MAXDRIVE];
      char dir  [MAXDIR];
      char file [MAXFILE];
      char ext  [MAXEXT];

      fnsplit(inName, drive, dir, file, ext);

      if (! strncmp(ext, outExt, 3)) {
        perr(Die, "Input file has '%s' extension, same as first 3 chars "
                  "of output file extension.", ext);
      }

      fnmerge(outBuf, drive, dir, file, outExt);
    #else   // Not DOS
      strcpy(outBuf, inName);

      char* extStart = strrchr(outBuf, '.');

      if (extStart == NULL) {
        extStart = outBuf + strlen(outBuf);
      }

      if (! strcmp(extStart, outExt)) {
        perr(Die, "Input file has '%s' extension, same as output.", outExt);
      }

      strcpy(extStart, outExt);
    #endif
  }

  strFixCase(outBuf);

  if (! outFile) {
    outFile = fopen(outBuf, "w");

    if (! outFile) {
      perr(Die, "Cannot open %s for writing.", outBuf);
    }
  }
}

void printHelp()
{
  puts("Orb usage:  orb { option | file }+");
  puts("File wildcards are supported per usual conventions.");
  puts("Options:");
  puts("  -b             Bashful - no copyright/ID message -");
  puts("                 must be first argument if used");
  puts("  -h             This help text");
  puts("  -q             Quiet mode (only report serious errors)");
  puts("  -v             Verbose mode (report detailed progress info)");
  puts("  -ofullname     Set output file name (e.g., -odoc1.html)");
  puts("  -o+fullname    Send output to stdout, w/opt. pseudonym");
  puts("  -xext          Set output file extension (e.g., -xshtml)");
  puts("  -s             Do not splice lines ending in \\");
  puts("  -Ccase         Force case for filenames: L=lower, U=upper,");
  puts("                 M=mixed - default L on DOS, M elsewhere");
  puts("  -Dvar=val      Define a variable (as if through $def)");
  puts("  -Dvar val      Alternate syntax for variable definition");
  puts("  -Ipath         Add directory in which to find include files");
  puts("");
  puts("Options apply only to files listed after the option on the");
  puts("command line.");
  puts("Author:       Craig Berry (cberry@cinenet.net");
  puts("Orb web page: http://www.cinenet.net/users/cberry/orbinfo.html");
}
