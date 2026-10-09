// SPDX-FileCopyrightText: 2008-2026 Edward Kmett <ekmett@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR Apache-2.0

#include <glog/logging.h>
import jitpp;
#include <stdio.h>

using namespace jitpp;

jitpp::interpreter t;


int main(int argc, char ** argv) { 
    jitpp::application(argc,argv);
    VLOG(1) << "options set";
    VLOG(1) << "tracer constructed";
    printf("0123456789\n");
    fflush(stdout);
    t.start();
    printf("0123456789\n");
    fflush(stdout);
    t.stop();
    puts("rejoined\n");
    return 0;
}
