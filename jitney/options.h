#ifndef INCLUDED_JITNEY_OPTIONS_H
#define INCLUDED_JITNEY_OPTIONS_H

namespace jitney {
    // this object needs to exist for the life of the options.
    // generally its safest to just do
    // int main(int argv, char * argv[]) { 
    //     jitney::options options(argc,argv);
    //     // proceed as normal here
    //     return 0;
    // } 
    class options { 
    public:
        options(int & argv, char ** & argv, bool remove_flags = true);
        ~options();
    };
}

#endif
