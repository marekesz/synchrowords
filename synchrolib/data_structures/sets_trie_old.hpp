#pragma once
// This code is taken from the older version of the algorithm.
// It is used mainly for comparison.
#include <vector>

#include "synchrolib/data_structures/automaton.hpp"
#include "synchrolib/data_structures/subset.hpp"
#include "synchrolib/utils/general.hpp"

namespace synchrolib {
// ***************************************************************************

struct SetsTrieNode {
  int32_t zero, one;
  SetsTrieNode() : zero(0), one(0) {}
};

template <int N>
struct SetsTrie {
  FastVector<SetsTrieNode> nodes;
  FastVector<Subset<N> > setnodes;

  // ***********************************************************************
  // Construction

  SetsTrie() { clear(); }

  void clear() {
    nodes.clear();
    nodes.shrink_to_fit();
    setnodes.clear();
    setnodes.shrink_to_fit();
    nodes.push_back(SetsTrieNode());
    setnodes.push_back(Subset<N>());
  }

  void free() { clear(); }

  // ***********************************************************************
  // Insert

  __attribute__((hot)) inline void insert_leaf_double(const Subset<N> subset1,
      const Subset<N> subset2, uint n, int node) {
    for (;; n++) {
      if (subset1.is_set(n)) {
        if (subset2.is_set(n)) {
          node = nodes[node].one = nodes.size();
          nodes.push_back(SetsTrieNode());
        } else {
          nodes[node].one = -setnodes.size();
          setnodes.push_back(subset1);
          nodes[node].zero = -setnodes.size();
          setnodes.push_back(subset2);
          return;
        }
      } else {
        if (subset2.is_set(n)) {
          nodes[node].zero = -setnodes.size();
          setnodes.push_back(subset1);
          nodes[node].one = -setnodes.size();
          setnodes.push_back(subset2);
          return;
        } else {
          node = nodes[node].zero = nodes.size();
          nodes.push_back(SetsTrieNode());
        }
      }
    }
  }

  __attribute__((hot)) inline void insert_compress_subset(
      const Subset<N> subset, uint n, int node) {
    for (;; n++) {
      if (subset.is_set(n)) {
        int oldnode = nodes[node].one;
        if (oldnode > 0)
          node = oldnode;
        else {
          if (oldnode < 0) {
            if (!setnodes[-oldnode].is_subset(subset)) {
              nodes[node].one = nodes.size();
              nodes.push_back(SetsTrieNode());
              insert_leaf_double(
                  subset, setnodes[-oldnode], n + 1, nodes[node].one);
            } else {
              setnodes[-oldnode] = subset;
            }
          } else {
            nodes[node].one = -setnodes.size();
            setnodes.push_back(subset);
          }
          return;
        }
      } else {
        int oldnode = nodes[node].zero;
        if (oldnode > 0)
          node = oldnode;
        else {
          if (oldnode < 0) {
            if (!setnodes[-oldnode].is_subset(subset)) {
              nodes[node].zero = nodes.size();
              nodes.push_back(SetsTrieNode());
              insert_leaf_double(
                  subset, setnodes[-oldnode], n + 1, nodes[node].zero);
            } else {
              setnodes[-oldnode] = subset;
            }
          } else {
            nodes[node].zero = -setnodes.size();
            setnodes.push_back(subset);
          }
          return;
        }
      }
    }
  }

  __attribute__((hot)) inline void insert_compress_superset(
      const Subset<N> subset, uint n, int node) {
    for (;; n++) {
      if (subset.is_set(n)) {
        int oldnode = nodes[node].one;
        if (oldnode > 0)
          node = oldnode;
        else {
          if (oldnode < 0) {
            if (!subset.is_subset(setnodes[-oldnode])) {
              nodes[node].one = nodes.size();
              nodes.push_back(SetsTrieNode());
              insert_leaf_double(
                  subset, setnodes[-oldnode], n + 1, nodes[node].one);
            } else {
              setnodes[-oldnode] = subset;
            }
          } else {
            nodes[node].one = -setnodes.size();
            setnodes.push_back(subset);
          }
          return;
        }
      } else {
        int oldnode = nodes[node].zero;
        if (oldnode > 0)
          node = oldnode;
        else {
          if (oldnode < 0) {
            if (!subset.is_subset(setnodes[-oldnode])) {
              nodes[node].zero = nodes.size();
              nodes.push_back(SetsTrieNode());
              insert_leaf_double(
                  subset, setnodes[-oldnode], n + 1, nodes[node].zero);
            } else {
              setnodes[-oldnode] = subset;
            }
          } else {
            nodes[node].zero = -setnodes.size();
            setnodes.push_back(subset);
          }
          return;
        }
      }
    }
  }

