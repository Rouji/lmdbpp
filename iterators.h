#ifndef __lmdbpp_iterators
#define __lmdbpp_iterators

#include "lmdbpp.h"

namespace lmdbpp::iterators
{

const struct IteratorSentinel {} _sentinel;

template <typename TNextable, typename TOut>
class NextIteratable
{
public:
    typedef TOut t_out;

    class Iterator
    {
    public:
        Iterator(TNextable& nextable) : _nextable(nextable)
        {
            ++(*this);
        }

        bool operator!=(const IteratorSentinel& o) { return !_end; }
        TOut& operator*() { return _out; }
        TOut* operator->() { return &_out; }

        Iterator& operator++()
        {
            _end = !_nextable.next(_out);
            return *this;
        }

    private:
        TNextable& _nextable;
        TOut _out;
        bool _end = false;
    };

    NextIteratable(TNextable& nextable) : _nextable(nextable) {}

    const IteratorSentinel& end() { return _sentinel; }

    Iterator begin()
    {
        return Iterator{_nextable};
    }
private:
    TNextable& _nextable;
};


template <typename TNextable, typename TOut>
class OwningNextIteratable : public NextIteratable<TNextable, TOut>
{
public:
    template<typename... Args>
    OwningNextIteratable(Args&... args) : _owned_nextable{args...}, NextIteratable<TNextable, TOut>(_owned_nextable)
    {}

private:
    TNextable _owned_nextable;
};

template <typename TKey, typename TVal>
class KeyValNextable
{
public:

    KeyValNextable(Cursor& c) : _c(c) {}

    bool next(KeyVal<const TKey, const TVal>& out)
    {
        try
        {
            if (_first)
            {
                _c.first(out);
                _first = false;
            }
            else
            {
                _c.next(out);
            }
            return true;
        }
        catch (NotFoundError& e)
        {
            return false;
        }
    }
private:
    bool _first = true;
    Cursor& _c;
};
template <typename TKey, typename TVal> using KeyValIterator = OwningNextIteratable<KeyValNextable<TKey, TVal>, KeyVal<const TKey, const TVal>>;


template <typename TKey, typename TVal>
class MultiValNextable
{
public:
    MultiValNextable(Txn& txn, Dbi dbi, const Val<TKey>& key) : _c{txn, dbi}, _key(key) {}

    bool next(Val<const TVal>& out)
    {
        try
        {
            if (_first)
            {
                _c.seek(_key);
                _c.get_multiple(_key, out);
                _first = false;
            }
            else
            {
                _c.next_multiple(_key, out);
            }
            return true;
        }
        catch (NotFoundError& e)
        {
            return false;
        }
    }

private:
    Cursor _c;
    Val<TKey> _key;
    bool _first = true;
};
template <typename TKey, typename TVal> using MultiValIterator = OwningNextIteratable<MultiValNextable<TKey, const TVal>, Val<const TVal>>;

template <typename TKey, typename TVal>
class DupValNextable
{
public:
    DupValNextable(Txn& txn, Dbi dbi, const Val<TKey>& key) : _c{txn, dbi}, _key(key) {}

    bool next(Val<const TVal>& out)
    {
        try
        {
            if (_first)
            {
                _c.seek(_key);
                _c.first_dup(_key, out);
                _first = false;
            }
            else
            {
                _c.next_dup(_key, out);
            }
            return true;
        }
        catch (NotFoundError& e)
        {
            return false;
        }
    }

private:
    Cursor _c;
    Val<TKey> _key;
    bool _first = true;
};
template <typename TKey, typename TVal> using DupValIterator = OwningNextIteratable<DupValNextable<TKey, TVal>, Val<const TVal>>;


}  // namespace lmdbpp
#endif
