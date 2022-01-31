#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iomanip>
#include <iterator>
#include <stdexcept>
#include <string>

#include "book.hpp"
#include "book_list.hpp"

bool BookList::containers_are_consistent() const {
  // If the sizes of the containers are not all equal, the containers are not
  // consistent.
  if (books_array_size_ != books_vector_.size()
      || books_array_size_ != books_dl_list_.size()
      || books_array_size_ !=  books_sl_list_size()) {
    return false;
  }

  // Element content and order must be equal to each other
  auto current_array_position = books_array_.cbegin();
  auto current_vector_position = books_vector_.cbegin();
  auto current_dl_list_position = books_dl_list_.cbegin();
  auto current_sl_list_position = books_sl_list_.cbegin();

  while (current_vector_position != books_vector_.cend()) {
    if (*current_array_position != *current_vector_position
        || *current_array_position != *current_dl_list_position
        || *current_array_position != *current_sl_list_position) {
      return false;
    }

    // Advance the iterators to the next element in unison
    ++current_array_position;
    ++current_vector_position;
    ++current_dl_list_position;
    ++current_sl_list_position;
  }

  return true;
}

std::size_t BookList::books_sl_list_size() const {
  // Get the size of the SLL.
  return std::distance(books_sl_list_.begin(), books_sl_list_.end());
}

//
// Constructors, Assignments, and Destructor
//

BookList::BookList() = default;

BookList::BookList(const BookList& other) = default;

BookList::BookList(BookList&& other) = default;

BookList& BookList::operator=(const BookList& rhs) = default;

BookList& BookList::operator=(BookList&& rhs) = default;

BookList::~BookList() = default;

BookList::BookList(const std::initializer_list<Book>& init_list) {
  for (const Book& book : init_list) {
    insert(book, Position::BOTTOM);
  }

  // Verify the internal book list state is still consistent amongst the four
  // containers.
  if (!containers_are_consistent()) {
    throw BookList::InvalidInternalStateException(
        "Container consistency error in initializer_list constructor");
  }
}

BookList& BookList::operator+=(const std::initializer_list<Book>& rhs) {
  // Insert each book to the bottom one by one.
  for (const Book& book : rhs) {
    BookList::insert(book, Position::BOTTOM);
  }

  // Verify the internal book list state is still consistent amongst the four containers.
  if (!containers_are_consistent()) {
    throw BookList::InvalidInternalStateException(
        "Container consistency error in operator+= for initializer list");
  }
  return *this;
}

BookList& BookList::operator+=(const BookList& rhs) {
  // Insert each book to the bottom one by one.
  for (const Book& book : rhs.books_vector_) {
    BookList::insert(book, Position::BOTTOM);
  }

  // Verify the internal book list state is still consistent amongst the four
  // containers.
  if (!containers_are_consistent()) {
    throw BookList::InvalidInternalStateException(
        "Container consistency error in operator+= for BookList");
  }
  return *this;
}

//
// Queries
//

std::size_t BookList::size() const {
  // Verify the internal book list state is still consistent amongst the four
  // containers.
  if (!containers_are_consistent()) {
    throw BookList::InvalidInternalStateException(
      "Container consistency error in size");
  }

  // Get the size of one container to return.
  return books_vector_.size();
}

std::size_t BookList::find(const Book& book) const {
  // Verify the internal book list state is still consistent amongst the four
  // containers.
  if (!containers_are_consistent()) {
    throw BookList::InvalidInternalStateException(
        "Container consistency error in find");
        
  
  }

  //Search for a book in the book list.
  for(int i = 0; i < size(); ++i) {
    if (books_vector_[i] == book) {
      return i; // Book is found at index i.
    }
  }
  return size(); // Book doesn't exist.
}

//
// Mutators
//

BookList& BookList::insert(const Book& book, Position position) {
  // Convert the TOP and BOTTOM enumerations to an offset and delegate the work.
  switch (position) {
    case Position::TOP: {
      insert(book, 0);
      break;
    }
    case Position::BOTTOM: {
      insert(book, size());
      break;
    }
  }
  return *this;
}

