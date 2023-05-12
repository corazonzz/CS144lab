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

StreamReassembler::StreamReassembler(const size_t capacity) : c_store(capacity + 99999, '/'), c_judge(capacity + 99999,false), first_unread(0), first_unassembled(0), first_unacceptable(capacity), byte_unassembled(0) ,eof_(false), _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    //
    first_unread = _output.bytes_read();
    first_unacceptable = first_unread + _capacity;

    for(size_t i = index; i < index + data.size(); ++i){
        if(c_judge[i] == true){
                continue;
        }else{
            c_store[i] = data[i-index];
            c_judge[i] = true;
            byte_unassembled++;
        }
    }

    if(c_judge[first_unassembled] == true){
        string s;
        for(size_t i = first_unassembled; c_judge[i] && i < first_unacceptable; ++i){
            s += c_store[i];
        }
        _output.write(s);
        first_unassembled += s.size();
        byte_unassembled -= s.size();
    }

    if(eof){
        eof_ = true;
    }
    if(eof_ && empty())
        _output.end_input();


    //insert 
  /*   for(auto iter = idx_str.begin(); iter != idx_str.end();){
        if(first_unassembled >= iter->first){
            if(iter->first + iter->second.size() < first_unassembled){
                idx_str.erase(iter);
            }else{
                string s = iter->second.substr(iter->first - first_unassembled);
                size_t writesize = _output.write(s);
                if(writesize < s.size()){
                    string str = s.substr(s.size() - writesize);
                    idx_str[iter->first + writesize] = str;
                }
                idx_str.erase(iter);
            }
        }else{
            break;
        }
    } */

    


/*     //merge the data to the 
    auto iter_ = idx_str.upper_bound(index);
    if(iter_ != idx_str.begin()){
        iter_--;
    }
    //update insert index
    size_t ready_insert = index;
    // if the idx_str has only one element && 
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
        _output.end_input(); */
}

size_t StreamReassembler::unassembled_bytes() const {return byte_unassembled;}

bool StreamReassembler::empty() const { return unassembled_bytes() == 0; }
