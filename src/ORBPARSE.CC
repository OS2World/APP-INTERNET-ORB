// orb - HTML preprocessor
// orbparse.cc - parse engine
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
#include "orbstate.hpp"
#include "orbver.h"


void OrbState::setDefaults()
{
  // Provide the current date/time in standard formats.

  const size_t tbufMax = 80;
  char         tbuf[tbufMax];
  const tm*    tm0;

  tm0 = gmtime(&t0);
  gmt = *tm0;
  tm0 = localtime(&t0);
  lcl = *tm0;

  strftime(tbuf, tbufMax, "%A, %d-%b-%y %H:%M:%S GMT", &gmt);
  value("datetime_gmt", tbuf);

  strftime(tbuf, tbufMax, "%A, %d-%b-%y %H:%M:%S %Z", &lcl);
  value("datetime_local", tbuf);

  // Provide hard-to-generate items.

  value("NL",   "\n");
  value("SP",   " ");

  // Brackets are endcoded then translated to protect them
  // from being interpreted by the parser.

  value("OBR",  "\x01");
  value("OBR2", "\x01\x01");
  value("CBR",  "\x02");
  value("CBR2", "\x02\x02");

  // Provide config information.

  value("orb_vernum",    ORB_VERSION);
  value("orb_version",   ORB_ID);
  value("orb_generator", "<META NAME=\"generator\" CONTENT=\"" ORB_ID "\">");
}


void OrbState::resetParser()
{
  clear();
  setDefaults();
}


OrbState::Action OrbState::parse(const char* cmd,
                                 ValArray&   work)
{
//  printf("*** Parsing: %s\n", cmd);

  const char* const ws = " \t";

  Action      act     = Literal;
  char* const buf     = new char[strlen(cmd) + 1];
  const int   maxToks = 3;
  const char* wstr;
  const char* tok[maxToks];
  unsigned    toks;
  VarStr      valb;

  assert(buf != NULL);
  strcpy(buf, cmd);

  for (toks = 0;
       toks < maxToks &&
         (tok[toks] = strtok(toks ? (char*) NULL : buf, ws)) != NULL;
       toks++); // Empty body.

  if (tok[0][0] != '$' && (wstr = value(tok[0])) != NULL) {
//    printf("wstr = /%s/\n", wstr);

    // Variable reference.

    act = Subs;

    const char* args = toks >= 2 ? cmd + (tok[1] - buf) :
                                   (const char*) NULL;

    if (args == NULL) {
      // The old, simple case...no args.

      processString(wstr, valb, 0, 1);

//      printf("args==NULL: valb = /%s/\n", (const char*) valb);

      work.store(0, valb);
    }
    else {
      // Parse out the args and substitute in their values.

      push();   // Set up a new local variable scope.

      char        sep[3] = ", ";
      char*       temp   = new char[strlen(args) + 1];
      char*       p      = temp;
      const char* arg;

      strcpy(temp, args);

      if (strspn(temp, "/:\\|~!") > 0) {
        sprintf(sep, "%c", temp[0]);
        p++;
      }

//      printf("Separator is (%s)\n", sep);

      const size_t  maxLocals = 20;
      VarStr        locals[maxLocals];
      unsigned      ix;

      // The following three for loops parse out the arguments to
      // the macro call and set corresponding local variables:
      // 1) Pull out the string for each argument.
      // 2) Turn the raw string into its processed version.
      // 3) Set the corresponding local variable.

      for (ix = 0;
           (arg = strtok(ix ? (char*) NULL : p /*temp*/, sep)) != NULL;
           ix++) {
        assert(ix < maxLocals);

        locals[ix] = arg;

//        printf("Raw local %d = /%s/\n", ix + 1, arg);
      }

      for (size_t n = 0; n < ix; n++) {
        processString(locals[n], valb, 0);
        locals[n] = valb;

//        printf("Proc'd local %lu = /%s/\n", n + 1, (const char*) valb);
      }

      for (size_t n = 0; n < ix; n++) {
        char ixstr[10];
        sprintf(ixstr, "%u", (unsigned) (n + 1));   // Bug fix 19981027

        local(ixstr, locals[n]);

//        printf("Setting local var %s = %s\n", ixstr, (const char*) locals[n]);
      }
         
      processString(wstr, valb, 1);  // Added 3rd arg of 1

//      printf("wstr = /%s/, valb = /%s/\n", wstr, (const char*) valb);

      pop();   // Give up the local variable scope we were working in.

      work.store(0, valb);

      delete [] temp;
    }
  }
  else if (toks >= 1 && (! strcmp(tok[0], "$error") ||
                         ! strcmp(tok[0], "$err"))) {
    // User-generated error indicator.

    act  = UserError;
    wstr = toks >= 2 ? cmd + (tok[1] - buf) : "";
    work.store(0, wstr);
  }
  else if (toks >= 1 && (! strcmp(tok[0], "$message") ||
                         ! strcmp(tok[0], "$msg"))) {
    // User-generated message.

    act  = UserMessage;
    wstr = toks >= 2 ? cmd + (tok[1] - buf) : "";
    work.store(0, wstr);
  }
  else if (toks >= 2 && ! strcmp(tok[0], "$val")) {
    // Variable value directive.

    wstr = value(tok[1]);

    if (wstr) {
      act = Subs;
      work.store(0, wstr);
    }
    else if (toks < 3) {
      act = None;
    }
    else {
      act  = Subs;
      wstr = cmd + (tok[2] - buf);

      processString(wstr, valb);

      work.store(0, valb);
    }
  }
  else if (toks == 2 && ! strcmp(tok[0], "$include")) {
    // File inclusion.

    act  = Include;
    wstr = cmd + (tok[1] - buf);
    processString(wstr, valb);
    work.store(0, valb);
  }
  else if (toks == 2 && ! strcmp(tok[0], "$target")) {
    // New target file.

    act  = NewTarget;
    wstr = cmd + (tok[1] - buf);
    processString(wstr, valb);
    work.store(0, valb);
  }
  else if (toks >= 1 && ! strcmp(tok[0], "$time")) {
    // Formatted time.

    act = Subs;

    const int         gmtFlag = toks == 1 ? 0 : ! strcmp(tok[1], "gmt");
    const char* const fmt     = (toks < 3) ?
                                  "%Y/%m/%d %H:%M:%S" :
                                  cmd + (tok[2] - buf);

    const size_t tbufMax = 300;
    char tbuf[tbufMax];

    strftime(tbuf, tbufMax, fmt, (gmtFlag ? &gmt : &lcl));

    work.store(0, tbuf);
  }
  else if (toks >= 1 && (! strcmp(tok[0], "$rem") ||
                         ! strcmp(tok[0], "$//"))) {
    // Remark (comment).

    act = None;
  }
  else if (toks >= 2 &&
           (! strcmp(tok[0], "$set") ||
            ! strcmp(tok[0], "$def") ||
            ! strcmp(tok[0], "$define"))) {
    // Set variable value.

    act  = Set;
    wstr = (toks >= 3) ? (cmd + (tok[2] - buf)) : "";

//  processString(wstr, valb, 1);

    work.store(0, tok[1]);
    work.store(1, wstr /*valb*/ );
  }
  else if (toks == 2 && (! strcmp(tok[0], "$undef") ||
                         ! strcmp(tok[0], "$unset"))) {
    act = None;
    remove(tok[1]);
  }
  else if (toks == 2 && ! strcmp(tok[0], "$ifdef")) {
    act = value(tok[1]) ? If1 : If0;
  }
  else if (toks == 2 && ! strcmp(tok[0], "$ifndef")) {
    act = value(tok[1]) ? If0 : If1;
  }
  else if (toks == 1 && ! strcmp(tok[0], "$else")) {
    act = Else;
  }
  else if (toks == 1 && ! strcmp(tok[0], "$endif")) {
    act = Endif;
  }
  else if (toks == 1 &&
           (! strcmp(tok[0], "$comment") ||
            ! strcmp(tok[0], "$/*"))) {
    act = Comment;
  }
  else if (toks == 1 &&
           (! strcmp(tok[0], "$endcomment") ||
            ! strcmp(tok[0], "$*/"))) {
    act = EndComment;
  }
  else {
    act = Error;
  }

  delete [] buf;
  return act;
}


