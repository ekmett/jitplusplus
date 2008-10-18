#include <sys/types.h> // size_t, ssize_t
// #include <sys/stat.h>
#include <fcntl.h>   // O_RDONLY

#include <algorithm> // std::lower_bound, std::max
#include <functional> // std::less<>

#include <jit++/internal.h>
#include <jit++/memory.h> // *memory*

using namespace jitpp;

// file reading stuff
namespace {
    // trick from the boehm collector, calculate its length to see if the maps file is mutating underneath us.
    // done the hard way because we can't seek through it
    size_t proc_self_maps_length() {
        int f = open("/proc/self/maps",O_RDONLY);
        size_t total = 0;
	static const size_t buf_size = 512;
        char buf[buf_size];
        ssize_t result;
        do {
            result = read(f, buf, buf_size);
            if (result == -1) return 0;
            total += result;
        } while (result > 0);
        close(f);
	VLOG(2) << total << " bytes in /proc/self/maps";
        return total;
    }

    ssize_t repeated_read(int fd, char * buf, size_t count) {
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

    class linux_proc_self_maps_parser {
    private:
	typedef native_memory_permission_model::range range;
        const char * i;
    public:
        linux_proc_self_maps_parser(const char * body) : i(body) {}
        inline bool eof() const { 
	    return i == 0 || *i == '\0'; 
	}
        range parse() { 
	    range result;
            while (isspace(*i)) ++i;
	    char * endi;
            result.start = reinterpret_cast<const void *>(strtoul(i, &endi, 16)); 
	    i = endi;
            ++i;
            result.end = reinterpret_cast<const void *>(strtoul(i, &endi, 16));
	    i = endi;
            while (isspace(*i)) ++i;
	    result.flags = 0;
	    if (i[0] == 'r') result.flags |= memory_permission::read;
	    if (i[1] == 'w') result.flags |= memory_permission::write;
	    if (i[2] == 'x') result.flags |= memory_permission::execute;
            while (*i && *i++ != '\n');
            return result;
        }
    };
}


namespace jitpp {
    native_memory_permission_model::native_memory_permission_model() : m_mutex() {} 
    native_memory_permission_model::~native_memory_permission_model() {}
    native_memory_permission_model::range * native_memory_permission_model::find_entry(const void * p) { 
        std::vector<range>::iterator i = std::lower_bound(
	    m_ranges.begin(),
	    m_ranges.end(),
	    p,
	    std::less<const void *>()
	);
        return (i == m_ranges.end() || i->end < p) ? 0 : &*i;
    }

    // precondition: read lock held
    memory_permissions native_memory_permission_model::operator[](const void * p) { 
        range * m = find_entry(p);
        if (m != 0) return m->flags;
        rebuild();
        m = find_entry(p);
        return (m != 0)
	    ? m->flags 
	    : memory_permission::read|memory_permission::write|memory_permission::execute;
    }

    // precondition: none
    void native_memory_permission_model::clear() { 
	mutex::scoped_lock lock(m_mutex,true);
	m_ranges.clear();
    }

    // precondition: none
    void native_memory_permission_model::rebuild() { 

	size_t file_size = 0;
	size_t old_file_size = 0;
	size_t buf_size = 0;
	char * buf = 0;

	mutex::scoped_lock lock(m_mutex,true);

	m_ranges.clear();

        do {
            if (file_size >= buf_size) {
                file_size = proc_self_maps_length();
                CHECK_NE(file_size,0) << "Unable to read /proc/self/maps";
		if (file_size >= buf_size) {
		    buf_size = std::max(buf_size,file_size + (file_size >> 1) + 1);
		    VLOG(2) << "growing buffer to " << buf_size;
                    buf = reinterpret_cast<char*>(alloca(buf_size));
	        }
            }

	    // check for alloca failure
	    CHECK(buf != 0) << "Unable to alloca " << file_size << " bytes in tracer.";

            int f = open("/proc/self/maps",O_RDONLY);
            CHECK_NE(f,-1) << "Unable to reread /proc/self/maps";

            old_file_size = file_size;
            file_size = 0;

	    ssize_t result;
            do {         
                result = repeated_read(f,buf,buf_size-1);
                if (result <= 0) break;
                file_size += result;
            } while (result == buf_size-1);
            close(f);
        } while (file_size >= buf_size || file_size != old_file_size);
        buf[file_size] = '\0';

        VLOG(1) << "read /proc/self/maps";
	VLOG(2) << buf;

	// now that we've read the file, parse the file.
	linux_proc_self_maps_parser parser(buf);
	while (!parser.eof()) {
	    m_ranges.push_back(parser.parse());
	}
	VLOG(1) << "parsed /proc/self/maps";
    }

    namespace { 
        native_memory_permission_model g_native_model;
    }

    native_memory_permission_model & native_memory_permission_model::instance() { 
	return g_native_model;
    }
} // namespace jitpp
