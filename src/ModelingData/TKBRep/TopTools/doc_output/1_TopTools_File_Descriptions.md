# TopTools Package File Descriptions

This document provides a detailed explanation of each file within the `TopTools` package of the Open CASCADE Technology (OCCT) library. The `TopTools` package is a fundamental component of the Modeling Data module, providing essential data structures and utilities for managing Boundary Representation (B-Rep) shapes.

## 1. TopTools.cxx / TopTools.hxx
**Package Definition and Global Utilities**
These files define the `TopTools` package itself. They contain global functions and enumerations used throughout the package.
- **TopTools.hxx**: The header file declaring the package class `TopTools`. It often includes utility functions like `Dump()` for debugging shapes and typedefs or forward declarations for the package's classes.
- **TopTools.cxx**: The implementation file for the package methods. It typically contains the logic for the `Dump()` function, which outputs the topology of a shape to a stream for inspection.

## 2. TopTools_ShapeSet.cxx / TopTools_ShapeSet.hxx
**Shape Serialization and Persistence**
These files implement the `TopTools_ShapeSet` class, which is responsible for reading and writing sets of `TopoDS_Shape` objects.
- **TopTools_ShapeSet.hxx**: Declares the `ShapeSet` class. It provides methods to `Add()` shapes to the set and `Write()` them to a file or stream, as well as `Read()` them back. It handles the recursive storage of sub-shapes (faces, edges, vertices) to ensure topological connectivity is preserved during persistence.
- **TopTools_ShapeSet.cxx**: Implements the serialization logic. It iterates through the shapes, writing their geometry (via `BRepTools`) and their topological structure (TShapes) in a compact format.

## 3. TopTools_LocationSet.cxx / TopTools_LocationSet.hxx
**Location Serialization and Persistence**
These files implement the `TopTools_LocationSet` class, which manages the storage of `TopLoc_Location` objects.
- **TopTools_LocationSet.hxx**: Declares the `LocationSet` class. Since shapes in OCCT often share the same underlying geometry but at different locations (transformations), this class creates a unique set of locations to avoid duplication when writing shapes to a file.
- **TopTools_LocationSet.cxx**: Implements the logic to collect unique coordinate reference systems and transformations and write/read them to/from a stream.

## 4. TopTools_MutexForShapeProvider.cxx / TopTools_MutexForShapeProvider.hxx
**Thread Safety Mechanism**
These files provide synchronization primitives for shape operations.
- **TopTools_MutexForShapeProvider.hxx**: Declares a mutex mechanism specifically designed for shape providers. This is crucial in multi-threaded environments where multiple algorithms might attempt to access or modify shared topological data simultaneously.
- **TopTools_MutexForShapeProvider.cxx**: Implements the mutex locking and unlocking logic.

## 5. Collection Instantiations (Lists, Maps, Arrays)
The bulk of the `TopTools` package consists of instantiations of OCCT's generic collection classes (from `NCollection` or `TCollection`) specifically for `TopoDS_Shape` and related types. These provide type-safe containers used extensively by modeling algorithms.

### Lists (Linked Lists)
Used for dynamic collections where order matters and random access is not required.
- **TopTools_ListOfShape.hxx**: Defines `TopTools_ListOfShape`, a list of `TopoDS_Shape` objects. Used universally to return lists of faces, edges, etc.
- **TopTools_ListOfListOfShape.hxx**: Defines a list of lists of shapes. Used for complex data structures like grouping shapes into shells or solids.
- **TopTools_ListIteratorOfListOfShape.hxx**: Defines the iterator to traverse a `TopTools_ListOfShape`.

### Maps (Hash Sets)
Used for fast lookup to check if a shape exists in a collection.
- **TopTools_MapOfShape.hxx**: A set of `TopoDS_Shape` objects. Uses hashing for O(1) average lookup.
- **TopTools_MapOfOrientedShape.hxx**: Similar to `MapOfShape`, but distinguishes between shapes with different orientations (FORWARD vs REVERSED).
- **TopTools_MapIteratorOfMapOfShape.hxx**: Iterator for `TopTools_MapOfShape`.
- **TopTools_MapIteratorOfMapOfOrientedShape.hxx**: Iterator for `TopTools_MapOfOrientedShape`.

### Indexed Maps (Hash Set + Array)
Used when you need both fast lookup (like a Map) and access by index (like an Array). Crucial for algorithms that need to map shapes to integers (indices).
- **TopTools_IndexedMapOfShape.hxx**: Stores unique shapes and assigns each an index (1 to N).
- **TopTools_IndexedMapOfOrientedShape.hxx**: Same as above, but respects orientation.

