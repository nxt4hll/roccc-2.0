function complete()
{
    if (cur_name != "")
    {
        if (substr(source_name,length(source_name)-3) == ".cpp") 
            compiler = "$(CXX) $(EXTRA_CXXFLAGS)";
        else if (substr(source_name,length(source_name)-2) == ".cc")
            compiler = "$(CXX) $(EXTRA_CXXFLAGS)";
        else if (substr(source_name,length(source_name)-1) == ".c")
            compiler = "$(CC) $(EXTRA_CFLAGS)";
        else
            compiler = "ERROR_NO_COMPILER_CHOSEN_DUE_TO_BAD_FILE_EXTENSION";
        printf("	%s -c -o %s $(PROFILE_COMPILE) $(COMPILER_SPECIFIC_CXXFLAGS) \\\n",compiler,cur_name);
        printf("	$(CCFLAGS) $(INCLDIRS) $(SYSINCLDIRS) $(PREPROCESSORFLAGS) \\\n");
        printf("	$(SUIF_MODULE) %s\n\n",source_name);
    }

}

/:/ {
    complete();
    cur_name = $1;
    sub(":","",cur_name);
    source_name = $2;

    # JOHN Sunday 20060716 8:50pm, update this because this parser doesn't work correctly
    # sometimes if the filename is too long, it will put the source_name on the next
    # line, I take care of that by reading the next line and setting it as the source.

    # JOHN Tuesday 2007-Oct-17, fixed above update to take care of weird case where we have
    # more than one data in there.  Didn't know about $0 and $1 difference.  In future
    # probably should consider moving this to python.

    if( source_name == "\\")
    {
        getline;
        the_whole_line = $0

        # source name has to be assigned so 'complete()' function remembers what type of file this was
        source_name = $1;
        # since the search below (on every line awk search) spits out lines
        # we can't call getline, but I need to so therefore spit out 
        # what the bottom search would have found if we hadn't of called getline :-D
        printf("%s: %s\n",cur_name, the_whole_line);
        getline;

        #consume current line because our print takes care of just re-printing it out
        # in following search pattern
    }
}

{
    for (i=1;i < NF; i++)
    {
        if (!match($i,":"))
            deps[$i] = 1;
    }
    print;
}



END {
    complete();
    printf("MAKEFILE_DEPS_RULE = defined\n");
    printf("Makefile.deps: Makefile ");
    for (i in deps)
        printf("\\\n	%s ",i);
    printf("\n");
    printf("	@echo '# Dependencies for C files' > Makefile.deps\n");
    printf("	@echo >> Makefile.deps\n");
    printf("	@rm -f dependencies\n");
#### JUL modif (7/7/06)
# printf("	$(CC) $(DEPSFLAG) $(EXTRA_CFLAGS) $(EXTRA_CXXFLAGS) $(COMPILER_SPECIFIC_CXXFLAGS) $(CCFLAGS) \\\n");
    printf("	$(CC) $(DEPSFLAG) $(EXTRA_CFLAGS) $(COMPILER_SPECIFIC_CXXFLAGS) $(CCFLAGS) \\\n");
    printf("	$(INCLDIRS) $(SYSINCLDIRS) $(PREPROCESSORFLAGS) $(SRCS) > dependencies\n");
    printf("	$(AWK) -f $(NCIHOME)/dep2make.awk < dependencies >> Makefile.deps\n");
    printf("\n");

}

BEGIN {
    cur_name = "";
    printf("CPP_TO_O_RULE = defined\n");
    printf("C_TO_O_RULE = defined\n");
}
