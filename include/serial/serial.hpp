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

#ifndef SERIAL_SERIAL_HPP
#define SERIAL_SERIAL_HPP

#include <array>
#include <chrono>
#include <string>
#include <type_traits>
#include <vector>

#if __cplusplus >= 201703L
#include <string_view>
#endif

namespace serial
{
    /**
     * Parity types
     */
    enum class Parity : std::uint8_t
    {
        None = 0,
        Odd  = 1,
        Even = 2
    };

    /**
     * Serial port byte size
     */
    enum class ByteSize : std::uint8_t
    {
        Five  = 5,
        Six   = 6,
        Seven = 7,
        Eight = 8
    };

    /**
     * Number of stop bits
     */
    enum class StopBits : std::uint8_t
    {
        One = 1,
        Two = 2
    };

    /**
     * Flow control type
     */
    enum class FlowControl : std::uint8_t
    {
        None,
        Software,
        Hardware
    };

    /**
     *
     */
    struct Timeout
    {
        std::chrono::milliseconds read_interval_timeout          = std::chrono::milliseconds {50};
        std::chrono::milliseconds read_total_timeout_constant    = std::chrono::milliseconds {50};
        std::chrono::milliseconds read_total_timeout_multiplier  = std::chrono::milliseconds {50};
        std::chrono::milliseconds write_total_timeout_constant   = std::chrono::milliseconds {50};
        std::chrono::milliseconds write_total_timeout_multiplier = std::chrono::milliseconds {10};
    };

    /**
     * Serial/COM port
     */
    class Serial
    {
        /**
         * Internal open
         */
        void _open();

    public:
        Serial() = default;

        Serial(
            std::string port,
            std::uint32_t baud_rate,
            ByteSize byte_size       = ByteSize::Eight,
            Parity parity            = Parity::None,
            StopBits stop_bits       = StopBits::One,
            FlowControl flow_control = FlowControl::None,
            Timeout timeout          = {});

        Serial(Serial &&rhs) noexcept;

        Serial(const Serial &) = delete;

        Serial &operator=(const Serial &) = delete;

        Serial &operator=(Serial &&rhs) noexcept;

        void open(
            const std::string &port,
            std::uint32_t baudrate,
            ByteSize byte_size       = ByteSize::Eight,
            Parity parity            = Parity::None,
            StopBits stop_bits       = StopBits::One,
            FlowControl flow_control = FlowControl::None,
            Timeout timeout          = {});

        /**
         * Close serial port
         */
        void close();

        /**
         * Serial dtor, it closes serial port if open
         */
        ~Serial();

        /**
         * Read data from the serial port
         * @param data data buffer
         * @param max_bytes max number of bytes that can be read
         * @return number of bytes read
         */
        std::size_t read(std::uint8_t *data, std::size_t max_bytes);

        /**
         * Read data from the serial port
         * @param data data buffer
         * @return number of bytes read
         * @details The max number of bytes that can be read is data.size()
         */
        std::size_t read(std::string &data)
        {
            return read(reinterpret_cast<std::uint8_t *>(&data[0]), data.size());
        }

        /**
         * Read data from the serial port
         * @param data data buffer
         * @return number of bytes read
         * @details The max number of bytes that can be read is data.size()
         */
        std::size_t read(std::vector<std::uint8_t> &data)
        {
            return read(reinterpret_cast<std::uint8_t *>(data.data()), data.size());
        }

        /**
         * Read data from the serial port
         * @tparam size array size
         * @param data data buffer
         * @return number of bytes read
         * @details The max number of bytes that can be read is data.size()
         */
        template <std::size_t size>
        std::size_t read(std::array<std::uint8_t, size> &data)
        {
            read(data.data(), data.size());
        }

        /**
         * Read data from the serial port
         * @tparam size array size
         * @param data data buffer
         * @return read bytes
         * @details The max number of bytes that can be read is template param 'size'
         */
        template <std::size_t size>
        std::size_t read(std::uint8_t data[size])
        {
            read(data, size);
        }

        /**
         * Write data to the serial port
         * @param data data to be written
         * @param bytes_to_write max number of bytes to write
         * @return written bytes
         */
        std::size_t write(const std::uint8_t *data, std::size_t bytes_to_write);

        /**
         * Write data
         * @tparam T Type
         * @param data data to be written
         * @return written bytes
         * @remarks T needs to be a trivial type
         */
        template <typename T>
        std::size_t write(const T &data)
        {
            static_assert(std::is_trivial<T>::value, "T needs to be a trivial type");
            return write(reinterpret_cast<const std::uint8_t *>(data), sizeof(T));
        }

        /**
         * Write a std string
         * @param data string to be written
         * @return written bytes
         */
        template <>
        std::size_t write<std::string>(const std::string &data)
        {
            return write(reinterpret_cast<const std::uint8_t *>(data.data()), data.size());
        }

        /**
         * Write a vector of data
         * @tparam U Vector type
         * @param vector data to be written
         * @return written bytes
         * @remarks U needs to be a trivial type
         */
        template <typename U>
        std::size_t write(const std::vector<U> &vector)
        {
            static_assert(std::is_trivial<U>::value, "vector<U>: U needs to be a trivial type");
            return write(reinterpret_cast<const std::uint8_t *>(vector.data()), sizeof(U) * vector.size());
        }

        /**
         * Write an array of trivial data
         * @tparam U Array type
         * @tparam size array size
         * @param array array
         * @return written bytes
         * @remarks U needs to be a trivial type
         */
        template <typename U, std::size_t size>
        std::size_t write(const std::array<U, size> &array)
        {
            return write(reinterpret_cast<const std::uint8_t *>(array.data()), sizeof(U) * array.size());
        }

        /**
         * Gets native handle
         * @return
         */
        auto native_handle() const
        {
            return m_native_handle;
        }

    private:
        std::string m_port;
        Timeout m_timeout {};
        std::uint32_t m_baud_rate {};
        ByteSize m_byte_size {};
        Parity m_parity {};
        StopBits m_stop_bits {};
        FlowControl m_flow_control {};
        void *m_native_handle {nullptr};
    };

    /**
     * List all serial ports
     * @return List of serial ports
     */
    [[nodiscard]] std::vector<std::string> list_ports();
} // namespace serial

#endif // SERIAL_SERIAL_HPP
