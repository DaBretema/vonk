#pragma once

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define dac_ClassNotCopy(ClassName)      \
public:                                  \
  ClassName(ClassName const &) = delete; \
  ClassName &operator=(ClassName const &) = delete;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define dac_ClassNotMove(ClassName)          \
public:                                      \
  ClassName(ClassName &&) noexcept = delete; \
  ClassName &operator=(ClassName &&) noexcept = delete;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define dac_ClassMoveH(ClassName)                            \
public:                                                      \
  friend void swap(ClassName &lhs, ClassName &rhs) noexcept; \
  ClassName(ClassName &&rhs) noexcept { swap(*this, rhs); }  \
  ClassName &operator=(ClassName &&rhs) noexcept             \
  {                                                          \
    swap(*this, rhs);                                        \
    return *this;                                            \
  }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define dac_ClassMoveC(ClassName, code)              \
  void swap(ClassName &lhs, ClassName &rhs) noexcept \
  {                                                  \
    using std::swap;                                 \
    code;                                            \
  }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define dac_ClassNotMoveCopy(ClassName) \
public:                                 \
  explicit ClassName() = default;       \
  dac_ClassNotCopy(ClassName);          \
  dac_ClassNotMove(ClassName);          \
  ~ClassName() = default;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define dac_Singleton(ClassName) \
public:                          \
  dac_ClassNotCopy(ClassName);   \
  dac_ClassNotMove(ClassName);   \
  ~ClassName() = default;        \
                                 \
private:                         \
  explicit ClassName() = default;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
