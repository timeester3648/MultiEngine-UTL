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
    size_type row_stride() const; // requires MATRIX
    size_type col_stride() const; // requires MATRIX
    
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
    self& for_each(Callable<void(reference)>                       func);
    self& for_each(Callable<void(reference, size_type)>            func);
    self& for_each(Callable<void(reference, size_type, size_type)> func); // requires MATRIX
    self& fill(const_reference value);
    
    self&        sort(); // requires value_type::operator<()
    self& stable_sort(); // requires value_type::operator<()
    self&        sort(Callable<bool(const_reference, const_reference)> cmp);
    self& stable_sort(Callable<bool(const_reference, const_reference)> cmp);
    
    // - Block Subviews -
    using block_view_type;
    using block_const_view_type;
    
    block_view_type       block(size_type i, size_type j, size_type rows, size_type cols);       // TODO:TODO:TODO:
    block_const_view_type block(size_type i, size_type j, size_type rows, size_type cols) const; // TODO:TODO:TODO:
    
    block_view_type       row();       // TODO:TODO:TODO:
    block_const_view_type row() const; // TODO:TODO:TODO:
    
    block_view_type       col();       // TODO:TODO:TODO:
    block_const_view_type col() const; // TODO:TODO:TODO:
    
    // - Sparse Subviews -
    using sparse_view_type;
    using sparse_const_view_type;
    
    sparse_view_type filter(Callable<bool(const_reference)>                       predicate);
    sparse_view_type filter(Callable<bool(const_reference, size_type)>            predicate);
    sparse_view_type filter(Callable<bool(const_reference, size_type, size_type)> predicate); // requires MATRIX
    
    sparse_const_view_type filter(Callable<bool(const_reference)>                       predicate) const;
    sparse_const_view_type filter(Callable<bool(const_reference, size_type)>            predicate) const;
    sparse_const_view_type filter(Callable<bool(const_reference, size_type, size_type)> predicate) const; // requires MATRIX
    
    sparse_view_type       diagonal();
    sparse_const_view_type diagonal() const; // requires MATRIX
    
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

## Example N (NAME)

[ [Run this code](LINK) ]
```cpp
using namespace utl;

CODE
```

## Example 1 (Declaring and indexing a matrix)

[ [Run this code](LINK) ]
```cpp
using namespace utl;

// Declare regular dense matrix
mvl::Matrix<int> A = {
    { 1, 2, 3 },
    { 4, 5, 6 }
};

// 2D indexation
assert( A(0, 1) == 2 );

// 1D indexation
assert( A[3] == 3 );

// Range-based loops
for (const auto &element : A) assert( element > 0 );

// std::vector-like API & iterators
assert(  A.front()     = 1 );
assert(  A.back()      = 6 );
assert( *A.cbegin()    = 1 );
assert( *A.cend() - 1  = 6 );

// Printing
std::cout << A.stringify();
```

## Example N (Various math operations)

[ [Run this code](LINK) ]
```cpp
using namespace utl;

// Compute ||A||_inf norm
const auto norm = A.transform(std::abs).sum()
    
// Compute tr(A)
const auto tr = A.diagonal().sum();

// Split matrix into block views
auto upper_half = A.block(0, 0,            0, A.size() / 2 - 1 );
auto lower_half = A.block(0, 0, A.size() / 2, A.size() - 1     );

// Set diagonal to { 1, 2, 3, ... , N }
A.diagonal().for_each([](int &elem, size_t idx){ elem = idx; });
```

## Example N (Wrapping external data into views)

[ [Run this code](LINK) ]
```cpp
using namespace utl;

// Some raw data
// (for example, received from external 'C' library)
// (let's also assume it's immutable and uses col-major layout for added challenge)
const float  data[] = { 1.f, 2.f, 3.f, 4.f, 5.f, 6.f };
const size_t rows   = 2;
const size_t cols   = 3;

// Wrap data into MVL view and use it as a regular matrix
mvl::ConstMatrixView<float, mvl::Checking::NONE, mvl::Layout::CR> A(
    rows, cols, data
);

// This makes MVL easily compatible with almost every 3rd party
// matrix library, views merely wrap around external data and expose
// MVL matrix functionality with no copying/conversion overhead
```

## Example N (Working with images)

[ [Run this code](LINK) ]
```cpp
using namespace utl;

// Raw image RGB data
// (outputted by most image decoders)
const uint8_t* data     = { /* ... */ };
const size_t   channels = 3;
const size_t   w        = 300;
const size_t   h        = 200;

// View into R-G-B channels of an image as individual matrices
mvl::StridedMatrixView<uint8_t> R(rows, cols, 0, channels, data + 0);
mvl::StridedMatrixView<uint8_t> G(rows, cols, 0, channels, data + 1);
mvl::StridedMatrixView<uint8_t> B(rows, cols, 0, channels, data + 2);

// Convert image to grayscale using linear formula
mvl::Matrix grayscale(w, h);
grayscale.for_each([&](uint8_t &elem, size_t i, size_t j){
    elem = 0.2126 * R(i, j)  + 0.7152 * G(i, j) + 0.0722 * B(i, j);
});
```

## Example N (Working with sparse matrices)

[ [Run this code](LINK) ]
```cpp
using namespace utl;

CODE
```

## Benchmarks