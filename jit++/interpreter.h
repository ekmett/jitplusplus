#ifndef INCLUDED_JITPP_INTERPRETER_H
#define INCLUDED_JITPP_INTERPRETER_H

namespace jitpp { 
    class interpreter_impl;
    class interpreter { 
    public: 
	interpreter();
	~interpreter();
	void start();
        void stop();
    private:
	interpreter_impl * impl;
    };
}

#endif
