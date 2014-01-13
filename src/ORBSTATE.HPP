// orb - HTML preprocessor
// orbstate.hpp - state control header
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


#ifndef _ORBSTATE_HPP
#define _ORBSTATE_HPP

#include <stdlib.h>
#include <time.h>
#include "strassoc.hpp"
#include "varstr.hpp"

enum Cond    { C_None, C_If, C_Else };
enum ActStat { A_Off, A_On, A_Blocked };

struct CondLevel {
  Cond    cond;
  ActStat active;
};

class ValArray
{
  public:
    enum { Slots = 3 };

    ValArray();
    ~ValArray();

    void store(size_t slot, const char* s);

    const char* operator [] (size_t n) const { return val[n]; }

  private:
    char* val[Slots];
};

class OrbState
{
  public:
    enum Action {
      None, Subs,
      Set, Include, NewTarget, Literal,
      If0, If1, Else, Endif,
      Comment, EndComment,
      UserError, UserMessage, Error
    };

    OrbState(FILE* _in, FILE* _out,
             const char* _iname);

    OrbState(const OrbState& other, FILE* _in, const char* _iname);

    ~OrbState();

    void         compile();
    void         resetParser();
    int          processString(const char* inStr,
                               VarStr&     outStr,
                               int         ignoreLocals    = 0,
                               int         ignoreMalformed = 0);
    Action       parse(const char* cmd, ValArray& work);

    operator     StringAssoc() { return stack[sp]; };

    int          push();
    int          pop();

    // Operations forwarded to internal stack of StringAssocs:

    const char*  value(const char* key, const char* val = NULL);
    const char*  local(const char* key, const char* val = NULL);
    int          remove(const char* key);
    void         clear();
    void         validate() const;
    void         dump() const;

    void         operator += (const StringAssoc& other);

    FILE*       in;
    FILE*       out;
    const char* inName;
    unsigned    includeLevel;
    unsigned    lineNum;
    int         inComment;

    void reset()
    { lineNum = 0; inComment = 0; csp = -1; };

  protected:
    enum { MaxDepth = 10 };

    void         setDefaults();

    StringAssoc* stack;
    int          ownsStack;
    unsigned     sp;

    time_t       t0;
    tm           gmt;
    tm           lcl;

  private:
    enum { MaxCondLevel = 20 };

    CondLevel   condStack[MaxCondLevel];
    int         csp;

    FILE* openInclude(const char* name);

    int active()
    { return ! inComment && (csp < 0 || condStack[csp].active == A_On); }

    Cond condMode()
    { return csp >= 0 ? condStack[csp].cond : C_None; }

    int condOpen(ActStat a);
    int condClose();
    int condSet(Cond c, ActStat a);
    int condFlip(Cond c);
};

#endif
