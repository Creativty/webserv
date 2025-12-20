- [x] Project directory de-cluttering
- [x] Determine how to know EOF for non-chunked bodies
- [x] Multiplex Writing to Sockets
- [x] Remove all usage of std::string where not required
- [x] Remove all usage of std::map where not required
- [x] Fix Formatting
- [x] Client Timeouts
- [x] On client hang up kill respective child process
- [x] CGI Path resolution
    - Assuming CGI route on /cgi
    - No other route even if it is CGI should be definable under /cgi/*
    - ```
      /cgi/my_script.sh/whatever/blabla
      <===============><==============>
         SCRIPT_NAME       PATH_INFO
    ```
    -
- [ ] Restore file server functionality
- [ ] Restore directory listing functionality
- [ ] Restore upload functionality
- [ ] Restore redirection functionality
- [ ] Answer whether to make directory listing stricter that requires '/' suffix, and maybe redirect there?
- [ ] Add script_executable config for self-executable CGI's
- [ ] Forbidden functions removal (Make sure printf is passable; we can just use std::..printf., and ask about ::fcntl)
- [ ] Pessimist code path testing
- [ ] Siege testing
- [ ] 42 testing
- [ ] Evaluation page
