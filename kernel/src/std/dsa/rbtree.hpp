#pragma once

#include <algorithm>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>

#include <concepts>
#include <cstdint>

namespace std {
namespace detail {
enum class rb_color : std::uint8_t { red, black };
struct rb_node_base {
  rb_node_base *parent = nullptr;
  rb_node_base *left = nullptr;
  rb_node_base *right = nullptr;
  rb_color color = rb_color::red;
};

using base_ptr = rb_node_base *;

[[nodiscard]] inline base_ptr rb_min(base_ptr x) noexcept {
  while (x->left)
    x = x->left;
  return x;
}
[[nodiscard]] inline base_ptr rb_max(base_ptr x) noexcept {
  while (x->right)
    x = x->right;
  return x;
}

[[nodiscard]] inline base_ptr rb_increment(base_ptr x) noexcept {
  if (x->right)
    return rb_min(x->right);
  base_ptr y = x->parent;
  while (x == y->right) {
    x = y;
    y = y->parent;
  }
  if (x->right != y)
    x = y;
  return x;
}

[[nodiscard]] inline base_ptr rb_decrement(base_ptr x) noexcept {
  if (x->color == rb_color::red && x->parent->parent == x)
    return x->right;
  if (x->left)
    return rb_max(x->left);
  base_ptr y = x->parent;
  while (x == y->left) {
    x = y;
    y = y->parent;
  }
  return y;
}

inline void rb_rotate_left(base_ptr x, base_ptr &root) noexcept {
  base_ptr y = x->right;
  x->right = y->left;
  if (y->left)
    y->left->parent = x;
  y->parent = x->parent;
  if (x == root)
    root = y;
  else if (x == x->parent->left)
    x->parent->left = y;
  else
    x->parent->right = y;
  y->left = x;
  x->parent = y;
}

inline void rb_rotate_right(base_ptr x, base_ptr &root) noexcept {
  base_ptr y = x->left;
  x->left = y->right;
  if (y->right)
    y->right->parent = x;
  y->parent = x->parent;
  if (x == root)
    root = y;
  else if (x == x->parent->right)
    x->parent->right = y;
  else
    x->parent->left = y;
  y->right = x;
  x->parent = y;
}

inline void rb_insert_rebalance(bool insert_left, base_ptr x, base_ptr p,
                                rb_node_base &header) noexcept {
  using enum rb_color;
  base_ptr &root = header.parent;

  x->parent = p;
  x->left = x->right = nullptr;
  x->color = red;

  if (insert_left) {
    p->left = x;
    if (p == &header) {
      header.parent = x;
      header.right = x;
    } else if (p == header.left) {
      header.left = x;
    }
  } else {
    p->right = x;
    if (p == header.right)
      header.right = x;
  }

  while (x != root && x->parent->color == red) {
    base_ptr const xpp = x->parent->parent;
    if (x->parent == xpp->left) {
      base_ptr const y = xpp->right;
      if (y && y->color == red) {
        x->parent->color = black;
        y->color = black;
        xpp->color = red;
        x = xpp;
      } else {
        if (x == x->parent->right) {
          x = x->parent;
          rb_rotate_left(x, root);
        }
        x->parent->color = black;
        xpp->color = red;
        rb_rotate_right(xpp, root);
      }
    } else {
      base_ptr const y = xpp->left;
      if (y && y->color == red) {
        x->parent->color = black;
        y->color = black;
        xpp->color = red;
        x = xpp;
      } else {
        if (x == x->parent->left) {
          x = x->parent;
          rb_rotate_right(x, root);
        }
        x->parent->color = black;
        xpp->color = red;
        rb_rotate_left(xpp, root);
      }
    }
  }
  root->color = black;
}

inline base_ptr rb_erase_rebalance(base_ptr z, rb_node_base &header) noexcept {
  using enum rb_color;
  base_ptr &root = header.parent;
  base_ptr &leftmost = header.left;
  base_ptr &rightmost = header.right;

  base_ptr y = z;
  base_ptr x = nullptr;
  base_ptr x_parent = nullptr;

  if (!y->left)
    x = y->right;
  else if (!y->right)
    x = y->left;
  else {
    y = rb_min(y->right);
    x = y->right;
  }

  if (y != z) {
    z->left->parent = y;
    y->left = z->left;
    if (y != z->right) {
      x_parent = y->parent;
      if (x)
        x->parent = y->parent;
      y->parent->left = x;
      y->right = z->right;
      z->right->parent = y;
    } else {
      x_parent = y;
    }
    if (root == z)
      root = y;
    else if (z->parent->left == z)
      z->parent->left = y;
    else
      z->parent->right = y;
    y->parent = z->parent;
    std::swap(y->color, z->color);
    y = z;
  } else {
    x_parent = y->parent;
    if (x)
      x->parent = y->parent;
    if (root == z)
      root = x;
    else if (z->parent->left == z)
      z->parent->left = x;
    else
      z->parent->right = x;
    if (leftmost == z)
      leftmost = z->right ? rb_min(x) : z->parent;
    if (rightmost == z)
      rightmost = z->left ? rb_max(x) : z->parent;
  }

  if (y->color != red) {
    while (x != root && (x == nullptr || x->color == black)) {
      if (x == x_parent->left) {
        base_ptr w = x_parent->right;
        if (w->color == red) {
          w->color = black;
          x_parent->color = red;
          rb_rotate_left(x_parent, root);
          w = x_parent->right;
        }
        if ((!w->left || w->left->color == black) &&
            (!w->right || w->right->color == black)) {
          w->color = red;
          x = x_parent;
          x_parent = x_parent->parent;
        } else {
          if (!w->right || w->right->color == black) {
            w->left->color = black;
            w->color = red;
            rb_rotate_right(w, root);
            w = x_parent->right;
          }
          w->color = x_parent->color;
          x_parent->color = black;
          if (w->right)
            w->right->color = black;
          rb_rotate_left(x_parent, root);
          break;
        }
      } else {
        base_ptr w = x_parent->left;
        if (w->color == red) {
          w->color = black;
          x_parent->color = red;
          rb_rotate_right(x_parent, root);
          w = x_parent->left;
        }
        if ((!w->right || w->right->color == black) &&
            (!w->left || w->left->color == black)) {
          w->color = red;
          x = x_parent;
          x_parent = x_parent->parent;
        } else {
          if (!w->left || w->left->color == black) {
            w->right->color = black;
            w->color = red;
            rb_rotate_left(w, root);
            w = x_parent->left;
          }
          w->color = x_parent->color;
          x_parent->color = black;
          if (w->left)
            w->left->color = black;
          rb_rotate_right(x_parent, root);
          break;
        }
      }
    }
    if (x)
      x->color = black;
  }
  return y;
}

} // namespace detail

struct rb_identity {
  template <class T> constexpr T &operator()(T &x) const noexcept { return x; }
  template <class T> constexpr const T &operator()(const T &x) const noexcept {
    return x;
  }
};

struct rb_select_first {
  template <class P>
  constexpr const auto &operator()(const P &p) const noexcept {
    return p.first;
  }
};

template <class Value, class KeyOfValue = rb_identity,
          class Compare = std::less<>, class Allocator = std::allocator<Value>>

class rb_tree {
  static_assert(
      std::is_same_v<typename std::allocator_traits<Allocator>::value_type,
                     Value>,
      "Allocator::value_type must be the same as Value");

