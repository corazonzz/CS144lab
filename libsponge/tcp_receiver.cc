#include "tcp_receiver.hh"
#include "wrapping_integers.hh"
#include <cstddef>
#include <cstdint>
#include <optional>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

bool TCPReceiver::segment_received(const TCPSegment &seg) {
    static uint64_t seqno = 0;
    size_t data_size = 0;
    if(seg.header().syn){
        if(syn_rec == true)
            return false;
        syn_rec = true;
        isn_ = seg.header().seqno.raw_value();
        seqno = 1;
        cnt = 1;
        data_size = seg.length_in_sequence_space() - 1;
        if(data_size == 0)
            return  true;
    }else if (syn_rec == false){
        return false;
    }else{
        seqno = unwrap(seg.header().seqno, WrappingInt32(isn_), seqno);
        data_size = seg.length_in_sequence_space();
    }

    if(seg.header().fin){
        if(fin_rec)
            return false;
        fin_rec = true;
    }else if(seg.length_in_sequence_space() == 0 && seqno == cnt){
        return true;
    }else if(seqno >= cnt + window_size() || seqno + data_size < cnt){
        return false;
    }

    _reassembler.push_substring(seg.payload().copy(), seqno - 1, seg.header().fin);
    cnt = _reassembler.unassembled_pos() + 1;
    if(_reassembler.stream_out().input_ended())
        cnt += 1;
    return true;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if(cnt > 0)
        return WrappingInt32(wrap(cnt, WrappingInt32(isn_)));
    return std::nullopt;
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
