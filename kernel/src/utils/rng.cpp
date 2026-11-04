#include <limine/requests.hpp>
#include <utils/rng.hpp>
#include <utils/panic.hpp>

namespace utils {
namespace {

constexpr std::uint8_t domain[] = {'p', 'r', 'o', 't', 'e',
                                   'c', 't', 'e', 'd', '!'};

void secure_bzero(void *ptr, std::size_t len) {
  volatile std::uint8_t *p = static_cast<volatile std::uint8_t *>(ptr);

  while (len--) {
    *p++ = 0;
  }
}

constexpr std::uint32_t rotr32(std::uint32_t x, unsigned n) {
  return (x >> n) | (x << (32u - n));
}

constexpr std::uint32_t rotl32(std::uint32_t x, unsigned n) {
  return (x << n) | (x >> (32u - n));
}

constexpr std::uint32_t load32_be(const std::uint8_t *p) {
  return (static_cast<std::uint32_t>(p[0]) << 24) |
         (static_cast<std::uint32_t>(p[1]) << 16) |
         (static_cast<std::uint32_t>(p[2]) << 8) |
         static_cast<std::uint32_t>(p[3]);
}

constexpr std::uint32_t load32_le(const std::uint8_t *p) {
  return static_cast<std::uint32_t>(p[0]) |
         (static_cast<std::uint32_t>(p[1]) << 8) |
         (static_cast<std::uint32_t>(p[2]) << 16) |
         (static_cast<std::uint32_t>(p[3]) << 24);
}

void store32_be(std::uint8_t *p, std::uint32_t x) {
  p[0] = static_cast<std::uint8_t>(x >> 24);
  p[1] = static_cast<std::uint8_t>(x >> 16);
  p[2] = static_cast<std::uint8_t>(x >> 8);
  p[3] = static_cast<std::uint8_t>(x);
}

class sha256 {
public:
  sha256() {
    m_state[0] = 0x6a09e667u;
    m_state[1] = 0xbb67ae85u;
    m_state[2] = 0x3c6ef372u;
    m_state[3] = 0xa54ff53au;
    m_state[4] = 0x510e527fu;
    m_state[5] = 0x9b05688cu;
    m_state[6] = 0x1f83d9abu;
    m_state[7] = 0x5be0cd19u;
    bit_count_ = 0;
    buffer_len_ = 0;
    secure_bzero(buffer_, sizeof(buffer_));
  }

  ~sha256() { secure_bzero(this, sizeof(*this)); }

  void update(const void *data, std::size_t len) {
    const std::uint8_t *src = static_cast<const std::uint8_t *>(data);

    bit_count_ += static_cast<std::uint64_t>(len) * 8u;

    while (len != 0) {
      std::size_t n = 64 - buffer_len_;

      if (n > len) {
        n = len;
      }

      for (std::size_t i = 0; i < n; ++i) {
        buffer_[buffer_len_ + i] = src[i];
      }

      buffer_len_ += n;
      src += n;
      len -= n;

      if (buffer_len_ == 64) {
        transform(buffer_);
        buffer_len_ = 0;
      }
    }
  }

  void update_u64_le(std::uint64_t value) {
    std::uint8_t bytes[8];

    for (std::size_t i = 0; i < 8; ++i) {
      bytes[i] = static_cast<std::uint8_t>(value >> (i * 8));
    }

    update(bytes, sizeof(bytes));

    secure_bzero(bytes, sizeof(bytes));
  }

  void final(std::uint8_t out[32]) {
    std::size_t i = buffer_len_;

    buffer_[i++] = 0x80;

    if (i > 56) {
      while (i < 64) {
        buffer_[i++] = 0;
      }

      transform(buffer_);
      i = 0;
    }

    while (i < 56) {
      buffer_[i++] = 0;
    }

    std::uint64_t bits = bit_count_;

    for (std::size_t j = 0; j < 8; ++j) {
      buffer_[56 + j] = static_cast<std::uint8_t>(bits >> (56 - j * 8));
    }

    transform(buffer_);

    for (std::size_t j = 0; j < 8; ++j) {
      store32_be(out + j * 4, m_state[j]);
    }

    secure_bzero(this, sizeof(*this));
  }

private:
  static constexpr std::uint32_t ch(std::uint32_t x, std::uint32_t y,
                                    std::uint32_t z) {
    return (x & y) ^ (~x & z);
  }

