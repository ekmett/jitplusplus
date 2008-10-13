AC_DEFUN([AX_STACK_GROWS_DOWNWARD],[
    AC_CACHE_CHECK(
        [stack growth direction],
        [ac_cv_c_stack_grows_downward],
        [AC_RUN_IFELSE(
    	    [AC_LANG_SOURCE(
                [   
		    AC_INCLUDES_DEFAULT
                    int find_stack_direction () {
                        static char *addr = 0;
                        auto char dummy;
                        if (addr == 0) {
                            addr = &dummy;
                            return find_stack_direction ();
                        } else return (&dummy > addr) ? 0 : 1;
                    }

                    int main () {
                        return find_stack_direction () < 0;
                    }
               ]
            )],
            [ac_cv_c_stack_grows_downward=0],
            [ac_cv_c_stack_grows_downward=1],
            [AC_ERROR([Unable to determine the direction of stack growth])
        ])
])
AH_VERBATIM([STACK_GROWS_DOWNWARD],
[/* Define if you know the direction of stack growth for your system
	STACK_GROWS_DOWNWARD = 0 grows toward higher addresses
	STACK_GROWS_DOWNWARD = 1 grows toward lower addresses */
@%:@undef STACK_GROWS_DOWNWARD])dnl
AC_DEFINE_UNQUOTED(STACK_GROWS_DOWNWARD, $ac_cv_c_stack_grows_downward)
])