// Insert the new book at offset_from_top, which places it before the current
// book at that position.
BookList& BookList::insert(const Book& book, std::size_t offset_from_top) {
  // Validate offset parameter before attempting the insertion. As std::size_t
  // is an unsigned type, there is no need to check for negative offsets. And an
  // offset equal to the size of the list says to insert at the end (bottom) of
  // the list. Anything strictly greater than the current size is an error.
  if (offset_from_top > size()) {
    throw InvalidOffsetException(
        "Insertion position beyond end of current list size in insert");
  }

  //
  // Prevent duplicate entries
  //

  // Return if a duplicate is found.
  if (find(book) != size()) {
    return *this;
  }

  // Inserting into the book list means you insert the book into each of the
  // containers (array, vector, forward_list, and list).
  //
  // Because the data structure concept is different for each container, the
  // way a book gets inserted is a little different for each. You are to insert
  // the book into each container such that the ordering of all the containers
  // is the same. A check is made at the end of this function to verify the
  // contents of all four containers are indeed the same.

  //
  // Insert into array
  //

  {
    // Verifies books_array_size_ is less than books_array_.size().
    if (books_array_size_ >= books_array_.size()) {
      throw CapacityExceededException("Capacity Exceeded");
    }
    // Shift the affected books.
    for (int i = books_array_size_; i > offset_from_top; --i) {
      books_array_[i] = books_array_[i - 1];
    }
    // Insert book in correct position.
    books_array_[offset_from_top] = book;
    // Increase size of array.
    ++books_array_size_;
  }

  //
  // Insert into vector
  //

  {
    // Create vector iterator.
    std::vector<Book>::iterator iter = books_vector_.begin();
    // Advance the iterator to the offset.
    std::advance(iter, offset_from_top);
    // Insert the book at the zero-based offset.
    books_vector_.insert(iter, book);
  }

  //
  // Insert into singly-linked list
  //

  {
    // Create a forward_list iterator.
    std::forward_list<Book>::iterator iter = books_sl_list_.before_begin();
    // Advance the iterator to the offset.
    std::advance(iter, offset_from_top);
    // Insert the book at the zero-based offset.
    books_sl_list_.insert_after(iter, book);
  }

  //
  // Insert into doubly-linked list
  //

  {
    // Create a list iterator.
    std::list<Book>::iterator iter = books_dl_list_.begin();
    // Advance the iterator to the offset.
    std::advance(iter, offset_from_top);
    // Insert the book at the offset.
    books_dl_list_.insert(iter, book);
  }

  // Verify the internal book list state is still consistent amongst the four
  // containers.
  if (!containers_are_consistent()) {
    throw BookList::InvalidInternalStateException(
        "Container consistency error in insert");
  }
  return *this;
}

BookList& BookList::remove(const Book& book) {
  remove(find(book));
  return *this;
}

BookList& BookList::remove(std::size_t offset_from_top) {
  // Removing from the book list means you remove the book from each of the
  // containers (array, vector, list, and forward_list).
  //
  // Because the data structure concept is different for each container, the
  // way a book gets removed is a little different for each. You are to remove
  // the book from each container such that the ordering of all the containers
  // is the same. A check is made at the end of this function to verify the
  // contents of all four containers are indeed the same.

  // If offset_from_top isn't a valid offset, no change occurs.
  if (offset_from_top >= size()) {
    return *this;
  }

  //
  // Remove from array
  //

  { 
    // Shift all books after the remove point to the left to close the hole.
    std::move(offset_from_top + 1 + books_array_.begin(), books_array_.end(),   
      offset_from_top + books_array_.begin());
    // Decrease books_array_size_ since an element is removed.
    --books_array_size_;
  }

  //
  // Remove from vector
  //

  {
    // Create a vector iterator.
    std::vector<Book>::iterator iter = books_vector_.begin();
    // Advance the iterator to the offset.
    std::advance(iter, offset_from_top);
    // Erase the iterator at the offset.
    books_vector_.erase(iter);
  }

  //
  // Remove from singly-linked list
  //

  {
    // Create a forward_list iterator.
    std::forward_list<Book>::iterator iter = books_sl_list_.before_begin();
    // Advance the iterator to 1 before the offset.
    std::advance(iter, offset_from_top);
    // Erase the iterator at the offset.
    books_sl_list_.erase_after(iter);
  }

  //
  // Remove from doubly-linked list
  //

  {
    // Create a list iterator.
    std::list<Book>::iterator iter = books_dl_list_.begin();
    // Advance the iterator to the offset.
    std::advance(iter, offset_from_top);
    // Erase the iterator at the offset.
    books_dl_list_.erase(iter);
  }

  // Verify the internal book list state is still consistent amongst the four
  // containers.
  if (!containers_are_consistent()) {
    throw BookList::InvalidInternalStateException(
        "Container consistency error in remove");
  }
  return *this;
}

