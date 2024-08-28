# utl::mvl

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**mvl** (aka **M**atrix **V**iew **L**ibrary) implements generic classes for dense/strided/sparse vectors, matrices and views.

Unlike most existing matrix implementations, **mvl** focuses on data-oriented matrices that support arbitrary element types and can be used similarly to [std::vector](https://en.cppreference.com/w/cpp/container/vector). It's main goal is simplicity of API and interoperability with most existing implementations.

> [!Important]
> Due to rather extensive API, seeing [usage examples](##example-1-(basic-usage)) first might be helpful.

> [!Tip]
> Use GitHub built-in [table of contents](https://github.blog/changelog/2021-04-13-table-of-contents-support-in-markdown-files/) to navigate this page.

## Class structure

All vectors, matrices and views in **mvl** stem from a single generic template class, that can be specialized into any available behavior through its template parameters:
```cpp
// Generic template
template <
    typename    T,
    Dimension   dimension,
    Type        type,
    Ownership   ownership,
    Checking    checking,
    Layout      layout
>
class GenericTensor;

// Example of manual specialization
using IntegerMatrix = GenericTensor<int, Dimension::MATRIX, Type::DENSE, Ownership::CONTAINER, Checking::NONE, Layout::RC>;
```

> [!Note]
> Here **tensor** will be used to refer to arbitrary vectors, matrices and views, not necessarily in a [mathematical sense](https://en.wikipedia.org/wiki/Tensor).

This approach provides a generic way of working with matrices that is mostly agnostic to underlying layout and implementation. In practical usage manual specialization is rarely needed due to provided typedefs, however it is a powerful tool for implementing generic APIs.

**Below in an overview of all available arguments, typedefs and defaults:**

<img src ="images/mvl_diagram.svg">

## Performance & linear algebra operations

**mvl** classes are intented to be lightweight wrappers that allow convenient data manipulation without any performance tradeoffs on basic operations (loops, indexing, standard algorithms, etc.), this is achieved through conditional compilation and compile-time resolution of all indexing formulas. See [benchmarks]() for details.

Due to its arbitrary-data approach, linear algebra operations are intentionally **NOT** implemented by **mvl**. Numeric computation is a separate field and is much better handled by existing libraries like [XTensor](https://xtensor.readthedocs.io), [Eigen](https://eigen.tuxfamily.org) and [Blaze](https://bitbucket.org/blaze-lib/blaze). In cases where matrix computation is needed, it's heavily recommended to use matrices provided by those libraries as a main implementation, and whenever **mvl** functions are needed, those can be wrapped in **mvl** view. For example:

```cpp
Eigen::MatrixXd A; // in Eigen col-major by default

// Do some computation with 'A'

mvl::MatrixView<double, Checking::NONE, Layout::CR> view(A.rows(), A.cols(), A.data());

// Use 'view' like any other MVL matrix
```

## Definitions

```cpp
// Generic template
// 'requires' tag specifies methods that get conditionally compiled by some specializations
// 'Callable<Args...>' refers to a template parameter, restricted to callable objects with specific signature
template <
    typename    T,
    Dimension   dimension,
    Type        type,
    Ownership   ownership,
    Checking    checking,
    Layout      layout
>
class GenericTensor {
    // - Generic constructors -
    GenericTensor(const self&  other);
    GenericTensor(      self&& other);
    
    self& operator=(self&  other);
    self& operator=(self&& other);
        
    // - Parameter reflection -
    struct params {
        constexpr static Dimension   dimension
        constexpr static Type        type;
        constexpr static Ownership   ownership;
        constexpr static Checking    checking;
        constexpr static Layout      layout;
    };
    
    // - Member types -
    using self            = GenericTensor;
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;
    
    // - Iterators -
    using               iterator;
    using       reverse_iterator;
    using         const_iterator;
    using const_reverse_iterator;
    
    iterator begin();
    iterator   end();
    
    reverse_iterator rbegin();
    reverse_iterator   rend();
    
    const_iterator  begin() const;
    const_iterator    end() const;
    const_iterator cbegin() const;
    const_iterator   cend() const;
    
    const_reverse_iterator  rbegin() const;
    const_reverse_iterator    rend() const;
    const_reverse_iterator crbegin() const;
    const_reverse_iterator   crend() const;   
    
    // - Basic getters -
    size_type       size() const;
    size_type       rows() const; // requires MATRIX
    size_type       cols() const; // requires MATRIX
    size_type row_stride() const; // requires MATRIX && STRIDED
    size_type col_stride() const; // requires MATRIX && STRIDED
    
    const_pointer  data() const; // requires DENSE || STRIDED
    pointer        data();       // requires DENSE || STRIDED
    bool          empty() const;
    
    // - Advanced getters -
    bool      contains(const_reference value) const; // requires value_type::operator==()
    size_type    count(const_reference value) const; // requires value_type::operator==()
    
    bool is_sorted()                                                     const; // requires value_type::operator<()
    bool is_sorted(Callable<bool(const_reference, const_reference)> cmp) const; // requires value_type::operator<()
    
    std::vector<value_type> to_std_vector() const;
    
    // - Indexation -
    const_reference front() const;
    const_reference  back() const;
    reference       front();
    reference        back();
    
    reference       operator[](size_type idx);
    reference       operator()(size_type i, size_type j);       // requires MATRIX
    const_reference operator[](size_type idx)            const;
    const_reference operator()(size_type i, size_type j) const; // requires MATRIX
    
    // - Index conversions -
    size_type get_idx_of_ij(size_type i, size_type j) const; // requires MATRIX
    Index2D   get_ij_of_idx(size_type idx)            const; // requires MATRIX
    
    bool contains_index(size_type i, size_type j) const; // requires MATRIX && SPARSE
    
    size_type extent_major() const; // requires MATRIX && (DENSE || STRIDED)
    size_type extent_minor() const; // requires MATRIX && (DENSE || STRIDED)
    
    // - Reductions -
    value_type     sum() const; // requires value_type::operator+()
    value_type product() const; // requires value_type::operator*()
    value_type     min() const; // requires value_type::operator<()
    value_type     max() const; // requires value_type::operator<()
    
    // - Predicate operations -
    bool true_for_any(Callable<bool(const_reference)>                       predicate) const;
    bool true_for_any(Callable<bool(const_reference, size_type)>            predicate) const;
    bool true_for_any(Callable<bool(const_reference, size_type, size_type)> predicate) const; // requires MATRIX
    bool true_for_all(Callable<bool(const_reference)>                       predicate) const;
    bool true_for_all(Callable<bool(const_reference, size_type)>            predicate) const;
    bool true_for_all(Callable<bool(const_reference, size_type, size_type)> predicate) const; // requires MATRIX
    
    // - Const algorithms -
    const self& for_each(Callable<void(const_reference)>                       func) const;
    const self& for_each(Callable<void(const_reference, size_type)>            func) const;
    const self& for_each(Callable<void(const_reference, size_type, size_type)> func) const; // requires MATRIX
    
    // - Mutating algorithms -
    const self& for_each(Callable<void(reference)>                       func) const;
    const self& for_each(Callable<void(reference, size_type)>            func) const;
    const self& for_each(Callable<void(reference, size_type, size_type)> func) const; // requires MATRIX
    self& fill(const_reference value);
    
    self&        sort(); // requires value_type::operator<()
    self& stable_sort(); // requires value_type::operator<()
    self&        sort(Callable<bool(const_reference, const_reference)> cmp);
    self& stable_sort(Callable<bool(const_reference, const_reference)> cmp);
    
    // - Sparse operations - (require SPARSE)
    using triplet_type = SparseEntry2D<                             value_type >; // requires MATRIX && CONTAINER
    using triplet_type = SparseEntry2D<std::reference_wrapper<      value_type>>; // requires MATRIX && VIEW
    using triplet_type = SparseEntry2D<std::reference_wrapper<const value_type>>; // requires MATRIX && CONST_VIEW
    
    self&  insert_triplets(const std::vector<triplet_type>&  triplets); // requires MATRIX
    self& rewrite_triplets(      std::vector<triplet_type>&& triplets); // requires MATRIX
    self&   erase_triplets(      std::vector<Index2D     >   indices ); // requires MATRIX
};
```

> [!Note]
> `noexcept` specifiers are omitted in this section to reduce verbosity.

## Methods

### Parameter reflection

### Member types

### Indexation

### Index conversions

### Reductions

### Predicate operations

### Algorithms

### Sparse operations

> ```cpp
> mvl::METHOD_NAME();
> ```

METHOD_DESCRIPTION.

## Example 1 (EXAMPLE_NAME)

[ [Run this code](LINK) ]
```cpp
CODE
```

## Benchmarks