  using node_base = detail::rb_node_base;
  using base_ptr = detail::base_ptr;
  using rb_color = detail::rb_color;

  struct node : node_base {
    union {
      Value value;
    };
    node() noexcept {}
    ~node() {}
    node(const node &) = delete;
    node &operator=(const node &) = delete;
  };

  using alloc_traits = std::allocator_traits<Allocator>;
  using node_allocator = typename alloc_traits::template rebind_alloc<node>;
  using node_alloc_traits = std::allocator_traits<node_allocator>;

  static constexpr bool transparent_compare =
      requires { typename Compare::is_transparent; };

public:
  using value_type = Value;
  using key_type = std::remove_cvref_t<
      std::invoke_result_t<const KeyOfValue &, const Value &>>;
  using key_compare = Compare;
  using allocator_type = Allocator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = typename alloc_traits::pointer;
  using const_pointer = typename alloc_traits::const_pointer;

  class value_compare {
  public:
    constexpr bool operator()(const value_type &a, const value_type &b) const {
      return comp(KeyOfValue()(a), KeyOfValue()(b));
    }

  protected:
    friend class rb_tree;
    explicit value_compare(const Compare &c) : comp(c) {}
    Compare comp;
  };

  template <bool Const> class basic_iterator {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = Value;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<Const, const Value *, Value *>;
    using reference = std::conditional_t<Const, const Value &, Value &>;

    basic_iterator() noexcept = default;
    template <bool C>
      requires(Const && !C)
    basic_iterator(const basic_iterator<C> &o) noexcept : n_(o.n_) {}

    reference operator*() const noexcept {
      return static_cast<node *>(n_)->value;
    }
    pointer operator->() const noexcept {
      return std::addressof(static_cast<node *>(n_)->value);
    }

    basic_iterator &operator++() noexcept {
      n_ = detail::rb_increment(n_);
      return *this;
    }
    basic_iterator operator++(int) noexcept {
      auto t = *this;
      ++*this;
      return t;
    }
    basic_iterator &operator--() noexcept {
      n_ = detail::rb_decrement(n_);
      return *this;
    }
    basic_iterator operator--(int) noexcept {
      auto t = *this;
      --*this;
      return t;
    }

    friend bool operator==(const basic_iterator &a,
                           const basic_iterator &b) noexcept {
      return a.n_ == b.n_;
    }

  private:
    friend class rb_tree;
    template <bool> friend class basic_iterator;
    explicit basic_iterator(base_ptr n) noexcept : n_(n) {}
    base_ptr n_ = nullptr;
  };

