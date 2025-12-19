- [x] Project directory de-cluttering
- [x] Determine how to know EOF for non-chunked bodies
- [x] Multiplex Writing to Sockets
- [x] Remove all usage of std::string where not required
- [x] Remove all usage of std::map where not required
- [x] Fix Formatting
- [ ] Forbidden functions removal
- [x] Client Timeouts
- [ ] On client hang up kill respective child process
- [ ] Pessimist code path testing
- [x] CGI Path resolution
    - Assuming CGI route on /cgi
    - No other route even if it is CGI should be definable under /cgi/*
    - ```
      /cgi/my_script.sh/whatever/blabla
      <===============><==============>
         SCRIPT_NAME       PATH_INFO
    ```