int OrbState::push()
{
  const int ret = ++sp < MaxDepth;
  if (ret) stack[sp].clear();
  return ret;
}


int OrbState::pop()
{
  const int ret = sp > 0;
  if (ret) sp--;
  return ret;
}


const char* OrbState::value(const char* key, const char* val)
{
  const char* ret = NULL;
  unsigned    n;

  for (n = sp; n > 0; n--) {
    if (stack[n].value(key)) {
      ret = stack[n].value(key, val);
      break;
    }
  }

  if (n == 0) ret = stack[0].value(key, val);
  return ret;
}


const char* OrbState::local(const char* key, const char* val)
{
  return stack[sp].value(key, val);
}


int OrbState::remove(const char* key)
{
  return stack[0].remove(key);
}


void OrbState::clear()
{
  sp = 0;
  stack[0].clear();
}


void OrbState::validate() const
{
  printf("Validating: Stack is %d level(s) deep.\n", sp + 1);

  for (int n = sp; n >= 0; n--) {
    printf("Validating stack level %d.\n", sp);
    stack[n].validate();
  }
}


void OrbState::dump() const
{
  printf("Dumping: Stack is %d level(s) deep.\n", sp + 1);

  for (int n = sp; n >= 0; n--) {
    printf("Dumping stack level %d.\n", sp);
    stack[n].dump();
  }
}


void OrbState::operator += (const StringAssoc& other)
{
  stack[0] += other;
}


ValArray::ValArray()
{
  for (size_t n = 0; n < Slots; n++) val[n] = NULL;
}


ValArray::~ValArray()
{
  for (size_t n = 0; n < Slots; n++) delete [] val[n];
}


void ValArray::store(size_t slot, const char* s)
{
  delete val[slot];
  assert((val[slot] = new char[strlen(s) + 1]) != NULL);
  strcpy(val[slot], s);
}

