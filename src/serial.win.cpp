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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#pragma comment(lib, "Setupapi.lib")

#ifndef UNICODE
#define UNICODE 1
#endif

#include <Windows.h>
#include <algorithm>
#include <devguid.h>
#include <initguid.h>
#include <setupapi.h>

using namespace std::string_literals;

std::string utf8_encode(const std::wstring &wstring)
{
    auto size_needed = WideCharToMultiByte(CP_UTF8, 0, wstring.data(), wstring.size(), nullptr, 0, nullptr, nullptr);
    std::string str(size_needed, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstring.data(), wstring.size(), &str[0], size_needed, nullptr, nullptr);
    return str;
}

std::wstring wide_encode(const std::string &string)
{
    auto size_needed = MultiByteToWideChar(CP_UTF8, 0, string.data(), string.size(), nullptr, 0);
    std::wstring str(size_needed, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, string.data(), string.size(), &str[0], size_needed);
    return str;
}

std::vector<std::string> serial::list_ports()
{
    std::vector<std::string> devices;
    devices.reserve(20);

    GUID class_guid;

    DWORD required_size {};
    if (!SetupDiClassGuidsFromName(L"ports", reinterpret_cast<LPGUID>(&class_guid), 1, &required_size))
    {
        // ToDo throw
    }

    HDEVINFO device_info_set = SetupDiGetClassDevs((const GUID *)&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);

    SP_DEVINFO_DATA device_info_data;

    bool found = false;

    if (device_info_set)
    {
        DWORD member_index = 0;
        HKEY hkey {nullptr};
        TCHAR port_name[MAX_PATH];
        DWORD type;

        device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

        while (SetupDiEnumDeviceInfo(device_info_set, member_index, &device_info_data))
        {
            hkey = SetupDiOpenDevRegKey(device_info_set, &device_info_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);

            if (hkey)
            {
                long ret;
                required_size = sizeof(port_name);
                ret           = RegQueryValueEx(hkey, L"PortName", nullptr, &type, reinterpret_cast<LPBYTE>(&port_name), &required_size);
                RegCloseKey(hkey);
                devices.push_back(utf8_encode(port_name));
            }
            ++member_index;
        }
    }
    std::sort(std::begin(devices), std::end(devices));
    return devices;
}

void serial::Serial::_open()
{
    HANDLE serial_handle =
        CreateFile(wide_encode(R"(\\.\)" + m_port).c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    DCB serial_parameters       = {0};
    serial_parameters.DCBlength = sizeof(serial_parameters);
    serial_parameters.BaudRate  = m_baud_rate;
    serial_parameters.ByteSize  = static_cast<std::uint8_t>(m_byte_size);
    serial_parameters.StopBits  = static_cast<std::uint8_t>(m_stop_bits);
    serial_parameters.Parity    = static_cast<std::uint8_t>(m_parity);

    // ToDo: use flow control

    SetCommState(serial_handle, &serial_parameters);

    COMMTIMEOUTS timeout = {0};

    timeout.ReadIntervalTimeout         = m_timeout.read_interval_timeout.count();
    timeout.ReadTotalTimeoutConstant    = m_timeout.read_total_timeout_constant.count();
    timeout.ReadTotalTimeoutMultiplier  = m_timeout.read_total_timeout_multiplier.count();
    timeout.WriteTotalTimeoutConstant   = m_timeout.write_total_timeout_constant.count();
    timeout.WriteTotalTimeoutMultiplier = m_timeout.write_total_timeout_multiplier.count();

    SetCommTimeouts(serial_handle, &timeout);

    m_native_handle = static_cast<void *>(serial_handle);
}

void serial::Serial::close()
{
    if (m_native_handle != nullptr)
    {
        CloseHandle(static_cast<HANDLE>(m_native_handle));
        m_native_handle = nullptr;
    }
}

std::size_t serial::Serial::write(const std::uint8_t *data, std::size_t bytes_to_write)
{
    DWORD bytes_written;
    auto result = WriteFile(static_cast<HANDLE>(m_native_handle), data, bytes_to_write, &bytes_written, nullptr);
    return bytes_written;
}

serial::Serial::~Serial()
{
    // Close handle if open
    close();
}

std::size_t serial::Serial::read(std::uint8_t *data, std::size_t max_bytes)
{
    DWORD bytes_to_read;
    ReadFile(static_cast<HANDLE>(m_native_handle), data, max_bytes, &bytes_to_read, nullptr);
    return bytes_to_read;
}
