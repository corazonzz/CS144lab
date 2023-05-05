#include "byte_stream.hh"
#include <cstddef>
#include <ios>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity): buffer_(), _capacity(capacity), writepos(0), readpos(0), writebytes(0), readbytes(0), _end(false){} 

size_t ByteStream::write(const string &data) {
    size_t re = remaining_capacity() ;
    string s = data;
    if(data.size() > re){
        s = data.substr(0,re);
    }
    for(auto & c: s){
        buffer_.emplace_back(c);
    }
    writepos += s.size();
    writebytes += s.size();
    return s.size();
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t copy_ = len;
    if(len > writepos){
        copy_ = writepos;
    }
    string s = string(buffer_.begin() + readpos, buffer_.begin() + copy_);
    return s;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t del_ = min(writepos, len);
    for(size_t i = 0; i < del_; ++i){
        buffer_.pop_front();
    }
    writepos -= del_;
    readbytes += del_;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string s = peek_output(len);
    pop_output(len);
    return s;
}

void ByteStream::end_input() {
    _end = true;
}

bool ByteStream::input_ended() const {
    return _end;
 }

size_t ByteStream::buffer_size() const {
    return writepos;
}

bool ByteStream::buffer_empty() const {
    return writepos == 0;
}

bool ByteStream::eof() const {
    return _end && buffer_empty();
}

size_t ByteStream::bytes_written() const {
    return writebytes;
}

size_t ByteStream::bytes_read() const {
    return readbytes;
}

size_t ByteStream::remaining_capacity() const {
    return _capacity - writepos;
}
