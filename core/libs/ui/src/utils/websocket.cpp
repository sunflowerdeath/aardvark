#include "utils/websocket.hpp"

namespace aardvark {

void Websocket::open() {
    state = WebsocketState::connecting;
    resolver.async_resolve(
        host.c_str(), port.c_str(),
        beast::bind_front_handler(&Websocket::on_resolve, shared_from_this()));
}

void Websocket::send(std::string message) {
    if (state == WebsocketState::open) {
        ws.async_write(asio::buffer(message),
                       beast::bind_front_handler(&Websocket::on_write,
                                                 shared_from_this()));
    }
}

void Websocket::close() {
    if (state == WebsocketState::connecting) {
        // Cancel any asynchronous operations that are waiting on the resolver.
        resolver.cancel();
        // Close the socket
        beast::get_lowest_layer(ws).close();
    } else if (state == WebsocketState::open) {
        state = WebsocketState::closing;
        ws.async_close(beast::websocket::close_code::normal,
                       beast::bind_front_handler(&Websocket::on_close,
                                                 shared_from_this()));
    }
}

void Websocket::on_resolve(beast::error_code error,
                           asio::ip::tcp::resolver::results_type results) {
    if (error) return error_signal("Resolve error. " + error.message());
    beast::get_lowest_layer(ws).async_connect(
        results,
        beast::bind_front_handler(&Websocket::on_connect, shared_from_this()));
}

void Websocket::on_connect(
    beast::error_code error,
    asio::ip::tcp::resolver::results_type::endpoint_type) {
    if (error) return error_signal("Connect error. " + error.message());
    ws.async_handshake(host, "/",
                       beast::bind_front_handler(&Websocket::on_handshake,
                                                 shared_from_this()));
}

void Websocket::on_handshake(beast::error_code error) {
    if (error) return error_signal("Handshake error. " + error.message());
    state = WebsocketState::open;
    open_signal();
    ws.async_read(buffer, beast::bind_front_handler(&Websocket::on_read,
                                                    shared_from_this()));
}

void Websocket::on_read(beast::error_code error,
                        std::size_t bytes_transferred) {
    if (error) return error_signal("Read error. " + error.message());
    message_signal(beast::buffers_to_string(buffer.data()));
    buffer.clear();
    ws.async_read(buffer, beast::bind_front_handler(&Websocket::on_read,
                                                    shared_from_this()));
}

void Websocket::on_write(beast::error_code error,
                         std::size_t bytes_transferred) {
    if (error) return error_signal("Write error. " + error.message());
}

void Websocket::on_close(beast::error_code error) {
    if (error) return error_signal("Close error. " + error.message());
    state = WebsocketState::closed;
    close_signal();
}

}  // namespace aardvark
