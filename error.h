#ifndef __lmdbpp_error_h
#define __lmdbpp_error_h

#include <lmdb.h>
#include <stdexcept>

namespace lmdbpp
{
class Error : public std::runtime_error
{
public:
    const int code;
    Error(int return_code)
        : runtime_error{mdb_strerror(return_code)}
        , code(return_code)
    {
    }
};

// clang-format off
#define _ERR(NAME) class NAME : public Error{using Error::Error;}
_ERR(KeyExistsError);
_ERR(NotFoundError);
_ERR(PageNotFoundError);
_ERR(CorruptedError);
_ERR(PanicError);
_ERR(VersionMismatchError);
_ERR(InvalidError);
_ERR(MapFullError);
_ERR(DbsFullError);
_ERR(ReadersFullError);
_ERR(TlsFullError);
_ERR(TxnFullError);
_ERR(CursorFullError);
_ERR(PageFullError);
_ERR(MapResizedError);
_ERR(IncompatibleError);
_ERR(BadRslotError);
_ERR(BadTxnError);
_ERR(BadValsizeError);
_ERR(BadDbiError);
_ERR(ProblemError);
#undef _ERR

void check(int return_code)
{
    if (return_code == 0)
    {
        return;
    }
    switch (return_code)
    {
        case  MDB_KEYEXIST:          throw KeyExistsError(return_code);        break;
        case  MDB_NOTFOUND:          throw NotFoundError(return_code);         break;
        case  MDB_PAGE_NOTFOUND:     throw PageNotFoundError(return_code);     break;
        case  MDB_CORRUPTED:         throw CorruptedError(return_code);        break;
        case  MDB_PANIC:             throw PanicError(return_code);            break;
        case  MDB_VERSION_MISMATCH:  throw VersionMismatchError(return_code);  break;
        case  MDB_INVALID:           throw InvalidError(return_code);          break;
        case  MDB_MAP_FULL:          throw MapFullError(return_code);          break;
        case  MDB_DBS_FULL:          throw DbsFullError(return_code);          break;
        case  MDB_READERS_FULL:      throw ReadersFullError(return_code);      break;
        case  MDB_TLS_FULL:          throw TlsFullError(return_code);          break;
        case  MDB_TXN_FULL:          throw TxnFullError(return_code);          break;
        case  MDB_CURSOR_FULL:       throw CursorFullError(return_code);       break;
        case  MDB_PAGE_FULL:         throw PageFullError(return_code);         break;
        case  MDB_MAP_RESIZED:       throw MapResizedError(return_code);       break;
        case  MDB_INCOMPATIBLE:      throw IncompatibleError(return_code);     break;
        case  MDB_BAD_RSLOT:         throw BadRslotError(return_code);         break;
        case  MDB_BAD_TXN:           throw BadTxnError(return_code);           break;
        case  MDB_BAD_VALSIZE:       throw BadValsizeError(return_code);       break;
        case  MDB_BAD_DBI:           throw BadDbiError(return_code);           break;
        default:
            throw Error(return_code);
    }
}
// clang-format on

}

#endif