  __attribute__((hot)) inline void insert(
      const Subset<N> subset, uint n, int node) {
    for (;; n++) {
      if (subset.is_set(n)) {
        int oldnode = nodes[node].one;
        if (oldnode > 0)
          node = oldnode;
        else {
          if (oldnode < 0) {
            nodes[node].one = nodes.size();
            nodes.push_back(SetsTrieNode());
            insert_leaf_double(
                subset, setnodes[-oldnode], n + 1, nodes[node].one);
          } else {
            nodes[node].one = -setnodes.size();
            setnodes.push_back(subset);
          }
          return;
        }
      } else {
        int oldnode = nodes[node].zero;
        if (oldnode > 0)
          node = oldnode;
        else {
          if (oldnode < 0) {
            nodes[node].zero = nodes.size();
            nodes.push_back(SetsTrieNode());
            insert_leaf_double(
                subset, setnodes[-oldnode], n + 1, nodes[node].zero);
          } else {
            nodes[node].zero = -setnodes.size();
            setnodes.push_back(subset);
          }
          return;
        }
      }
    }
  }

  inline void insert_compress_subset(const Subset<N> &subset) {
    insert_compress_subset(subset, 0, 0);
  }

  inline void insert_compress_superset(const Subset<N> &subset) {
    insert_compress_superset(subset, 0, 0);
  }

  inline void insert(const Subset<N> &subset) { insert(subset, 0, 0); }

  inline void insert(const FastVector<Subset<N> > &list) {
    for (uint i = 0; i < list.size(); i++) insert(list[i]);
  }

  // ***********************************************************************
  // Searching

  __attribute__((pure)) bool contains_set(const Subset<N> &set,
      const uint n, const uint node) const {
    if (set.is_set(n)) {
      if (nodes[node].one) {
        if (nodes[node].one > 0)
          return contains_set(set, n + 1, nodes[node].one);
        return (setnodes[-nodes[node].one] == set);
      }
    } else {
      if (nodes[node].zero) {
        if (nodes[node].zero > 0)
          return contains_set(set, n + 1, nodes[node].zero);
        return (setnodes[-nodes[node].zero] == set);
      }
    }
    return false;
  }

  __attribute__((pure)) inline bool contains_set(const Subset<N> &set) const {
    return contains_set(set, 0, 0);
  }

  // ***************************************************************************
  // Content

  void get_sets_list(const int node, FastVector<Subset<N> > &list) const {
    if (node < 0)
      list.push_back(setnodes[-node]);
    else {
      if (nodes[node].zero) get_sets_list(nodes[node].zero, list);
      if (nodes[node].one) get_sets_list(nodes[node].one, list);
    }
  }

  inline void get_sets_list(FastVector<Subset<N> > &list) const {
    get_sets_list(0, list);
  }

  void get_sets_list(
      const int node, Subset<N> *list, uint &size) const {
    if (node < 0)
      list[size++] = setnodes[-node];
    else {
      if (nodes[node].zero) get_sets_list(nodes[node].zero, list, size);
      if (nodes[node].one) get_sets_list(nodes[node].one, list, size);
    }
  }

  inline void get_sets_list(Subset<N> *list, uint &size) const {
    get_sets_list(0, list, size);
  }

  void get_sets_list(const int node, Subset<N> *list, uint &size,
      const uint max_size) const {
    if (size >= max_size) return;
    if (node < 0)
      list[size++] = setnodes[-node];
    else {
      if (nodes[node].zero)
        get_sets_list(nodes[node].zero, list, size, max_size);
      if (nodes[node].one) get_sets_list(nodes[node].one, list, size, max_size);
    }
  }

