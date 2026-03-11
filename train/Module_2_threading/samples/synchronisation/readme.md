
# RAII locks
* lock_guard
* lock + lock_guard with adopt_lock (lock is not RAII, need lock_guard after to get RAII)
* unique_lock
* shared_lock

# mutexes
* mutex
* recursive_mutex
* shared_mutex -- unique_lock -- write locks and shared_lock for read

# learnings