  static constexpr std::uint32_t maj(std::uint32_t x, std::uint32_t y,
                                     std::uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
  }

  static constexpr std::uint32_t bsig0(std::uint32_t x) {
    return rotr32(x, 2) ^ rotr32(x, 13) ^ rotr32(x, 22);
  }

  static constexpr std::uint32_t bsig1(std::uint32_t x) {
    return rotr32(x, 6) ^ rotr32(x, 11) ^ rotr32(x, 25);
  }

  static constexpr std::uint32_t ssig0(std::uint32_t x) {
    return rotr32(x, 7) ^ rotr32(x, 18) ^ (x >> 3);
  }

  static constexpr std::uint32_t ssig1(std::uint32_t x) {
    return rotr32(x, 17) ^ rotr32(x, 19) ^ (x >> 10);
  }

  void transform(const std::uint8_t block[64]) {
    static constexpr std::uint32_t k[64] = {
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu,
        0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u, 0xd807aa98u, 0x12835b01u,
        0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u,
        0xc19bf174u, 0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
        0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau, 0x983e5152u,
        0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u,
        0x06ca6351u, 0x14292967u, 0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu,
        0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
        0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u,
        0xd6990624u, 0xf40e3585u, 0x106aa070u, 0x19a4c116u, 0x1e376c08u,
        0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu,
        0x682e6ff3u, 0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
        0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u};

    std::uint32_t w[64];

    for (std::size_t i = 0; i < 16; ++i) {
      w[i] = load32_be(block + i * 4);
    }

    for (std::size_t i = 16; i < 64; ++i) {
      w[i] = ssig1(w[i - 2]) + w[i - 7] + ssig0(w[i - 15]) + w[i - 16];
    }

    std::uint32_t a = m_state[0];
    std::uint32_t b = m_state[1];
    std::uint32_t c = m_state[2];
    std::uint32_t d = m_state[3];
    std::uint32_t e = m_state[4];
    std::uint32_t f = m_state[5];
    std::uint32_t g = m_state[6];
    std::uint32_t h = m_state[7];

    for (std::size_t i = 0; i < 64; ++i) {
      std::uint32_t t1 = h + bsig1(e) + ch(e, f, g) + k[i] + w[i];
      std::uint32_t t2 = bsig0(a) + maj(a, b, c);

      h = g;
      g = f;
      f = e;
      e = d + t1;
      d = c;
      c = b;
      b = a;
      a = t1 + t2;
    }

    m_state[0] += a;
    m_state[1] += b;
    m_state[2] += c;
    m_state[3] += d;
    m_state[4] += e;
    m_state[5] += f;
    m_state[6] += g;
    m_state[7] += h;

    secure_bzero(w, sizeof(w));
  }