  inline void get_sets_list(
      Subset<N> *list, uint &size, const uint max_size) const {
    get_sets_list(0, list, size, max_size);
  }

  __attribute__((pure)) uint get_sets_count(const int node) const {
    if (node < 0) return 1;
    return (nodes[node].one ? get_sets_count(nodes[node].one) : 0) +
        (nodes[node].zero ? get_sets_count(nodes[node].zero) : 0);
  }

  __attribute__((pure)) inline uint get_sets_count() const {
    return get_sets_count(0);
  }

  void get_cardinalities(const int node, uint &sets_count,
      unsigned long long &card) const {
    if (node < 0) {
      sets_count++;
      card += setnodes[-node].size();
      return;
    }
    if (nodes[node].one) get_cardinalities(nodes[node].one, sets_count, card);
    if (nodes[node].zero) get_cardinalities(nodes[node].zero, sets_count, card);
  }

  inline void get_cardinalities(
      uint &sets_count, unsigned long long &card) const {
    get_cardinalities(0, sets_count, card);
  }

  // ***********************************************************************
};

// ***************************************************************************

template <int N, int B, typename V>
struct SetsTrieMap {
  struct SetMapNode {
    Subset<N> set;
    V val;
  };

  FastVector<SetsTrieNode> nodes;
  FastVector<SetMapNode> setnodes;

  // ***********************************************************************
  // Construction

  SetsTrieMap() { clear(); }

  void clear() {
    nodes.clear();
    setnodes.clear();
    nodes.push_back(SetsTrieNode());
    SetMapNode new_node;
    setnodes.push_back(new_node);
    nodes.shrink_to_fit();
    setnodes.shrink_to_fit();
  }

  void free() { clear(); }

  // ***********************************************************************
  // Insert

  __attribute__((hot)) inline void insert_leaf_double(
      const int setindex1, const int setindex2, uint n, int node) {
    for (;; n++) {
      if (setnodes[setindex1].set.is_set(n)) {
        if (setnodes[setindex2].set.is_set(n)) {
          node = nodes[node].one = nodes.size();
          nodes.push_back(SetsTrieNode());
        } else {
          nodes[node].one = -setindex1;
          nodes[node].zero = -setindex2;
          return;
        }
      } else {
        if (setnodes[setindex2].set.is_set(n)) {
          nodes[node].zero = -setindex1;
          nodes[node].one = -setindex2;
          return;
        } else {
          node = nodes[node].zero = nodes.size();
          nodes.push_back(SetsTrieNode());
        }
      }
    }
  }

  __attribute__((hot)) inline void insert(
      const Subset<N> subset, uint n, int node, const V val) {
    for (;; n++) {
      if (subset.is_set(n)) {
        int oldnode = nodes[node].one;
        if (oldnode > 0)
          node = oldnode;
        else {
          SetMapNode new_node;
          new_node.set = subset;
          new_node.val = val;
          setnodes.push_back(new_node);
          if (oldnode < 0) {
            nodes[node].one = nodes.size();
            nodes.push_back(SetsTrieNode());
            insert_leaf_double(
                setnodes.size() - 1, -oldnode, n + 1, nodes[node].one);
          } else {
            nodes[node].one = -(setnodes.size() - 1);
          }
          return;
        }
      } else {
        int oldnode = nodes[node].zero;
        if (oldnode > 0)
          node = oldnode;
        else {
          SetMapNode new_node;
          new_node.set = subset;
          new_node.val = val;
          setnodes.push_back(new_node);
          if (oldnode < 0) {
            nodes[node].zero = nodes.size();
            nodes.push_back(SetsTrieNode());
            insert_leaf_double(
                setnodes.size() - 1, -oldnode, n + 1, nodes[node].zero);
          } else {
            nodes[node].zero = -(setnodes.size() - 1);
          }
          return;
        }
      }
    }
  }

