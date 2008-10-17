AC_DEFUN([AX_UDIS86], [
    AC_ARG_WITH(udis86,
        AS_HELP_STRING([--with-udis86=<path>], [Use external disassembler]),
        [
            AC_SUBST(WITH_UDIS86, [1])
            case "$withval" in
                /usr/lib|yes) ;;
                *) LDFLAGS="$LDFLAGS -L${withval}" ;;
            esac
            AC_CHECK_LIB(udis86, ud_init, [], [
                echo "Unable to locate libudis86."
                exit -1
            ])
        ],
        AC_CHECK_LIB(udis86, ud_init, [
	    AC_SUBST(WITH_UDIS86,[1])
	    LIBS="$LIBS -ludis86"
	],[AC_SUBST(WITH_UDIS86, [0])])
    )
    AC_DEFINE_UNQUOTED([WITH_UDIS86],$WITH_UDIS86,[Define if using udis86 as a disassembler.])
])
