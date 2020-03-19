#ifndef __lmdbpp_val_h
#define __lmdbpp_val_h

#include <lmdb.h>
#include <vector>
#include <string>
#include <string_view>
#include <cstring>
#include "error.h"

namespace lmdbpp
{

template <typename T>
class base_val
{
public:
    typedef T t;

    void set(T* data, size_t size) { _val.mv_data = (void*)data; _val.mv_size = size; }

    void data(const T* data) { _val.mv_data = (void*)data; }
    void data(const T& data) { _val.mv_data = (void*)&data; }
    T* data() const { return (T*)_val.mv_data; }

    size_t size() const { return _val.mv_size; }
    void size(size_t s) { _val.mv_size = s; }

    MDB_val* mdb_val() const
    {
        return const_cast<MDB_val*>(&_val); //NOTE: there simply is no const in lmdb-land
    }

protected:
    base_val() = default;
    ~base_val() = default;
    base_val(const base_val&) = default;
    base_val(base_val&&) = default;
    base_val& operator=(const base_val&) = default;
    base_val& operator=(base_val&&) = default;

    base_val(T* data)
        : _val{sizeof(T), (void*)data}
    {
    }

    base_val(T* data, size_t size)
        : _val{size, (void*)data}
    {
    }

    MDB_val _val{0, nullptr};
};

template<typename T>
class Val : public base_val<T>
{
public:
    //using base_val<T>::base_val; //TODO: constructor inheritance somehow breaks template argument deduction(?)
    Val() = default;
    ~Val() = default;
    Val(const Val&) = default;
    Val(Val&&) = default;
    Val& operator=(const Val&) = default;
    Val& operator=(Val&&) = default;
    Val(T* data) : base_val<T>(data) {}
    Val(T* data, size_t size) : base_val<T>(data,size) {}
};

template<>
class Val<const char> : public base_val<const char>
{
public:
    Val() : base_val<const char>(0) {}
    Val(const char* str) : base_val<const char>(str, std::strlen(str)) {}
    Val(const std::string& str) : base_val<const char>(str.c_str(), str.length()) {}
    Val(const std::string_view& str) : base_val<const char>(str.data(), str.length()) {}
    std::string to_str() { return std::string{data(), size()}; }
    std::string_view to_strview() { return std::string_view{data(), size()}; }
};
Val(const std::string& str) -> Val<const char>;
Val(const std::string_view& str) -> Val<const char>;


// wrapper for MDB_val[2], use for MDB_MULTIPLE
template <typename T>
class MultiVal
{
public:
    MultiVal() = default;
    ~MultiVal() = default;
    MultiVal(const MultiVal&) = default;
    MultiVal(MultiVal&&) = default;
    MultiVal& operator=(const MultiVal&) = default;
    MultiVal& operator=(MultiVal&&) = default;

    MultiVal(T* array, size_t element_size, size_t element_count)
        : _val{{element_size, array}, {element_count, 0}}
    {
    }

    MultiVal(const std::vector<T>& vec)
        : _val{{sizeof(T), const_cast<T*>(vec.data())}, {vec.size(), 0}}
    {
    }

    MDB_val* mdb_val() const
    {
        return const_cast<MDB_val*>(_val); //NOTE: there simply is no const in lmdb-land
    }

private:
    MDB_val _val[2]{{}, {}};
};

template <typename TKey, typename TVal>
struct KeyVal
{
    Val<TKey> key;
    Val<TVal> val;
};

}

#endif