  using const_iterator = basic_iterator<true>;
  using iterator = std::conditional_t<std::is_same_v<key_type, value_type>,
                                      const_iterator, basic_iterator<false>>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  rb_tree() : rb_tree(Compare(), Allocator()) {}

  explicit rb_tree(const Compare &comp, const Allocator &alloc = Allocator())
      : alloc_(node_allocator(alloc)), comp_(comp) {
    reset_header();
  }
  explicit rb_tree(const Allocator &alloc) : rb_tree(Compare(), alloc) {}

  template <std::input_iterator It>
  rb_tree(It first, It last, const Compare &comp = Compare(),
          const Allocator &alloc = Allocator())
      : rb_tree(comp, alloc) {
    insert(first, last);
  }
  template <std::input_iterator It>
  rb_tree(It first, It last, const Allocator &alloc)
      : rb_tree(first, last, Compare(), alloc) {}

  rb_tree(std::initializer_list<Value> il, const Compare &comp = Compare(),
          const Allocator &alloc = Allocator())
      : rb_tree(comp, alloc) {
    insert(il);
  }
  rb_tree(std::initializer_list<Value> il, const Allocator &alloc)
      : rb_tree(il, Compare(), alloc) {}

#if defined(__cpp_lib_containers_ranges)
  template <std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, Value>
  rb_tree(std::from_range_t, R &&rg, const Compare &comp = Compare(),
          const Allocator &alloc = Allocator())
      : rb_tree(comp, alloc) {
    insert_range(std::forward<R>(rg));
  }
#endif

  rb_tree(const rb_tree &o)
      : rb_tree(o, alloc_traits::select_on_container_copy_construction(
                       o.get_allocator())) {}

  rb_tree(const rb_tree &o, const std::type_identity_t<Allocator> &alloc)
      : rb_tree(o.comp_, alloc) {
    copy_from(o);
  }

  rb_tree(rb_tree &&o) noexcept(std::is_nothrow_move_constructible_v<Compare>)
      : alloc_(std::move(o.alloc_)), comp_(std::move(o.comp_)) {
    steal(o);
  }

  rb_tree(rb_tree &&o, const std::type_identity_t<Allocator> &alloc)
      : alloc_(node_allocator(alloc)), comp_(std::move(o.comp_)) {
    if (alloc_ == o.alloc_) {
      steal(o);
    } else {
      reset_header();
      move_elements(o);
    }
  }

  ~rb_tree() { clear(); }

  rb_tree &operator=(const rb_tree &o) {
    if (this == &o)
      return *this;
    clear();
    if constexpr (alloc_traits::propagate_on_container_copy_assignment::value)
      alloc_ = o.alloc_;
    comp_ = o.comp_;
    copy_from(o);
    return *this;
  }

  rb_tree &operator=(rb_tree &&o) noexcept(
      (alloc_traits::propagate_on_container_move_assignment::value ||
       alloc_traits::is_always_equal::value) &&
      std::is_nothrow_move_assignable_v<Compare>) {
    if (this == &o)
      return *this;
    clear();
    comp_ = std::move(o.comp_);
    if constexpr (alloc_traits::propagate_on_container_move_assignment::value) {
      alloc_ = std::move(o.alloc_);
      steal(o);
    } else if (alloc_ == o.alloc_) {
      steal(o);
    } else {
      move_elements(o);
    }
    return *this;
  }

  rb_tree &operator=(std::initializer_list<Value> il) {
    clear();
    insert(il);
    return *this;
  }

  [[nodiscard]] allocator_type get_allocator() const noexcept {
    return allocator_type(alloc_);
  }

