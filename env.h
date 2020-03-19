#ifndef __lmdbpp_env_h
#define __lmdbpp_env_h

#include <lmdb.h>
#include "error.h"

namespace lmdbpp
{

struct EnvArgs
{
    enum class Flags
    {
        NONE = 0,
        CREATE = MDB_CREATE,
        FIXEDMAP = MDB_FIXEDMAP,
        NOSUBDIR = MDB_NOSUBDIR,
        RDONLY = MDB_RDONLY,
        WRITEMAP = MDB_WRITEMAP,
        NOMETASYNC = MDB_NOMETASYNC,
        NOSYNC = MDB_NOSYNC,
        MAPASYNC = MDB_MAPASYNC,
        NOTLS = MDB_NOTLS,
        NOLOCK = MDB_NOLOCK,
        NORDAHEAD = MDB_NORDAHEAD,
        NOMEMINIT = MDB_NOMEMINIT
    };
    Flags flags = Flags::NONE;
    size_t mapsize = 0;
    unsigned int maxdbs = 1;
    mdb_mode_t mode = 0644;
};
constexpr EnvArgs::Flags operator|(EnvArgs::Flags a, EnvArgs::Flags b) { return (EnvArgs::Flags)((int)a|(int)b); }
constexpr EnvArgs::Flags operator&(EnvArgs::Flags a, EnvArgs::Flags b) { return (EnvArgs::Flags)((int)a&(int)b); }

class Env
{
public:
    Env(const std::string& path, EnvArgs args = EnvArgs{})
    {
        mdb_env_create(&_env);
        if (args.mapsize > 0)
        {
            set_mapsize(args.mapsize);
        }
        if (args.maxdbs > 1)
        {
            set_maxdbs(args.maxdbs);
        }
        check(mdb_env_open(_env, path.c_str(), (unsigned int)args.flags, args.mode));
    }

    ~Env() { mdb_env_close(_env); }

    void set_maxdbs(MDB_dbi n) { check(mdb_env_set_maxdbs(_env, n)); }
    void set_mapsize(size_t size) { check(mdb_env_set_mapsize(_env, size)); }
    void set_flags(unsigned int flags, int onoff) { check(mdb_env_set_flags(_env, flags, onoff)); }

    MDB_env* mdb_env() const { return (MDB_env*)_env; }

private:
    void set_maxreaders(unsigned int readers) { check(mdb_env_set_maxreaders(_env, readers)); }
    MDB_env* _env = nullptr;
};

}  // namespace lmdbpp

#endif
