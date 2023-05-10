#include "stream_reassembler.hh"
#include "byte_stream.hh"
#include <cstddef>
#include <string>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : idx_str(), first_unread(), first_unassembled(0), first_unacceptable(capacity), byte_unassembled(0) ,eof_(0), _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // first insert index
    auto iter_ = idx_str.upper_bound(index);
    if(iter_ != idx_str.begin()){
        iter_--;
    }
    //update insert index
    size_t ready_insert = index;

    if(iter_ != idx_str.end() && iter_->first <= index){
        size_t up_idx = iter_->first;

        if(index < up_idx + iter_->second.size())
            ready_insert = up_idx + iter_->second.size();
    }else if(index < first_unassembled)
        ready_insert = first_unassembled;

    // start to read
    size_t start_ = ready_insert - index;
    // remain 
    size_t d_size = data.size() - start_;

    iter_ = idx_str.lower_bound(ready_insert);

    while(iter_ != idx_str.end() && ready_insert <= iter_->first){
        size_t end_pos = ready_insert + data.size();

        if(iter_->first < end_pos){
            if(end_pos < iter_->first + iter_->second.size()){
                d_size = iter_->first - ready_insert;
                break;
            }else{
                iter_ = idx_str.erase(iter_);
            }
        }else
            break;
    }

    first_unacceptable = first_unread + _capacity - _output.buffer_size();
    if(first_unacceptable <= ready_insert)
        return;
    
    if(d_size > 0){
        string new_data = data.substr(start_, d_size);
        if(ready_insert == first_unread){
            size_t write_byte = _output.write(new_data);
            first_unassembled += write_byte;
            if(write_byte < new_data.size()){
                string store_data = new_data.substr(write_byte, new_data.size() - write_byte);
                byte_unassembled += store_data.size();
                idx_str[first_unassembled] = store_data;
            }
        }else{
            string store_data = new_data.substr(0, new_data.size());
            byte_unassembled += store_data.size();
            idx_str[first_unassembled] = store_data;
        }
    }


    for(auto iter = idx_str.begin(); iter != idx_str.end(); ){
        if(first_unassembled == iter->first){
            size_t write_bytes = _output.write(iter->second);
            if(write_bytes < iter->second.size()){
                byte_unassembled += iter->second.size() - write_bytes;
                idx_str[first_unassembled] = iter->second.substr(write_bytes);

                byte_unassembled -= iter->second.size();
                idx_str.erase(iter);
                break;
            }
            byte_unassembled -= iter->second.size();
            iter = idx_str.erase(iter);
        }else{
            break;
        }
    }
    if(eof){
        eof_ = index + data.size();
    }
    if(eof_ <= first_unassembled)
        _output.end_input();
}

size_t StreamReassembler::unassembled_bytes() const {return byte_unassembled;}

bool StreamReassembler::empty() const { return byte_unassembled == 0; }