  // ---- iterators ---------------------------------------------------------
  [[nodiscard]] iterator begin() noexcept { return iterator(header_.left); }
  [[nodiscard]] const_iterator begin() const noexcept {
    return const_iterator(header_.left);
  }
  [[nodiscard]] iterator end() noexcept { return iterator(&header_); }
  [[nodiscard]] const_iterator end() const noexcept {
    return const_iterator(&header_);
  }
  [[nodiscard]] reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }
  [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  [[nodiscard]] reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }
  [[nodiscard]] const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }
  [[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }
  [[nodiscard]] const_iterator cend() const noexcept { return end(); }
  [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
    return rbegin();
  }
  [[nodiscard]] const_reverse_iterator crend() const noexcept { return rend(); }

  [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
  [[nodiscard]] size_type size() const noexcept { return size_; }
  [[nodiscard]] size_type max_size() const noexcept {
    return node_alloc_traits::max_size(alloc_);
  }

  void clear() noexcept {
    erase_subtree(header_.parent);
    reset_header();
  }

  std::pair<iterator, bool> insert(const value_type &v) {
    return insert_unique_value(v);
  }
  std::pair<iterator, bool> insert(value_type &&v) {
    return insert_unique_value(std::move(v));
  }
  iterator insert(const_iterator hint, const value_type &v) {
    return insert_hint_value(hint, v);
  }
  iterator insert(const_iterator hint, value_type &&v) {
    return insert_hint_value(hint, std::move(v));
  }

  template <std::input_iterator It> void insert(It first, It last) {
    for (; first != last; ++first)
      emplace_hint(cend(), *first);
  }
  void insert(std::initializer_list<Value> il) { insert(il.begin(), il.end()); }

  template <std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, Value>
  void insert_range(R &&rg) {
    for (auto &&e : rg)
      emplace_hint(cend(), std::forward<decltype(e)>(e));
  }

  template <class... Args> std::pair<iterator, bool> emplace(Args &&...args) {
    node *n = create_node(std::forward<Args>(args)...);
    insert_pos pos;

    pos = get_insert_unique_pos(key_of(n));

    if (pos.existing) {
      destroy_node(n);
      return {iterator(pos.existing), false};
    }
    return {link_node(pos, n), true};
  }

  template <class... Args>
  iterator emplace_hint(const_iterator hint, Args &&...args) {
    node *n = create_node(std::forward<Args>(args)...);
    insert_pos pos;

    pos = get_insert_hint_unique_pos(hint.n_, key_of(n));

    if (pos.existing) {
      destroy_node(n);
      return iterator(pos.existing);
    }
    return link_node(pos, n);
  }

  iterator erase(const_iterator pos) noexcept {
    assert(pos != cend());
    base_ptr next = detail::rb_increment(pos.n_);
    erase_node(pos.n_);
    return iterator(next);
  }

  iterator erase(const_iterator first, const_iterator last) noexcept {
    if (first == cbegin() && last == cend()) {
      clear();
      return end();
    }
    while (first != last)
      first = erase(first);
    return iterator(last.n_);
  }

  size_type erase(const key_type &k) { return erase_key(k); }

  template <class K>
    requires(transparent_compare && !std::convertible_to<K, iterator> &&
             !std::convertible_to<K, const_iterator>)
  size_type erase(K &&k) {
    return erase_key(k);
  }

  void
  swap(rb_tree &o) noexcept((alloc_traits::propagate_on_container_swap::value ||
                             alloc_traits::is_always_equal::value) &&
                            std::is_nothrow_swappable_v<Compare>) {
    if (this == &o)
      return;
    using std::swap;
    if constexpr (alloc_traits::propagate_on_container_swap::value)
      swap(alloc_, o.alloc_);
    else
      assert(alloc_ == o.alloc_ &&
             "swap of containers with unequal non-propagating allocators");
    swap(comp_, o.comp_);
    swap(header_.parent, o.header_.parent);
    swap(header_.left, o.header_.left);
    swap(header_.right, o.header_.right);
    swap(size_, o.size_);
    fix_header_after_swap();
    o.fix_header_after_swap();
  }

  [[nodiscard]] key_compare key_comp() const { return comp_; }
  [[nodiscard]] value_compare value_comp() const {
    return value_compare(comp_);
  }
  [[nodiscard]] iterator find(const key_type &k) {
    return iterator(find_node(k));
  }
  [[nodiscard]] const_iterator find(const key_type &k) const {
    return const_iterator(find_node(k));
  }
  template <class K>
    requires transparent_compare
  [[nodiscard]] iterator find(const K &k) {
    return iterator(find_node(k));
  }
  template <class K>
    requires transparent_compare
  [[nodiscard]] const_iterator find(const K &k) const {
    return const_iterator(find_node(k));
  }

  [[nodiscard]] bool contains(const key_type &k) const {
    return find_node(k) != end_node();
  }
  template <class K>
    requires transparent_compare
  [[nodiscard]] bool contains(const K &k) const {
    return find_node(k) != end_node();
  }

  [[nodiscard]] size_type count(const key_type &k) const {
    return count_impl(k);
  }
  template <class K>
    requires transparent_compare
  [[nodiscard]] size_type count(const K &k) const {
    return count_impl(k);
  }

  [[nodiscard]] iterator lower_bound(const key_type &k) {
    return iterator(lower_bound_node(k));
  }
  [[nodiscard]] const_iterator lower_bound(const key_type &k) const {
    return const_iterator(lower_bound_node(k));
  }
  template <class K>
    requires transparent_compare
  [[nodiscard]] iterator lower_bound(const K &k) {
    return iterator(lower_bound_node(k));
  }
  template <class K>
    requires transparent_compare
  [[nodiscard]] const_iterator lower_bound(const K &k) const {
    return const_iterator(lower_bound_node(k));
  }

  [[nodiscard]] iterator upper_bound(const key_type &k) {
    return iterator(upper_bound_node(k));
  }
  [[nodiscard]] const_iterator upper_bound(const key_type &k) const {
    return const_iterator(upper_bound_node(k));
  }
  template <class K>
    requires transparent_compare
  [[nodiscard]] iterator upper_bound(const K &k) {
    return iterator(upper_bound_node(k));
  }
  template <class K>
    requires transparent_compare
  [[nodiscard]] const_iterator upper_bound(const K &k) const {
    return const_iterator(upper_bound_node(k));
  }

  [[nodiscard]] std::pair<iterator, iterator> equal_range(const key_type &k) {
    return {iterator(lower_bound_node(k)), iterator(upper_bound_node(k))};
  }
  [[nodiscard]] std::pair<const_iterator, const_iterator>
  equal_range(const key_type &k) const {
    return {const_iterator(lower_bound_node(k)),
            const_iterator(upper_bound_node(k))};
  }
  template <class K>
    requires transparent_compare
  [[nodiscard]] std::pair<iterator, iterator> equal_range(const K &k) {
    return {iterator(lower_bound_node(k)), iterator(upper_bound_node(k))};
  }
  template <class K>
    requires transparent_compare
  [[nodiscard]] std::pair<const_iterator, const_iterator>
  equal_range(const K &k) const {
    return {const_iterator(lower_bound_node(k)),
            const_iterator(upper_bound_node(k))};
  }

  [[nodiscard]] bool check_invariants() const {
    if (!header_.parent)
      return size_ == 0 && header_.left == &header_ &&
             header_.right == &header_;
    base_ptr root = header_.parent;
    if (root->color != rb_color::black || root->parent != &header_)
      return false;
    size_type cnt = 0;
    if (check_subtree(root, cnt) < 0)
      return false;
    if (cnt != size_)
      return false;
    if (header_.left != detail::rb_min(root) ||
        header_.right != detail::rb_max(root))
      return false;
    for (auto it = begin(), nx = std::next(begin()); nx != end(); ++it, ++nx)
      if (!comp_(KeyOfValue()(*it), KeyOfValue()(*nx)))
        return false;
    return true;
  }

  friend bool operator==(const rb_tree &a, const rb_tree &b)
    requires std::equality_comparable<Value>
  {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
  }
  friend auto operator<=>(const rb_tree &a, const rb_tree &b)
    requires std::three_way_comparable<Value>
  {
    return std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(),
                                                  b.end());
  }
  friend void swap(rb_tree &a, rb_tree &b) noexcept(noexcept(a.swap(b))) {
    a.swap(b);
  }

private:
  struct insert_pos {
    base_ptr parent = nullptr;
    base_ptr existing = nullptr;
    bool left = false;
  };

  void reset_header() noexcept {
    header_.color = rb_color::red;
    header_.parent = nullptr;
    header_.left = header_.right = &header_;
    size_ = 0;
  }
  base_ptr end_node() const noexcept { return &header_; }

  void fix_header_after_swap() noexcept {
    if (!header_.parent) {
      header_.left = header_.right = &header_;
    } else {
      header_.parent->parent = &header_;
    }
  }

  void steal(rb_tree &o) noexcept {
    reset_header();
    if (o.header_.parent) {
      header_.parent = o.header_.parent;
      header_.left = o.header_.left;
      header_.right = o.header_.right;
      size_ = o.size_;
      header_.parent->parent = &header_;
      o.reset_header();
    }
  }

  void move_elements(rb_tree &o) {
    for (base_ptr n = o.header_.left; n != &o.header_;
         n = detail::rb_increment(n))
      emplace_hint(cend(), std::move(static_cast<node *>(n)->value));
    o.clear();
  }

  template <class... Args> node *create_node(Args &&...args) {
    auto p = node_alloc_traits::allocate(alloc_, 1);
    node *n = std::to_address(p);
    std::construct_at(n);

    node_alloc_traits::construct(alloc_, std::addressof(n->value),
                                 std::forward<Args>(args)...);

    return n;
  }

  void destroy_node(node *n) noexcept {
    node_alloc_traits::destroy(alloc_, std::addressof(n->value));
    auto p =
        std::pointer_traits<typename node_alloc_traits::pointer>::pointer_to(
            *n);
    std::destroy_at(n);
    node_alloc_traits::deallocate(alloc_, p, 1);
  }

  node *clone_node(const node *src) {
    node *n = create_node(src->value);
    n->color = src->color;
    return n;
  }

  void erase_subtree(base_ptr x) noexcept {
    while (x) {
      erase_subtree(x->right);
      base_ptr y = x->left;
      destroy_node(static_cast<node *>(x));
      x = y;
    }
  }

  node *clone_subtree(const node *x, base_ptr p) {
    node *top = clone_node(x);
    top->parent = p;

    if (x->right)
      top->right = clone_subtree(static_cast<const node *>(x->right), top);
    p = top;
    x = static_cast<const node *>(x->left);
    while (x) {
      node *y = clone_node(x);
      p->left = y;
      y->parent = p;
      if (x->right)
        y->right = clone_subtree(static_cast<const node *>(x->right), y);
      p = y;
      x = static_cast<const node *>(x->left);
    }

    return top;
  }

  void copy_from(const rb_tree &o) {
    if (!o.header_.parent)
      return;
    node *root =
        clone_subtree(static_cast<const node *>(o.header_.parent), &header_);
    header_.parent = root;
    header_.left = detail::rb_min(root);
    header_.right = detail::rb_max(root);
    size_ = o.size_;
  }

  void erase_node(base_ptr n) noexcept {
    destroy_node(static_cast<node *>(detail::rb_erase_rebalance(n, header_)));
    --size_;
  }

  template <class K> size_type erase_key(const K &k) {
    auto [first, last] = equal_range_nodes(k);
    size_type n = 0;
    while (first != last) {
      base_ptr next = detail::rb_increment(first);
      erase_node(first);
      first = next;
      ++n;
    }
    return n;
  }

  iterator link_node(const insert_pos &pos, node *n) noexcept {
    detail::rb_insert_rebalance(pos.left, n, pos.parent, header_);
    ++size_;
    return iterator(n);
  }

  const key_type &key_of(const node_base *n) const noexcept {
    return KeyOfValue()(static_cast<const node *>(n)->value);
  }

  template <class K> base_ptr lower_bound_node(const K &k) const {
    base_ptr y = &header_;
    base_ptr x = header_.parent;
    while (x) {
      if (!comp_(key_of(x), k)) {
        y = x;
        x = x->left;
      } else
        x = x->right;
    }
    return y;
  }

  template <class K> base_ptr upper_bound_node(const K &k) const {
    base_ptr y = &header_;
    base_ptr x = header_.parent;
    while (x) {
      if (comp_(k, key_of(x))) {
        y = x;
        x = x->left;
      } else
        x = x->right;
    }
    return y;
  }

  template <class K> base_ptr find_node(const K &k) const {
    base_ptr j = lower_bound_node(k);
    return (j == &header_ || comp_(k, key_of(j))) ? &header_ : j;
  }

  template <class K>
  std::pair<base_ptr, base_ptr> equal_range_nodes(const K &k) const {
    return {lower_bound_node(k), upper_bound_node(k)};
  }

  template <class K> size_type count_impl(const K &k) const {
    auto [f, l] = equal_range_nodes(k);
    size_type n = 0;
    for (; f != l; f = detail::rb_increment(f))
      ++n;
    return n;
  }

  template <class K> insert_pos get_insert_unique_pos(const K &k) const {
    base_ptr x = header_.parent;
    base_ptr y = &header_;
    bool go_left = true;
    while (x) {
      y = x;
      go_left = comp_(k, key_of(x));
      x = go_left ? x->left : x->right;
    }
    base_ptr j = y;
    if (go_left) {
      if (j == header_.left)
        return {y, nullptr, true};
      j = detail::rb_decrement(j);
    }
    if (comp_(key_of(j), k))
      return {y, nullptr, go_left};
    return {nullptr, j, false};
  }

  template <class K>
  insert_pos get_insert_hint_unique_pos(base_ptr pos, const K &k) const {
    if (pos == &header_) {
      if (size_ > 0 && comp_(key_of(header_.right), k))
        return {header_.right, nullptr, false};
      return get_insert_unique_pos(k);
    }
    if (comp_(k, key_of(pos))) {
      if (pos == header_.left)
        return {pos, nullptr, true};
      base_ptr before = detail::rb_decrement(pos);
      if (comp_(key_of(before), k)) {
        return before->right ? insert_pos{pos, nullptr, true}
                             : insert_pos{before, nullptr, false};
      }
      return get_insert_unique_pos(k);
    }
    if (comp_(key_of(pos), k)) {
      if (pos == header_.right)
        return {pos, nullptr, false};
      base_ptr after = detail::rb_increment(pos);
      if (comp_(k, key_of(after))) {
        return pos->right ? insert_pos{after, nullptr, true}
                          : insert_pos{pos, nullptr, false};
      }
      return get_insert_unique_pos(k);
    }
    return {nullptr, pos, false};
  }

  template <class V> std::pair<iterator, bool> insert_unique_value(V &&v) {
    insert_pos pos = get_insert_unique_pos(KeyOfValue()(v));
    if (pos.existing)
      return {iterator(pos.existing), false};
    node *n = create_node(std::forward<V>(v));
    return {link_node(pos, n), true};
  }

  template <class V> iterator insert_hint_value(const_iterator hint, V &&v) {
    insert_pos pos = get_insert_hint_unique_pos(hint.n_, KeyOfValue()(v));
    if (pos.existing)
      return iterator(pos.existing);
    node *n = create_node(std::forward<V>(v));
    return link_node(pos, n);
  }

  int check_subtree(base_ptr n, size_type &cnt) const {
    if (!n)
      return 1;
    if (n->left && n->left->parent != n)
      return -1;
    if (n->right && n->right->parent != n)
      return -1;
    if (n->color == rb_color::red &&
        ((n->left && n->left->color == rb_color::red) ||
         (n->right && n->right->color == rb_color::red)))
      return -1;
    int l = check_subtree(n->left, cnt);
    int r = check_subtree(n->right, cnt);
    if (l < 0 || r < 0 || l != r)
      return -1;
    ++cnt;
    return l + (n->color == rb_color::black ? 1 : 0);
  }

  mutable node_base header_;
  size_type size_ = 0;
  [[no_unique_address]] node_allocator alloc_;
  [[no_unique_address]] Compare comp_;
};

