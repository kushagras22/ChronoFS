#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace util
{

    class Sha256
    {
    public:
        Sha256();
        void update(const uint8_t *data, size_t len);
        void update(const std::string &s) { update(reinterpret_cast<const uint8_t *>(s.data()), s.size()); }
        std::array<uint8_t, 32> digest();
        static std::string toHex(const std::array<uint8_t, 32> &d);
        static std::string hashHex(const std::string &s)
        {
            Sha256 h;
            h.update(s);
            return toHex(h.digest());
        }

    private:
        void transform(const uint8_t *chunk);
        uint64_t bitlen_;
        std::array<uint8_t, 64> buffer_;
        size_t buffer_len_;
        uint32_t state_[8];
    };

}
