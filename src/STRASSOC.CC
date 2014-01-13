// orb - HTML preprocessor
// strassoc.cc - string associative array class implementation
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
#include "strassoc.hpp"

StringAssoc::StringAssoc() :
  root(new Cell)
{
  assert(root != NULL);
}

StringAssoc::~StringAssoc()
{
  clear();
  delete root;
}

void StringAssoc::clear()
{
  Cell* nxt;

  for (Cell* cur = root->next();
       (nxt = cur->next()) != cur;
       cur = nxt) {
    delete cur;
  }
}

const char* StringAssoc::value(const char* key, const char* val)
{
  Cell*       cur;
  int         cmp = 1;
  const char* ret = NULL;

  for (cur = root->next();
       cur != root && (cmp = strcmp(cur->key(), key)) < 0;
       cur = cur->next());  // Empty body.

  if (cmp == 0) {
    ret = cur->val(val);
  }
  else if (val) {
    ret = val;
    assert(new Cell(cur->prev(), key, val));
  }

  return ret;
}

int StringAssoc::remove(const char* key)
{
  Cell* cur;

  for (cur = root->next();
       cur != root && strcmp(cur->key(), key);
       cur = cur->next());  // Empty body.

  const int ret = cur != root;

  if (ret) delete cur;

  return ret;
}  

StringAssoc& StringAssoc::operator += (const StringAssoc& other)
{
  for (Cell* cur = other.root->next();
       cur != other.root;
       cur = cur->next()) {
    value(cur->key(), cur->val());
  }

  return *this;
}

void StringAssoc::validate() const
{
  int count = 1;

  for (Cell* cur = root->next();
       cur != root;
       cur = cur->next(), ++count) {
    printf("  StrAssoc checking node %d...\n", count);

    assert(cur->prev()->next() == cur);
    assert(cur->next()->prev() == cur);
  }

  printf("Check completed.\n");
}

void StringAssoc::dump() const
{
  for (Cell* cur = root->next();
       cur != root;
       cur = cur->next()) {
    printf("  %-16s = /%s/\n", cur->key(), cur->val());
  }
}

StringAssoc::Cell::Cell(void) :
  keyp(new char[5]),
  valp(new char[5]),
  prevp(this),
  nextp(this)
{
  assert(keyp != NULL);
  assert(valp != NULL);

  strcpy(keyp, "ROOT");
  strcpy(valp, "ROOT");
}

StringAssoc::Cell::Cell(Cell* _prev,
                        const char* _key, const char* _val) :
  keyp(new char[strlen(_key) + 1]),
  valp(new char[strlen(_val) + 1]),
  prevp(_prev),
  nextp(_prev->next(this))
{
  assert(keyp != NULL);
  assert(valp != NULL);

  strcpy(keyp, _key);
  strcpy(valp, _val);

  nextp->prev(this);
}

StringAssoc::Cell::~Cell()
{
  delete keyp;
  delete valp;

  prevp->next(nextp);
  nextp->prev(prevp);
}

const char* StringAssoc::Cell::val(const char* v)
{
  if (v) {
    delete valp;
    assert((valp = new char[strlen(v) + 1]) != NULL);
    strcpy(valp, v);
  }

  return valp;
}

StringAssoc::Cell* StringAssoc::Cell::prev(Cell* p)
{
  Cell* ret = prevp;
  if (p) prevp = p;
  return ret;
}

StringAssoc::Cell* StringAssoc::Cell::next(Cell* n)
{
  Cell* ret = nextp;
  if (n) nextp = n;
  return ret;
}