  __attribute__((hot)) inline void insert_val(
      const Subset<N> subset, uint n, int node, const V val) {
    for (;; n++) {
      if (subset.is_set(n)) {
        int oldnode = nodes[node].one;
        if (oldnode > 0)
          node = oldnode;
        else {
          if (oldnode < 0) {
            setnodes[-oldnode].val = val;
          }
          return;
        }
      } else {
        int oldnode = nodes[node].zero;
        if (oldnode > 0)
          node = oldnode;
        else {
          if (oldnode < 0) {
            setnodes[-oldnode].val = val;
          }
          return;
        }
      }
    }
  }

  inline void insert(const Subset<N> &subset, V val) {
    insert(subset, 0, 0, val);
  }

  inline void insert_val(const Subset<N> &subset, V val) {
    insert_val(subset, 0, 0, val);
  }

  // ***********************************************************************
  // Searching

  bool contains_set(const Subset<N> &set, const uint n,
      const uint node, V &val) const {
    if (set.is_set(n)) {
      if (nodes[node].one) {
        if (nodes[node].one > 0)
          return contains_set(set, n + 1, nodes[node].one, val);
        if (setnodes[-nodes[node].one].set == set) {
          val = setnodes[-nodes[node].one].val;
          return true;
        } else
          return false;
      }
    } else {
      if (nodes[node].zero) {
        if (nodes[node].zero > 0)
          return contains_set(set, n + 1, nodes[node].zero, val);
        if (setnodes[-nodes[node].zero].set == set) {
          val = setnodes[-nodes[node].zero].val;
          return true;
        } else
          return false;
      }
    }
    return false;
  }

  inline bool contains_set(const Subset<N> &set, V &val) const {
    return contains_set(set, 0, 0, val);
  }

  // ***************************************************************************
  // Content

  void get_sets_list(const int node, FastVector<Subset<N> > &list) const {
    if (node < 0)
      list.push_back(setnodes[-node].set);
    else {
      if (nodes[node].zero) get_sets_list(nodes[node].zero, list);
      if (nodes[node].one) get_sets_list(nodes[node].one, list);
    }
  }

  inline void get_sets_list(FastVector<Subset<N> > &list) const {
    get_sets_list(0, list);
  }

  void get_sets_list(
      const int node, Subset<N> *list, uint &size) const {
    if (node < 0)
      list[size++] = setnodes[-node].set;
    else {
      if (nodes[node].zero) get_sets_list(nodes[node].zero, list, size);
      if (nodes[node].one) get_sets_list(nodes[node].one, list, size);
    }
  }

  inline void get_sets_list(Subset<N> *list, uint &size) const {
    get_sets_list(0, list, size);
  }

  void get_sets_list(const int node, Subset<N> *list, uint &size,
      const uint max_size) const {
    if (size >= max_size) return;
    if (node < 0)
      list[size++] = setnodes[-node].set;
    else {
      if (nodes[node].zero)
        get_sets_list(nodes[node].zero, list, size, max_size);
      if (nodes[node].one) get_sets_list(nodes[node].one, list, size, max_size);
    }
  }

  inline void get_sets_list(
      Subset<N> *list, uint &size, const uint max_size) const {
    get_sets_list(0, list, size, max_size);
  }

  __attribute__((pure)) uint get_sets_count(const int node) const {
    if (node < 0) return 1;
    return (nodes[node].one ? get_sets_count(nodes[node].one) : 0) +
        (nodes[node].zero ? get_sets_count(nodes[node].zero) : 0);
  }

  __attribute__((pure)) inline uint get_sets_count() const {
    return get_sets_count(0);
  }

  void get_cardinalities(const int node, uint &sets_count,
      unsigned long long &card) const {
    if (node < 0) {
      sets_count++;
      card += setnodes[-node].size();
      return;
    }
    if (nodes[node].one) get_cardinalities(nodes[node].one, sets_count, card);
    if (nodes[node].zero) get_cardinalities(nodes[node].zero, sets_count, card);
  }

  inline void get_cardinalities(
      uint &sets_count, unsigned long long &card) const {
    get_cardinalities(0, sets_count, card);
  }

  // ***********************************************************************
};

// ***************************************************************************
}  // namespace synchrolib