template <class V, class K, class C, class A, class Pred>
typename rb_tree<V, K, C, A>::size_type erase_if(rb_tree<V, K, C, A> &t,
                                                 Pred pred) {
  auto old = t.size();
  for (auto it = t.begin(); it != t.end();) {
    if (pred(*it))
      it = t.erase(it);
    else
      ++it;
  }
  return old - t.size();
}

template <class T>
concept interval_integer = std::integral<T> && !std::same_as<T, bool>;

template <interval_integer T> struct interval {
  using value_type = T;
  using size_type = std::uintmax_t;

  T lo{};
  T hi{};

  [[nodiscard]] static constexpr interval half_open(T begin, T end) noexcept {
    assert(begin < end);
    return {begin, static_cast<T>(end - 1)};
  }
  [[nodiscard]] static constexpr interval point(T p) noexcept { return {p, p}; }

  [[nodiscard]] constexpr bool valid() const noexcept { return lo <= hi; }
  [[nodiscard]] constexpr size_type size() const noexcept {
    return static_cast<size_type>(hi) - static_cast<size_type>(lo) + 1u;
  }
  [[nodiscard]] constexpr bool contains(T p) const noexcept {
    return lo <= p && p <= hi;
  }
  [[nodiscard]] constexpr bool contains(const interval &o) const noexcept {
    return lo <= o.lo && o.hi <= hi;
  }
  [[nodiscard]] constexpr bool overlaps(const interval &o) const noexcept {
    return lo <= o.hi && o.lo <= hi;
  }
  [[nodiscard]] constexpr bool adjacent_to(const interval &o) const noexcept {
    constexpr T mx = std::numeric_limits<T>::max();
    return (hi != mx && hi + 1 == o.lo) || (o.hi != mx && o.hi + 1 == lo);
  }

  friend constexpr bool operator==(const interval &,
                                   const interval &) = default;
  friend constexpr auto operator<=>(const interval &,
                                    const interval &) = default;
};

