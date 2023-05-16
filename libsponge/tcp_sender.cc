#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const {
    return _bytes_in_flight;
}

void TCPSender::fill_window() {
    if(!send_syn){
        //wrap to TCPsegment
        TCPSegment seg;
        seg.header().syn = true;
        seg.header().seqno = wrap(_next_seqno,_isn);
        // push to queue
        _segments_out.push(seg);
        _segments_outstanding.push(seg);
        //update seqno
        _next_seqno += seg.length_in_sequence_space();
        _bytes_in_flight += seg.length_in_sequence_space();
        //timer start
        if(!retransmission_start){
            retransmission_start = true;
            _time = 0;
        }

        send_syn = true;

        return;
    }
    // if window_size == 0 act_window = 1 window_size
    size_t act_window = _window_size == 0 ? 1 : _window_size;
    //window remain space
    size_t _remain = act_window - (_next_seqno - recvackno);
    //window is not full
    while(_remain != 0 && !send_fin){
        size_t _load = min(TCPConfig::MAX_PAYLOAD_SIZE, _remain);
        string str = _stream.read(_load);
        //wrap to TCPSegment
        TCPSegment seg;
        seg.payload() = Buffer(std::move(str));
        seg.header().seqno = wrap(_next_seqno,_isn);
        // if end
        if(_stream.eof() && seg.length_in_sequence_space() < act_window){
            seg.header().fin = true;
            // fin flag is true
            send_fin = true;
        }

        if(seg.length_in_sequence_space() == 0)
            return;
        //push to queue
        _segments_out.push(seg);
        _segments_outstanding.push(seg);

        _next_seqno += seg.length_in_sequence_space();
        _bytes_in_flight += seg.length_in_sequence_space();

        if(!retransmission_start){
            retransmission_start = true;
            _time = 0;
        }
        _remain -= _load;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
//! \returns `false` if the ackno appears invalid (acknowledges something the TCPSender hasn't sent yet)
bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    size_t left = unwrap(ackno,_isn, recvackno);
    if(left > _next_seqno){
        return false;
    }
    _window_size = window_size;

    if(left <= recvackno){
        return true;
    }

    recvackno = left;
    //pop
    while(!_segments_outstanding.empty()){
        TCPSegment temp = _segments_outstanding.front();
        if(unwrap(temp.header().seqno,_isn,_next_seqno) + temp.length_in_sequence_space() <= left) {
            _bytes_in_flight -= temp.length_in_sequence_space();
            _segments_outstanding.pop();
        }else
            break;
    }

    fill_window();

    retransmission_timeout = _initial_retransmission_timeout;
    _consecutive_retransmission = 0;

    if(!_segments_outstanding.empty()){
        retransmission_start = true;
        _time = 0;
    }

    return true;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if(_bytes_in_flight == 0) return;
    _time += ms_since_last_tick;
    if(_time >= retransmission_timeout && !_segments_outstanding.empty()){
        _segments_out.push(_segments_outstanding.front());
        if(_window_size != 0)
            retransmission_timeout *= 2;
        _consecutive_retransmission++;
        retransmission_start = true;
        _time = 0;
    }
    if(_segments_outstanding.empty()){
        retransmission_start = false;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const {
    return _consecutive_retransmission;
}

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.emplace(seg);
}
