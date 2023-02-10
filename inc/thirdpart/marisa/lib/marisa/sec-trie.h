#ifndef MARISA_SEC_TRIE_H_
#define MARISA_SEC_TRIE_H_

#include "lib/marisa/trie.h"

namespace marisa {

class SecTrie {
  friend class SecTrieIO;

 public:
  SecTrie();
  ~SecTrie();

  void mmap(const char *filename);
  void map(const void *ptr, std::size_t size);

  void save(const char *filename) const;
  std::string cache() const;

  void build(Keyset &keyset, int config_flags = 0);
  bool lookup(Agent &agent) const;
  void reverse_lookup(Agent &agent) const;
  bool common_prefix_search(Agent &agent) const;
  bool predictive_search(Agent &agent) const;

  std::size_t num_tries() const;
  std::size_t num_keys() const;
  std::size_t num_nodes() const;

  TailMode tail_mode() const;
  NodeOrder node_order() const;

  bool empty() const;
  std::size_t size() const;
  std::size_t total_size() const;
  std::size_t io_size() const;

  void clear();
  void swap(SecTrie &rhs);

 private:
     marisa_uint64 crc64_;
    Trie trie_;

  // Disallows copy and assignment.
  SecTrie(const SecTrie &);
  SecTrie &operator=(const SecTrie &);
};

}  // namespace marisa

#endif  // MARISA_TRIE_H_
