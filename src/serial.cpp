/*
MIT License

Copyright (c) 2020 Alan Ramírez Herrera

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "serial/serial.hpp"

#include <utility>

serial::Serial::Serial(
    std::string port,
    std::uint32_t baud_rate,
    serial::ByteSize byte_size,
    serial::Parity parity,
    serial::StopBits stop_bits,
    serial::FlowControl flow_control,
    Timeout timeout)
    : m_port {std::move(port)}
    , m_baud_rate {baud_rate}
    , m_byte_size {byte_size}
    , m_parity {parity}
    , m_stop_bits {stop_bits}
    , m_flow_control {flow_control}
    , m_timeout {timeout}
{
    _open();
}

serial::Serial::Serial(serial::Serial &&rhs) noexcept
    : m_port {std::move(rhs.m_port)}
    , m_baud_rate {rhs.m_baud_rate}
    , m_byte_size {rhs.m_byte_size}
    , m_parity {rhs.m_parity}
    , m_stop_bits {rhs.m_stop_bits}
    , m_flow_control {rhs.m_flow_control}
    , m_native_handle {rhs.m_native_handle}
{
    rhs.m_native_handle = nullptr;
}

void serial::Serial::open(
    const std::string &port,
    std::uint32_t baudrate,
    serial::ByteSize byte_size,
    serial::Parity parity,
    serial::StopBits stop_bits,
    serial::FlowControl flow_control,
    Timeout timeout)
{
    // If native_handle is already open, we should throw an exception to inform that
    if (m_native_handle != nullptr)
    {
        // ToDo throw already open
    }

    m_port         = port;
    m_baud_rate    = baudrate;
    m_byte_size    = byte_size;
    m_parity       = parity;
    m_stop_bits    = stop_bits;
    m_flow_control = flow_control;
    m_timeout      = timeout;

    _open();
}

serial::Serial &serial::Serial::operator=(Serial &&rhs) noexcept
{
    if (this == &rhs)
        return *this;

    m_port          = std::move(rhs.m_port);
    m_baud_rate     = std::move(rhs.m_baud_rate);
    m_byte_size     = rhs.m_byte_size;
    m_parity        = rhs.m_parity;
    m_stop_bits     = rhs.m_stop_bits;
    m_flow_control  = rhs.m_flow_control;
    m_timeout       = rhs.m_timeout;
    m_native_handle = rhs.m_native_handle;
    return *this;
}
