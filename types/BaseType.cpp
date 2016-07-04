//
//  BaseTypes.cpp
//
//  Created by Jignesh Patel on 7/4/16.
//  Copyright © 2016 Gift-ed Research Group. All rights reserved.
//

#include <iostream>

/**
 * @brief Gift-ed base types. All types are derived from this base class.
**/
class GiftedBaseType {

public:

  /**
   * @brief List of all the types in the Quickstep system. Each type has a
   *        unique id.
   **/
  enum GiftedTypeId {
    _GiftedUnknownTypeId = -1,
    _GiftedIntTypeId
  };

  GiftedBaseType() {}; // Constructor.
  virtual ~GiftedBaseType() {}; // Pure virtual destructor.

  /**
   * @brief Clone the type (aka. a factory). Note this method create an empty
   *        new instance of the specific (C++ subclass) type.
   *
   *        WARNING: Use with care! The caller is responsible for cleaning
   *                 up the object that is allocated.
   **/
  virtual GiftedBaseType* Clone () const = 0;

  /**
   * @brief Interface to describe the type of the specific C++ class instance.
   *
   * @return Return the type. If not defined, then return an unknown type.
   **/
  virtual GiftedTypeId myType() const {return GiftedTypeId::_GiftedUnknownTypeId;}

  /**
   * @brief Interface to determine if the type's storage representation is 
   *        fixed length or variable length. If the type is fixed length a
   *        length. If it is length of the type for fixed length.
   *
   * @return Return a length greater than 0 if the type is fixed length, else 
   *         for variable length return 0.
   **/
  virtual std::size_t getLength() = 0;

  /**
   * @brief Turn a disk representation to an in-memory represenation.
   *
   * @return none TODO: Worry about error handling.
   *              TODO: Instead of pointing a char* and a size, we should have a
   *                    protected structure to pass around (that can also deal
   *                    array bounds).
   **/
  virtual void UnMarshall(const char* const payload, const std::size_t length) = 0;

  // TODO: need the assocaited similar marshall call

  // Comparison Operators ...
  // Must define these. TODO: Worry about three-valued logic.
  virtual void Equal(const GiftedBaseType* const right, bool &result) const = 0;
  virtual void LessThan(const GiftedBaseType* const right, bool &result) const = 0;

  // Can override if needed.
  virtual void NotEqual(const GiftedBaseType* const right, bool &result) const {
    Equal(right, result);
    result =! result;  // flip the result from IsEqual
  }
  virtual void LessThanOrEqual(const GiftedBaseType* const right, bool &result) const {
    LessThan(right, result);
    if (!result) return Equal(right, result);
  }
  virtual void GreaterThan(const GiftedBaseType* const right, bool &result) const {
    LessThanOrEqual(right, result); // Can make this all more efficient by using base operators.
    result =! result;  // flip the result
  }
  virtual void GreaterThanOrEqual(const GiftedBaseType* const right, bool &result) const {
    LessThan(right, result);
    result =! result;  // flip the result
  }

  // Now the arithmetic operators add, subtract, divide, multiply, modulo, ....
  /**
    * @brief Add the argument to the current value pointed to by "this".
    *        Note that this modifies the class instance that is called.
    *
    * @return None
   **/
  virtual void AddToLeft(const GiftedBaseType* const right) = 0;
  // TODO: Add other operations like this for each operator.

  // TODO: Add batch/bulk version of all of the comparison and arithmetic ops.
  //       Only for fixed length types, otherwise return an error. Here is
  //       what one would look like.
  virtual void VectorizedEqual(const std::size_t elementLength,      // Size of each element.
                               const char* const vectorDataElements, // Raw vector data.
                               const std::size_t vectorLength,       // Size of the vector.
                               const char* const rawLiteralData,     // Literal in the raw data form.
                               bool *result) // somewhat crude would need a TupleIdSequence...
  {
    std::size_t i;
    GiftedBaseType *_callerTypeInstance = Clone(); // Clone an instance of the caller type.
    GiftedBaseType *_literalInstance = Clone();    // Clone an instance of the literal type.

    // Initialize the literal.
    _literalInstance->UnMarshall(rawLiteralData, elementLength);

    for (i=0; i<vectorLength; i++) {
      _callerTypeInstance->UnMarshall(vectorDataElements+(i*elementLength), elementLength);
      _callerTypeInstance->Equal(_literalInstance, result[i]);
    }

    // TODO: Better to use auto pointer of some sort.
    delete _callerTypeInstance;
    delete _literalInstance;
  };

  // TODO: Define a VectorizedEqual in which the raw vector data is spread apart
  //       by a stride between two vectors (for evaluating predicates on fixed
  //       length attributes in a split row store.

  // TODO: Define a fast projection from a columnar vector to another columnar
  //       vector. This function would take as input a bitVector that indicates
  //       which columns to project out.
  //       Also create a "strided" version of this for packed row store.

