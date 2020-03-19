#ifndef __lmdbpp_txn_h
#define __lmdbpp_txn_h

#include <lmdb.h>
#include "error.h"
#include "env.h"
#include "val.h"

namespace lmdbpp
{

typedef MDB_dbi Dbi;

enum class DbiFlags
{
    NONE = 0,
    REVERSEKEY = MDB_REVERSEKEY,
    DUPSORT = MDB_DUPSORT,
    INTEGERKEY = MDB_INTEGERKEY,
    DUPFIXED = MDB_DUPFIXED,
    INTEGERDUP = MDB_INTEGERDUP,
    REVERSEDUP = MDB_REVERSEDUP,
    CREATE = MDB_CREATE
};
constexpr DbiFlags operator|(DbiFlags a, DbiFlags b) { return (DbiFlags)((int)a|(int)b); }
constexpr DbiFlags operator&(DbiFlags a, DbiFlags b) { return (DbiFlags)((int)a&(int)b); }

class Txn
{
public:
    Txn(Env& env, unsigned int flags = 0)
        : _env(env)
        , _autocommit(true)
    {
        check(mdb_txn_begin(_env.mdb_env(), nullptr, flags, &_txn));
    };

    ~Txn()
    {
        if (_autocommit && _txn != nullptr)
            commit();
    }

    Txn& operator=(Txn&& o) = delete;
    Txn(Txn&& o) = delete;
    Txn(Txn& o) = delete;

    void commit()
    {
        check(mdb_txn_commit(_txn));
    }

    void abort()
    {
        mdb_txn_abort(_txn);
        _autocommit = false;
    }

#define TPL_KV template <typename TKey, typename TVal>
#define TPL_VK template <typename TKey, typename TVal>

    TPL_KV void get(Dbi dbi, const Val<TKey>& key, Val<const TVal>& val) { check(mdb_get(_txn, dbi, key.mdb_val(), val.mdb_val())); }
    TPL_VK auto get(Dbi dbi, const Val<TKey>& key) { Val<const TVal> v{}; get(dbi, key, v); return v; }
    TPL_VK auto get(Dbi dbi, const TKey* key) { return get(dbi, Val{key}); }
    TPL_VK auto get(Dbi dbi, const TKey& key) { return get(dbi, Val{&key}); }

#define FUNC(NAME, FLAGS) \
    TPL_KV void NAME(Dbi dbi, const Val<TKey>& key, const Val<TVal>& val) { _put(dbi, key.mdb_val(), val.mdb_val(), FLAGS); } \
    TPL_KV void NAME(Dbi dbi, const KeyVal<TKey, TVal>& kv) { NAME(dbi, kv.key, kv.val); } \
    TPL_KV void NAME(Dbi dbi, const TKey* key, const TVal* val) { NAME(dbi, Val{key}, Val{val}); } \
    TPL_KV void NAME(Dbi dbi, const TKey& key, const TVal& val) { NAME(dbi, Val{&key}, Val{&val}); }

    FUNC(put, 0)
    FUNC(append, MDB_APPEND)
    FUNC(append_dup, MDB_APPENDDUP)

#undef FUNC

    TPL_KV void del(Dbi dbi, const Val<TKey>& key, const Val<TKey>& val) { _del(dbi, key.mdb_val(), val.mdb_val()); }
    TPL_KV void del(Dbi dbi, const KeyVal<TKey,TVal>& kv) { del(kv.key, kv.val); }
    TPL_KV void del(Dbi dbi, const TKey* key, const TVal* val) { del(Val{key}, Val{val}); }
    TPL_KV void del(Dbi dbi, const TKey& key, const TVal& val) { del(Val{&key}, Val{&val}); }
    TPL_KV void del(Dbi dbi, const Val<TKey>& key) { _del(dbi, key.mdb_val(), nullptr); }
    TPL_KV void del(Dbi dbi, const TKey* key) { del(Val{key}); }
    TPL_KV void del(Dbi dbi, const TKey& key) { del(Val{&key}); }

    Dbi open_dbi(const char* name = nullptr, DbiFlags flags = DbiFlags::NONE)
    {
        Dbi dbi;
        check(mdb_dbi_open(_txn, name, (unsigned int)flags, &dbi));
        return dbi;
    }

    DbiFlags dbi_flags(Dbi dbi)
    {
        unsigned int f = 0;
        check(mdb_dbi_flags(_txn, dbi, &f));
        return (DbiFlags)f;
    }

    MDB_txn* mdb_txn() const
    {
        return const_cast<MDB_txn*>(_txn);
    }

#undef TPL_KV
#undef TPL_VK

private:
    void _put(MDB_dbi dbi, MDB_val* key, MDB_val* val, unsigned int flags) { check(mdb_put(_txn, dbi, key, val, flags)); }
    void _del(MDB_dbi dbi, MDB_val* key, MDB_val* val) { check(mdb_del(_txn, dbi, key, val)); }
    MDB_txn* _txn = nullptr;
    Env& _env;
    bool _autocommit = false;
};


}

#endif
