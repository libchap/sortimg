#include "fbiterator.h"


#include <algorithm> // max, min

using std::max;
using std::min;


const QString FBIterator::null_string = QString();
const ScaleCropRule FBIterator::null_scr = ScaleCropRule();
// just constant dummy objects to be returned in case of error as fallback value

// delta between two QMap iteraors
static int iterator_minus(const _QMQSSCRCI & end, const _QMQSSCRCI & begin) {
  _QMQSSCRCI ix = begin;
  int res = 0;

  while (ix != end) {
    ++ix;
    res++;
  }
  return res;
}


FBIterator::FBIterator(_QMQSSCRCI list_begin, _QMQSSCRCI list_end, _QMQSSCRCI list_curr) {
  it = list_curr;
  before_begin = -(iterator_minus(list_curr, list_begin));
  behind_last = -(iterator_minus(list_end, list_curr) - 1);
  if (!isValid()) it = list_begin;
}
FBIterator::FBIterator(_QMQSSCRCI list_begin, _QMQSSCRCI list_end) {
  it = list_begin;
  before_begin = 0;
  behind_last = -(iterator_minus(list_end, list_begin) - 1);
}
FBIterator::FBIterator(const FBIterator & fbi) {
  it = fbi.it; before_begin = fbi.before_begin; behind_last = fbi.behind_last;
}

// is the iterator pointing inside of the map ?
bool FBIterator::isValid() const {
  return ((before_begin <= 0) && (behind_last <= 0));
}
const QString & FBIterator::operator*() {
  if (isValid()) return it.key();
  else return null_string;
}
const ScaleCropRule & FBIterator::getSCR() const {
  if (isValid()) return it.value();
  else return null_scr;
}
void FBIterator::setSCR(const ScaleCropRule & newscr) {
  if (isValid()) it.value() = newscr;
}
bool FBIterator::hasNext(int how_many) const {
  return (behind_last <= -how_many);
}
bool FBIterator::hasPrev(int how_many) const {
  return (before_begin <= -how_many);
}

// jump to next key and return modified iterator
FBIterator & FBIterator::next_go(int how_many) {
  bool valid_before = isValid();
  before_begin -= how_many;
  behind_last += how_many;
  if (valid_before && isValid()) it += how_many;
  else if (valid_before) it -= -(before_begin + how_many); // it = begin()
  else if (isValid()) it += -before_begin; // move it from begin() to correct position
  // else do nothing
  return *this;
}
FBIterator & FBIterator::prev_go(int how_many) {
  return next_go(-how_many);
}

// keep the current iterator unchanged, return a new one ponting at the next key
FBIterator FBIterator::next_get(int how_many) {
  FBIterator creat(*this);
  return creat.next_go(how_many);
}
FBIterator FBIterator::prev_get(int how_many) {
  FBIterator creat(*this);
  return creat.next_go(-how_many);
}

// return an iterator for subarray, beginning at the currect iterator position
// having max_size items (or less, if the current iterator position is near the end)
FBIterator FBIterator::subiterator_post(int max_size) {
  FBIterator creat(*this);
  creat.before_begin = max(0, before_begin);
  creat.behind_last = max(-max_size, behind_last);
  return creat;
}
FBIterator FBIterator::subiterator_pre(int max_size) {
  FBIterator creat(*this);
  creat.before_begin = max(-max_size, before_begin);
  creat.behind_last = max(0, behind_last);
  return creat;
}