struct interval_less {
  using is_transparent = void;

  template <interval_integer T>
  constexpr bool operator()(const interval<T> &a,
                            const interval<T> &b) const noexcept {
    return a.hi < b.lo;
  }
  template <interval_integer T>
  constexpr bool operator()(const interval<T> &a,
                            const std::type_identity_t<T> &p) const noexcept {
    return a.hi < p;
  }
  template <interval_integer T>
  constexpr bool operator()(const std::type_identity_t<T> &p,
                            const interval<T> &a) const noexcept {
    return p < a.lo;
  }
};

template <interval_integer T, class Allocator = std::allocator<interval<T>>>
using interval_tree =
    rb_tree<interval<T>, rb_identity, interval_less, Allocator>;

template <interval_integer T, class Allocator = std::allocator<interval<T>>>
class interval_set {
public:
  using tree_type = interval_tree<T, Allocator>;
  using point_type = T;
  using key_type = interval<T>;
  using value_type = interval<T>;
  using allocator_type = Allocator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type &;
  using const_reference = const value_type &;
  using iterator = typename tree_type::const_iterator;
  using const_iterator = typename tree_type::const_iterator;
  using reverse_iterator = typename tree_type::const_reverse_iterator;
  using const_reverse_iterator = typename tree_type::const_reverse_iterator;