### Data Maps (Hash Maps / Dictionaries)
Used to associate data (Values) with Shapes (Keys).
- **TopTools_DataMapOfShapeInteger.hxx**: Maps a `TopoDS_Shape` to an `Standard_Integer`. Used for counting, indexing, or flagging shapes.
- **TopTools_DataMapOfShapeReal.hxx**: Maps a `TopoDS_Shape` to a `Standard_Real` (float). Used for assigning tolerances, areas, or other physical properties to shapes.
- **TopTools_DataMapOfShapeShape.hxx**: Maps a `TopoDS_Shape` to another `TopoDS_Shape`. Extremely common in modeling history (e.g., mapping a generated face back to its generator edge).
- **TopTools_DataMapOfShapeListOfShape.hxx**: Maps a `TopoDS_Shape` to a list of shapes. Used for adjacency graphs (e.g., Edge -> List of Faces sharing it).
- **TopTools_DataMapOfShapeListOfInteger.hxx**: Maps a shape to a list of integers.
- **TopTools_DataMapOfShapeBox.hxx**: Maps a shape to its Bounding Box (`Bnd_Box`). Used for caching collision detection data.
- **TopTools_DataMapOfShapeSequenceOfShape.hxx**: Maps a shape to a sequence of shapes.
- **TopTools_DataMapOfIntegerShape.hxx**: Reverse map: Integer -> Shape.
- **TopTools_DataMapOfIntegerListOfShape.hxx**: Integer -> List of Shapes.
- **TopTools_DataMapOfOrientedShapeShape.hxx**: Maps oriented shapes to shapes.
- **TopTools_DataMapOfOrientedShapeInteger.hxx**: Maps oriented shapes to integers.

### Indexed Data Maps
Combines Indexed Map and Data Map features.
- **TopTools_IndexedDataMapOfShapeShape.hxx**: Maps Shape -> Shape, but also allows iteration by index.
- **TopTools_IndexedDataMapOfShapeListOfShape.hxx**: Maps Shape -> List of Shapes, accessible by index.
- **TopTools_IndexedDataMapOfShapeAddress.hxx**: Maps Shape -> Address (pointer). Used for low-level associations.
- **TopTools_IndexedDataMapOfShapeReal.hxx**: Maps Shape -> Real, accessible by index.

### Arrays
Fixed-size collections.
- **TopTools_Array1OfShape.hxx**: 1D array of shapes (indices usually 1 to N).
- **TopTools_Array2OfShape.hxx**: 2D array of shapes (matrix).
- **TopTools_HArray1OfShape.hxx**: Handle (smart pointer) version of `Array1OfShape`. Used when the array needs to be shared or passed by reference counting.
- **TopTools_HArray2OfShape.hxx**: Handle version of `Array2OfShape`.
- **TopTools_Array1OfListOfShape.hxx**: Array where each element is a list of shapes.
- **TopTools_HArray1OfListOfShape.hxx**: Handle version of the above.

### Sequences
Ordered collections with index access (like C++ `std::vector` or `std::deque`).
- **TopTools_SequenceOfShape.hxx**: Sequence of `TopoDS_Shape`.
- **TopTools_HSequenceOfShape.hxx**: Handle version of `SequenceOfShape`.

## 6. Iterators
These files define iterators for the specific map types listed above. They allow traversing the collections.
- **TopTools_DataMapIteratorOfDataMapOfShapeInteger.hxx**
- **TopTools_DataMapIteratorOfDataMapOfShapeShape.hxx**
- **TopTools_DataMapIteratorOfDataMapOfShapeListOfShape.hxx**
- ... (and so on for each DataMap type).

## 7. Hashing Utilities
- **TopTools_ShapeMapHasher.hxx**: Defines how to compute a hash code for a `TopoDS_Shape` and how to check equality. It typically hashes the underlying `TopoDS_TShape` pointer and the `TopLoc_Location`.
- **TopTools_FormatVersion.hxx**: Defines version constants for file formats used by `ShapeSet`, ensuring backward compatibility.

## 8. LocationSet Pointer
- **TopTools_LocationSetPtr.hxx**: Defines a pointer type for `LocationSet`, likely for internal use or forward declarations.

## 9. FILES.cmake
**Build Configuration**
- **FILES.cmake**: A CMake configuration file that lists all the source files in this package. It is used by the build system to know which files to compile and link into the `TKBRep` library.
