#ifndef _SI_FBITERATOR_H_
#define _SI_FBITERATOR_H_

#include <QMap>
#include <QString>

#include "scr.h"

typedef QMap<QString, ScaleCropRule>::iterator _QMQSSCRCI;


// a custom iterator type for handy use with FileBank
// able to go out of bounds of the array, control of validity (ability to be dereferenced)
// map keys considered fixed, values can be changed
// posibility of constraining on subarrays

class FBIterator {

public:
  FBIterator(_QMQSSCRCI list_begin, _QMQSSCRCI list_end, _QMQSSCRCI list_curr);
  FBIterator(_QMQSSCRCI list_begin, _QMQSSCRCI list_end);
  FBIterator(const FBIterator & fbi);

  const QString & operator*();
  const ScaleCropRule & getSCR() const;
  void setSCR(const ScaleCropRule & newscr);
  bool hasNext(int how_many = 1) const;
  bool hasPrev(int how_many = 1) const;
  FBIterator & next_go(int how_many = 1);
  FBIterator & prev_go(int how_many = 1);
  FBIterator next_get(int how_many = 1);
  FBIterator prev_get(int how_many = 1);
  bool isValid() const;
  //bool isPointing() { return valid && (qslcurr != qslend); }

  FBIterator subiterator_post(int max_size);
  FBIterator subiterator_pre(int max_size);

  int total_items() { return -before_begin + -behind_last + 1; }
  int item_index() { return -before_begin; }

private:
  _QMQSSCRCI it;
  int before_begin, behind_last;

  static const QString null_string;
  static const ScaleCropRule null_scr;

};


#endif // #ifndef _SI_FBITERATOR_H_