  interval_set() = default;
  explicit interval_set(const Allocator &a) : tree_(interval_less{}, a) {}
  interval_set(std::initializer_list<value_type> il,
               const Allocator &a = Allocator())
      : tree_(interval_less{}, a) {
    insert(il);
  }
  template <std::input_iterator It>
  interval_set(It first, It last, const Allocator &a = Allocator())
      : tree_(interval_less{}, a) {
    insert(first, last);
  }
#if defined(__cpp_lib_containers_ranges)
  template <std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
  interval_set(std::from_range_t, R &&rg, const Allocator &a = Allocator())
      : tree_(interval_less{}, a) {
    insert_range(std::forward<R>(rg));
  }
#endif
  interval_set(const interval_set &) = default;
  interval_set(interval_set &&) noexcept = default;
  interval_set &operator=(const interval_set &) = default;
  interval_set &operator=(interval_set &&) = default;
  interval_set &operator=(std::initializer_list<value_type> il) {
    clear();
    insert(il);
    return *this;
  }

  [[nodiscard]] allocator_type get_allocator() const noexcept {
    return tree_.get_allocator();
  }

  [[nodiscard]] const_iterator begin() const noexcept { return tree_.begin(); }
  [[nodiscard]] const_iterator end() const noexcept { return tree_.end(); }
  [[nodiscard]] const_iterator cbegin() const noexcept { return tree_.begin(); }
  [[nodiscard]] const_iterator cend() const noexcept { return tree_.end(); }
  [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
    return tree_.rbegin();
  }
  [[nodiscard]] const_reverse_iterator rend() const noexcept {
    return tree_.rend();
  }

  [[nodiscard]] bool empty() const noexcept { return tree_.empty(); }

  [[nodiscard]] size_type size() const noexcept { return tree_.size(); }

  [[nodiscard]] size_type max_size() const noexcept { return tree_.max_size(); }