  std::uint32_t m_state[8];
  std::uint64_t bit_count_;
  std::uint8_t buffer_[64];
  std::size_t buffer_len_;
};

void chacha_quarter_round(std::uint32_t &a, std::uint32_t &b, std::uint32_t &c,
                          std::uint32_t &d) {
  a += b;
  d ^= a;
  d = rotl32(d, 16);

  c += d;
  b ^= c;
  b = rotl32(b, 12);

  a += b;
  d ^= a;
  d = rotl32(d, 8);

  c += d;
  b ^= c;
  b = rotl32(b, 7);
}

void chacha20_block(const std::uint32_t in[16], std::uint8_t out[64]) {
  std::uint32_t x[16];

  for (std::size_t i = 0; i < 16; ++i) {
    x[i] = in[i];
  }

  for (std::size_t i = 0; i < 10; ++i) {
    chacha_quarter_round(x[0], x[4], x[8], x[12]);
    chacha_quarter_round(x[1], x[5], x[9], x[13]);
    chacha_quarter_round(x[2], x[6], x[10], x[14]);
    chacha_quarter_round(x[3], x[7], x[11], x[15]);

    chacha_quarter_round(x[0], x[5], x[10], x[15]);
    chacha_quarter_round(x[1], x[6], x[11], x[12]);
    chacha_quarter_round(x[2], x[7], x[8], x[13]);
    chacha_quarter_round(x[3], x[4], x[9], x[14]);
  }

  for (std::size_t i = 0; i < 16; ++i) {
    std::uint32_t v = x[i] + in[i];

    out[i * 4 + 0] = static_cast<std::uint8_t>(v);
    out[i * 4 + 1] = static_cast<std::uint8_t>(v >> 8);
    out[i * 4 + 2] = static_cast<std::uint8_t>(v >> 16);
    out[i * 4 + 3] = static_cast<std::uint8_t>(v >> 24);
  }

  secure_bzero(x, sizeof(x));
}

} // namespace

void rng::seed(std::span<const std::uint64_t> entropy) {
  if (entropy.empty())
    panic("no entropy");

  sha256 hash;
  std::uint8_t digest[32];

  hash.update(domain, sizeof(domain));

  for (std::uint64_t val : entropy) {
    hash.update_u64_le(val);
  }

  hash.final(digest);

  m_state[0] = 0x61707865u;
  m_state[1] = 0x3320646eu;
  m_state[2] = 0x79622d32u;
  m_state[3] = 0x6b206574u;

  for (std::size_t i = 0; i < 8; ++i) {
    m_state[4 + i] = load32_le(digest + i * 4);
  }

  m_state[12] = 0;
  m_state[13] = 0;
  m_state[14] = 0;
  m_state[15] = 0;

  secure_bzero(digest, sizeof(digest));
}

rng::~rng() { secure_bzero(m_state, sizeof(m_state)); }

void rng::rekey() {
  std::uint8_t material[64];
  std::uint8_t new_key[32];
  sha256 hash;

  chacha20_block(m_state, material);

  hash.update(domain, sizeof(domain));
  hash.update(material, sizeof(material));
  hash.final(new_key);

  for (std::size_t i = 0; i < 8; ++i) {
    m_state[4 + i] = load32_le(new_key + i * 4);
  }

  m_state[12] = 0;
  m_state[13] = 0;

  secure_bzero(material, sizeof(material));
  secure_bzero(new_key, sizeof(new_key));
}

void rng::fill_bytes(std::span<std::uint8_t> out) {
  if (out.empty()) {
    return;
  }

  std::uint8_t block[64];
  std::uint8_t *dst = out.data();
  std::size_t amount = out.size();

  while (amount != 0) {
    chacha20_block(m_state, block);

    ++m_state[12];

    if (m_state[12] == 0) {
      ++m_state[13];
    }

    std::size_t n = amount < sizeof(block) ? amount : sizeof(block);

    for (std::size_t i = 0; i < n; ++i) {
      dst[i] = block[i];
    }

    dst += n;
    amount -= n;

    secure_bzero(block, sizeof(block));
  }

  rekey();
}

void seed_generator::initialize() {
  auto *resp = limine::entropy_request.response;
  assert(resp && "entropy request must be fulfilled!");
  assert(resp->value_count >= 1 &&
         "entropy request must have atleast one value");

  std::span<const std::uint64_t> entropy{resp->values, resp->value_count};
  s_rng.seed(entropy);
}

void seed_generator::generate_seed(std::span<std::uint64_t> at) {
  // make an std::uint8_t span from at because fill bytes accepts
  // only std::uint8_t spans
  std::span<std::uint8_t> sp{reinterpret_cast<std::uint8_t *>(at.data()),
                             at.size() * sizeof(std::uint64_t)};

  s_rng.fill_bytes(sp);
}

rng seed_generator::s_rng{};
logging::named_logger seed_generator::s_logger{"seed_generator"};
} // namespace utils