BookList& BookList::move_to_top(const Book& book) {
  // If the book exists, it moves to the top of the list.
  if (find(book) != size()) {
    remove(book); // Remove book.
    insert(book, 0); // Reinsert the book at the top.
  }

  // Verify the internal book list state is still consistent amongst the four
  // containers.
  if (!containers_are_consistent()) {
    throw BookList::InvalidInternalStateException(
        "Container consistency error in move_to_top");
  }
  return *this;
}

void BookList::swap(BookList& rhs) noexcept {
  if (this == &rhs) {
    return;
  }

  books_array_.swap(rhs.books_array_);
  books_vector_.swap(rhs.books_vector_);
  books_dl_list_.swap(rhs.books_dl_list_);
  books_sl_list_.swap(rhs.books_sl_list_);

  std::swap(books_array_size_, rhs.books_array_size_);
}

//
// Insertion and Extraction Operators
//

std::ostream& operator<<(std::ostream& stream, const BookList& book_list) {
  if (!book_list.containers_are_consistent()) {
    throw BookList::InvalidInternalStateException(
        "Container consistency error in operator<<");
  }

  int count = 0;
  stream << book_list.size();
  for (const Book& book : book_list.books_sl_list_) {
    stream << '\n' << std::setw(5) << count++ << ":  " << book;
  }
  stream << '\n';
  return stream;
}

std::istream& operator>>(std::istream& stream, BookList& book_list) {
  if (!book_list.containers_are_consistent()) {
    throw BookList::InvalidInternalStateException(
        "Container consistency error in operator>>");
  }
  std::string label_holder;
  size_t count;
  
  // Read from the stream.
  BookList temp_list; // Create a temporary book list.
  stream >> count; // Read in the size of the list.
  for (int i = 0; i < count; ++i) { // Iterates for every book in the list.
    // Create a temporary book.
    Book temp;
    // Read in the ":  ".
    stream >> label_holder;
    // Read in the book from the book list.
    stream >> temp;
    //Insert the book to the bottom of our temporary list.
    temp_list.insert(temp, BookList::Position::BOTTOM);
  }
  // Modify book_list.
  book_list = std::move(temp_list);
  
  return stream;
}

//
// Relational Operators
//

int BookList::compare(const BookList& other) const {
  if (!containers_are_consistent() || !other.containers_are_consistent()) {
    throw BookList::InvalidInternalStateException(
        "Container consistency error in compare");
  }

  if (size() < other.size()) {
    return -1; // Return -1 if this BookList is smaller than the other.
  } else if (size() > other.size()) { 
    return 1; // Return 1 if this BookList is bigger than the other.
  } else {
    // Else means their sizes are equal.
    auto others_iter = other.books_vector_.begin();
    // The iterators goes through the Booklists and compare contents.
    for (auto iter = books_vector_.begin();
        iter != books_vector_.end(); ++iter) {
      if (*iter > *others_iter) { // This BookList is greater.
        return 1;
      } else if (*iter < *others_iter) { // This BookList is smaller.
        return -1;
      }
      ++others_iter;
    }
    return 0; // The BookLists are equal.
  }
}

bool operator==(const BookList& lhs, const BookList& rhs) {
  return lhs.compare(rhs) == 0;
}

bool operator!=(const BookList& lhs, const BookList& rhs) {
  return lhs.compare(rhs) != 0;
}

bool operator<(const BookList& lhs, const BookList& rhs) {
  return lhs.compare(rhs) < 0;
}

bool operator<=(const BookList& lhs, const BookList& rhs) {
  return lhs.compare(rhs) <= 0;
}

bool operator>(const BookList& lhs, const BookList& rhs) {
  return lhs.compare(rhs) > 0;
}

bool operator>=(const BookList& lhs, const BookList& rhs) {
  return lhs.compare(rhs) >= 0;
}