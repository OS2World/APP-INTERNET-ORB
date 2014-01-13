// orb - HTML preprocessor
// orbstate.cc - state control
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


#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "errrpt.h"
#include "orbstate.hpp"
#include "util.hpp"

OrbState::OrbState(FILE* _in, FILE* _out,
                   const char* _iname) :
  in(_in), out(_out),
  inName(_iname),
  includeLevel(0),
  lineNum(0), inComment(0),
  stack(new StringAssoc[MaxDepth]),
  ownsStack(1),
  sp(0),
  csp(-1)
{
  time(&t0);      // Record the time of Orb execution.
  setDefaults();
}

OrbState::OrbState(const OrbState& other, FILE* _in, const char* _iname) :
  in(_in), out(other.out),
  inName(_iname),
  includeLevel(other.includeLevel + 1),
  lineNum(0), inComment(0),
  stack(other.stack),
  ownsStack(0),
  sp(0),
  t0(other.t0), gmt(other.gmt), lcl(other.lcl),
  csp(-1)
{}

OrbState::~OrbState()
{
  if (ownsStack) delete [] stack;
}

int OrbState::condOpen(ActStat a)
{
  if (csp < ((int) MaxCondLevel - 1)) {
    ++csp;
    condStack[csp].cond   = C_If;
    condStack[csp].active = a;
    return 1;
  }
  else {
    return 0;
  }
}

int OrbState::condClose()
{
  if (csp >= 0) {
    --csp;
    return 1;
  }
  else {
    return 0;
  }
}

int OrbState::condSet(Cond c, ActStat a)
{
  if (csp >= 0) {
    condStack[csp].cond   = c;
    condStack[csp].active = a;
    return 1;
  }
  else {
    return 0;
  }
}

int OrbState::condFlip(Cond c)
{
  if (csp >= 0) {
    condStack[csp].cond   = c;
    condStack[csp].active =
      condStack[csp].active == A_Blocked ? A_Blocked :
        (condStack[csp].active == A_On ? A_Off : A_On);
    return 1;
  }
  else {
    return 0;
  }
}

void OrbState::compile()
{
  assert(in);
  assert(out);
  assert(inName);

  perr(Detail, "Processing file %s", inName);

  value("source", inName);
  reset();

  const size_t biteSize = 128;
  char         bite[biteSize];
  VarStr       inLine;
  VarStr       outLine;
  size_t       inLen;

  do {         // Loop for independent lines of input
    inLine.clear();

    do {       // Loop for continued lines using backslash
      do {     // Loop for pieces of a single line
        if (! fgets(bite, biteSize, in)) {
          inLine.valid(0);
          break;
        }

        inLine.append(bite);
      } while (strlen(bite)   == (biteSize - 1) &&
               bite[biteSize] != '\n');

      if (! inLine.isValid()) break;

      inLen = strlen(inLine);

      if (inLen == 0) {    // Not even a newline; must be weird EOF.
        inLine.valid(0);
        break;
      }

      ++lineNum;

      // Splice together lines ending with a backslash.
    } while (inLen > 2                 &&
             lineSpliceFlag            &&
             inLine[inLen - 2] == '\\' &&
             ! (inLine[inLen - 2] = '\0'));   // For side effect

    if (inLine.isValid()) {
      outLine.clear();

      processString(inLine, outLine);

      outLine.substitute('\x01', '[');
      outLine.substitute('\x02', ']');

      if (outLine.isValid()) {
        fputs(outLine, out);
      }
    }
  } while (inLine.isValid());

  if (inComment) {
    perr(Warn, "Comment block unclosed at end of file %s.",
         inName);
  }

  if (condMode() != C_None) {
    perr(Warn, "Conditional block unclosed at end of file %s.",
         inName);
  }
}