  // Printing Functions
  friend std::ostream& operator<<(std::ostream& out, const GiftedBaseType& instance);
  virtual void Print(std::ostream& os) const = 0;
};

/**
 * @brief A generic output operator that works with any Quickstep type. It 
 *        simply calls the virtual Print function.
 **/

std::ostream& operator<<(std::ostream& out, const GiftedBaseType& instance)
{
  instance.Print(out);
  return out;
}


/**
 * @brief The IntegerType.
 **/
class GiftedIntegerType : public GiftedBaseType {
public:

  // Cover the basics ... constructor, desctuctor, and clone function.
  GiftedIntegerType():_value(0) {};
  ~GiftedIntegerType() {};
  virtual GiftedBaseType* Clone () const override {return new GiftedIntegerType;};

  GiftedTypeId myType() const override {return _GiftedIntTypeId;}

  std::size_t getLength() override {
    return sizeof(std::uint64_t);
  }

  void UnMarshall(const char* const payload, const std::size_t length) override {
    _value = *reinterpret_cast<const std::uint64_t*>(payload);
    return;
  }

  // Define the bare minimum functions.
  // TODO: This is not very efficient and the type compatibility checks should
  //       happen outside.
  virtual void Equal(const GiftedBaseType* const right, bool &result) const override {
    if (right->myType() == _GiftedIntTypeId)
      result = (_value == dynamic_cast<const GiftedIntegerType*>(right)->_value);
    else
      return; // TODO: Throw an error
  }

  void LessThan(const GiftedBaseType* const right, bool &result) const override {
    if (right->myType() == _GiftedIntTypeId) {
      result = (_value < dynamic_cast<const GiftedIntegerType*>(right)->_value);
    } else {
      return; // TODO: Throw an error
    }
  }

  void AddToLeft(const GiftedBaseType* const right) override {
    if (right->myType() == _GiftedIntTypeId) {
      _value += dynamic_cast<const GiftedIntegerType*>(right)->_value;
    } else {
      return; // TODO: Throw an error
    }
  }

  virtual void Print(std::ostream& os) const override {
    os << _value;
  }

  // Can also have special function only for this type ...
  void Increment() {_value++;}

    // Now here we can have a specialized highly-tuned version of VectorizedEqual

protected:
  std::uint64_t _value; // Value for the integer type
};


int main(int argc, const char * argv[]) {

  // Create a pointer to a Quickstep integer type
  std::unique_ptr<GiftedBaseType> anAttr (new GiftedIntegerType);

  // Load data from "storage", here just a variable in memory.
  std::int64_t _onDisk = 13;
  const char* _storagePtr = reinterpret_cast<char*>(&_onDisk);
  anAttr->UnMarshall(_storagePtr, sizeof(std::int64_t));

  // Create a new variable and add and compare.
  _onDisk *= 2;
  GiftedBaseType *anotherAttr = new GiftedIntegerType;
  anotherAttr->UnMarshall(_storagePtr, sizeof(std::int64_t));

  // Print out the variable.
  std::cout << "Create two variables: " << *anAttr << " and " << *anotherAttr << std::endl;

  // Test comparion operators
  bool result;
  anAttr->Equal(anotherAttr, result);
  std::cout << "= : " << result;

  anAttr->LessThan(anotherAttr, result);
  std::cout << "; < : " << result;

  anAttr->NotEqual(anotherAttr, result);
  std::cout << "; != : " << result;

  anAttr->LessThanOrEqual(anotherAttr, result);
  std::cout << "; <= : " << result;

  anAttr->GreaterThan(anotherAttr, result);
  std::cout << "; > : " << result;

  anAttr->GreaterThanOrEqual(anotherAttr, result);
  std::cout << "; >= : " << result << std::endl;

  // Add
  anAttr->AddToLeft(anotherAttr);

  // Print out the variable.
  std::cout << "Sum of the two variables is: " << *anAttr << std::endl;

  static const std::size_t _vectorCardinality = 1024;
  std::int64_t _onDiskA[_vectorCardinality];
  std::int64_t _onDiskB[_vectorCardinality];
  bool _resultArray[_vectorCardinality];
  std::size_t i;

  // create the disk representations
  for (i = 0; i< _vectorCardinality; i++) {
    _onDiskA[i] = i;
    _onDiskB[i] = i%2 ? i : i*2;
  }

  GiftedIntegerType _anInstance;

  _anInstance.VectorizedEqual(_anInstance.getLength(),
                              reinterpret_cast<char*>(_onDiskA),
                              _vectorCardinality,
                              _storagePtr,
                              _resultArray);

  for (i = 0; i< _vectorCardinality; i++) {
    std::cout << _resultArray[i];
  }
  std::cout << std::endl;

  delete anotherAttr;

  return 0;
}
