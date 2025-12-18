- [x] Project directory de-cluttering
- [x] Determine how to know EOF for non-chunked bodies
- [x] Multiplex Writing to Sockets
- [x] Remove all usage of std::string where not required
- [x] Remove all usage of std::map where not required
- [x] Fix Formatting
- [ ] Forbidden functions removal
- [x] Client Timeouts
- [ ] Pessimist code path testing
- [ ] CGI Path resolution
    - Assuming CGI route on /cgi
    - No other route even if it is CGI should be definable under /cgi/*
    - ```
      /cgi/my_script.sh/whatever/blabla
      <===============><==============>
         SCRIPT_NAME       PATH_INFO
    ```
