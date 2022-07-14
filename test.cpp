#include <vector>
#include <string>
#include <filesystem>
#include "lmdbpp.h"
#include "iterators.h"
#include <iostream>
#include <assert.h>

using namespace lmdbpp;
using namespace lmdbpp::iterators;

typedef void (*test_func)(Env&);

template<typename T>
bool operator==(const Val<T>& a, const Val<T>& b)
{
    return a.mdb_val()->mv_data == b.mdb_val()->mv_data
        && a.mdb_val()->mv_size == b.mdb_val()->mv_size;
}
template<typename TKey, typename TVal>
bool operator==(const KeyVal<TKey,TVal>& a, const KeyVal<TKey,TVal>& b)
{
    return a.key == b.key && a.val == b.val;
}

void cursor(Env& env)
{
    Txn txn{env};
    Dbi dbi = txn.open_dbi();
    Cursor c{txn, dbi};
    c.put(Val{"test"}, Val{"hello"});
    c.put(Val{"woo"}, Val{"hoo"});

    auto kv = c.first<char, char>();
    assert(kv.key.to_str() == "test" && kv.val.to_str() == "hello");

    kv = c.last<char, char>();
    assert(kv.key.to_str() == "woo" && kv.val.to_str() == "hoo");

    c.seek(Val{"test"});
    kv = c.current<char, char>();
    assert(kv.key.to_str() == "test" && kv.val.to_str() == "hello");
}

void multi(Env& env)
{
    Val key{"test"};
    Txn txn{env};
    auto dbi = txn.open_dbi(nullptr, DbiFlags::DUPFIXED | DbiFlags::DUPSORT);
    Cursor c{txn, dbi};
    const int n = 10;
    for (int i=0; i<n; ++i)
        c.put(key, Val{&i});

    MultiValIterator<decltype(key)::t, int> it1{txn,dbi,key};
    for (auto& vals : it1)
        assert(vals.size() == n*sizeof(int));
    DupValIterator<decltype(key)::t, int> it2{txn,dbi,key};
    for (auto& vals : it2)
        assert(vals.size() == sizeof(int));
}

void dup(Env& env)
{
    Val key{"test"};
    Txn txn{env};
    Dbi dbi = txn.open_dbi(nullptr, DbiFlags::DUPSORT);
    Cursor c{txn, dbi};
    c.put(key, Val{"1 these values"});
    c.put(key, Val{"2 all share"});
    c.put(key, Val{"3 the same key"});

    c.seek(key);
    assert(c.dup_count() == 3);

    DupValIterator<decltype(key)::t, char> it{txn,dbi,key};
    int i=0;
    for (auto& val : it)
    {
        ++i;
    }
    assert(i == 3);
}

void abort(Env& env)
{
    Dbi dbi;
    {
        Txn txn{env};
        dbi = txn.open_dbi();
        Cursor c{txn, dbi};
        c.put(Val{"sup"}, Val{"?"});
        txn.abort();
    }
    {
        Txn txn{env};
        Cursor c{txn, dbi};
        try
        {
            c.first<char,char>();
            assert(0);
        }
        catch(NotFoundError& e)
        { }
    }
}

int main()
{
    std::string env_path{"test.mdb"};

    std::vector<test_func> tests{
        cursor,
        abort,
        multi,
        dup
    };
    for (auto test : tests)
    {
        std::filesystem::remove_all(env_path);
        std::filesystem::create_directory(env_path);
        Env env{env_path, {.flags=EnvArgs::Flags::CREATE, .mapsize=1024*1024}};
        test(env);
    }

    return 0;
}
