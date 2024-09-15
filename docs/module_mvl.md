# utl::mvl

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**mvl** (aka **M**atrix **V**iew **L**ibrary) implements generic classes for dense/strided/sparse vectors, matrices and views.

Unlike most existing matrix implementations, **mvl** focuses on data-oriented matrices that support arbitrary element types and can be used similarly to [std::vector](https://en.cppreference.com/w/cpp/container/vector). It's main goal is the simplicity of API and interoperability with most existing implementations.

> [!Important]
> Due to rather extensive API, seeing [usage examples](#example-1-declaring-and-indexing-a-matrix) first might be helpful.

> [!Tip]
> Use GitHub's built-in [table of contents](https://github.blog/changelog/2021-04-13-table-of-contents-support-in-markdown-files/) to navigate this page.

## Class structure

All vectors, matrices and views in **mvl** stem from a single generic template class, that can be specialized into any available behavior through its template parameters:
```cpp
// Generic template
template <
    class       T,
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

**mvl** classes are intented to be lightweight wrappers that allow convenient data manipulation without any performance tradeoffs on basic operations (loops, indexing, standard algorithms, etc.), this is achieved through conditional compilation and compile-time resolution of all indexing formulas. See [benchmarks](TODO:) for details.

Due to its arbitrary-data approach, linear algebra operations are intentionally **NOT** implemented by **mvl**. Numeric computation is a separate field and is much better handled by existing libraries like [XTensor](https://xtensor.readthedocs.io), [Eigen](https://eigen.tuxfamily.org) and [Blaze](https://bitbucket.org/blaze-lib/blaze). In cases where matrix computation is needed, it's heavily recommended to use matrices provided by those libraries as a main implementation, and whenever **mvl** functions are needed, those can be wrapped in **mvl** view. For example:

```cpp
Eigen::MatrixXd A; // in Eigen col-major by default

// Do some computation with 'A'

mvl::MatrixView<double, Checking::NONE, Layout::CR> view(A.rows(), A.cols(), A.data());

// Use 'view' like any other MVL matrix
```

## Definitions

> [!Important]
> `requires` tag specifies methods that get conditionally compiled by some specializations.

> [!Important]
> `Callable<Args...>` is a shortcut for a template parameter, restricted to callable objects with specific signature.

```cpp
// Generic template
template <
    class       T,
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
    size_type row_stride() const; // requires MATRIX && (DENSE || STRIDED)
    size_type col_stride() const; // requires MATRIX && (DENSE || STRIDED)
    
    const_pointer  data() const; // requires DENSE || STRIDED
    pointer        data();       // requires DENSE || STRIDED
    bool          empty() const;
    
    // - Advanced getters -
    bool      contains(const_reference value) const; // requires value_type::operator==()
    size_type    count(const_reference value) const; // requires value_type::operator==()
    
    bool is_sorted()                                                     const; // requires value_type::operator<()
    bool is_sorted(Callable<bool(const_reference, const_reference)> cmp) const;
    
    std::vector<value_type> to_std_vector() const;
    
    self transposed() const; // requires MATRIX && DENSE
    
    // - Indexation -
    const_reference front() const;
    const_reference  back() const;
    reference       front();
    reference        back();
    
    reference       operator[](size_type idx);
    reference       operator()(size_type i);                    // [TODO:] requires VECTOR
    reference       operator()(size_type i, size_type j);       // requires MATRIX
    const_reference operator[](size_type idx)            const;
    const_reference operator()(size_type i)              const; // [TODO:] requires VECTOR
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
    bool true_for_any(Callable<bool(const_reference, size_type)>            predicate) const; // [TODO:] requires VECTOR
    bool true_for_any(Callable<bool(const_reference, size_type, size_type)> predicate) const; // requires MATRIX
    bool true_for_all(Callable<bool(const_reference)>                       predicate) const;
    bool true_for_all(Callable<bool(const_reference, size_type)>            predicate) const; // [TODO:] requires VECTOR
    bool true_for_all(Callable<bool(const_reference, size_type, size_type)> predicate) const; // requires MATRIX
    
    // - Const algorithms -
    const self& for_each(Callable<void(const_reference)>                       func) const;
    const self& for_each(Callable<void(const_reference, size_type)>            func) const; // [TODO:] requires VECTOR
    const self& for_each(Callable<void(const_reference, size_type, size_type)> func) const; // requires MATRIX
    
    // - Mutating algorithms -
    self& for_each(Callable<void(reference)>                       func);
    self& for_each(Callable<void(reference, size_type)>            func); // [TODO:] requires VECTOR
    self& for_each(Callable<void(reference, size_type, size_type)> func); // requires MATRIX
    self& fill(const_reference value);
    
    self&        sort(); // requires value_type::operator<()
    self& stable_sort(); // requires value_type::operator<()
    self&        sort(Callable<bool(const_reference, const_reference)> cmp);
    self& stable_sort(Callable<bool(const_reference, const_reference)> cmp);
    
    // - Block Subviews -
    using block_view_type;
    using block_const_view_type;
    
    block_view_type       block(size_type i, size_type j, size_type rows, size_type cols);       // requires MATRIX
    block_const_view_type block(size_type i, size_type j, size_type rows, size_type cols) const; // requires MATRIX
    
    block_view_type       row();       // requires MATRIX
    block_const_view_type row() const; // requires MATRIX
    
    block_view_type       col();       // requires MATRIX
    block_const_view_type col() const; // requires MATRIX
    
    // - Sparse Subviews -
    using sparse_view_type;
    using sparse_const_view_type;
    
    sparse_view_type filter(Callable<bool(const_reference)>                       predicate);
    sparse_view_type filter(Callable<bool(const_reference, size_type)>            predicate); // [TODO:] requires VECTOR
    sparse_view_type filter(Callable<bool(const_reference, size_type, size_type)> predicate); // requires MATRIX
    
    sparse_const_view_type filter(Callable<bool(const_reference)>                       predicate) const;
    sparse_const_view_type filter(Callable<bool(const_reference, size_type)>            predicate) const; // [TODO:] requires VECTOR
    sparse_const_view_type filter(Callable<bool(const_reference, size_type, size_type)> predicate) const; // requires MATRIX
    
    sparse_view_type       diagonal();       // requires MATRIX
    sparse_const_view_type diagonal() const; // requires MATRIX
    
    // - Sparse operations - (requires SPARSE)
    using sparse_entry_type; // requires SPARSE
    
    self& rewrite_triplets(      std::vector<triplet_type>&& triplets); // requires MATRIX && SPARSE
    self&  insert_triplets(const std::vector<triplet_type>&  triplets); // requires MATRIX && SPARSE
    self&   erase_triplets(      std::vector<Index2D     >   indices ); // requires MATRIX && SPARSE
};

// - Tensor IO formats -
namespace format {
    // Human-readable formats
    std::string as_vector(    const GenericTensor<...> &tensor);
    std::string as_matrix(    const GenericTensor<...> &tensor);
    std::string as_dictionary(const GenericTensor<...> &tensor);
    // Export formats
    std::string as_raw_text(  const GenericTensor<...> &tensor);
    std::string as_json_array(const GenericTensor<...> &tensor);
}
```

> [!Note]
> `noexcept` specifiers are omitted in this section to reduce verbosity.

> [!Note]
> From now on `mvl::GenericTensor::` will be omitted for same reason.

## Methods

### Parameter reflection

> ```cpp
> struct params {
>     constexpr static Dimension   dimension
>     constexpr static Type        type;
>     constexpr static Ownership   ownership;
>     constexpr static Checking    checking;
>     constexpr static Layout      layout;
> };
> ```

Compile-time reflection of template parameters defining current `GenericTensor`.

Useful for implementing templates over tensor types and conditional compilation  (for example, template over `TensorType` can use `if constexpr (TensorType::params::dimension == Dimension::MATRIX) {}` to conditionally enable logic for matrices).

### Member types

> ```cpp
> using self            = GenericTensor;
> using value_type      = T;
> using size_type       = std::size_t;
> using difference_type = std::ptrdiff_t;
> using reference       = T&;
> using const_reference = const T&;
> using pointer         = T*;
> using const_pointer   = const T*;
> ```

A set of member types analogous to member types of [std::vector](https://en.cppreference.com/w/cpp/container/vector).

### Iterators

> ```cpp
> using               iterator;
> using       reverse_iterator;
> using         const_iterator;
> using const_reverse_iterator;
> 
> iterator begin();
> iterator   end();
> 
> reverse_iterator rbegin();
> reverse_iterator   rend();
> 
> const_iterator  begin() const;
> const_iterator    end() const;
> const_iterator cbegin() const;
> const_iterator   cend() const;
> 
> const_reverse_iterator  rbegin() const;
> const_reverse_iterator    rend() const;
> const_reverse_iterator crbegin() const;
> const_reverse_iterator   crend() const;
> ```

Iterators for 1D iteration over the underlying array. API and functionality is exactly the same as the one provided by `std::vector` iterators.

### Basic getters

> ```cpp
> size_type size() const;
> ```

Returns the number of elements stored by the tensor. Size of the underlying array.

**Note:** While `size() == rows() * cols()` is a valid assumption for dense matrices, the same cannot be said about the sparse case.

> ```cpp
> size_type rows() const; // requires MATRIX
> size_type cols() const; // requires MATRIX
> ```

Returns number of rows/columns in the matrix.

> ```cpp
> size_type row_stride() const; // requires MATRIX && (DENSE || STRIDED)
> size_type col_stride() const; // requires MATRIX && (DENSE || STRIDED)
> ```

Returns strides of the matrix.

**Note 1:** Most linear algebra libraries use only one stride per matrix. In context of the `Layout::RC` that "conventional" stride will correspond to `col_stride()`. `row_stride()` denotes the number of elements which get "skipped" by indexing after the end of each row. Using two strides like this gives us a more general way of viewing into data, particularly in regard to blocking.

**Note 2:** Dense non-strided matrices return trivial strides to accommodate API's that can work with both dense and strided inputs. There is no overhead from using actual unit-strides. What constitutes as "trivial strides" can be seen in the table below:

| Matrix type | Layout | Trivial `row_stride()` / `col_stride()` |
| - | - | - |
| `DENSE` | `RC` | **0** / **1** |
| `DENSE` | `CR` | **1** / **0** |

> ```cpp
> const_pointer  data() const; // requires DENSE || STRIDED
> pointer        data();       // requires DENSE || STRIDED
> ```

Returns the pointer to the underlying array.

> ```cpp
> bool empty() const;
> ```

Returns whether there are any elements stored in the tensor.

### Advanced getters

> ```cpp
> bool contains(const_reference value) const; // requires value_type::operator==()
> ```

Returns whether matrix contains element equal to `value`.

> ```cpp
> size_type count(const_reference value) const; // requires value_type::operator==()
> ```

Returns number of matrix elements equal to`value`.

> ```cpp
> bool is_sorted()                                                     const; // requires value_type::operator<()
> bool is_sorted(Callable<bool(const_reference, const_reference)> cmp) const;
> ```

Returns whether the underlying array is sorted. Uses `operator<` to perform comparison by default, callable `cmp` can be passed to use a custom comparison instead.

> ```cpp
> std::vector<value_type> to_std_vector() const;
> ```

Returns a copy of the underlying array as `std::vector`.

> ```cpp
> self transposed() const; // requires MATRIX && DENSE
> ```

Returns [transpose](https://en.wikipedia.org/wiki/Transpose) of the matrix.

### Indexation

> ```cpp
> const_reference front() const;
> const_reference  back() const;
> reference       front();
> reference        back();
> ```

Returns first/last element from the underlying 1D representation of the tensor.

> ```cpp
> reference       operator[](size_type idx);
> const_reference operator[](size_type idx) const;
> ```

Returns element number `idx` from the underlying 1D representation of the tensor.

> ```cpp
> reference       operator()(size_type i);                    // requires VECTOR
> const_reference operator()(size_type i)              const; // requires VECTOR
> reference       operator()(size_type i, size_type j);       // requires MATRIX
> const_reference operator()(size_type i, size_type j) const; // requires MATRIX
> ```

Returns element `i` (for vectors) or element `(i, j)` (for matrices) according to a logical index.

**Note:** `operator()` is a *default way of indexing tensors* and should be used in vast majority of cases, the main purpose of `operator[]` is to allow efficient looping over the underlying data when uniform transforms are applied. It is a good idea to prefer `operator()` even for 1D vectors since in some cases (like sparse vectors) logical index `i` may not be the same as "in-memory index" `idx`.

### Index conversions

> ```cpp
> size_type get_idx_of_ij(size_type i, size_type j) const; // requires MATRIX
> Index2D   get_ij_of_idx(size_type idx)            const; // requires MATRIX
> ```

Conversion between underlying index `idx` and logical index `(i, j)` for matrices.

> ```cpp
> bool contains_index(size_type i, size_type j) const; // requires MATRIX && SPARSE
> ```

Returns whether sparse matrix contains an element with a given index.

> ```cpp
> size_type extent_major() const; // requires MATRIX && (DENSE || STRIDED)
> size_type extent_minor() const; // requires MATRIX && (DENSE || STRIDED)
> ```

Returns dense matrix extents according to a memory layout. For example: with a row-major layout (aka `Layout::RC`) `extent_major()` will return the number of rows and `extent_minor()` will return the number of columns.

This is useful for creating generic logic for different layouts.

### Reductions

> ```cpp
> value_type     sum() const; // requires value_type::operator+()
> value_type product() const; // requires value_type::operator*()
> value_type     min() const; // requires value_type::operator<()
> value_type     max() const; // requires value_type::operator<()
> ```

Reduces matrix over a binary operation `+`, `*`, `min` or `max`.

Particularly useful in combination with [subviews](#block-subviews).

### Predicate operations

> ```cpp
> bool true_for_any(Callable<bool(const_reference)>                       predicate) const;
> bool true_for_any(Callable<bool(const_reference, size_type)>            predicate) const; // requires VECTOR
> bool true_for_any(Callable<bool(const_reference, size_type, size_type)> predicate) const; // requires MATRIX
> ```

Returns whether  `predicate` evaluates to `true` for any elements in the tensor.

Overloads **(2)** and **(3)** allow predicates to also take element index into condition.

> ```cpp
> bool true_for_all(Callable<bool(const_reference)>                       predicate) const;
> bool true_for_all(Callable<bool(const_reference, size_type)>            predicate) const; // requires VECTOR
> bool true_for_all(Callable<bool(const_reference, size_type, size_type)> predicate) const; // requires MATRIX
> ```

Returns whether  `predicate` evaluates to `true` for all elements in the tensor.

Overloads **(2)** and **(3)** allow predicates to also take element index into condition.

### Const algorithms

> ```cpp
> const self& for_each(Callable<void(const_reference)>                       func) const;
> const self& for_each(Callable<void(const_reference, size_type)>            func) const; // requires VECTOR
> const self& for_each(Callable<void(const_reference, size_type, size_type)> func) const; // requires MATRIX
> ```

Invokes non-mutating `func` for all elements in the tensor. 

Overloads **(2)** and **(3)** allow `func` to also use element index as an argument.

### Mutating algorithms

> ```cpp
> self& for_each(Callable<void(reference)>                       func);
> self& for_each(Callable<void(reference, size_type)>            func); // requires VECTOR
> self& for_each(Callable<void(reference, size_type, size_type)> func); // requires MATRIX
> ```

Invokes mutating `func` for all elements in the tensor. 

Overloads **(2)** and **(3)** allow `func` to also use element index as an argument.

> ```cpp
> self& fill(const_reference value);
> ```

Sets all elements of the tensor to `value`.

> ```cpp
> self&        sort(); // requires value_type::operator<()
> self& stable_sort(); // requires value_type::operator<()
> self&        sort(Callable<bool(const_reference, const_reference)> cmp);
> self& stable_sort(Callable<bool(const_reference, const_reference)> cmp);
> ```

Sorts elements of the underlying array according to `operator<` or a custom comparison `cmp`.

`stable_sort()` uses [stable sorting algorithms](https://en.wikipedia.org/wiki/Category:Stable_sorts) to maintain the relative order of entries with equal values, however it may come at a cost of some performance (specifics depend on a compiler implementation).

### Block subviews

> ```cpp
> using block_view_type;
> using block_const_view_type;
> ```

Types returned by blocking subview methods of the tensor.

Dense and strided matrices will use apropriately strided views to efficiently represent blocking, while sparse matrices returns sparse views.

All subviews inherit `T`, `Dimension` and `Checking` configuration from the parent.

> ```cpp
> block_view_type       block(size_type i, size_type j, size_type rows, size_type cols);        // requires MATRIX
> block_const_view_type block(size_type i, size_type j, size_type rows, size_type cols) const;  // requires MATRIX
> ```

Returns block subview starting at `(i, j)` with size `rows` x `cols`. 

> ```cpp
> block_view_type       row(size_type i);       // requires MATRIX
> block_const_view_type row(size_type i) const; // requires MATRIX
> ```

Returns block subview corresponding to the `i`-th row.

> ```cpp
> block_view_type       col(size_type j);       // requires MATRIX
> block_const_view_type col(size_type j) const; // requires MATRIX
> ```

Returns block subview corresponding to the `j`-th column.

### Sparse subviews

> ```cpp
> using sparse_view_type;
> using sparse_const_view_type;
> ```

Types returned by sparse subview methods of the tensor.

Evaluate to appropriate sparse tensor views that inherits `T`, `Dimension` and `Checking` configuration from the parent.

> ```cpp
> sparse_view_type filter(Callable<bool(const_reference)>                       predicate);
> sparse_view_type filter(Callable<bool(const_reference, size_type)>            predicate); // [???] requires VECTOR
> sparse_view_type filter(Callable<bool(const_reference, size_type, size_type)> predicate); // requires MATRIX
> ```

Returns a sparse view to all elements satisfying the `predicate`.

Overloads **(2)** and **(3)** allow predicates to also take element index into condition.

> ```cpp
> sparse_const_view_type filter(Callable<bool(const_reference)>                       predicate) const;
> sparse_const_view_type filter(Callable<bool(const_reference, size_type)>            predicate) const; // [???] requires VECTOR
> sparse_const_view_type filter(Callable<bool(const_reference, size_type, size_type)> predicate) const; // requires MATRIX
> ```

`const` version of the previous method.

> ```cpp
> sparse_view_type       diagonal();       // requires MATRIX
> sparse_const_view_type diagonal() const; // requires MATRIX
> ```

Returns sparse view to a matrix diagonal.

### Sparse operations

> ```cpp
> using sparse_entry_type; // requires SPARSE
> ```

`index` + `value` entry type corresponding to a sparse  tensor (aka pairs for vector and triplets for matrices). Such entries are used to initialize and fill sparse matrices with values (similarly to the vast majority of other sparse matrix implementation).

A table of appropriate sparse entry types can be seen below:

| Tensor dimension | Ownership    | Value of `sparse_entry_type`                              |
| ---------------- | ------------ | --------------------------------------------------------- |
| `VECTOR`         | `CONTAINER`  | `SparseEntry1D<value_type>>`                              |
| `VECTOR`         | `VIEW`       | `SparseEntry1D<std::reference_wrapper<value_type>>`       |
| `VECTOR`         | `CONST_VIEW` | `SparseEntry1D<std::reference_wrapper<const value_type>>` |
| `MATRIX`         | `CONTAINER`  | `SparseEntry2D<value_type>>`                              |
| `MATRIX`         | `VIEW`       | `SparseEntry2D<std::reference_wrapper<value_type>>`       |
| `MATRIX`         | `CONST_VIEW` | `SparseEntry2D<std::reference_wrapper<const value_type>>` |

> ```cpp
> self& rewrite_triplets(std::vector<triplet_type>&& triplets); // requires MATRIX && SPARSE
> ```

Replaces all existing entries in the sparse matrix with given `triplets`. 

> ```cpp
> self& insert_triplets(const std::vector<triplet_type>&  triplets); // requires MATRIX && SPARSE
> ```

Inserts given `triplets` into a sparse matrix.

> ```cpp
> self& erase_triplets(std::vector<Index2D> indices ); // requires MATRIX && SPARSE
> ```

Erases entries with given `indices` from the sparse matrix.

### Tensor IO formats

> ```cpp
> // Human-readable formats
> std::string mvl::format::as_vector(    const GenericTensor<...> &tensor);
> std::string mvl::format::as_matrix(    const GenericTensor<...> &tensor);
> std::string mvl::format::as_dictionary(const GenericTensor<...> &tensor);
> // Export formats
> std::string mvl::format::as_raw_text(  const GenericTensor<...> &tensor);
> std::string mvl::format::as_json_array(const GenericTensor<...> &tensor);
> ```

Converts tensor to string, formatted according to a chosen schema. All formats accept arbitrary tensors and properly handle sparse matrices.

| Method          | Output                                                       |
| --------------- | ------------------------------------------------------------ |
| `as_vector`     | Human-readable format. Formats tensor as a flat array of values. |
| `as_matrix`     | Human-readable format. Formats tensor as a 1- or 2-D matrix. Missing sparse matrix entries are marked with `-`. |
| `as_dictionary` | Human-readable format. Formats tensor as a list of `key`-`value` pairs. Useful for sparse matrices. |
| `as_raw_text`   | Export format. Formats tensor as a raw data separated by spaces and newlines (in case of matrices). |
| `as_json_array` | Export format. Formats tensor as a 1- or 2-D [JSON](https://en.wikipedia.org/wiki/JSON) array. |

See corresponding [example](#example-2-IO-formats) to get a better idea of what each output looks like.

**Note:** Stringification works for any type with an existing `operator<<(std::ostream&, const T&)` overload.

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
assert(  A.front()       = 1 );
assert(  A.back()        = 6 );
assert( *A.cbegin()      = 1 );
assert( *(A.cend() - 1)  = 6 );

// Basic getters
assert( A.rows()  == 2     );
assert( A.cols()  == 3     );
assert( A.size()  == 6     );
assert( A.empty() == false );
```

## Example 2 (IO formats)

[ [Run this code](LINK) ]
```cpp
using namespace utl;

// Create sparse matrix from triplets
mvl::SparseMatrix<double> mat(3, 4,
{
    {0, 0, 3.14    },
    {0, 1, 4.24    },
    {1, 1, 7.15    },
    {2, 2, 2.38    },
    {2, 3, 734.835 }
});

// Showcase different IO formats
std::cout
    // Human-readable formats
    << "\n## as_vector() ##\n\n"     << mvl::format::as_vector(    mat)
    << "\n## as_matrix() ##\n\n"     << mvl::format::as_matrix(    mat)
    << "\n## as_dictionary() ##\n\n" << mvl::format::as_dictionary(mat)
    // Export formats
    << "\n## as_raw_text() ##\n\n"   << mvl::format::as_raw_text(  mat)
    << "\n## as_json_array() ##\n\n" << mvl::format::as_json_array(mat);
```

Output:
```
## as_vector() ##

Tensor [size = 5] (3 x 4):
  { 3.14, 4.24, 7.15, 2.38, 734.835 }

## as_matrix() ##

Tensor [size = 5] (3 x 4):
  [ 3.14 4.24    -       - ]
  [    - 7.15    -       - ]
  [    -    - 2.38 734.835 ]

## as_dictionary() ##

Tensor [size = 5] (3 x 4):
  (0, 0) = 3.14
  (0, 1) = 4.24
  (1, 1) = 7.15
  (2, 2) = 2.38
  (2, 3) = 734.835
  
## as_raw_text() ##

3.14 4.24 0 0 
0 7.15 0 0 
0 0 2.38 734.835

## as_json_array() ##

[
  [ 3.14, 4.24,    0,       0 ], 
  [    0, 7.15,    0,       0 ], 
  [    0,    0, 2.38, 734.835 ] 
]
```

## Example N (Various math operations)

[ [Run this code](LINK) ]
```cpp
using namespace utl;

// Create 7x7 matrix with random values in [0, 1] range
auto A = mvl::create::random_matrix(7, 7, 0., 1.); // [TODO:]

// Compute ||A||_inf norm
const auto norm = A.transform(std::abs).sum(); // [TODO:]
    
// Compute tr(A)
const auto tr = A.diagonal().sum();

// Split matrix into block views
auto upper_half = A.block(0, 0,            0, A.size() / 2 - 1 );
auto lower_half = A.block(0, 0, A.size() / 2, A.size() - 1     );

// Set diagonal to { 1, 2, 3, ... , N }
A.diagonal().for_each([](int &elem, size_t i, size_t){ elem = i; });
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

## Example N (NAME)

[ [Run this code](LINK) ]
```cpp
using namespace utl;

CODE
```

## Benchmarks