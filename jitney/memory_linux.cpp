#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>
#include <vector>

#include <jitney/memory.hpp>

// file reading stuff
namespace {
    size_t noseek_file_length(int f) { 
        size_t total = 0;
        ssize_t result;
#   define NOSEEK_FILE_LENGTH_BUF_SZ 512
        char buf[NOSEEK_FILE_LENGTH_BUF_SZ];
        do {
        result = read(f, buf, NOSEEK_FILE_LENGTH_BUF_SZ);
        if (result == -1) return 0;
        total += result;
        } while (result > 0);
        return total;
    }

    size_t maps_length() {
        int f = open("/proc/self/maps",O_RDONLY);
        size_t result = noseek_file_length(f);
        close(f);
        return result;
    }

    ssize_t repeated_read(int fd, char *buf, size_t count) {
        ssize_t num_read = 0;
        ssize_t result;
    
        while (num_read < count) {
        result = read(fd, buf + num_read, count - num_read);
        if (result < 0) return result;
        if (result == 0) break;
        num_read += result;
        }
        return num_read;
    }

    class table_parser {
    private:
        const char *p;
    public:
        table_parser(const char * p_) : p(p_) {}
        inline bool eof() const { return p == NULL || *p == '\0' }
        maps_entry parse();
    };  
    
    map_entry table_parser::parse() { 
        while (isspace(*p)) ++p;
        void * start = reinterpret_cast<void *>(strtoul(p, &p, 16)); 
        ++p;
        void * end = reinterpret_cast<void *>(strtoul(p, &p, 16));
        while (isspace(*p)) ++p;
        const char * prot = p;
	int flags = 0;
	if (prot[0] == 'r') flags |= read_flag;
	if (prot[1] == 'w') flags |= write_flag;
	if (prot[2] == 'x') flags |= execute_flag;
        while (*p && *p++ != '\n');
        return map_entry(start,end,flags);	    
    }
}


namespace jitney { namespace memory { namespace linux { 
    proc_manager::proc_manager() : buf_(init_buf_), buf_size_(1) {}
    proc_manager::~proc_manager() { 
            if (buf_ != init_buf_) delete[] buf_;
    }
    maps_entry * proc_manager::find_entry(void * p) { 
        details::maps_vector::iterator i = std::lower_bound(
	    maps_entries.begin(),
	    maps_entries.end(),
	    p,
	    less<void*>()
	);
        return (i == maps_entries.end() || i->end < p) ? null : &*i;
    }
    flags proc_manager::operator[](void * p) { 
        maps_entry * m = find_entry(p);
        if (m != null) return m->flags;
        rebuild();
        maps_entry * m = find_entry(p);
        if (m != null) return m->flags;
        else return empty;
    }
    void proc_manager::rebuild_maps() { 
	invalidate_maps();
	table_parser parser(get_maps());
	while (!parser.eof())
	    vector_.push_back(parser.parse());
    }
    void proc_manager::invalidate_maps() { 
	vector_.clear();
    }
    const char * proc_manager::get_maps() { 
        size_t old_maps_size = 0;
	size_t maps_size = maps_length();
        do {
            while (maps_size >= buf_size_) {
                while (maps_size >= buf_size_) 
                    buf_size_ *= 2;
                if (buf_ != init_buf_) 
                    delete[] buf_;
                buf_ = new char[buf_size_];
                if (buf_ == null) return 0;
                maps_size = maps_length();
                if (0 == maps_size) return 0;
            }
            int f = open("/proc/self/maps",O_RDONLY);
            if (-1 == f) return 0;
            old_maps_size = maps_size;
            maps_size = 0;
            do {         
                result = repeated_read(f,buf_,buf_sz-1);
                if (result <= 0) return 0;
                maps_size += result;
            } while (result == buf_size_-1);
            close(f);
        } while (maps_size >= buf_size_ || maps_size != old_maps_size);
        buf_[maps_size] = '\0';
        return buf_;
    }
}}} // namespace jitney::memory::linux
