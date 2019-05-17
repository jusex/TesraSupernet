



#include "table/merger.h"

#include "leveldb/comparator.h"
#include "leveldb/iterator.h"
#include "table/iterator_wrapper.h"

namespace leveldb {

namespace {
class MergingIterator : public Iterator {
 public:
  MergingIterator(const Comparator* comparator, Iterator** children, int n)
      : comparator_(comparator),
        children_(new IteratorWrapper[n]),
        n_(n),
        current_(NULL),
        direction_(kForward) {
    for (int i = 0; i < n; i++) {
      children_[i].Set(children[i]);
    }
  }

  virtual ~MergingIterator() {
    delete[] children_;
  }

  virtual bool Valid() const {
    return (current_ != NULL);
  }

  virtual void SeekToFirst() {
    for (int i = 0; i < n_; i++) {
      children_[i].SeekToFirst();
    }
    FindSmallest();
    direction_ = kForward;
  }

  virtual void SeekToLast() {
    for (int i = 0; i < n_; i++) {
      children_[i].SeekToLast();
    }
    FindLargest();
    direction_ = kReverse;
  }

  virtual void Seek(const Slice& target) {
    for (int i = 0; i < n_; i++) {
      children_[i].Seek(target);
    }
    FindSmallest();
    direction_ = kForward;
  }

  virtual void Next() {
    assert(Valid());

    
    
    
    
    
    if (direction_ != kForward) {
      for (int i = 0; i < n_; i++) {
        IteratorWrapper* child = &children_[i];
        if (child != current_) {
          child->Seek(key());
          if (child->Valid() &&
              comparator_->Compare(key(), child->key()) == 0) {
            child->Next();
          }
        }
      }
      direction_ = kForward;
    }

    current_->Next();
    FindSmallest();
  }

  virtual void Prev() {
    assert(Valid());

    
    
    
    
    
    if (direction_ != kReverse) {
      for (int i = 0; i < n_; i++) {
        IteratorWrapper* child = &children_[i];
        if (child != current_) {
          child->Seek(key());
          if (child->Valid()) {
            
            child->Prev();
          } else {
            
            child->SeekToLast();
          }
        }
      }
      direction_ = kReverse;
    }

    current_->Prev();
    FindLargest();
  }

  virtual Slice key() const {
    assert(Valid());
    return current_->key();
  }

  virtual Slice value() const {
    assert(Valid());
    return current_->value();
  }

  virtual Status status() const {
    Status status;
    for (int i = 0; i < n_; i++) {
      status = children_[i].status();
      if (!status.ok()) {
        break;
      }
    }
    return status;
  }

 private:
  void FindSmallest();
  void FindLargest();

  
  
  
  const Comparator* comparator_;
  IteratorWrapper* children_;
  int n_;
  IteratorWrapper* current_;

  
  enum Direction {
    kForward,
    kReverse
  };
  Direction direction_;
};

void MergingIterator::FindSmallest() {
  IteratorWrapper* smallest = NULL;
  for (int i = 0; i < n_; i++) {
    IteratorWrapper* child = &children_[i];
    if (child->Valid()) {
      if (smallest == NULL) {
        smallest = child;
      } else if (comparator_->Compare(child->key(), smallest->key()) < 0) {
        smallest = child;
      }
    }
  }
  current_ = smallest;
}

void MergingIterator::FindLargest() {
  IteratorWrapper* largest = NULL;
  for (int i = n_-1; i >= 0; i--) {
    IteratorWrapper* child = &children_[i];
    if (child->Valid()) {
      if (largest == NULL) {
        largest = child;
      } else if (comparator_->Compare(child->key(), largest->key()) > 0) {
        largest = child;
      }
    }
  }
  current_ = largest;
}
}  

Iterator* NewMergingIterator(const Comparator* cmp, Iterator** list, int n) {
  assert(n >= 0);
  if (n == 0) {
    return NewEmptyIterator();
  } else if (n == 1) {
    return list[0];
  } else {
    return new MergingIterator(cmp, list, n);
  }
}

}  
