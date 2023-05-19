#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity();}

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const {
    return _time_since_last_segment_received;
}

void TCPConnection::segment_received(const TCPSegment &seg) {
    if(!active())
        return;
    _time_since_last_segment_received = 0;

    if(seg.header().ack){
        _sender.ack_received(seg.header().ackno,seg.header().win);
    }
    if(seg.header().rst){
        if(_listen()){
            return;
        }
        connect_shutdown();
        return;
    }
    if(seg.length_in_sequence_space() == 0){
        return;
    }
    _receiver.segment_received(seg);

    push_out();
}


void TCPConnection::push_out(){
    _sender.fill_window();
    TCPSegment seg;
    while(!_sender.segments_out().empty()){
        seg = _sender.segments_out().front();
        _sender.segments_out().pop();
        if(_receiver.ackno().has_value()){
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
        }
        if(_sender.stream_in().error() || _receiver.stream_out().error()){
            seg.header().rst = true;
        }
        segments_out().emplace(seg);
    }
}
bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    size_t _write = _sender.stream_in().write(data);
    push_out();
    return _write;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    if(!active()) return;
    _time_since_last_segment_received += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
    if(_sender.consecutive_retransmissions() >= TCPConfig::MAX_RETX_ATTEMPTS){
        _active = false;
        connect_shutdown();
    }
    push_out();
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    push_out();
}

void TCPConnection::connect() {
    push_out();
}
void TCPConnection::connect_shutdown() {
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _active = false;
    if(_sender.segments_out().empty()){

    }
    push_out();
}
bool TCPConnection::connect_close() {
    if (_receiver.stream_out().input_ended() && !_sender.stream_in().input_ended()){
        _linger_after_streams_finish = false;
    }
    if(_receiver.stream_out().input_ended() && _sender.stream_in().eof() && _sender.bytes_in_flight() == 0){
        if(!_linger_after_streams_finish || _time_since_last_segment_received >= _cfg.rt_timeout){
            _active = false;
        }
    }
    return _active;
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            connect_shutdown();
            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

bool TCPConnection::syn_sent() {
    return _sender.next_seqno_absolute() > 0 && _sender.next_seqno_absolute() == bytes_in_flight();
}

bool TCPConnection::syn_recv(){
    return _receiver.ackno().has_value() && !_receiver.stream_out().input_ended();
}

bool TCPConnection::_listen() {
    return !_receiver.ackno().has_value();
}