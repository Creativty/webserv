- [ ] Project directory de-cluttering
- [ ] Remove all usage of std::string where not required
- [ ] Remove all usage of std::map where not required
- [ ] Fix Formatting
- [ ] Forbidden functions removal
- [ ] Client Timeouts
- [ ] CGI Path resolution
    - Assuming CGI route on /cgi
    - No other route even if it is CGI should be definable under /cgi/*
    - ```
      /cgi/my_script.sh/whatever/blabla
      <===============><==============>
         SCRIPT_NAME       PATH_INFO
    ```
- [ ] Pessimist code path testing