int OrbState::processString(const char* inStr,
                            VarStr&     outStr,
                            int         ignoreLocals,
                            int         ignoreMalformed)
{
//  printf("Processing str: /%s/\n", inStr);

  int         actFlag = 0;     // True if any 'real' output has happened.
  const char* inPtr   = inStr; // Marks place in input string.
  const char* startPtr;        // Marks start of current 'mark'.
  VarStr      full;            // The full mark, with [[ and ]].
  VarStr      middle;          // The text inside [[ ]].
  VarStr      valb;            // Holds results of internal marks.

  outStr.clear();

  while ((startPtr = strstr(inPtr, startMark)) != NULL) {
    // Copy up to the match, if we're outputting.

    if (active()) {
      const size_t preLen = startPtr - inPtr;
      outStr.append(inPtr, preLen);
    }

    inPtr = startPtr;

    // Scan for embedded start and end markers; exit when we're
    // either balanced or out of chars to process.

    int               level  = 1;
    const char* const midPtr = startPtr + startMarkLen;
    const char*       cur    = midPtr;

    while (level > 0) {
      const char* const nextStart = strstr(cur, startMark);
      const char* const nextEnd   = strstr(cur, endMark);

      if (! nextEnd) {
        level = -1;  // Unterminated mark.
      }
      else if (! nextStart || nextEnd < nextStart) {
        cur = nextEnd + endMarkLen;
        --level;
      }
      else {
        cur = nextStart + startMarkLen;
        ++level;
      }
    }

    if (level) {
      // Mismatched brackets -- output the first set as literals and
      // try again.

      if (active()) {
        outStr.append(startMark, startMarkLen);

        if (! ignoreMalformed) {
          perr_fl(inName, lineNum, Warn,
                  "Unmatched %s.  Copying to output.",
                  startMark);
        }
      }

      inPtr += startMarkLen;
      continue;   // To while loop above.
    }

    // Figure out what's between the markers and proceed accordingly.

    const size_t midLen  = cur - endMarkLen - midPtr;
    const size_t markLen = cur - startPtr;

    middle.set(midPtr, midLen);
    full.set(startPtr, markLen);

    ValArray     work;
    const Action act = parse(middle, work);

    // If it's anything other than a literal or error mark, we've
    // got a 'real' action for this line.

    actFlag |= (act != Literal &&
                act != Error);

    if (inComment) {
      if (act == EndComment) {
        inComment = 0;
      }
    }
    else if (! active()) {
      // Must be in the 'false' part of a conditional.  Only process
      // things that would break this

      switch (act) {
        case If0:
        case If1: {
          if (! condOpen(A_Blocked)) {
            perr_fl(inName, lineNum, Die,
                    "Conditionals nested too deeply.");
          }

          break;
        }

        case Else: {
          if (condMode() == C_If) {
            condFlip(C_Else);
          }
          else {
            perr_fl(inName, lineNum, Die,
                    "Misplaced '$else'.");
          }

          break;
        }

        case Endif: {
          condClose();
          break;
        }

        case Comment: {
          inComment = 1;
          break;
        }

        default:
          break;
      }
    }
    else {
      switch (act) {
        case None:
          break;

        case Comment: {
          inComment = 1;
          break;
        }

        case If0:
        case If1: {
          const ActStat a = act == If1 ? A_On : A_Off;

          if (! condOpen(a)) {
            perr_fl(inName, lineNum, Die,
                    "Conditionals nested too deeply.");
          }

          break;
        }

        case Else: {
          if (condMode() == C_If) {
            condFlip(C_Else);
          }
          else {
            perr_fl(inName, lineNum, Die,
                    "Misplaced '$else'.");
          }

          break;
        }

        case Endif: {
          if (! condClose()) {
            perr_fl(inName, lineNum, Die,
                    "Misplaced '$endif'.");
          }

          break;
        }

        case Subs: {
          outStr.append(work[0]);
          break;
        }

        case Set: {
          value(work[0], work[1]);
          break;
        }

        case Include: {
          if (strlen(outStr) > 0) {
            fputs(outStr, out);
            outStr.clear();
          }

          FILE* const inclFile = openInclude(work[0]);

          if (inclFile == NULL) {
            if (includeLevel >= 10) {
              perr_fl(inName, lineNum, Die,
                      "Cannot open include file %s for reading.  "
                      "May be caused by recursive includes; %d levels "
                      "deep at point of failure.",
                      (const char*) work[0], includeLevel); // First arg was 'valb'.
            }
            else {
              perr_fl(inName, lineNum, Die,
                      "Cannot open include file %s for reading.",
                      (const char*) work[0]); // First arg was 'valb'.
            }
          }

          OrbState incl(*this, inclFile, work[0]);

          incl.compile();

          fclose(inclFile);
          value("source", inName);

          break;
        }

        case NewTarget: {
          if (strlen(outStr) > 0) {
            fputs(outStr, out);
            outStr.clear();
          }

          char temp[MAXPATH];
          strcpy(temp, work[0]);
          strFixCase(temp);

          if (freopen(temp, "w", out)) {
            perr(Info, "Redirecting output to file %s.",
                 (const char*) temp);
          }
          else {
            perr_fl(inName, lineNum, Die,
                    "Cannot open file %s for output redirection.",
                    (const char*) temp);
          }

          value("target", temp);

          break;
        }

        case UserError:
        case UserMessage: {
          const ErrLevel lev = act == UserError ? Die : Always;

          perr_fl(inName, lineNum, lev,
                  "(user) %s", work[0]);
          break;
        }

        case Error: {
          // During processing of a $def directive, we must let local
          // variables of the form [[N]] (where N is a positive integer)
          // flow through.

          int warnUndef = 1;

          if (ignoreLocals) {
            char*    p;
            unsigned ix = strtoul(middle, &p, 10);

            // If the whole string is a positive unsigned int, it's a
            // local variable -- so don't warn about it being undefined,
            // just leave it for eventual use during variable expansion.

            warnUndef = *p || ! ix;
          }

          if (warnUndef) {
            perr_fl(inName, lineNum, Warn,
                    "Cannot parse %s.  Using literal value.",
                    (const char*) full);
          }

          // Drop through to Literal case.
        }

        case Literal: {
          outStr.append(full);
          break;
        }

        default: {
          perr_fl(inName, lineNum, Die,
                  "Impossible OrbParser case.  Unknown type %d.",
                  act);
          break;
        }
      }
    }

    inPtr += markLen;
  }

  // If we're doing output, copy whatever's left over after the
  // last substitution, then output the resulting line.

  if (active()) outStr.append(inPtr);

  // Suppress line output if we've done actions on this line
  // and the resulting line consists of just a newline.

  if (actFlag && strlen(outStr) <= 1) {
    outStr.valid(0);
  }

//  printf("ProcessString result = /%s/\n", (const char*) outStr);

  return 1;
}

FILE* OrbState::openInclude(const char* name)
{
  FILE* ret = NULL;
  char  fullPath[MAXPATH];

  for (size_t n = 0; ! ret && n < inclPathCount; n++) {
    const size_t len = strlen(inclPath[n]);

    if (strcmp(inclPath[n] + len - 1, DIRSEP)) {  // Doesn't end with sep.
      sprintf(fullPath, "%s%s%s", inclPath[n], DIRSEP, name);
    }
    else {  // Does end with dir separator.
      sprintf(fullPath, "%s%s", inclPath[n], name);
    }

    strFixCase(fullPath);

    if (debugFlag) printf("*** Trying include path %s...\n", fullPath);

    ret = fopen(fullPath, "r");
  }

  return ret;
}
