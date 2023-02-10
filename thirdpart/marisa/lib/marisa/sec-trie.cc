#include "lib/marisa/stdio.h"
#include "lib/marisa/iostream.h"
#include "lib/marisa/sec-trie.h"
#include <algorithm>
#define _CRT_SECURE_NO_WARNINGS


namespace marisa {

marisa_uint64 getsimplehash(const unsigned char* data, size_t len) {
    marisa_uint64 result = 0;
    marisa_uint32 block_count = (len / sizeof(marisa_uint32));
    marisa_uint32 left = (len % sizeof(marisa_uint32));
    marisa_uint32* p32 = (marisa_uint32*)data;
    marisa_uint64 d = 0;
    marisa_uint32 len_2 = (len / 2 );
    marisa_uint32 len_3 = (len / 3 );
    for(size_t i  = 0; i < block_count; i++) {
        d = len_3 + p32[i] + i;
        d = d * (i + 1);
        result += d;
    }
    for(size_t i = sizeof(marisa_uint32) * block_count; i < len; i++) {
        d = len_2 + data[i] + i;
        d = d * (i + 1);
        result += d;
    }
    return result;
}

SecTrie::SecTrie() : crc64_(0), trie_() {}

SecTrie::~SecTrie() {}

void SecTrie::build(Keyset &keyset, int config_flags) {
    trie_.build(keyset, config_flags);
}

void SecTrie::mmap(const char *filename) {
  grimoire::Mapper mapper;
  mapper.open(filename);
  const unsigned char* base = (const unsigned char*)mapper.origin();
  std::size_t len = mapper.avail();
  MARISA_THROW_IF(len <= sizeof(crc64_), MARISA_SIZE_ERROR);
  memcpy(&crc64_, base, sizeof(crc64_));
  marisa_uint64 crc64 = getsimplehash(base + sizeof(crc64_), len -  sizeof(crc64_));
  if(crc64 != crc64_) {
        char* pos = (char*)(&crc64);
        std::reverse(pos, pos + 4);
        std::reverse(pos + 4, pos + sizeof(crc64));
  }
  MARISA_THROW_IF(crc64 != crc64_, MARISA_FORMAT_ERROR);
  mapper.seek(sizeof(crc64_));
  trie_.map(mapper);
}

void SecTrie::map(const void *ptr, std::size_t size) {
  grimoire::Mapper mapper;
  mapper.open(ptr, size);
  const unsigned char* base = (const unsigned char*)mapper.origin();
  std::size_t len = mapper.avail();
  MARISA_THROW_IF(len <= sizeof(crc64_), MARISA_SIZE_ERROR);
  memcpy(&crc64_, base, sizeof(crc64_));
  marisa_uint64 crc64 = getsimplehash(base + sizeof(crc64_), len -  sizeof(crc64_));
  if(crc64 != crc64_) {
        char* pos = (char*)(&crc64);
        std::reverse(pos, pos + 4);
        std::reverse(pos + 4, pos + sizeof(crc64));
  }
  MARISA_THROW_IF(crc64 != crc64_, MARISA_FORMAT_ERROR);
  mapper.seek(sizeof(crc64_));
  trie_.map(mapper);
}

void SecTrie::save(const char *filename) const {
  std::string trie_cache = trie_.cache();
  const unsigned char* base = (const unsigned char*)trie_cache.data();
  std::size_t len = trie_cache.size();
  marisa_uint64 crc64 = getsimplehash(base, len);
  FILE* file = NULL;
#ifdef WIN32
  fopen_s(&file, filename, "wb");
#else
  FILE* file = fopen(filename, "wb");
#endif

  if(file == NULL) {
      return;
  }
  fwrite(&crc64,  1,  sizeof(crc64), file);
  trie_.save(file);
  fclose(file);
}

std::string SecTrie::cache() const  {
    std::string trie_cache = trie_.cache();
    std::string cache;
    cache.reserve(trie_cache.size() + sizeof(crc64_));
    cache.append((const char*)&crc64_, sizeof(crc64_)).append(trie_cache);
    return cache;
}

bool SecTrie::lookup(Agent &agent) const {
  return trie_.lookup(agent);
}

void SecTrie::reverse_lookup(Agent &agent) const {
  trie_.reverse_lookup(agent);
}

bool SecTrie::common_prefix_search(Agent &agent) const {
  return trie_.common_prefix_search(agent);
}

bool SecTrie::predictive_search(Agent &agent) const {
  return trie_.predictive_search(agent);
}

std::size_t SecTrie::num_tries() const {
  return trie_.num_tries();
}

std::size_t SecTrie::num_keys() const {
  return trie_.num_keys();
}

std::size_t SecTrie::num_nodes() const {
  return trie_.num_nodes();
}

TailMode SecTrie::tail_mode() const {
  return trie_.tail_mode();
}

NodeOrder SecTrie::node_order() const {
  return trie_.node_order();
}

bool SecTrie::empty() const {
  return trie_.empty();
}

std::size_t SecTrie::size() const {
  return trie_.size();
}

std::size_t SecTrie::total_size() const {
  return trie_.total_size();
}

std::size_t SecTrie::io_size() const {
  return trie_.io_size();
}

void SecTrie::clear() {
  SecTrie().swap(*this);
}

void SecTrie::swap(SecTrie &rhs)  {
  trie_.swap(rhs.trie_);
  crc64_ = rhs.crc64_;
}


}  // namespace marisa