  [[nodiscard]] std::uintmax_t covered_count() const noexcept {
    std::uintmax_t n = 0;
    for (const auto &r : tree_)
      n += r.size();
    return n;
  }

  void clear() noexcept { tree_.clear(); }

  std::pair<iterator, bool> insert(interval<T> r) {
    assert(r.valid());
    constexpr T mn = std::numeric_limits<T>::min();
    constexpr T mx = std::numeric_limits<T>::max();
    const interval<T> window{r.lo > mn ? static_cast<T>(r.lo - 1) : r.lo,
                             r.hi < mx ? static_cast<T>(r.hi + 1) : r.hi};
    auto [first, last] = tree_.equal_range(window);
    if (first != last) {
      if (std::next(first) == last && first->contains(r))
        return {first, false};
      r.lo = std::min(r.lo, first->lo);
      r.hi = std::max(r.hi, std::prev(last)->hi);
      first = tree_.erase(first, last);
    }
    return {tree_.emplace_hint(first, r), true};
  }

  std::pair<iterator, bool> insert(T lo, T hi) {
    return insert(interval<T>{lo, hi});
  }

  std::pair<iterator, bool> insert_point(T p) {
    return insert(interval<T>{p, p});
  }

  template <std::input_iterator It> void insert(It first, It last) {
    for (; first != last; ++first)
      insert(static_cast<interval<T>>(*first));
  }

  void insert(std::initializer_list<value_type> il) {
    insert(il.begin(), il.end());
  }

  template <std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
  void insert_range(R &&rg) {
    for (auto &&e : rg)
      insert(static_cast<interval<T>>(e));
  }
  size_type erase(interval<T> r) {
    assert(r.valid());
    auto [first, last] = tree_.equal_range(r);
    if (first == last)
      return 0;

    const size_type n = static_cast<size_type>(std::distance(first, last));
    const bool has_left = first->lo < r.lo;
    const bool has_right = std::prev(last)->hi > r.hi;
    const interval<T> left_part{first->lo, static_cast<T>(r.lo - 1)};
    const interval<T> right_part{static_cast<T>(r.hi + 1), std::prev(last)->hi};

    auto it = tree_.erase(first, last);
    if (has_right)
      it = tree_.emplace_hint(it, right_part);
    if (has_left)
      tree_.emplace_hint(it, left_part);
    return n;
  }
  size_type erase(T lo, T hi) { return erase(interval<T>{lo, hi}); }
  size_type erase_point(T p) { return erase(interval<T>{p, p}); }
  iterator erase(const_iterator pos) { return tree_.erase(pos); }
  iterator erase(const_iterator first, const_iterator last) {
    return tree_.erase(first, last);
  }

  void swap(interval_set &o) noexcept(noexcept(tree_.swap(o.tree_))) {
    tree_.swap(o.tree_);
  }

  [[nodiscard]] const_iterator find(T p) const { return tree_.find(p); }
  [[nodiscard]] const_iterator find(const interval<T> &r) const {
    return tree_.find(r);
  }

  [[nodiscard]] bool contains(T p) const { return tree_.contains(p); }
  [[nodiscard]] bool contains(const interval<T> &r) const {
    auto it = tree_.find(r);
    return it != tree_.end() && it->contains(r);
  }
  [[nodiscard]] bool intersects(const interval<T> &r) const {
    return tree_.contains(r);
  }

  [[nodiscard]] const_iterator lower_bound(T p) const {
    return tree_.lower_bound(p);
  }
  [[nodiscard]] const_iterator upper_bound(T p) const {
    return tree_.upper_bound(p);
  }
  [[nodiscard]] std::pair<const_iterator, const_iterator>
  equal_range(const interval<T> &r) const {
    return tree_.equal_range(r);
  }
  [[nodiscard]] std::pair<const_iterator, const_iterator>
  equal_range(T p) const {
    return tree_.equal_range(p);
  }

  [[nodiscard]] std::optional<T> next_free(T from) const {
    auto it = tree_.find(from);
    if (it == tree_.end())
      return from;
    if (it->hi == std::numeric_limits<T>::max())
      return std::nullopt;
    return static_cast<T>(it->hi + 1);
  }

  [[nodiscard]] const tree_type &tree() const noexcept { return tree_; }
  [[nodiscard]] bool check_invariants() const {
    if (!tree_.check_invariants())
      return false;
    for (auto it = tree_.begin(); it != tree_.end(); ++it) {
      if (!it->valid())
        return false;
      auto nx = std::next(it);
      if (nx != tree_.end() && (it->hi == std::numeric_limits<T>::max() ||
                                static_cast<T>(it->hi + 1) >= nx->lo))
        return false;
    }
    return true;
  }

  friend bool operator==(const interval_set &a, const interval_set &b) {
    return a.tree_ == b.tree_;
  }
  friend void swap(interval_set &a,
                   interval_set &b) noexcept(noexcept(a.swap(b))) {
    a.swap(b);
  }

private:
  tree_type tree_;
};

} // namespace std
