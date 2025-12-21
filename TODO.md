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
- [x] Restore file server functionality
- [x] Restore directory listing functionality
- [x] Restore upload functionality
- [x] Restore redirection functionality
- [x] Answer whether to make directory listing stricter that requires '/' suffix, and maybe redirect there? (NO)
- [x] Add script_executable config for self-executable CGI's ? (NO)
- [x] Forbidden functions removal (Make sure std::fprintf is passable; we can just use std::..std::fprintf., and ask about ::fcntl)
- [x] Pessimist code path testing
- [x] Siege testing
- [x] 42 testing
- [x] Evaluation page
