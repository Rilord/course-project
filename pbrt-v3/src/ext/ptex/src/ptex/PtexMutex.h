#ifndef PtexMutex_h
#define PtexMutex_h



PTEX_NAMESPACE_BEGIN


template <class T>
class AutoLock {
public:
    AutoLock(T& m) : _m(m) { _m.lock(); }
    ~AutoLock()            { _m.unlock(); }
private:
    T& _m;
};

typedef AutoLock<Mutex> AutoMutex;
typedef AutoLock<SpinLock> AutoSpin;

PTEX_NAMESPACE_END

#endif
