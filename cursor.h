#ifndef __lmdbpp_cursor_h
#define __lmdbpp_cursor_h

#include <lmdb.h>
#include "env.h"
#include "error.h"
#include "txn.h"
#include "val.h"

// TODO: check if flags mess with const-ness and stuff
namespace lmdbpp
{
class Cursor
{
public:
    Cursor(Txn& txn, Dbi dbi)
        : _txn(txn)
        , _dbi(dbi)
    {
        check(mdb_cursor_open(_txn.mdb_txn(), _dbi, &_cursor));
    }

    ~Cursor()
    {
        if (_cursor != nullptr)
        {
            close();
        }
    }

    Cursor(Cursor&& o) : _txn(o._txn), _dbi(o._dbi), _cursor(o._cursor)
    {
        o._cursor = nullptr;
    }

    Cursor(const Cursor&) = delete;
    Cursor& operator=(const Cursor&) = delete;
    Cursor& operator=(Cursor&&) = delete;

    void close() { mdb_cursor_close(_cursor); }

    MDB_cursor* mdb_cursor() { return _cursor; }

#define TPL_K template <typename TKey>
#define TPL_KV template <typename TKey, typename TVal>
#define TPL_VK template <typename TVal, typename TKey>

#define FUNC(NAME, OP) \
    TPL_KV void NAME(Val<const TKey>& key, Val<const TVal>& val) { _get(key.mdb_val(), val.mdb_val(), OP); } \
    TPL_KV void NAME(KeyVal<const TKey, const TVal>& kv) { NAME(kv.key, kv.val); } \
    TPL_KV auto NAME() { KeyVal<const TKey, const TVal> kv; NAME(kv); return kv; }

    FUNC(current, MDB_GET_CURRENT)
    FUNC(first, MDB_FIRST)
    FUNC(next, MDB_NEXT)
    FUNC(prev, MDB_PREV)
    FUNC(last, MDB_LAST)
#undef FUNC

#define FUNC(NAME, OP) \
    TPL_KV void NAME(const Val<TKey>& key, Val<const TVal>& val) { _get(key.mdb_val(), val.mdb_val(), OP); } 
    TPL_KV void NAME(KeyVal<TKey, const TVal>& kv) { NAME(kv.key, kv.val); } \
    TPL_VK auto NAME(const Val<TKey>& key) { Val<const TVal> v; NAME(key, v); return v; }

    FUNC(get_multiple,  MDB_GET_MULTIPLE)
    FUNC(next_multiple, MDB_NEXT_MULTIPLE)
    FUNC(prev_multiple, MDB_PREV_MULTIPLE)
    FUNC(first_dup,     MDB_FIRST_DUP)
    FUNC(last_dup,      MDB_LAST_DUP)
    FUNC(next_dup,      MDB_NEXT_DUP)
    FUNC(prev_dup,      MDB_PREV_DUP)
    FUNC(seek,          MDB_SET_KEY)
#undef FUNC

#define FUNC(NAME, FLAGS) \
    TPL_KV void NAME(const Val<TKey>& key, const Val<TVal>& val) { _put(key.mdb_val(), val.mdb_val(), FLAGS); } \
    TPL_KV void NAME(const KeyVal<TKey, TVal>& kv) { NAME(kv.key, kv.val); }

    FUNC(put, MDB_NOOVERWRITE)
    FUNC(overwrite, 0)
    FUNC(replace, MDB_CURRENT)
    FUNC(append, MDB_APPEND)
    FUNC(append_dup, MDB_APPENDDUP)
#undef FUNC

    TPL_KV void put(const Val<TKey>& key, const MultiVal<TVal>& val) { _put(key.mdb_val(), val.mdb_val(), MDB_MULTIPLE); }

    TPL_K void seek(const Val<TKey>& key) { _get(key.mdb_val(), nullptr, MDB_SET); }
    TPL_KV void seek(const Val<TKey>& key, const Val<const TVal>& val) { _get(key.mdb_val(), val.mdb_val(), MDB_GET_BOTH); }


#undef TPL_K
#undef TPL_KV
#undef TPL_VK

    void del(bool no_dupdata = false) { check(mdb_cursor_del(_cursor, no_dupdata ? MDB_NODUPDATA : 0)); }

    size_t dup_count() { size_t c; check(mdb_cursor_count(_cursor, &c)); return c; }

private:
    void _get(MDB_val* key, MDB_val* val, MDB_cursor_op op) { check(mdb_cursor_get(_cursor, key, val, op)); }
    void _put(MDB_val* key, MDB_val* val, unsigned int flags) { check(mdb_cursor_put(_cursor, key, val, flags)); }

    Txn& _txn;
    Dbi _dbi;
    MDB_cursor* _cursor = nullptr;
};

}  // namespace lmdbpp

#endif
